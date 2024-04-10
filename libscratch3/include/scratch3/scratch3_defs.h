#ifndef _SCRATCH3_DEFS_H_
#define _SCRATCH3_DEFS_H_

#ifdef __cplusplus
#define S3_EXTERN_C extern "C"
#else
#define S3_EXTERN_C
#endif // __cplusplus

#ifdef __cplusplus
class Scratch3;
#else
typedef struct Scratch3 Scratch3;
#endif // __cplusplus

#if _WIN32
#define S3_EXPORT __declspec(dllexport)
#else
#define S3_EXPORT
#endif // _WIN32

#include <stdint.h>

#endif // _SCRATCH3_DEFS_H_
