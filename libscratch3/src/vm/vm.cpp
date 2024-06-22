#include "vm.hpp"

#include <cassert>
#include <cstdio>
#include <cinttypes>
#include <algorithm>

#include <imgui_impl_sdl2.h>

#include <scratch3/scratch3.h>

#include "../resource.hpp"
#include "../render/renderer.hpp"
#include "../codegen/compiler.hpp"
#include "../codegen/util.hpp"

#include "sprite.hpp"
#include "io.hpp"
#include "debug.hpp"
#include "preload.hpp"
#include "exception.hpp"

static std::string trim(const std::string &str, const std::string &ws = " \t\n\r")
{
	size_t start = str.find_first_not_of(ws);
	if (start == std::string::npos)
		return "";

	size_t end = str.find_last_not_of(ws);
	return str.substr(start, end - start + 1);
}

int VirtualMachine::Load(const std::string &name, uint8_t *bytecode, size_t size)
{
	assert(_bytecode == nullptr);

	_bytecode = bytecode;
	_bytecodeSize = size;
	_progName = name;

	bc::Header *header = (bc::Header *)bytecode;
	bc::SpriteTable *st = (bc::SpriteTable *)(bytecode + header->stable);

	_sprites = new Sprite[st->count];
	_spritesEnd = _sprites + st->count;

	// load sprites
	bool foundStage = false;
	for (bc::uint64 i = 0; i < st->count; i++)
	{
		bc::Sprite &si = st->sprites[i];

		if (si.isStage)
		{
			if (foundStage)
			{
				Scratch3Logf(S, SCRATCH3_SEVERITY_ERROR, "Multiple stages found");
				Cleanup();
				return SCRATCH3_ERROR_INVALID_PROGRAM;
			}

			foundStage = true;
		}

		// create sprite
		Sprite &sprite = _sprites[i];
		sprite.Init(bytecode, size, &si);

		_spriteNames[sprite.GetName()] = i + 1;

		// create initializer
		_initScripts.emplace_back();
		Script &init = _initScripts.back();
		init.Init(bytecode, size, &si.initializer);
		init.sprite = &sprite;
		init.vm = this;

		// load scripts
		bc::Script *scripts = (bc::Script *)(bytecode + si.scripts);
		for (bc::uint64 j = 0; j < si.numScripts; j++)
		{
			bc::Script &sci = scripts[j];

			_scripts.emplace_back();
			Script &s = _scripts.back();
			s.Init(bytecode, size, &sci);

			s.sprite = &sprite;
			s.vm = this;
		}

		if (si.isStage)
			_stage = &sprite;

		i++;
	}

	if (!foundStage)
	{
		Scratch3Logf(S, SCRATCH3_SEVERITY_ERROR, "No stage found");
		Cleanup();
		return SCRATCH3_ERROR_INVALID_PROGRAM;
	}

	return SCRATCH3_ERROR_SUCCESS;
}

// entry point for script fibers
static int ScriptEntryThunk(void *scriptPtr)
{
	Script *script = (Script *)scriptPtr;
	script->Main();
	return 0;
}

