#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <csetjmp>
#include <unordered_set>
#include <queue>

#include <lysys/lysys.hpp>

#include <ast/ast.hpp>

#include <cairo/cairo.h>
#include <SDL.h>

#include <scratch3/scratch3.h>

#include "memory.hpp"
#include "script.hpp"
#include "io.hpp"
#include "debug.hpp"

#define MAX_SCRIPTS 512

// Stack size for each script
#define STACK_SIZE 512

#define MAX_SPRITES 512

class Loader;
class VirtualMachine;
class AbstractSprite;
class Sprite;
class SpriteList;
class GLRenderer;
class AbstractSound;
class Voice;

class SpriteList
{
public:
	constexpr Sprite *Head() const { return _head; }
	constexpr Sprite *Tail() const { return _tail; }
	constexpr size_t Count() const { return _count; }

	//! \brief Add a sprite to the end of the list
	//! 
	//! \param sprite The sprite to add
	void Add(Sprite *sprite);

	//! \brief Remove a sprite from the list
	//! 
	//! The sprite must be in the list. The sprite will not be
	//! deleted.
	//! 
	//! \param sprite The sprite to remove
	//! 
	//! \return sprite
	Sprite *Remove(Sprite *sprite);

	//! \brief Insert a sprite into the list
	//! 
	//! before and sprite must be different.
	//! 
	//! \param before The sprite which will come before the new sprite,
	//! or nullptr to insert at the front. Must be in the list.
	//! \param sprite The sprite to insert. If already in the list,
	//! it will be moved.
	void Insert(Sprite *LS_RESTRICT before, Sprite *LS_RESTRICT sprite);

	//! \brief Move a sprite
	//! 
	//! A sprite attempting to move past the tail will be clamped
	//! to the end. A sprite cannot be become the head of the list.
	//! The head of the list cannot be moved.
	//! 
	//! \param sprite The sprite to move
	//! \param distance The distance to move the sprite
	void Move(Sprite *sprite, int64_t distance);

	//! \brief Remove all sprites from the list
	void Clear();

	SpriteList &operator=(const SpriteList &) = delete;
	SpriteList &operator=(SpriteList &&) = delete;

	SpriteList();
	SpriteList(const SpriteList &) = delete;
	SpriteList(SpriteList &&) = delete;
	~SpriteList();
private:
	Sprite *_head, *_tail;
	size_t _count;
};

//! \brief Scratch 3 virtual machine
class VirtualMachine final
{
public:
	//! \brief Load a project from bytecode
	//!
	//! \param name The name of the project
	//! \param bytecode The bytecode
	//! \param size The size of the bytecode
	//!
	//! \return An error code
	int Load(const std::string &name, uint8_t *bytecode, size_t size);

	//
	/////////////////////////////////////////////////////////////////
	// Global Control
	//

	//! \brief Start the VM
	//! 
	//! Sets up the VM and clicks the green flag. If the VM is already
	//! running, the function will return immediately with failure.
	//! 
	//! \return An error code
	int VMStart();

	//! \brief Update the VM
	//!	
	//! \return 0 if the VM should continue running, 1 if the VM should terminate,
	//! and -1 if an error occurred.
	int VMUpdate();

	//! \brief Terminate the VM
	//! 
	//! Schedule all scripts for termination. This function does not
	//! guarantee that the VM will terminate immediately. Call
	//! VMWait to ensure that the VM has terminated.
	void VMTerminate();

	//! \brief Suspend the VM
	//!
	//! Suspends execution of scripts. Does nothing if the VM is
	//! already suspended.
	void VMSuspend();

	//! \brief Resume the VM
	//!
	//! Resumes execution of scripts. Does nothing if the VM is
	//! already running.
	void VMResume();

	//
	/////////////////////////////////////////////////////////////////
	// Script Control
	//

	//! \brief Send the flag clicked event
	//! 
	//! If the flag is clicked, all scripts that are listening for
	//! the flag will be scheduled for execution. If the flag is
	//! clicked twice, they will restart.
	void SendFlagClicked();

