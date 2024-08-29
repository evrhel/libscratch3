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

void SpriteList::Add(Sprite *sprite)
{
	assert(sprite != nullptr);
	assert(sprite->_next == nullptr);
	assert(sprite->_prev == nullptr);

	if (_count == MAX_SPRITES)
		VM->Panic("Too many sprites");

	if (!_tail)
		_head = _tail = sprite;
	else
	{
		_tail->_next = sprite;
		sprite->_prev = _tail;

		if (_head == _tail)
			_head->_next = sprite;
		_tail = sprite;
	}

	_count++;
}

Sprite *SpriteList::Remove(Sprite *sprite)
{
	assert(sprite != nullptr);
	assert(_count > 0);

	if (sprite == _head)
		_head = sprite->_next;

	if (sprite == _tail)
		_tail = sprite->_prev;

	if (sprite->_prev)
		sprite->_prev->_next = sprite->_next;

	if (sprite->_next)
		sprite->_next->_prev = sprite->_prev;

	sprite->_prev = nullptr;
	sprite->_next = nullptr;

	_count--;

	return sprite;
}

void SpriteList::Insert(Sprite *before, Sprite *sprite)
{
	assert(sprite != nullptr);

	if (before == sprite)
		return; // no-op

	if (sprite->_next != nullptr || sprite->_prev != nullptr)
		Remove(sprite); // remove and reinsert
	else if (_count == MAX_SPRITES)
		VM->Panic("Too many sprites");

	if (before)
	{
		sprite->_next = before->_next;
		sprite->_prev = before;

		if (before->_next)
			before->_next->_prev = sprite;
		else
			_tail = sprite;

		before->_next = sprite;
	}
	else
	{
		if (_head)
		{
			_head->_prev = sprite;
			sprite->_next = _head;
		}
		else
			_tail = sprite;

		_head = sprite;
	}

	_count++;
}

void SpriteList::Move(Sprite *sprite, int64_t distance)
{
	assert(sprite != nullptr);
	assert(_count > 0);

	if (distance == 0 || sprite == _head)
		return;

	assert(sprite->_prev != nullptr);

	if (distance > 0)
	{
		Sprite *before = sprite->_prev;
		Remove(sprite);

		for (int64_t i = 0; i < distance; i++)
		{
			if (before == _tail)
				break; // clamp to end
			before = before->_next;
		}

		Insert(before, sprite);
	}
	else
	{
		Sprite *after = sprite->_next;
		Remove(sprite);

		distance = -distance;
		for (int64_t i = 0; i < distance; i++)
		{
			if (after == _head->_next)
				break; // clamp to start
			after = after->_prev;
		}

		Insert(after->_prev, sprite);
	}
}

void SpriteList::Clear()
{
	Sprite *next;
	for (Sprite *s = _head; s; s = next)
	{
		next = s->_next;
		s->GetBase()->Free(s);
	}

	_head = nullptr;
	_tail = nullptr;
	_count = 0;
}

SpriteList::SpriteList() :
	_head(nullptr), _tail(nullptr), _count(0) {}

SpriteList::~SpriteList()
{
	Clear();
}