int VirtualMachine::VMStart()
{
	if (_running)
		return SCRATCH3_ERROR_ALREADY_RUNNING;

	_shouldStop = false;

	_nextScript = 0;
	_enableScreenUpdates = true;

	// temporary panic handler
	_panicJmpSet = false;
	memset(_panicJmp, 0, sizeof(_panicJmp));
	int rc = setjmp(_panicJmp);
	if (rc == 1)
	{
		assert(_panicing);

		// panic
		printf("<PANIC> %s\n", _panicMessage);
		return SCRATCH3_ERROR_UNKNOWN;
	}

	// convert the VM thread to a fiber
	if (ls_convert_to_fiber(NULL) != 0)
		Panic("Failed to convert to fiber");

	// create fibers for initialization scripts
	for (Script &script : _initScripts)
	{
		script.fiber = ls_fiber_create(ScriptEntryThunk, &script);
		if (!script.fiber)
			Panic("Failed to create fiber");
	}

	// create fibers for scripts
	for (Script &script : _scripts)
	{
		script.fiber = ls_fiber_create(ScriptEntryThunk, &script);
		if (!script.fiber)
			Panic("Failed to create fiber");
	}

	// initialize graphics
	_render = new GLRenderer(_spritesEnd - _sprites - 1); // exclude the stage
	if (_render->HasError())
		Panic("Failed to initialize graphics");

	// Initialize graphics resources
	for (Sprite *s = _sprites; s < _spritesEnd; s++)
	{
		s->Load(this);
		for (int64_t i = 0; i < s->GetSoundCount(); i++)
			_sounds.push_back(&s->GetSounds()[i]);
	}

	_messageListeners.clear();
	_keyListeners.clear();
	_flagListeners.clear();

	// Find listeners
	for (Script &script : _scripts)
	{
		uint8_t *ptr = script.entry;
		uint8_t opcode = *ptr;
		ptr++;

		switch (opcode)
		{
		default:
			Panic("Script entry is not an event");
		case Op_onflag:
			_flagListeners.push_back(&script);
			break;
		case Op_onkey: {
			uint16_t sc = *(uint16_t *)ptr;
			ptr += sizeof(uint16_t);
			_keyListeners[(SDL_Scancode)sc].push_back(&script);
			break;
		}
		case Op_onclick:
			// Handled by the sprite
			break;
		case Op_onbackdropswitch:
			script.autoStart = true;
			break;
		case Op_ongt:
			script.autoStart = true;
			break;
		case Op_onevent: {
			char *evt = (char *)(_bytecode + *(uint64_t *)ptr);
			ptr += sizeof(uint64_t);
			_messageListeners[evt].push_back(&script);
			break;
		}
		case Op_onclone:
			// TODO: support
			break;
		}
	}

	SDL_Window *window = _render->GetWindow();
	SDL_SetWindowTitle(window, "Scratch 3");

	_flagClicked = false;
	_toSend.clear();
	_askQueue = std::queue<std::pair<Script *, std::string>>();
	_asker = nullptr;
	_question.clear();
	memset(_inputBuf, 0, sizeof(_inputBuf));

	_timerStart = 0.0;
	_deltaExecution = 0.0;

	_running = true;

	_epoch = ls_time64();

	// Run initialization scripts
	//
	// These scripts are run only once and set the initial values
	// for a sprite's variables. Anything may be done here, but
	// if an initialization script ever yields, the VM will panic.
	// The only way a script should return control is by terminating.
	for (Script &script : _initScripts)
	{
		script.Start();
		_current = &script;
		ls_fiber_switch(script.fiber);

		_current = nullptr;

		if (_panicing)
			longjmp(_panicJmp, 1);

		if (script.except != Exception_None)
			Panic("Initialization script raised an exception");

		if (script.state != TERMINATED)
			Panic("Initialization script did not terminate");

		// no longer needed
		ls_close(script.fiber);
		script.Destroy();
	}

	SendFlagClicked();

	_lastExecution = GetTime();

	return 0;
}

int VirtualMachine::VMUpdate()
{
	// set panic handler
	if (!_panicJmpSet)
	{
		_panicJmpSet = true;
		memset(_panicJmp, 0, sizeof(_panicJmp));
		int rc = setjmp(_panicJmp);
		if (rc == 1)
		{
			assert(_panicing);

			// panic
			printf("<PANIC> %s\n", _panicMessage);
			return -1;
		}
	}

	if (_shouldStop)
		return 1;

	_io.PollEvents();

	DispatchEvents();

	if (!_suspend)
	{
		double start = GetTime();

		_deltaExecution = start - _lastExecution;
		_lastExecution = start;

		Scheduler();

		_interpreterTime = GetTime() - start;
	}

	Render();

	return 0;
}

void VirtualMachine::VMTerminate()
{
	_shouldStop = true;
}

