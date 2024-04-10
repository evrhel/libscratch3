#ifndef _SCRATCH3_CORE_H_
#define _SCRATCH3_CORE_H_

#include "scratch3_defs.h"

#define S3_LOG_DEBUG 4
#define S3_LOG_INFO 3
#define S3_LOG_WARNING 2
#define S3_LOG_ERROR 1
#define S3_LOG_NONE 0

typedef struct Scratch3Info
{
	int loglevel;
	const char *logfile;
} Scratch3Info;

S3_EXTERN_C S3_EXPORT Scratch3 *Scratch3_Create(Scratch3Info *info);
S3_EXTERN_C S3_EXPORT void Scratch3_Destroy(Scratch3 *S);

S3_EXTERN_C S3_EXPORT int Scratch3_LoadProject(Scratch3 *S, const char *filename);
S3_EXTERN_C S3_EXPORT int Scratch3_LoadProjectFromMemory(Scratch3 *S, const char *data, size_t size);
S3_EXTERN_C S3_EXPORT int Scratch3_LoadProjectFromDir(Scratch3 *S, const char *dirname);

S3_EXTERN_C S3_EXPORT int Scratch3_Run(Scratch3 *S);
S3_EXTERN_C S3_EXPORT void Scratch3_Stop(Scratch3 *S);
S3_EXTERN_C S3_EXPORT int Scratch3_Wait(Scratch3 *S, unsigned long ms);

#endif // _SCRATCH3_CORE_H_
