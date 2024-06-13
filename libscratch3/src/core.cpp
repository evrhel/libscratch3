#include "core.hpp"

#include <lysys/lysys.hpp>
#include <glib.h>
#include <librsvg/rsvg.h>

#include "resource.hpp"
#include "ast/ast.hpp"
#include "vm/vm.hpp"
#include "codegen/compiler.hpp"

static void StdOutLogger(Scratch3 *S, const char *msg, size_t len, int severity, void *up)
{
	if (severity < S->minSeverity)
		return;

	switch (severity)
	{
	case SCRATCH3_SEVERITY_INFO:
		printf("INFO: ");
		break;
	case SCRATCH3_SEVERITY_WARNING:
		printf("WARN: ");
		break;
	case SCRATCH3_SEVERITY_ERROR:
		printf("ERRO: ");
		break;
	case SCRATCH3_SEVERITY_FATAL:
		printf("FATAL: ");
		break;
	}

	fwrite(msg, 1, len, stdout);
	putchar('\n');
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT const char *Scratch3GetErrorString(int error)
{
	switch (error)
	{
	default:
	case SCRATCH3_ERROR_UNKNOWN:
		return "Unknown error";
	case SCRATCH3_ERROR_SUCCESS:
		return "Success";
	case SCRATCH3_ERROR_IO:
		return "I/O error";
	case SCRATCH3_ERROR_OUT_OF_MEMORY:
		return "Out of memory";
	case SCRATCH3_ERROR_NO_PROGRAM:
		return "No program loaded";
	case SCRATCH3_ERROR_INVALID_PROGRAM:
		return "Invalid program";
	case SCRATCH3_ERROR_ALREADY_COMPILED:
		return "Program already compiled";
	case SCRATCH3_ERROR_NOT_COMPILED:
		return "Program not compiled";
	case SCRATCH3_ERROR_COMPILATION_FAILED:
		return "Compilation failed";
	case SCRATCH3_ERROR_NO_VM:
		return "No VM initialized";
	case SCRATCH3_ERROR_ALREADY_RUNNING:
		return "VM already running";
	case SCRATCH3_ERROR_TIMEOUT:
		return "Timeout";
	}
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT Scratch3 *Scratch3Create(void)
{
	Scratch3 *S = new Scratch3;
	memset(S, 0, sizeof(Scratch3));
	return S;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT void Scratch3Destroy(Scratch3 *S)
{
	if (!S)
		return;

	if (S->vm)
		delete S->vm;

	if (S->bytecode)
		delete[] S->bytecode;

	if (S->loader)
		delete S->loader;

	delete S;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT Scratch3LogFn Scratch3GetStdoutLog(void)
{
	return StdOutLogger;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT void Scratch3SetLog(Scratch3 *S, Scratch3LogFn log, int severity, void *up)
{
	S->log = log;
	S->minSeverity = severity;
	S->up = up;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT Scratch3LogFn Scratch3GetLog(Scratch3 *S)
{
	return S->log;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT void Scratch3VLogf(Scratch3 *S, int severity, const char *format, va_list args)
{
	if (!S->log || severity < S->minSeverity)
		return;

	char buf[4096];
	int len = vsnprintf(buf, sizeof(buf), format, args);
	if (len < 0)
		return;
	S->log(S, buf, len, severity, S->up);
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT void Scratch3Logf(Scratch3 *S, int severity, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	Scratch3VLogf(S, severity, format, args);
	va_end(args);
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3Load(Scratch3 *S, const char *name, const void *data, size_t size)
{
	if (size < 4)
		return SCRATCH3_ERROR_INVALID_PROGRAM;

	if (S->bytecode)
		return SCRATCH3_ERROR_ALREADY_LOADED;

	const ProgramHeader *header = (const ProgramHeader *)data;
	if (header->magic == PROGRAM_MAGIC)
	{
		// Program is compiled bytecode
		S->bytecode = (uint8_t *)malloc(size);
		if (!S->bytecode)
			return SCRATCH3_ERROR_OUT_OF_MEMORY;

		memcpy(S->bytecode, data, size);
		S->bytecodeSize = size;

		S->loader = CreateBytecodeLoader(S->bytecode, S->bytecodeSize);
		if (!S->loader)
		{
			free(S->bytecode), S->bytecode = nullptr;
			return SCRATCH3_ERROR_INVALID_PROGRAM;
		}
	}
	else
	{
		// Program is an uncompiled archive
		S->loader = CreateArchiveLoader(data, size);
		if (!S->loader)
			return SCRATCH3_ERROR_INVALID_PROGRAM;
	}

	return SCRATCH3_ERROR_SUCCESS;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3Compile(Scratch3 *S, const Scratch3CompilerOptions *options)
{
	if (!S->loader)
		return SCRATCH3_ERROR_NO_PROGRAM;

	if (S->bytecode)
		return SCRATCH3_ERROR_ALREADY_COMPILED;

	Resource *rsrc = S->loader->Find("project.json");
	if (!rsrc)
		return SCRATCH3_ERROR_IO;

	Program *prog = ParseAST(S, (const char *)rsrc->Data(), rsrc->Size(), options);
	if (!prog)
		return SCRATCH3_ERROR_COMPILATION_FAILED;
	Retain(prog);

	CompiledProgram *cprog = CompileProgram(prog, S->loader, options);
	if (!cprog)
	{
		Release(prog);
		return SCRATCH3_ERROR_COMPILATION_FAILED;
	}

	Release(prog);

	S->bytecode = cprog->Export(&S->bytecodeSize);
	delete cprog;

	if (!S->bytecode)
		return SCRATCH3_ERROR_OUT_OF_MEMORY;

	return SCRATCH3_ERROR_SUCCESS;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT const void *Scratch3GetProgram(Scratch3 *S, size_t *size)
{
	if (!S->bytecode)
		return nullptr;

	*size = S->bytecodeSize;
	return S->bytecode;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3VMInit(Scratch3 *S, const Scratch3VMOptions *options)
{
	if (!S->loader)
		return SCRATCH3_ERROR_NO_PROGRAM;

	if (!S->bytecode)
		return SCRATCH3_ERROR_NOT_COMPILED;

	if (S->vm)
		return SCRATCH3_ERROR_ALREADY_RUNNING;

	S->vm = new VirtualMachine(S, options);

	int rc = S->vm->Load(S->programName, S->bytecode, S->bytecodeSize);
	if (rc == SCRATCH3_ERROR_SUCCESS)
		return SCRATCH3_ERROR_SUCCESS;

	delete S->vm, S->vm = nullptr;
	return rc;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3VMRun(Scratch3 *S)
{
	if (!S->vm)
		return SCRATCH3_ERROR_NO_VM;
	return S->vm->VMStart();
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3VMTerminate(Scratch3 *S)
{
	if (!S->vm)
		return SCRATCH3_ERROR_NO_VM;

	S->vm->Terminate();
	return SCRATCH3_ERROR_SUCCESS;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3VMWait(Scratch3 *S, unsigned long timeout)
{
	if (!S->vm)
		return SCRATCH3_ERROR_NO_VM;
	return S->vm->VMWait(timeout);
}