void VirtualMachine::VMSuspend()
{
	if (!_suspend)
	{
		_suspend = true;
		_suspendStart = ls_time64();

		SDL_SetWindowTitle(_render->GetWindow(), "Scratch 3 [Suspended]");
	}
}

void VirtualMachine::VMResume()
{
	if (_suspend)
	{
		_suspend = false;

		double suspendTime = ls_time64() - _suspendStart;

		// adjust timers to account for suspension
		_epoch += suspendTime;

		SDL_SetWindowTitle(_render->GetWindow(), "Scratch 3");
	}
}

void VirtualMachine::SendFlagClicked()
{
	_flagClicked = true;
}

void VirtualMachine::Send(const std::string &message)
{
	_toSend.insert(message);
}

void VirtualMachine::SendAndWait(const std::string &message)
{
	Panic("SendAndWait");
}

void VirtualMachine::SendKeyPressed(int scancode)
{
	Panic("SendKeyPressed");
}

void VirtualMachine::EnqueueAsk(Script *script, const std::string &question)
{
	_askQueue.push(std::make_pair(script, question));
}

void VirtualMachine::Panic(const char *message)
{
	_panicing = true;
	_panicMessage = message;

	if (_current)
	{
		ls_fiber_sched();
		abort(); // should be unreachable
	}

	longjmp(_panicJmp, 1);
}

Value &VirtualMachine::GetVariableRef(const Value &name)
{
	assert(_current != nullptr);

	if (name.type != ValueType_String)
		Panic("Variable name must be a string");

	auto it = _variables.find(name.u.string);
	if (it == _variables.end())
	{
		// create a new variable
		// TODO: throw an exception if the variable is not found
		Value &v = _variables[name.u.string];
		InitializeValue(v);
		return v;
	}

	return it->second;
}

Sprite *VirtualMachine::FindSprite(const Value &name)
{
	if (name.type != ValueType_String)
		return nullptr;

	auto it = _spriteNames.find(name.u.string);
	return it != _spriteNames.end() ? FindSprite(it->second) : nullptr;
}

Sprite *VirtualMachine::FindSprite(intptr_t id)
{
	Sprite *s = _sprites + id - 1;
	if (s >= _sprites && s < _spritesEnd)
		return s;
	return nullptr;
}

void VirtualMachine::ResetTimer()
{
	_timerStart = GetTime();
}

void VirtualMachine::PlaySound(Sound *sound)
{
	if (sound->IsPlaying())
	{
		sound->Stop();
		sound->Play();
		return; // already in list
	}

	_playingSounds.push_back(sound);
	sound->Play();
}

void VirtualMachine::StopAllSounds()
{
	for (Sound *snd : _playingSounds)
		snd->Stop();
	_playingSounds.clear();
}

void VirtualMachine::OnClick(int64_t x, int64_t y)
{
	Vector2 point(x, y);
	for (const int64_t *id = _render->RenderOrderEnd() - 1; id >= _render->RenderOrderBegin(); id--)
	{
		Sprite *sprite = reinterpret_cast<Sprite *>(_render->GetRenderInfo(*id)->userData);
		if (sprite->TouchingPoint(point))
		{
			// sprite was clicked
			for (Script *script : sprite->GetClickListeners())
				_clickQueue.push(script);
			break;
		}
	}
}

void VirtualMachine::OnKeyDown(int scancode)
{
	auto it = _keyListeners.find((SDL_Scancode)scancode);
	if (it == _keyListeners.end())
		return;

	for (Script *script : it->second)
		script->Start();

	// "any" key
	it = _keyListeners.find((SDL_Scancode)-1);
	if (it != _keyListeners.end())
	{
		for (Script *script : it->second)
			script->Start();
	}
}

void VirtualMachine::OnResize()
{
	for (Sprite *s = _sprites; s < _spritesEnd; s++)
		s->InvalidateTransform(); // force recompute
}

