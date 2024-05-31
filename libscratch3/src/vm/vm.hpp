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

#include "memory.hpp"
#include "costume.hpp"
#include "script.hpp"
#include "io.hpp"
#include "debug.hpp"

// Rate at which scripts are executed
#define CLOCK_SPEED 30

class Loader;
class VirtualMachine;
class Sprite;
class GLRenderer;

enum ExceptionType
{
	Exception_None,

	OutOfMemory,
	StackOverflow,
	StackUnderflow,
	VariableNotFound,
	IllegalOperation,
	InvalidArgument,
	UnsupportedOperation,

	NotImplemented,

	VMError // Internal error
};

class VirtualMachine final
{
public:

	//! \brief Load a program into the VM
	//! 
	//! Will fail if the VM alreadu has a program loaded.
	//! 
	//! \param prog Program to load
	//! \param name Name of the program
	//! \param loader Loader for the program
	//! 
	//! \return 0 on success, -1 on failure.
	int Load(Program *prog, const std::string &name, Loader *loader);

	//
	/////////////////////////////////////////////////////////////////
	// Global Control
	//

	//! \brief Start the VM
	//! 
	//! Sets up the VM and clicks the green flag. If the VM is already
	//! running, the function will return immediately with failure.
	//! 
	//! \return 0 on success, -1 on failure.
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
	//! \return 0 on success, -1 on failure, 1 on timeout.
	int VMWait(unsigned long ms);

	void VMSuspend();

	void VMResume();

	//
	/////////////////////////////////////////////////////////////////
	// Script Control (requires lock)
	//

	//! \brief Send the flag clicked event
	//! 
	//! If the flag is clicked, all scripts that are listening for
	//! the flag will be scheduled for execution. If the flag is
	//! clicked twice, they will restart.
	void SendFlagClicked();

	void Send(const std::string &message);

	void SendAndWait(const std::string &message);

	void SendKeyPressed(int scancode);

	//! \brief Sleep for a specified number of seconds
	//! 
	//! Causes the script to not be scheduled for execution for the
	//! specified number of seconds.
	//! 
	//! \param seconds Number of seconds to sleep
	void Sleep(double seconds);

	//! \brief Wait until an expression is true
	//! 
	//! The script will not be scheduled for execution until the
	//! expression evaluates to true.
	//! 
	//! \param expr Expression to evaluate
	void WaitUntil(Expression *expr);

	//! \brief Ask the user for input and wait for a response
	//! 
	//! The script will not be scheduled for execution until the
	//! user has provided input.
	//! 
	//! \param question Question to ask the user
	void AskAndWait(const std::string &question);

	//! \brief Terminate the current script
	void Terminate();

	//! \brief Raise an exception, does not return.
	//! 
	//! \param type Type of exception to raise
	//! \param message Message to associate with the exception
	void LS_NORETURN Raise(ExceptionType type, const char *message = nullptr);

	void LS_NORETURN Panic(const char *message = nullptr);

	//! \brief Push a value onto the stack
	//! 
	//! Raises a StackOverflow exception if the stack is full.
	Value &Push();

	//! \brief Pop a value from the stack
	//! 
	//! Raises a StackUnderflow exception if the stack is empty.
	void Pop();

	//! \brief Retrieve a value from the stack
	//! 
	//! Raises a StackUnderflow exception if the index indexes
	//! outside the stack.
	//! 
	//! \param i Index of the value to retrieve
	//! 
	//! \return A reference to the value at the
	//! specified index
	Value &StackAt(size_t i);

	//! \brief Push an execution frame onto the stack.
	//! 
	//! Raises a StackOverflow exception if the stack is full.
	//! 
	//! \param sl Statement list to execute
	//! \param count Number of times to execute the statement list
	//! \param flags Flags to control execution
	void PushFrame(StatementList *sl, int64_t count, uint32_t flags);

	//
	/////////////////////////////////////////////////////////////////
	// Internals
	//

	constexpr Program *GetProgram() const { return _prog; }
	constexpr const std::string &GetProgramName() const { return _progName; }

	Value &FindVariable(const std::string &id);

	constexpr double GetTime() const { return _time; }

	Sprite *FindSprite(const std::string &name);

	Sprite *FindSprite(intptr_t id);

	void ResetTimer();

	void Glide(Sprite *sprite, double x, double y, double s);

	//! \brief Schedule a script for execution
	void Sched();

	constexpr Loader *GetLoader() const { return _loader; }
	constexpr GLRenderer *GetRenderer() const { return _render; }
	constexpr IOHandler &GetIO() const { return _io; }
	constexpr Debugger &GetDebugger() const { return _debug; }

	constexpr bool IsSuspended() const { return _suspend; }

	constexpr const std::vector<Script> &GetScripts() const { return _scripts; }

	void OnClick(int64_t x, int64_t y);
	void OnKeyDown(int scancode);

	VirtualMachine &operator=(const VirtualMachine &) = delete;
	VirtualMachine &operator=(VirtualMachine &&) = delete;

	VirtualMachine();
	VirtualMachine(const VirtualMachine &) = delete;
	VirtualMachine(VirtualMachine &&) = delete;
	~VirtualMachine();
private:
	Program *_prog; // Program to run
	std::string _progName; // Name of the program
	Loader *_loader; // Loader for the program

	Sprite *_sprites; // All sprites
	Sprite *_spritesEnd; // End of the sprite list

	std::unordered_map<std::string, intptr_t> _spriteNames; // Sprite name lookup

	std::unordered_map<std::string, Value> _variables; // Global variables

	std::vector<Script> _scripts; // All scripts

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

	ExceptionType _exceptionType; // Exception type
	const char *_exceptionMessage; // Exception message

	bool _panicing; // Panic flag
	const char *_panicMessage; // Panic message
	jmp_buf _panicJmp; // Panic jump buffer

	Script *_current; // Currently executing script
	double _epoch; // VM start time
	double _time; // Current time

	double _interpreterTime; // Time taken to run the interpreter once
	double _deltaExecution; // Time since last scheduled execution

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

	void ResetScript(Script &script);

	void StartScript(Script &script);

	void DispatchEvents();
	 
	//! \brief Handles script scheduling
	void Scheduler();

	//! \brief Main vm loop
	void Main();

	static int ThreadProc(void *data);

	friend class Debugger;
};