	//! \brief Send a message
	//!
	//! Queues a message to be sent to all listeners. If a script
	//! is listening for the message, it will be scheduled for
	//! execution. If it is already running, it will restart.
	void Send(const std::string &message);

	void SendAndWait(const std::string &message);

	void SendKeyPressed(int scancode);

	//! \brief Queue a question to ask the user
	//!
	//! \brief script The script that is asking the question
	//! \brief question The question to ask
	void EnqueueAsk(Script *script, const std::string &question);

	//! \brief Panic the VM
	//!
	//! This function will panic the VM with the given message. The
	//! VM will terminate immediately and the message will be printed
	//! to the console. This function does not return.
	//!
	//! It is always safe to call this function from within the VM's
	//! thread. Internally, this performs a longjmp to the VM's panic
	//! handler.
	//!
	//! \param message The panic message
	void LS_NORETURN Panic(const char *message = nullptr);

	//! \brief Get a reference to a static variable
	//!
	//! Panics if the variable does not exist.
	//!
	//! \param id The variable ID
	//!
	//! \return A reference to the variable
	Value &GetStaticVariable(uint32_t id);

	//
	/////////////////////////////////////////////////////////////////
	// Internals
	//

	constexpr uint8_t *GetBytecode() const { return _bytecode; }
	constexpr size_t GetBytecodeSize() const { return _bytecodeSize; }
	constexpr const std::string &GetProgramName() const { return _progName; }

	inline double GetTime() const { return ls_time64() - _epoch; }

	inline double GetTimer() const { return GetTime() - _timerStart; }

	constexpr AbstractSprite *GetAbstractSprites() const { return _abstractSprites; }
	constexpr size_t GetAbstractSpriteCount() const { return _nAbstractSprites; }

	constexpr SpriteList *GetSpriteList() const { return _spriteList; }

	Sprite *FindSprite(const Value &name);

	constexpr Sprite *GetStage() const { return _stage; }

	void ResetTimer();

	void StartVoice(Voice *voice);

	//! \brief Stop all sounds
	void StopAllSounds();

	constexpr bool HasAudio() const { return _hasAudio; }
	constexpr const std::vector<AbstractSound *> &GetSounds() const { return _sounds; }
	constexpr const std::list<Voice *> &GetVoices() const { return _activeVoices; }

	constexpr GLRenderer *GetRenderer() const { return _render; }
	constexpr IOHandler &GetIO() const { return _io; }
	constexpr Debugger &GetDebugger() const { return _debug; }

	constexpr bool IsSuspended() const { return _suspend; }

	constexpr const std::vector<SCRIPT_ALLOC_INFO> &GetScriptStubs() const { return _scriptStubs; }

	constexpr Script *GetScriptTable() { return _scriptTable; }
	constexpr size_t GetAllocatedScripts() const { return _allocatedScripts; }

	//! \brief Allocate a script
	//! 
	//! Allocates a script in the script table. The script will be
	//! put into a suspended state upon return. Start the script by
	//! calling RestartScript.
	//! 
	//! \param ai The allocation information
	//! 
	//! \return The allocated script, panics on failure
	Script *AllocScript(const SCRIPT_ALLOC_INFO &ai);

	void FreeScript(Script *script);

	void ReleaseStack(Script *script);

	//! \brief Get a script by ID
	//! 
	//! \param id The script ID, in the range [0, MAX_SCRIPTS)
	//! 
	//! \return The script, or nullptr if the script is not
	//! allocated
	Script *OpenScript(unsigned long id);

	//! \brief Restart a script
	//! 
	//! \param script The script to restart
	void RestartScript(Script *script);

	//! \brief Terminate a script
	//! 
	//! If the script is the currently running script, this function
	//! does not return. Otherwise, the script will be scheduled for
	//! termination. If the script was already terminated, this
	//! function does nothing.
	//! 
	//! \param script The script to terminate
	void TerminateScript(Script *script);

	void DeleteSprite(Sprite *sprite);
	void DeleteClones();

	constexpr Script *GetCurrentScript() const { return _current; }

	constexpr void Reschedule() { _nextScript = 0; }

	constexpr const Scratch3VMOptions &GetOptions() const { return _options; }