VirtualMachine::VirtualMachine(Scratch3 *S, const Scratch3VMOptions *options) :
	S(S), _io(this), _debug(this)
{
	_options = *options;
	if (_options.framerate <= 0)
		_options.framerate = SCRATCH3_FRAMERATE;

	_bytecode = nullptr;
	_bytecodeSize = 0;
	_loader = nullptr;

	_sprites = nullptr;
	_spritesEnd = nullptr;

	_flagClicked = false;

	_asker = nullptr;
	memset(_inputBuf, 0, sizeof(_inputBuf));

	_render = nullptr;

	_suspend = false;
	_suspendStart = 0.0;

	_timerStart = 0;

	_shouldStop = false;
	_waitCount = 0;

	_running = false;
	_activeScripts = 0;
	_waitingScripts = 0;

	_panicing = false;
	_panicMessage = nullptr;
	memset(_panicJmp, 0, sizeof(_panicJmp));

	_current = nullptr;
	_epoch = 0;

	_interpreterTime = 0;
	_deltaExecution = 0;

	PaError err = Pa_Initialize();
	if (err == paNoError)
		_hasAudio = true;
	else
	{
		_hasAudio = false;
		Scratch3Logf(S, SCRATCH3_SEVERITY_ERROR, "Pa_Initialize failed: %s", Pa_GetErrorText(err));
	}
}

VirtualMachine::~VirtualMachine()
{
	Cleanup();

	if (_hasAudio)
		Pa_Terminate();
}

void VirtualMachine::Render()
{
	_render->BeginRender();

	for (Sprite *s = _sprites; s < _spritesEnd; s++)
		s->Update();

	_render->Render();

	_io.RenderIO();

	const ImVec2 padding(5, 5);
	const ImU32 textColor = IM_COL32(0, 0, 0, 255); //IM_COL32(255, 255, 255, 255);
	const ImU32 hiddenColor = IM_COL32(128, 128, 128, 255);
	const ImU32 backColor = IM_COL32(255, 255, 255, 255); //IM_COL32(0, 0, 0, 128);

	ImDrawList *drawList = ImGui::GetBackgroundDrawList();
	for (Sprite *s = _sprites; s < _spritesEnd; s++)
	{
		const String *message = s->GetMessage();
		if (!message)
			continue;

		const AABB &bbox = s->GetBoundingBox();

		int x, y;
		_render->StageToScreen(s->GetX(), bbox.hi.y, &x, &y);

		ImVec2 position(x, y);

		const char *text = message->str;
		ImVec2 textSize = ImGui::CalcTextSize(text);
		
		ImVec2 topLeft(position.x - padding.x, position.y - padding.y);
		ImVec2 botRight(position.x + textSize.x + padding.x, position.y + textSize.y + padding.y);

		drawList->AddRectFilled(topLeft, botRight, backColor);
		drawList->AddText(position, textColor, text);
	}

	_debug.Render();

	_render->EndRender();
}

void VirtualMachine::Cleanup()
{
	for (Script &s : _scripts)
		ls_close(s.fiber), s.fiber = nullptr;
	ls_convert_to_thread();

	if (_sprites)
		delete[] _sprites, _sprites = nullptr;

	_io.Release();

	if (_render)
		delete _render, _render = nullptr;

	_flagClicked = false;
	_toSend.clear();
	_askQueue = std::queue<std::pair<Script *, std::string>>();
	_asker = nullptr;
	_question.clear();
	memset(_inputBuf, 0, sizeof(_inputBuf));

	_messageListeners.clear();
	_keyListeners.clear();

	for (auto &p : _variables)
		ReleaseValue(p.second);
	_variables.clear();

	for (Script &script : _scripts)
		script.Destroy();
	_scripts.clear();

	if (_sprites)
		delete[] _sprites, _sprites = nullptr;

	_io.Release();

	if (_render)
		delete _render, _render = nullptr;

	_loader = nullptr;
	_bytecode = nullptr;
	_bytecodeSize = 0;


	_activeScripts = 0;
	_waitingScripts = 0;
	_running = false;

	_spriteNames.clear();
}

