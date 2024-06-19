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
#include "costume.hpp"
#include "script.hpp"
#include "io.hpp"
#include "debug.hpp"
#include "sound.hpp"

class Loader;
class VirtualMachine;
class Sprite;
class GLRenderer;

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

	//! \brief Terminate the VM
	//! 
	//! Schedule all scripts for termination. This function does not
	//! guarantee that the VM will terminate immediately. Call
	//! VMWait to ensure that the VM has terminated.
	void VMTerminate();

	//! \brief Wait for the VM to terminate
	//! 
	//! Blocks until the VM has terminated. This does not stop the VM
	//! from running, only waits for all its scripts to terminate.
	//! 
	//! \param ms Number of milliseconds to wait
	//! 
	//! \return An error code
	int VMWait(unsigned long ms);

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

	//! \brief Get a reference to a variable
	//!
	//! Panics if the variable does not exist.
	//!
	//! \param name The name of the variable
	//!
	//! \return A reference to the variable
	Value &GetVariableRef(const Value &name);

	//
	/////////////////////////////////////////////////////////////////
	// Internals
	//

	constexpr uint8_t *GetBytecode() const { return _bytecode; }
	constexpr size_t GetBytecodeSize() const { return _bytecodeSize; }
	constexpr const std::string &GetProgramName() const { return _progName; }

	inline double GetTime() const { return ls_time64() - _epoch; }

	inline double GetTimer() const { return GetTime() - _timerStart; }

	Sprite *FindSprite(const Value &name);
	Sprite *FindSprite(intptr_t id);

	constexpr Sprite *GetStage() const { return _stage; }

	void ResetTimer();

	//! \brief Play a sound
	//!
	//! \param sound The sound to play
	void PlaySound(Sound *sound);

	//! \brief Stop all sounds
	void StopAllSounds();

	constexpr bool HasAudio() const { return _hasAudio; }
	constexpr const std::vector<Sound *> &GetSounds() const { return _sounds; }

	constexpr Loader *GetLoader() const { return _loader; }
	constexpr GLRenderer *GetRenderer() const { return _render; }
	constexpr IOHandler &GetIO() const { return _io; }
	constexpr Debugger &GetDebugger() const { return _debug; }

	constexpr bool IsSuspended() const { return _suspend; }

	constexpr const std::vector<Script> &GetScripts() const { return _scripts; }

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
	Loader *_loader; // Loader for the program

	Sprite *_sprites; // All sprites
	Sprite *_spritesEnd; // End of the sprite list

	Sprite *_stage; // Stage sprite

	std::unordered_map<const String *, intptr_t, _StringHasher, _StringEqual> _spriteNames; // Sprite name lookup

	std::unordered_map<String *, Value, _StringHasher, _StringEqual> _variables; // Variables

	std::vector<Sound *> _sounds; // All sounds
	std::list<Sound *> _playingSounds; // Playing sounds
	bool _hasAudio; // Host supports audio

	std::vector<Script> _initScripts; // Initialization scripts
	std::vector<Script> _scripts; // All scripts
	size_t _nextScript; // Next script to run

	std::vector<Script *> _flagListeners; // Flag listeners
	std::unordered_map<std::string, std::vector<Script *>> _messageListeners; // Message listeners
	std::unordered_map<SDL_Scancode, std::vector<Script *>> _keyListeners; // Key listeners
	
	bool _flagClicked; // Flag clicked event
	std::unordered_set<std::string> _toSend; // Messages to send
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

	Script *_current; // Currently executing script

	double _epoch; // VM start time

	double _interpreterTime; // Time taken to run the interpreter once
	double _deltaExecution; // Time since last scheduled execution

	long long _lastScreenUpdate; // Time of last screen update
	long long _nextScreenUpdate; // Time of next screen update
	bool _enableScreenUpdates; // Enable screen updates

	ls_handle _thread; // VM thread

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

	void ShutdownThread();

	void DispatchEvents();
	 
	//! \brief Handles script scheduling
	void Scheduler();

	//! \brief Main vm loop
	void Main();

	static int ThreadProc(void *data);

	friend class Debugger;
};
