#pragma once

#include <csetjmp>
#include <string>

#include <lysys/lysys.hpp>

#include "../codegen/opcode.hpp"
#include "../codegen/util.hpp"

#include "../ast/astdef.hpp"

#include "exception.hpp"

enum
{
	// Unallocated script
	EMBRYO = 0,

	// Can be scheduled to run
	RUNNABLE = 1,

	// Script is waiting for a condition to be met
	WAITING = 2,

	// Script is suspended
	SUSPENDED = 3,

	// Script has terminated and cannot be resumed
	TERMINATED = 4
};

// Stack size for each script
#define STACK_SIZE 512

class AbstractSprite;
class Sprite;
class Value;
class VirtualMachine;
class Voice;
class Script;

struct SCRIPT_ALLOC_INFO
{
	Sprite *sprite;
	bc::Script *info;
};

struct STATIC_EVENT_HANDLER
{
	SCRIPT_ALLOC_INFO ai;
	Script *script;
};

struct Script
{
	int state; // Script state
	Sprite *sprite; // Self

	ls_handle fiber; // Fiber handle

	double sleepUntil; // Time to wake up
	bool waitInput; // Wait for input
	bool askInput; // Ask for input
	Voice *waitVoice; // Voice to wait until finished

	uint64_t ticks; // Number of ticks executed since the last yield

	uint8_t *entry; // Entry point
	uint8_t *pc; // Program counter

	Value *stack; // Base of the stack (lowest address)
	Value *sp; // Stack pointer (highest address, grows downwards) sp - 1 is the next free slot

	Value *bp; // Base pointer (stack frame base, points to old bp)

	bool autoStart; // Auto-start flag
	bool scheduled; // Whether the script was scheduled this frame
	bool restart; // Whether the script should restart from the beginning

	jmp_buf entryJmp; // Script entry point

	ExceptionType except; // Exception type
	const char *exceptMessage; // Exception message

	VirtualMachine *vm; // Virtual machine

	size_t refCount; // Reference count

	//! \brief Execute the script.
	//!
	//! Should be called from within the Script's fiber. This will
	//! execute the script until it yields or terminates.
	void LS_NORETURN Main();

	//! \brief Push a value onto the stack.
	//!
	//! Raises a StackOverflow exception if the stack is full.
	//!
	//! \return A reference to the pushed value. Initialized to
	//! None.
	Value &Push();

	//! \brief Pop a value from the stack.
	//!
	//! Raises a StackUnderflow exception if the stack is empty.
	void Pop();

	//! \brief Get a value relative to the top of the stack.
	//!
	//! Raises a StackUnderflow exception if the index is out of
	//! bounds.
	//!
	//! \param i The index of the value, where -1 is the top of the
	//! stack, -2 is the second value from the top, and so on. 0 is
	//! the base of the stack in the current frame.
	//!
	//! \return A reference to the value.
	Value &StackAt(int i);

	//! \brief Yield control to the virtual machine.
	void Sched();

	//! \brief Terminate the script.
	//!
	//! Sets the state to TERMINATED and yields control to the
	//! virtual machine. The script will not be rescheduled until
	//! it is started from the beginning. This function does not
	//! return.
	void LS_NORETURN Terminate();

	//! \brief Raise an exception.
	//!
	//! Sets the exception type and message, then yields control to
	//! the virtual machine. This function does not return.
	//!
	//! \param type The exception type.
	//! \param message The exception message, optional.
	void LS_NORETURN Raise(ExceptionType type, const char *message = nullptr);

	//! \brief Sleep for a given number of seconds.
	//!
	//! Yields control to the virtual machine for the given number
	//! of seconds.
	//!
	//! \param seconds The number of seconds to sleep.
	void Sleep(double seconds);

	//! \brief Wait for a voice to finish playing.
	//!
	//! Yields control to the virtual machine until the voice has
	//! finished playing.
	//!
	//! \param voice The voice to wait for.
	void WaitForVoice(Voice *voice);

	//! \brief Glide to a given position.
	//!
	//! Yields control to the virtual machine until the sprite has
	//! reached the given position.
	//!
	//! \param x The x-coordinate to glide to.
	//! \param y The y-coordinate to glide to.
	//! \param t The time to glide for. If <= 0, the sprite will
	//! instantly move to the target position.
	void Glide(double x, double y, double t);

	//! \brief Ask the user for input.
	//!
	//! Yields control to the virtual machine until the user has
	//! entered a value.
	//!
	//! \param question The question to ask.
	void AskAndWait(const std::string &question);

	//! \brief Dump the script state.
	void Dump();
};

//! \brief Get the name of a script state.
//!
//! \param state The script state.
//!
//! \return A string representation of the script state.
const char *GetStateName(int state);