int VirtualMachine::Load(const std::string &name, uint8_t *bytecode, size_t size)
{
	assert(_bytecode == nullptr);

	VM = this;

	_bytecode = bytecode;
	_bytecodeSize = size;
	_progName = name;

	_sounds.clear();

	bc::Header *header = (bc::Header *)bytecode;
	bc::SpriteTable *st = (bc::SpriteTable *)(bytecode + header->stable);

	_abstractSprites = new AbstractSprite[st->count];
	_nAbstractSprites = st->count;

	_spriteList = new SpriteList();

	// load sprites
	bool foundStage = false;
	for (bc::uint64 i = 0; i < st->count; i++)
	{
		AbstractSprite &as = _abstractSprites[i];
		bc::Sprite &si = st->sprites[i];

		if (si.isStage)
		{
			if (foundStage)
			{
				Scratch3Logf(S, SCRATCH3_SEVERITY_ERROR, "Multiple stages found");
				Cleanup();
				VM = nullptr;
				return SCRATCH3_ERROR_INVALID_PROGRAM;
			}

			foundStage = true;
		}

		// Initialize the abstract sprite
		as.Init(bytecode, size, &si, _options.stream);

		// Create base sprite
		Sprite *sprite = as.Instantiate(nullptr);
		_baseSprites[as.GetName()] = sprite;

		// Create initializer, if any
		if (si.initializer.offset)
		{
			_initScripts.emplace_back();

			SCRIPT_ALLOC_INFO &ai = _initScripts.back();
			ai.sprite = sprite;
			ai.info = &si.initializer;
		}

		// Create script stubs
		bc::Script *scripts = (bc::Script *)(bytecode + si.scripts);
		for (bc::uint64 j = 0; j < si.numScripts; j++)
		{
			_scriptStubs.emplace_back();

			SCRIPT_ALLOC_INFO &ai = _scriptStubs.back();
			ai.sprite = sprite;
			ai.info = scripts + j;
		}

		for (AbstractSound *snd = as.GetSounds(); snd < as.GetSounds() + as.GetSoundCount(); snd++)
			_sounds.push_back(snd);

		if (si.isStage)
			_stage = sprite;
	}

	if (!foundStage)
	{
		Scratch3Logf(S, SCRATCH3_SEVERITY_ERROR, "No stage found");
		Cleanup();
		VM = nullptr;
		return SCRATCH3_ERROR_INVALID_PROGRAM;
	}

	VM = nullptr;
	return SCRATCH3_ERROR_SUCCESS;
}

// entry point for script fibers
static int ScriptEntryThunk(void *scriptPtr)
{
	Script *script = (Script *)scriptPtr;

	if (setjmp(script->entryJmp) == 1)
	{
		if (!script->restart)
			VM->Panic("Script restarted without restart flag");
	}

	script->restart = false;
	script->exitCode = -1;
	script->pc = script->entry;
	
	// Reset the stack
	VM->ReleaseStack(script);

	// Run the script
	script->exitCode = ScriptMain();

	VM->TerminateScript(script);
	abort(); // unreachable
}

int VirtualMachine::VMStart()
{
	if (_running)
		return SCRATCH3_ERROR_ALREADY_RUNNING;

	VM = this;

	_shouldStop = false;

	_nextScript = 0;

	_lastSlowRender = -1e9;
	_nextSchedule = 0;

	memset(_scriptTable, 0, sizeof(_scriptTable));
	_allocatedScripts = 0;
	_nextEntry = 0;
	_lastEntry = 0;

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

	// initialize graphics
	_render = GLRenderer::Create(_spriteList, _options);
	if (!_render)
		Panic("Failed to create graphics");

	// Initialize graphics resources
	for (AbstractSprite *as = _abstractSprites; as < _abstractSprites + _nAbstractSprites; as++)
		as->Load();

	_messageListeners.clear();
	_keyListeners.clear();
	_flagListeners.clear();

	// Find listeners
	for (SCRIPT_ALLOC_INFO &ai : _scriptStubs)
	{
		uint8_t *ptr = _bytecode + ai.info->offset;
		Opcode opcode = (Opcode)*ptr;
		ptr++;

		switch (opcode)
		{
		default:
			Panic("Script entry is not an event");
		case Op_onflag:
			_flagListeners.push_back(AllocScript(ai));
			break;
		case Op_onkey: {
			uint16_t sc = *(uint16_t *)ptr;
			ptr += sizeof(uint16_t);
			_keyListeners[(SDL_Scancode)sc].push_back(AllocScript(ai));
			break;
		}
		case Op_onclick:
			// Handled by the sprite
			break;
		case Op_onbackdropswitch: {
			Script *script = AllocScript(ai);
			script->autoStart = true;
			RestartScript(script);
			break;
		}
		case Op_ongt: {
			Script *script = AllocScript(ai);
			script->autoStart = true;
			RestartScript(script);
			break;
		}
		case Op_onevent: {
			char *evt = (char *)(_bytecode + *(uint64_t *)ptr);
			ptr += sizeof(uint64_t);
			_messageListeners[evt].push_back(AllocScript(ai));
			break;
		}
		case Op_onclone:
			// Handled by the sprite
			break;
		}
	}

	SDL_Window *window = _render->GetWindow();
#if LS_DEBUG
	SDL_SetWindowTitle(window, "Scratch 3 [DEBUG]");
#else
	SDL_SetWindowTitle(window, "Scratch 3");
#endif // LS_DEBUG

	_flagClicked = false;
	_askQueue = std::queue<std::pair<Script *, std::string>>();
	_asker = nullptr;
	_question.clear();
	memset(_inputBuf, 0, sizeof(_inputBuf));

	_timerStart = 0.0;
	_deltaExecution = 0;

	_running = true;

	_epoch = ls_time64();

	// Run initialization scripts
	//
	// These scripts are run only once and set the initial values
	// for a sprite's variables. Anything may be done here, but
	// if an initialization script ever yields, the VM will panic.
	// The only way a script should return control is by terminating.
	for (SCRIPT_ALLOC_INFO &ai : _initScripts)
	{
		Script *script = AllocScript(ai);
		RestartScript(script);

		// Run the script
		_current = script;
		ls_fiber_switch(script->fiber);
		_current = nullptr;

		if (_panicing)
			longjmp(_panicJmp, 1);

		if (script->except != Exception_None)
		{
			printf("<EXCEPTION> %s: %s\n", ExceptionString(script->except), script->exceptMessage);
			Panic("Initialization script raised an exception");
		}

		if (script->state != TERMINATED)
			Panic("Initialization script did not terminate");

		FreeScript(script);
	}
	_initScripts.clear();

	SendFlagClicked();

	_lastExecution = ls_nanotime();

	if (_options.suspend)
		VMSuspend();

	VM = nullptr;
	return 0;
}

