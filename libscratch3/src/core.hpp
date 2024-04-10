#pragma once

#include <scratch3/scratch3_core.h>

#include "ast/ast.hpp"

enum ExceptionCode
{
	Exception_None = 0,

	Exception_OutOfMemory = 0x1,
	Exception_InvalidArgumen = 0x2,
	Exception_InvalidOperation = 0x4,
	Exception_InvalidState = 0x8,
	Exception_UnknownVariable = 0x10,
	Exception_UnknownOpcode = 0x20,

	Exception_Any = 0x1f
};

class Loader;

class Scratch3 final
{
public:
    int LoadProject(const char *filename);
    int LoadProjectFromMemory(const char *data, size_t size);
    int LoadProjectFromDir(const char *dirname);

	int Run();
	void Stop();
	int Wait(unsigned long ms);

    Scratch3(Scratch3Info *info);
    virtual ~Scratch3();
private:
    Loader *_loader;
	Program *_program;

    int Load();
};
