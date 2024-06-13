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

#include "sprite.hpp"
#include "io.hpp"
#include "debug.hpp"
#include "preload.hpp"
#include "exception.hpp"

#define DEG2RAD (0.017453292519943295769236907684886)
#define RAD2DEG (57.295779513082320876798154814105)

static std::string trim(const std::string &str, const std::string &ws = " \t\n\r")
{
	size_t start = str.find_first_not_of(ws);
	if (start == std::string::npos)
		return "";

	size_t end = str.find_last_not_of(ws);
	return str.substr(start, end - start + 1);
}

int VirtualMachine::Load(const char *name, uint8_t *bytecode, size_t size)
{
	assert(_bytecode == nullptr);

	_bytecode = bytecode;
	_bytecodeSize = size;
	_progName = name;

	ParsedSprites sprites;
	ParseSprites(bytecode, size, &sprites);

	_sprites = new Sprite[sprites.size()];
	_spritesEnd = _sprites + sprites.size();

	// load sprites
	bool foundStage = false;
	size_t i = 0;
	for (SpriteInfo &si : sprites)
	{
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
		sprite.Init(&si);

		// load scripts
		for (ScriptInfo &ri : si.scripts)
		{
			_scripts.emplace_back();
			Script &s = _scripts.back();
			s.Init(&ri);

			s.sprite = &sprite;
			s.vm = this;
		}

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

int VirtualMachine::VMStart()
{
	if (_running || _panicing)
		return -1;

	_shouldStop = false;

	_thread = ls_thread_create(&ThreadProc, this);
	if (!_thread)
		return -1;

	return 0;
}

void VirtualMachine::VMTerminate()
{
	_shouldStop = true;
}

int VirtualMachine::VMWait(unsigned long ms)
{
	if (!_thread)
		return SCRATCH3_ERROR_SUCCESS;

	int rc = ls_timedwait(_thread, ms);
	if (rc == -1)
		return SCRATCH3_ERROR_UNKNOWN;

	return rc == 1 ? SCRATCH3_ERROR_TIMEOUT : SCRATCH3_ERROR_SUCCESS;
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
	printf("Flag clicked\n");
	_flagClicked = true;
}

void VirtualMachine::Send(const std::string &message)
{
	printf("Send: %s\n", message.c_str());
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

Sprite *VirtualMachine::FindSprite(const std::string &name)
{
	auto it = _spriteNames.find(name);
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

void VirtualMachine::OnClick(int64_t x, int64_t y)
{
	Vector2 point(x, y);
	for (const int64_t *id = _render->RenderOrderEnd() - 1; id >= _render->RenderOrderBegin(); id--)
	{
		Sprite *sprite = reinterpret_cast<Sprite *>(_render->GetRenderInfo(*id)->userData);
		if (sprite->TouchingPoint(point))
		{
			// sprite was clicked
			printf("Clicked %s\n", sprite->GetName().c_str());
			for (Script *script : sprite->GetClickListeners())
				_clickQueue.push(script);
			break;
		}
	}
}

void VirtualMachine::OnKeyDown(int scancode)
{
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

	_thread = nullptr;
}

VirtualMachine::~VirtualMachine()
{
	Cleanup();
}

void VirtualMachine::Render()
{
	_render->BeginRender();

	for (Sprite *s = _sprites; s < _spritesEnd; s++)
		s->Update();

	_render->Render();

	_io.RenderIO();

	const ImVec2 padding(5, 5);
	const ImU32 textColor = IM_COL32(255, 255, 255, 255);
	const ImU32 hiddenColor = IM_COL32(128, 128, 128, 255);
	const ImU32 backColor = IM_COL32(0, 0, 0, 128);

	ImDrawList *drawList = ImGui::GetBackgroundDrawList();
	for (Sprite *s = _sprites; s < _spritesEnd; s++)
	{
		int x, y;
		_render->StageToScreen(s->GetX(), s->GetY(), &x, &y);

		ImVec2 position(x, y);

		const char *text = s->GetName().c_str();
		ImVec2 textSize = ImGui::CalcTextSize(text);
		
		ImVec2 topLeft(position.x - padding.x, position.y - padding.y);
		ImVec2 botRight(position.x + textSize.x + padding.x, position.y + textSize.y + padding.y);

		drawList->AddRectFilled(topLeft, botRight, backColor);
		drawList->AddText(position, s->IsShown() ? textColor : hiddenColor, s->GetName().c_str());
	}

	_debug.Render();

	_render->EndRender();
}

void VirtualMachine::Cleanup()
{
	if (_thread && ls_thread_id_self() != ls_thread_id(_thread))
	{
		_shouldStop = true;
		ls_wait(_thread);
		ls_close(_thread), _thread = nullptr;
	}

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

	_io.Release();

	if (_render)
		delete _render, _render = nullptr;

	_loader = nullptr;
	_bytecode = nullptr;
	_bytecodeSize = 0;
}

void VirtualMachine::ShutdownThread()
{
	_activeScripts = 0;
	_waitingScripts = 0;
	_running = false;

	_io.Release();

	if (_render)
		delete _render, _render = nullptr;

	// clean up fibers
	for (Script &s : _scripts)
		ls_close(s.fiber), s.fiber = nullptr;
	ls_convert_to_thread();
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
			break; // a script is likely taking too long

		if (!script.fiber)
			continue; // cannot be scheduled

		if (script.state == EMBRYO || script.state == TERMINATED)
			continue; // not active

		_current = &script;

		if (script.state == WAITING)
		{
			double time = GetTime();

			bool gliding = script.sprite->GetGlide()->end > time;
			if (gliding)
				continue; // not waiting

			if (script.waitInput || script.askInput || script.sleepUntil > time)
			{
				waitingScripts++;
				continue;
			}

			// wake up
			script.state = RUNNABLE;
		}

		if (script.state != RUNNABLE)
			continue;

		activeScripts++;

		if (!screenUpdated)
			continue;

		// schedule the script
		ls_fiber_switch(script.fiber);

		if (_panicing)
			longjmp(_panicJmp, 1);

		if (_current->except != Exception_None)
		{
			// script raised an exception
			printf("<EXCEPTION> %s: %s\n", ExceptionString(_current->except), _current->exceptMessage);
			_current->Dump();
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception", ExceptionString(_current->except), _render->GetWindow());
			
			_activeScripts = 0;
			_waitingScripts = 0;
			_shouldStop = true;
			return;
		}

		if (script.state == TERMINATED)
			script.Reset();

		_current = nullptr;
	}

	_activeScripts = activeScripts;
	_waitingScripts = waitingScripts;
}

static int ScriptEntryThunk(void *scriptPtr)
{
	Script *script = (Script *)scriptPtr;
	script->Main();
	return 0;
}

void VirtualMachine::Main()
{
	_nextScript = 0;
	_enableScreenUpdates = true;

	memset(_panicJmp, 0, sizeof(_panicJmp));
	int rc = setjmp(_panicJmp);
	if (rc == 1)
	{
		assert(_panicing);

		// panic
		printf("<PANIC> %s\n", _panicMessage);
		ShutdownThread();
		return;
	}

	if (ls_convert_to_fiber(NULL) != 0)
		Panic("Failed to convert to fiber");

	for (Script &script : _scripts)
	{
		script.fiber = ls_fiber_create(ScriptEntryThunk, &script);
		if (!script.fiber)
			Panic("Failed to create fiber");
	}

	_render = new GLRenderer(_spritesEnd - _sprites - 1); // exclude the stage
	if (_render->HasError())
		Panic("Failed to initialize graphics");

	// Initialize graphics resources
	for (Sprite *s = _sprites; s < _spritesEnd; s++)
		s->Load(this);

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
			// TODO: support
			break;
		case Op_ongt:
			// TODO: support
			break;
		case Op_onevent: {
			char *evt = (char *)(_bytecode + *(uint64_t *)ptr);
			ptr += sizeof(uint64_t);
			_messageListeners[evt].push_back(&script);
			break;
		}
		case Op_onclone:
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
	
	SendFlagClicked();

	double lastExecution = GetTime();

	for (;;)
	{
		_io.PollEvents();

		if (_shouldStop)
			break;

		DispatchEvents();

		if (!_suspend)
		{
			double start = GetTime();

			_deltaExecution = start - lastExecution;
			lastExecution = start;

			Scheduler();

			_interpreterTime = GetTime() - start;
		}

		Render();
	}

	ShutdownThread();
}

int VirtualMachine::ThreadProc(void *data)
{
	VirtualMachine *vm = (VirtualMachine *)data;
	vm->Main();
	return 0;
}
