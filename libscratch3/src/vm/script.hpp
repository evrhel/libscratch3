#pragma once

#include <csetjmp>

#include <lysys/lysys.hpp>

// Script has been created but not yet started
#define EMBRYO 0

// Script can be scheduled to run
#define RUNNABLE 1

// Script is running
#define RUNNING 2

// Script is waiting for a condition to be met
#define WAITING 3

// Script is suspended
#define SUSPENDED 4

// Script has terminated
#define TERMINATED 5

// Stack size for each script
#define STACK_SIZE 512

// Maximum nesting depth of scripts
#define SCRIPT_DEPTH 32

// Execute the frame forever
#define FRAME_EXEC_FOREVER 0x1

// Execute the frame multiple times, as specified by the count field
#define FRAME_EXEC_MULTIPLE 0x2

// Disable screen updates
#define FRAME_NO_SCREEN_UPDATE 0x4

class StatementList;
class Sprite;
class Expression;
class Value;
class VirtualMachine;

struct Frame
{
	StatementList *sl; // Statement list to execute
	uintptr_t pc; // Program counter
	int64_t count; // Number of times to repeat this frame
	uint32_t flags; // Execution flags
};

struct Script
{
	int state;  // Script state
	Sprite *sprite;  // Owner
	StatementList *entry;  // Entry point

	ls_handle fiber;  // Fiber handle

	double sleepUntil;  // Time to wake up
	Expression *waitExpr;  // Expression to wait for
	bool waitInput;  // Wait for input
	bool askInput;  // Ask for input

	uint64_t ticks;  // Number of ticks executed since the last yield

	Value *stack;  // Base of the stack (lowest address)
	Value *sp;  // Stack pointer (highest address, grows downwards) sp - 1 is the next free slot

	Frame frames[SCRIPT_DEPTH]; // Execution stack
	uintptr_t fp; // Frame pointer (index of the top frame, grows upwards)

	VirtualMachine *vm; // Virtual machine

	jmp_buf scriptMain; // Jump buffer for script main loop
	bool restart; // Whether this script should restart itself
};

const char *GetStateName(int state);
