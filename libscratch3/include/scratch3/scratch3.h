#ifndef _SCRATCH3_H_
#define _SCRATCH3_H_

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
#define SCRATCH3_EXTERN_C extern "C"
#else
#define SCRATCH3_EXTERN_C
#endif // __cplusplus

typedef struct _Scratch3 Scratch3;

#if _WIN32
#define SCRATCH3_EXPORT __declspec(dllexport)
#else
#define SCRATCH3_EXPORT
#endif // _WIN32

// default framerate
#define SCRATCH3_FRAMERATE 30

enum
{
	SCRATCH3_ERROR_SUCCESS = 0,

	SCRATCH3_ERROR_UNKNOWN = 1,
	SCRATCH3_ERROR_IO,
	SCRATCH3_ERROR_OUT_OF_MEMORY,
	SCRATCH3_ERROR_NO_PROGRAM,
	SCRATCH3_ERROR_ALREADY_LOADED,
	SCRATCH3_ERROR_INVALID_PROGRAM,
	SCRATCH3_ERROR_ALREADY_COMPILED,
	SCRATCH3_ERROR_NOT_COMPILED,
	SCRATCH3_ERROR_COMPILATION_FAILED,
	SCRATCH3_ERROR_NO_VM,
	SCRATCH3_ERROR_ALREADY_RUNNING,
	SCRATCH3_ERROR_TIMEOUT
};

enum
{
	SCRATCH3_PROGRAM_TYPE_NONE,
	SCRATCH3_PROGRAM_TYPE_BYTECODE,
	SCRATCH3_PROGRAM_TYPE_DIR,
	SCRATCH3_PROGRAM_TYPE_ARCHIVE
};

enum
{
	SRCATCH3_SEVERITY_DEBUG = 0,
	SCRATCH3_SEVERITY_INFO,
	SCRATCH3_SEVERITY_WARNING,
	SCRATCH3_SEVERITY_ERROR,
	SCRATCH3_SEVERITY_FATAL,

	SCRATCH3_SEVERITY_MAX
};

typedef struct _Scratch3CompilerOptions
{
	int debug; // implies optimization = 0
	int optimization; // 0 = none, 1 = some, 2 = full
} Scratch3CompilerOptions;

typedef struct _Scratch3VMOptions
{
	int debug;
	int framerate;
	int suspend;

	int width, height;
	int resizable;

	int stream;

	int fullscreen;
	int borderless;
	int freeAspectRatio;
} Scratch3VMOptions;

typedef void (*Scratch3LogFn)(Scratch3 *S, const char *message, size_t len, int severity, void *up);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT const char *Scratch3GetErrorString(int error);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT Scratch3 *Scratch3Create(void);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT void Scratch3Destroy(Scratch3 *S);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT Scratch3LogFn Scratch3GetStdoutLog(void);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT void Scratch3SetLog(Scratch3 *S, Scratch3LogFn log, int severity, void *up);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT Scratch3LogFn Scratch3GetLog(Scratch3 *S);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT void Scratch3VLogf(Scratch3 *S, int severity, const char *format, va_list args);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT void Scratch3Logf(Scratch3 *S, int severity, const char *format, ...);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3Load(Scratch3 *S, const char *name, const void *data, size_t size);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3Compile(Scratch3 *S, const Scratch3CompilerOptions *options);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT const void *Scratch3GetProgram(Scratch3 *S, size_t *size);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3VMInit(Scratch3 *S, const Scratch3VMOptions *options);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3VMStart(Scratch3 *S);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3VMUpdate(Scratch3 *S);

SCRATCH3_EXTERN_C SCRATCH3_EXPORT int Scratch3VMTerminate(Scratch3 *S);

#endif // _SCRATCH3_H_
