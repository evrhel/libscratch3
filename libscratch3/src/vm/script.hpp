#pragma once

#include <csetjmp>

#include <lysys/lysys.hpp>

#include "../codegen/opcode.hpp"

#include "preload.hpp"

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

class StatementList;
class Sprite;
class Expression;
class Value;
class VirtualMachine;

struct Script
{
	int state;  // Script state
	Sprite *sprite;  // Owner

	ls_handle fiber;  // Fiber handle

	double sleepUntil;  // Time to wake up
	bool waitInput;  // Wait for input
	bool askInput;  // Ask for input

	uint64_t ticks;  // Number of ticks executed since the last yield

	uint8_t *entry;  // Entry point
	uint8_t *pc;  // Program counter

	Value *stack;  // Base of the stack (lowest address)
	Value *sp;  // Stack pointer (highest address, grows downwards) sp - 1 is the next free slot

	jmp_buf scriptMain; // Jump buffer for script main loop
	bool restart; // Whether this script should restart itself

	VirtualMachine *vm;  // Virtual machine
};

const char *GetStateName(int state);

void ScriptInit(Script &script, const ScriptInfo *info);
void ScriptDestroy(Script &script);

void ScriptReset(Script &script);
void ScriptStart(Script &script);

// entry point
int ScriptMain(void *scriptPtr);
