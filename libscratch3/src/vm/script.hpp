#pragma once

#include <csetjmp>

#include <lysys/lysys.hpp>

#include "../codegen/opcode.hpp"

#include "preload.hpp"
#include "exception.hpp"

enum
{
	// Script has been created but not yet started
	EMBRYO = 0,

	// can be scheduled to run
	RUNNABLE = 1,

	// Script is running
	RUNNING = 2,

	// Script is waiting for a condition to be met
	WAITING = 3,

	// Script is suspended
	SUSPENDED = 4,

	// Script has terminated
	TERMINATED = 5
};

// Stack size for each script
#define STACK_SIZE 512

class StatementList;
class Sprite;
class Expression;
class Value;
class VirtualMachine;
class Sound;

struct Script
{
	int state;  // Script state
	Sprite *sprite;  // Owner

	ls_handle fiber;  // Fiber handle

	double sleepUntil;  // Time to wake up
	bool waitInput;  // Wait for input
	bool askInput;  // Ask for input
	Sound *waitSound;  // Sound to wait until finished

	uint64_t ticks;  // Number of ticks executed since the last yield

	uint8_t *entry;  // Entry point
	uint8_t *pc;  // Program counter

	Value *stack;  // Base of the stack (lowest address)
	Value *sp;  // Stack pointer (highest address, grows downwards) sp - 1 is the next free slot

	jmp_buf scriptMain; // Jump buffer for script main loop
	bool isReset;  // Reset flag

	ExceptionType except; // Exception type
	const char *exceptMessage; // Exception message

	VirtualMachine *vm;  // Virtual machine

	void Init(const ScriptInfo *info);
	void Destroy();
	void Reset();
	void Start();
	void Main();

	Value &Push();
	void Pop();
	Value &StackAt(size_t i);

	void Sched();
	void LS_NORETURN Terminate();
	void LS_NORETURN Raise(ExceptionType type, const char *message = nullptr);
	void Sleep(double seconds);
	void WaitForSound(Sound *sound);

	void Glide(double x, double y, double t);
	void AskAndWait(const std::string &question);

	void Dump();
};

const char *GetStateName(int state);
