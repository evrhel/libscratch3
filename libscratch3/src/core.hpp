#pragma once

#include <scratch3/scratch3.h>

class Loader;
struct Program;

class Scratch3 final
{
public:
	constexpr Scratch3_LogCallback GetLog() const { return _log; }
	constexpr void *GetUp() const { return _up; }
	constexpr Program *GetProgram() const { return _program; }

	int Compile();

	int Run();
	int Suspend();
	int Resume();
	int Stop();
	int Wait(unsigned long ms);

	Scratch3(Loader *loader, Scratch3_LogCallback log, void *up);
    ~Scratch3();
private:
	Scratch3_LogCallback _log;
    void *_up; // user pointer
	Loader *_loader;
	Program *_program;
};