void VirtualMachine::DispatchEvents()
{
	// Flag clicked
	if (_flagClicked)
	{
		for (Script *script : _flagListeners)
			script->Start();
		_flagClicked = false;
	}

	// Broadcasts
	if (_toSend.size() != 0)
	{
		for (const std::string &message : _toSend)
		{
			auto it = _messageListeners.find(message);
			if (it == _messageListeners.end())
				continue;

			for (Script *script : it->second)
				script->Start();
		}

		_toSend.clear();
	}

	// Sprite clicked
	while (!_clickQueue.empty())
	{
		Script *s = _clickQueue.front();
		s->Start();
		_clickQueue.pop();
	}

	// Ask input
	if (_askQueue.size() != 0 && _asker == nullptr)
	{
		std::pair<Script *, std::string> &top = _askQueue.front();
		_asker = top.first;
		_question = top.second;
		memset(_inputBuf, 0, sizeof(_inputBuf));
		_askQueue.pop();
	}
}

void VirtualMachine::Scheduler()
{
	// update screen at a fixed interval
	const long long kUpdateInterval = 1000000000ll / _options.framerate;

	int activeScripts = 0, waitingScripts = 0;
	
	// current time in nanoseconds
	long long time = ls_nanotime();

	bool screenUpdated;
	if ((screenUpdated = time >= _nextScreenUpdate))
		_nextScreenUpdate = time + kUpdateInterval;

	size_t ran = 0;
	for (;;) // round-robin scheduler
	{
		if (ran >= _scripts.size())
			break; // iterated through all scripts

		Script &script = *(_scripts.data() + _nextScript);
		_nextScript = (_nextScript + 1) % _scripts.size();
		ran++;

		time = ls_nanotime();
		if (time >= _nextScreenUpdate)
			break; // a script is likely taking too long, dont miss the screen update

		if (!script.fiber)
			continue; // cannot be scheduled

		if (script.state == EMBRYO || script.state == TERMINATED)
		{
			if (!script.autoStart)
				continue; // not active
			script.Start();
		}

		_current = &script;

		if (script.state == WAITING)
		{
			double time = GetTime();

			bool gliding = script.sprite->GetGlide()->end > time;
			if (gliding)
			{
				// gliding to a point
				waitingScripts++;
				continue;
			}
			else if (script.waitSound)
			{
				// waiting for a sound to finish
				if (script.waitSound->IsPlaying())
				{
					// still playing, wait
					waitingScripts++;
					continue;
				}

				script.waitSound = nullptr;
			}
			else if (script.waitInput || script.askInput || script.sleepUntil > time)
			{
				// waiting for an answer or timed sleep
				waitingScripts++;
				continue;
			}

			// wake up
			script.state = RUNNABLE;
		}

		if (script.state != RUNNABLE)
			continue; // not ready to run

		activeScripts++;

		if (!screenUpdated)
			continue;

		// schedule the script
		ls_fiber_switch(script.fiber);
		_current = nullptr;

		if (_panicing)
			longjmp(_panicJmp, 1);

		if (script.except != Exception_None)
		{
			// script raised an exception
			printf("<EXCEPTION> %s: %s\n", ExceptionString(script.except), script.exceptMessage);
			script.Dump();
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception", ExceptionString(script.except), _render->GetWindow());
			
			_activeScripts = 0;
			_waitingScripts = 0;
			_shouldStop = true;
			return;
		}

		if (script.state == TERMINATED)
			script.Reset(); // script terminated, reset it
	}

	// remove any playing sounds from the list
	for (auto it = _playingSounds.begin(); it != _playingSounds.end();)
	{
		Sound *snd = *it;
		if (!snd->IsPlaying())
			it = _playingSounds.erase(it);
		else
			it++;
	}

	_activeScripts = activeScripts;
	_waitingScripts = waitingScripts;
}
