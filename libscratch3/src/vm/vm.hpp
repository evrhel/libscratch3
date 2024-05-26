#pragma once

#include <vector>
#include <unordered_map>

#include <lysys/lysys.hpp>

#include <ast/ast.hpp>

#include "value.hpp"

// Script has been created but not yet started
#define EMBRYO 0

// Script can be scheduled to run
#define RUNNABLE 1

// Script is waiting for a condition to be met
#define WAITING 2

// Script is suspended
#define SUSPENDED 3

// Script has terminated
#define TERMINATED 4

// Stack size for each script
#define STACK_SIZE 512

// Maximum nesting depth of scripts
#define SCRIPT_DEPTH 32

// Rate at which scripts are executed
#define FRAMERATE 60

class VirtualMachine;
struct Sprite;

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

// Execute the invoking statement again
#define FRAME_EXEC_AGAIN 0x1

// Execute the frame forever
#define FRAME_EXEC_FOREVER 0x2

// Execution frame
struct Frame
{
	StatementList *sl; // Statement list to execute
	uintptr_t pc; // Program counter (index into statement list)
	int64_t count; // Number of times to repeat this frame
	uint32_t flags; // Execution flags
};

// Script context
struct Script
{
	int state; // Script state
	Sprite *sprite; // Sprite that owns the script
	StatementList *entry; // Root statement list
	
	ls_handle lock; // lock for the script

	double sleepUntil; // Time to wake up
	Expression *waitExpr; // Expression to wait for
	bool waitInput; // Wait for input

	Value *stack; // Bottom of the stack
	Value *sp; // Stack pointer (grows upwards)

	Frame frames[SCRIPT_DEPTH]; // Execution frames
	uintptr_t fp; // Frame pointer (grows downwards)
};

#define MESSAGE_STATE_NONE 0
#define MESSAGE_STATE_SAY 1
#define MESSAGE_STATE_THINK 2

// Sprite state
struct Sprite
{
	std::string name;

	double x = 0.0, y = 0.0;
	double direction = 90.0;

	std::string message;
	int messageState = MESSAGE_STATE_NONE;

	int64_t costume = 1, ccostumes = 1;
	double size = 100.0;
	bool visible = true;
	int64_t layer = 0;

	struct
	{
		double color = 0;
		double fisheye = 0;
		double whirl = 0;
		double pixelate = 0;
		double mosaic = 0;
		double brightness = 0;
		double ghost = 0;
	} graphics;

	double volume = 100.0;
};

class VirtualMachine final
{
public:

	//! \brief Load a program into the VM
	//! 
	//! Will fail if the VM alreadu has a program loaded.
	//! 
	//! \param prog Program to load
	//! 
	//! \return 0 on success, -1 on failure.
	int Load(Program *prog);

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
	void AskAndWait();

	//! \brief Terminate the current script
	void Terminate();

	//! \brief Terminate a script by ID
	//! 
	//! \param id ID of the script to terminate
	void TerminateScript(unsigned long id);

	//! \brief Raise an exception.
	//! 
	//! \param type Type of exception to raise
	//! 
	//! \return A reference to the exception value
	Value &Raise(ExceptionType type);

	//! \brief Push a value onto the stack
	//! 
	//! Raises a StackOverflow exception if the stack is full.
	Value &Push();

	//! \brief Pop a value from the stack
	//! 
	//! Raises a StackUnderflow exception if the stack is empty.
	//! 
	//! \return true if the operation was successful, false otherwise
	bool Pop();

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

	bool Truth(const Value &val);
	bool Equals(const Value &lhs, const Value &rhs);
	Value &Assign(Value &lhs, const Value &rhs);
	Value &SetInteger(Value &lhs, int64_t rhs);
	Value &SetReal(Value &lhs, double rhs);
	Value &SetBool(Value &lhs, bool rhs);
	Value &SetString(Value &lhs, const std::string &rhs);
	Value &SetParsedString(Value &lhs, const std::string &rhs);
	Value &SetEmpty(Value &lhs);

	//! \brief Convert a value to a string
	//! 
	//! Will replace the value with a string representation. If the
	//! operation fails, the value will remain unchanged.
	//! 
	//! \param v Value to convert
	void CvtString(Value &v);

	int64_t ToInteger(const Value &val);
	double ToReal(const Value &val);
	std::string ToString(const Value &val);

	//! \brief Allocate a string
	//! 
	//! Allocates a string of the specified length. The string's
	//! reference count is set to 1. Its bytes are zeroed. The
	//! input value's reference count is decremented. If the
	//! operation failed, the contents of the input value are
	//! undefined.
	//! 
	//! \param v Value in which to store the string
	//! \param len Length of the string to allocate, not including
	//! the null terminator
	//! 
	//! \return v, or _exception if the operation failed
	Value &AllocString(Value &v, size_t len);

	Value &RetainValue(Value &val);
	void ReleaseValue(Value &val);
	void FreeValue(Value &val);

	Value &FindVariable(const std::string &id);

	constexpr const Value &GetAnswer() const { return _answer; }
	constexpr bool GetMouseDown() const { return _mouseDown; }
	constexpr int64_t GetMouseX() const { return _mouseX; }
	constexpr int64_t GetMouseY() const { return _mouseY; }
	constexpr double GetLoudness() const { return _loudness; }
	constexpr double GetTimer() const { return _timer; }
	constexpr const Value &GetUsername() const { return _username; }

	void ResetTimer();

	VirtualMachine();
	~VirtualMachine();
private:
	Program *_prog; // Program to run

	std::unordered_map<std::string, Sprite> _sprites;
	std::unordered_map<std::string, Value> _variables;

	std::vector<Script> _scripts; // All scripts

	Value _emptyString; // empty string
	Value _trueString; // true string
	Value _falseString; // false string
	
	//
	/////////////////////////////////////////////////////////////////
	// I/O
	//

	Value _answer; // answer
	bool _mouseDown; // mouse button state
	int64_t _mouseX, _mouseY; // mouse position
	double _loudness; // loudness
	double _timer; // timer value
	Value _username; // username

	//
	/////////////////////////////////////////////////////////////////
	// Threading
	//

	double _timerStart;	// Epoch for timer

	bool _shouldStop; // User requested termination
	size_t _waitCount; // Number of threads waiting

	bool _running; // VM is running
	size_t _activeScripts; // Number of active scripts
	Value _exception; // Exception value

	Script *_current; // Currently executing script
	double _time; // VM time

	ls_handle _lock;
	ls_handle _cond;

	//
	/////////////////////////////////////////////////////////////////
	//

	//! \brief Cleans up the VM
	//! 
	//! This is not a substitute for the destructor. Calls to this
	//! function still leave the VM in a usable state.
	void Cleanup();

	//! \brief Handles script scheduling
	void Scheduler();

	static int ThreadProc(void *data);
};