int VirtualMachine::VMUpdate()
{
	VM = this;

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
			VM = nullptr;
			return -1;
		}
	}

	if (_shouldStop)
	{
		VM = nullptr;
		return 1;
	}

	_io.PollEvents();

	DispatchEvents();

	long long ns = ls_nanotime();
	if (!_suspend && ns >= _nextSchedule)
	{
		const long long kUpdateInterval = 1000000000ll / _options.framerate;
		_nextSchedule = ns + kUpdateInterval;

		_deltaExecution = ns - _lastExecution;
		_lastExecution = ns;

		Scheduler();

		_interpreterTime = ls_nanotime() - ns;
	}

	Render();

	VM = nullptr;
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
	}
}

void VirtualMachine::SendFlagClicked()
{
	_flagClicked = true;
}

void VirtualMachine::Send(const std::string &message)
{
	assert(VM == this);

	auto it = _messageListeners.find(message);
	if (it == _messageListeners.end())
		return;

	// We set this to ensure that any not-running message
	// handlers for this message are started this frame.
	// Without this, some handlers may not be started until
	// the next frame, so this is necessary to ensure
	// fair scheduling.
	_nextScript = 0;

	// TODO: this does not handle sending messages to self

	// Start all message handlers
	for (Script *s : it->second)
		RestartScript(s);
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

Value &VirtualMachine::GetStaticVariable(uint32_t id)
{
	bc::Header *header = (bc::Header *)_bytecode;
	bc::uint64 count = *(bc::uint64 *)(_bytecode + header->rdata);
	Value *vars = (Value *)(_bytecode + header->data);

	if (id >= count)
		Panic("Invalid static variable ID");

	return vars[id];
}

Sprite *VirtualMachine::FindSprite(const Value &name)
{
	assert(VM == this);

	if (name.type != ValueType_String)
		return nullptr;

	auto it = _baseSprites.find(name.u.string);
	return it != _baseSprites.end() ? it->second : nullptr;
}

void VirtualMachine::ResetTimer()
{
	_timerStart = GetTime();
}

void VirtualMachine::StartVoice(Voice *voice)
{
	assert(VM == this);

	if (voice->IsPlaying())
	{
		voice->Play();
		return; // already in list
	}

	_activeVoices.push_back(voice);
	voice->Play();
}

void VirtualMachine::StopAllSounds()
{
	assert(VM == this);

	for (Voice *voice : _activeVoices)
		voice->Stop();
	_activeVoices.clear();
}

void VirtualMachine::OnClick(int64_t x, int64_t y)
{
	assert(VM == this);

	Vector2 point(x, y);
	for (Sprite *sprite = _spriteList->Tail(); sprite; sprite = sprite->GetPrev())
	{
		if (sprite->GetInstanceId() != BASE_INSTANCE_ID)
			continue; // Events only broadcast to base instances

		if (sprite->TouchingPoint(point))
		{
			// sprite was clicked
			for (Script *s : sprite->GetBase()->GetClickListeners())
				_clickQueue.push(s);
			break;
		}
	}
}

Script *VirtualMachine::AllocScript(const SCRIPT_ALLOC_INFO &ai)
{
	assert(VM == this);

	if (ai.sprite == nullptr || ai.info == nullptr)
		Panic("Invalid script start info");

	if (_allocatedScripts == MAX_SCRIPTS)
		Panic("Out of script slots");

	Script *script = _scriptTable + _nextEntry;

	_nextEntry++;
	if (_nextEntry > _lastEntry)
		_lastEntry = _nextEntry;

	assert(script->state == EMBRYO);

	script->sprite = ai.sprite;

	if (!script->fiber)
	{
		script->fiber = ls_fiber_create(ScriptEntryThunk, script);
		if (!script->fiber)
			Panic("Failed to create fiber");
	}

	script->sleepUntil = 0.0;
	script->waitInput = false;
	script->askInput = false;
	script->entry = _bytecode + ai.info->offset;
	script->pc = script->entry;
	script->autoStart = false;
	script->scheduled = false;
	script->restart = true;

	if (!script->stack)
	{
		script->stack = (Value *)malloc(STACK_SIZE * sizeof(Value));
		if (!script->stack)
		{
			ls_close(script->fiber), script->fiber = nullptr;
			Panic("Failed to allocate stack");
		}
	}

	script->sp = script->stack + STACK_SIZE;
	script->bp = script->sp;

	script->except = Exception_None;
	script->exceptMessage = nullptr;

	script->state = SUSPENDED;

	_allocatedScripts++;

	return script;
}

void VirtualMachine::FreeScript(Script *script)
{
	assert(VM == this);

	size_t index = script - _scriptTable;
	if (index >= MAX_SCRIPTS)
		Panic("Script is not owned by this VM");
	
	// Release the script
	// We reuse the fiber and the allocated stack

	script->sprite = nullptr;

	ReleaseStack(script);
	script->state = EMBRYO; // set last!

	if (index < _nextEntry)
		_nextEntry = index;

	// find the last allocated script
	while (_lastEntry > 0 && _scriptTable[_lastEntry - 1].state == EMBRYO)
		_lastEntry--;

	_allocatedScripts--;
}

void VirtualMachine::ReleaseStack(Script *script)
{
	assert(VM == this);

	size_t index = script - _scriptTable;
	if (index >= MAX_SCRIPTS)
		Panic("Script is not owned by this VM");

	if (script->state == EMBRYO)
		Panic("Unallocated script");

	assert(script->stack != nullptr);

	Value *const stackEnd = script->stack + STACK_SIZE;
	while (script->sp < stackEnd)
	{
		ReleaseValue(*script->sp);
		script->sp++;
	}

	assert(script->sp == stackEnd);

	script->bp = script->sp;
}

Script *VirtualMachine::OpenScript(unsigned long id)
{
	assert(VM == this);

	if (id >= MAX_SCRIPTS)
		Panic("Script ID out of range");
	Script *script = _scriptTable + id;
	if (script->state == EMBRYO)
		return nullptr;
	return script;
}

void VirtualMachine::RestartScript(Script *script)
{
	assert(VM == this);

	size_t index = script - _scriptTable;
	if (index >= MAX_SCRIPTS)
		Panic("Script is not owned by this VM");

	if (script->state == EMBRYO)
		Panic("Unallocated script");

	assert(script->fiber != nullptr);

	script->restart = true;
	script->state = RUNNABLE;

	if (_current == script) // jump to beginning of script
		longjmp(script->entryJmp, 1);
}

void VirtualMachine::TerminateScript(Script *script)
{
	assert(VM == this);

	size_t index = script - _scriptTable;
	if (index >= MAX_SCRIPTS)
		Panic("Script is not owned by this VM");

	if (script->state == EMBRYO)
		Panic("Unallocated script");

	if (script->state == TERMINATED)
	{
		assert(_current != script);
		return; // already terminated
	}

	script->state = TERMINATED;

	if (_current == script)
	{
		// yield control to the VM
		ls_fiber_sched();
		assert(_current == script);

		if (script->restart)
			longjmp(script->entryJmp, 1);

		Panic("Terminated script rescheduled");
	}
}

void VirtualMachine::DeleteSprite(Sprite *sprite)
{
	assert(VM == this);
	assert(_current == nullptr);

	for (Script *s = _scriptTable; s < _scriptTable + _lastEntry; s++)
	{
		if (s->sprite == sprite)
			FreeScript(s);
	}

	_spriteList->Remove(sprite);
	sprite->GetBase()->Free(sprite);
}

void VirtualMachine::DeleteClones()
{
	assert(VM == this);
	assert(_current == nullptr);

	for (Sprite *s = _spriteList->Head(); s;)
	{
		if (s->GetInstanceId() == BASE_INSTANCE_ID)
			s = s->GetNext();
		else
		{
			Sprite *next = s->GetNext();
			DeleteSprite(s);
			s = next;
		}
	}
}

void VirtualMachine::OnKeyDown(int scancode)
{
	auto it = _keyListeners.find((SDL_Scancode)scancode);
	if (it == _keyListeners.end())
		return;

	for (Script *s : it->second)
		RestartScript(s);

	// "any" key
	it = _keyListeners.find((SDL_Scancode)-1);
	if (it != _keyListeners.end())
	{
		for (Script *s : it->second)
			RestartScript(s);
	}
}

void VirtualMachine::OnResize()
{
	for (Sprite *s = _spriteList->Head(); s; s = s->GetNext())
		s->InvalidateTransform(); // force recompute
}

VirtualMachine::VirtualMachine(Scratch3 *S, const Scratch3VMOptions *options) :
	S(S), _io(this)
{
	_options = *options;
	if (_options.framerate <= 0)
		_options.framerate = SCRATCH3_FRAMERATE;

	_bytecode = nullptr;
	_bytecodeSize = 0;

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

	for (Sprite *s = _spriteList->Head(); s; s = s->GetNext())
		s->Update();

	_render->Render();

	_io.RenderIO();

	const ImVec2 padding(5, 5);
	const ImU32 textColor = IM_COL32(0, 0, 0, 255); //IM_COL32(255, 255, 255, 255);
	const ImU32 hiddenColor = IM_COL32(128, 128, 128, 255);
	const ImU32 backColor = IM_COL32(255, 255, 255, 255); //IM_COL32(0, 0, 0, 128);

	ImDrawList *drawList = ImGui::GetBackgroundDrawList();
	for (Sprite *s = _spriteList->Head(); s; s = s->GetNext())
	{
		const Value &message = s->GetMessage();
		if (message.type == ValueType_None)
			continue;

		const AABB &bbox = s->GetBoundingBox();

		int x, y;
		_render->StageToScreen(s->GetX(), bbox.hi.y, &x, &y);

		ImVec2 position(x, y);

		const char *text = ToString(message);
		ImVec2 textSize = ImGui::CalcTextSize(text);
		
		ImVec2 topLeft(position.x - padding.x, position.y - padding.y);
		ImVec2 botRight(position.x + textSize.x + padding.x, position.y + textSize.y + padding.y);

		drawList->AddRectFilled(topLeft, botRight, backColor);
		drawList->AddText(position, textColor, text);
	}

	if (_options.debug)
	{
		if (GetTime() - _lastSlowRender <= 2)
		{
			constexpr ImVec2 pos(0, 0);

			static const char message[] = "Can't keep up!";

			ImVec2 textSize = ImGui::CalcTextSize(message);

			ImVec2 topLeft(pos.x, pos.y);
			ImVec2 botRight(pos.x + textSize.x + padding.x * 2, pos.y + textSize.y + padding.y * 2);

			drawList->AddRectFilled(topLeft, botRight, IM_COL32(0, 0, 0, 255));
			drawList->AddText(ImVec2(pos.x + padding.x, pos.y + padding.y), IM_COL32(255, 0, 0, 255), message);
		}
	}

	if (_options.debug)
		_debug.Render();

	_render->EndRender();

	if (_render->GetFramerate() < _options.framerate)
		_lastSlowRender = GetTime();
}

void VirtualMachine::Cleanup()
{
	VM = this;

	for (unsigned long id = 0; id < MAX_SCRIPTS; id++)
	{
		Script &s = _scriptTable[id];
		if (s.state != EMBRYO)
			FreeScript(&s);

		if (s.stack)
			free(s.stack), s.stack = nullptr;

		if (s.fiber)
			ls_close(s.fiber);

		memset(&s, 0, sizeof(Script));
	}

	ls_convert_to_thread();

	if (_spriteList)
	{
		// will delete all sprites
		delete _spriteList, _spriteList = nullptr;
	}

	_stage = nullptr;

	_baseSprites.clear();

	_io.Release();

	if (_abstractSprites)
		delete[] _abstractSprites, _abstractSprites = nullptr;

	if (_render)
		delete _render, _render = nullptr;

	_flagClicked = false;
	_askQueue = std::queue<std::pair<Script *, std::string>>();
	_asker = nullptr;
	_question.clear();
	memset(_inputBuf, 0, sizeof(_inputBuf));

	_messageListeners.clear();
	_keyListeners.clear();

	_bytecode = nullptr;
	_bytecodeSize = 0;

	_activeScripts = 0;
	_waitingScripts = 0;
	_running = false;

	_baseSprites.clear();

	VM = nullptr;
}

void VirtualMachine::DispatchEvents()
{
	assert(VM == this);
	assert(_current == nullptr);

	// Flag clicked
	if (_flagClicked)
	{
		StopAllSounds();

		// delete all clones
		DeleteClones();

		// terminate all scripts
		for (Script *s = _scriptTable; s < _scriptTable + _lastEntry; s++)
		{
			if (s->state == EMBRYO)
				continue;
			TerminateScript(s);
		}

		for (Script *s : _flagListeners)
			RestartScript(s);

		_flagClicked = false;
	}

	// Sprite clicked
	if (!_clickQueue.empty())
	{
		do
		{
			RestartScript(_clickQueue.front());
			_clickQueue.pop();
		} while (!_clickQueue.empty());
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
	int activeScripts = 0, waitingScripts = 0;

	// round-robin scheduler
	_nextScript = 0;
	while (_nextScript < _lastEntry)
	{
		Script &script = *(_scriptTable + _nextScript);
		_nextScript++;

		if (script.state == EMBRYO)
			continue; // not allocated

		if (script.scheduled)
			continue; // already scheduled this frame

		if (script.state == SUSPENDED)
		{
			if (!script.autoStart)
				continue;
			script.state = RUNNABLE;
		}
		else if (script.state == TERMINATED)
		{
			if (!script.autoStart)
				continue;
			RestartScript(&script);
		}

		if (script.state == WAITING)
		{
			double time = GetTime();

			bool gliding = script.sprite->GetGlideInfo().end > time;
			if (gliding)
			{
				// gliding to a point
				waitingScripts++;
				continue;
			}
			else if (script.waitVoice)
			{
				// waiting for a sound to finish
				if (script.waitVoice->IsPlaying())
				{
					// still playing, wait
					waitingScripts++;
					continue;
				}

				script.waitVoice = nullptr;
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
			continue; // not runnable

		activeScripts++;

		script.scheduled = true;

		// schedule the script
		_current = &script;
		ls_fiber_switch(script.fiber);
		_current = nullptr;

		if (_panicing)
			longjmp(_panicJmp, 1);

		if (script.except != Exception_None)
		{
			// script raised an exception
			printf("<EXCEPTION> %s: %s\n", ExceptionString(script.except), script.exceptMessage);

			char message[256];
			snprintf(message, sizeof(message), "Exception: %s\nReason: %s", ExceptionString(script.except), script.exceptMessage);

			script.Dump();
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Exception", message, _render->GetWindow());
			
			_activeScripts = 0;
			_waitingScripts = 0;
			_shouldStop = true;
			return;
		}

		if (script.sprite->IsDeleted())
			DeleteSprite(script.sprite);
	}

	// Reset scheduled flag for next frame
	for (Script *s = _scriptTable; s < _scriptTable + MAX_SCRIPTS; s++)
		s->scheduled = false;

	// remove any playing sounds from the list
	for (auto it = _activeVoices.begin(); it != _activeVoices.end();)
	{
		Voice *voice = *it;
		if (!voice->IsPlaying())
			it = _activeVoices.erase(it);
		else
			it++;
	}

	_activeScripts = activeScripts;
	_waitingScripts = waitingScripts;
}

SCRATCH3_STORAGE VirtualMachine *VM = nullptr;