	void OnClick(int64_t x, int64_t y);
	void OnKeyDown(int scancode);

	void OnResize();

	VirtualMachine &operator=(const VirtualMachine &) = delete;
	VirtualMachine &operator=(VirtualMachine &&) = delete;

	VirtualMachine(Scratch3 *S, const Scratch3VMOptions *options);
	VirtualMachine(const VirtualMachine &) = delete;
	VirtualMachine(VirtualMachine &&) = delete;
	~VirtualMachine();
private:
	Scratch3 *S;
	Scratch3VMOptions _options; // VM options

	uint8_t *_bytecode; // Bytecode for the program
	size_t _bytecodeSize; // Size of the bytecode
	std::string _progName; // Name of the program

	AbstractSprite *_abstractSprites; // All abstract sprites
	size_t _nAbstractSprites; // Number of abstract sprites

	SpriteList *_spriteList; // All sprites
	Sprite *_stage; // Stage sprite

	StringMap<Sprite *> _baseSprites; // name -> base sprite instance
	
	std::vector<AbstractSound *> _sounds; // All sounds
	std::list<Voice *> _activeVoices; // Active voices
	bool _hasAudio; // Host supports audio

	Script _scriptTable[MAX_SCRIPTS]; // Script table
	size_t _allocatedScripts; // Number of allocated scripts
	size_t _nextEntry; // Next entry in the script table

	size_t _nextScript; // Next script to run

	std::vector<SCRIPT_ALLOC_INFO> _scriptStubs; // Script start stubs

	std::vector<SCRIPT_ALLOC_INFO> _initScripts; // Initialization scripts
	std::vector<Script *> _flagListeners; // Flag listeners
	std::unordered_map<std::string, std::vector<Script *>> _messageListeners; // Message listeners
	std::unordered_map<SDL_Scancode, std::vector<Script *>> _keyListeners; // Key listeners
	
	bool _flagClicked; // Flag clicked event
	std::queue<Script *> _clickQueue; // Scripts to send the click event

	std::queue<std::pair<Script *, std::string>> _askQueue; // Scripts waiting for input
	Script *_asker; // Current input requester
	std::string _question; // Current question
	char _inputBuf[512]; // Input buffer

	//
	/////////////////////////////////////////////////////////////////
	// Graphics
	//

	GLRenderer *_render; // Renderer
	double _lastSlowRender; // Time of last slow render

	//
	/////////////////////////////////////////////////////////////////
	// I/O
	//

	mutable IOHandler _io; // I/O handler

	//
	/////////////////////////////////////////////////////////////////
	// Debugging
	//

	mutable Debugger _debug; // Debugger

	//
	/////////////////////////////////////////////////////////////////
	// Threading
	//

	bool _suspend; // VM is suspended
	double _suspendStart; // Time at which the VM was suspended

	double _timerStart;	// Epoch for timer

	bool _shouldStop; // User requested termination
	size_t _waitCount; // Number of threads waiting

	bool _running; // VM is running
	int _activeScripts; // Number of active scripts
	int _waitingScripts; // Number of waiting scripts	

	bool _panicing; // Panic flag
	const char *_panicMessage; // Panic message
	jmp_buf _panicJmp; // Panic jump buffer
	bool _panicJmpSet; // Panic jump buffer is set

	Script *_current; // Currently executing script

	double _epoch; // VM start time

	long long _interpreterTime; // Time taken to run the interpreter once (ns)
	long long _deltaExecution; // Time since last scheduled execution (ns)
	long long _lastExecution; // Time of last scheduled execution (ns)

	long long _nextSchedule; // Time of next scheduled execution (ns)

	//
	/////////////////////////////////////////////////////////////////
	// Graphics Internals
	//

	void Render();

	//
	/////////////////////////////////////////////////////////////////
	//

	//! \brief Cleans up the VM
	//! 
	//! This is not a substitute for the destructor. Calls to this
	//! function still leave the VM in a usable state.
	void Cleanup();

	void DispatchEvents();
	 
	//! \brief Handles script scheduling
	void Scheduler();

	friend class Debugger;
};

//! \brief Current virtual machine
extern LS_THREADLOCAL VirtualMachine *VM;
