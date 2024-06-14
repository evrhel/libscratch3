#pragma once

#include <scratch3/scratch3.h>
#include <string>

class Loader;
struct Program;
class VirtualMachine;

struct _Scratch3
{
	Scratch3LogFn log;
	int minSeverity;
	void *up;

	std::string programName;
	Loader *loader;

	uint8_t *bytecode;
	size_t bytecodeSize;

	VirtualMachine *vm;
};
