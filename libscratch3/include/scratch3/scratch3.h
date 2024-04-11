#ifndef _SCRATCH3_H_
#define _SCRATCH3_H_

#ifdef __cplusplus
#define SCRATCH3_EXTERN_C extern "C"
#else
#define SCRATCH3_EXTERN_C
#endif // __cplusplus

#ifdef __cplusplus
class Scratch3;
#else
typedef struct Scratch3 Scratch3;
#endif // __cplusplus

#if _WIN32
#define SCRATCH3_EXPORT __declspec(dllexport)
#else
#define SCRATCH3_EXPORT
#endif // _WIN32

//! \brief The severity of a log message.
typedef enum Scratch3_Severity
{
	Scratch3_Debug,
	Scratch3_Info,
	Scratch3_Warning,
	Scratch3_Error
} Scratch3_Severity;

typedef void (*Scratch3_LogCallback)(void *up, Scratch3_Severity severity, const char *message);

//! \brief Get the default log callback that writes to stdout.
//! 
//! \return A log callback that writes to stdout.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
Scratch3_LogCallback Scratch3_GetStdoutLogCallback(void);

//! \brief Create a new Scratch3 instance.
//! 
//! Creates a new Scratch3 instance targeting the given path.
//! The file may be a .sb3 file or a directory containing a
//! project.json file.
//! 
//! \param path The path to the .sb3 file or directory.
//! \param log The log callback. If NULL, no logging will be done.
//! \param up The user pointer to pass to the log callback.
//! 
//! \return A new Scratch3 instance, or NULL if an error occurred.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
Scratch3 *Scratch3_Create(const char *path, Scratch3_LogCallback log, void *up);

//! \brief Destroy a Scratch3 instance created with Scratch3_Load.
//! 
//! \param S The instance.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
void Scratch3_Destroy(Scratch3 *S);

//! \brief Get the error message for the last error.
//! 
//! \param S The instance.
//! 
//! \return The error code, or 0 if there was no error.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_GetError(Scratch3 *S);

//! \brief Compile the project.
//! 
//! Compiles the project, preparing it for execution. Upon success,
//! the project is ready to be run. If a project has already been
//! compiled, the function will return immediately with success.
//! 
//! \param S The instance.
//! 
//! \return 0 on success, -1 on error.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Compile(Scratch3 *S);

//! \brief Dump the AST to stdout.
//! 
//! Dumps a compiled project's Abstract Syntax Tree to stdout.
//! 
//! \param S The instance.
//! 
//! \return 0 on success, -1 on error.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_DumpAST(Scratch3 *S);

//! \brief Run a compiled project.
//! 
//! Runs a compiled project. The project must have had a
//! successful call to Scratch3_Compile before this function
//! is called. The interpreter runs asynchronously, so the
//! function will return immediately.
//! 
//! \param S The instance.
//! 
//! \return 0 on success, -1 on error.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Run(Scratch3 *S);

//! \brief Check if the interpreter is running.
//! 
//! \param S The instance.
//! 
//! \return 1 if the interpreter is running, 0 otherwise.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_IsRunning(Scratch3 *S);

//! \brief Suspend the interpreter.
//! 
//! Suspends the execution of the interpreter. The interpreter
//! can be resumed with a call to Scratch3_Resume. If the
//! interpreter is suspended, not running, or stopped, this
//! function will return immediately with success.
//! 
//! \param S The instance.
//! 
//! \return 0 on success, -1 on error.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Suspend(Scratch3 *S);

//! \brief Resume the interpreter.
//! 
//! Resumes the execution of the interpreter. If the interpreter
//! is not suspended, this function will return immediately with
//! success.
//! 
//! \param S The instance.
//! 
//! \return 0 on success, -1 on error.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Resume(Scratch3 *S);

//! \brief Stop the interpreter.
//! 
//! Stops the execution of the interpreter. The interpreter
//! can be resumed with a call to Scratch3_Run. If the
//! interpreter is stopped, this function will return
//! immediately with success.
//! 
//! \param S The instance.
//! 
//! \return 0 on success, -1 on error.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Stop(Scratch3 *S);

//! \brief Wait for the interpreter to finish.
//! 
//! Waits for the interpreter to finish executing. If the
//! interpreter is not running, this function will return
//! immediately with success.
//! 
//! \param S The instance.
//! \param ms The maximum number of milliseconds to wait
//! for the interpreter to finish.
//! 
//! \return 0 on success, 1 on a timeout, -1 on error.
SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Wait(Scratch3 *S, unsigned long ms);

#endif // _SCRATCH3_H_
