#include "core.hpp"

#include <lysys/lysys.hpp>

#include "resource.hpp"
#include "ast/ast.hpp"

static void StdoutLogCallback(void *up, Scratch3_Severity severity, const char *message)
{
	if (severity == Scratch3_Error)
		printf("\033[31;1m[ERRO]\033[0m %s\n", message);
	else if (severity == Scratch3_Warning)
		printf("\033[33;1m[WARN]\033[0m %s\n", message);
	else if (severity == Scratch3_Info)
		printf("\033[32m[INFO]\033[0m %s\n", message);
	else
		printf("%s\n", message);
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
Scratch3_LogCallback Scratch3_GetStdoutLogCallback(void)
{
	return &StdoutLogCallback;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
Scratch3 *Scratch3_Create(const char *path, Scratch3_LogCallback log, void *up)
{
	struct ls_stat st;
	int rc;

	rc = ls_init(NULL);
	if (rc == -1)
		return nullptr;

	// Get type of input file
	rc = ls_stat(path, &st);
	if (rc == -1)
	{
		// doesn't exist
		ls_shutdown();
		return nullptr;
	}

	// Create loader
	Loader *loader = nullptr;
	if (st.type = LS_FT_FILE)
		loader = CreateArchiveLoader(path);
	else if (st.type == LS_FT_DIR)
		loader = CreateDirectoryLoader(path);

	if (!loader)
	{
		ls_shutdown();
		return nullptr;
	}

	// Create the instance
	return new Scratch3(loader, log, up);
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
void Scratch3_Destroy(Scratch3 *scratch3)
{
	delete scratch3;
	ls_shutdown();
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_GetError(Scratch3 *S)
{
	return 0;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Compile(Scratch3 *S)
{
	return S->Compile();
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_DumpAST(Scratch3 *S)
{
	Program *program = S->GetProgram();
	if (!program)
		return -1;

	Visitor *dump = CreateDumpVisitor();
	program->Accept(dump);

	return 0;
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Run(Scratch3 *S)
{
	return S->Run();
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Suspend(Scratch3 *S)
{
	return S->Suspend();
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Resume(Scratch3 *S)
{
	return S->Resume();
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Stop(Scratch3 *S)
{
	return S->Stop();
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
int Scratch3_Wait(Scratch3 *S, unsigned long ms)
{
	return S->Wait(ms);
}

int Scratch3::Compile()
{
	if (_program)
		return 0;

	Resource *res = _loader->Find("project.json");
	if (!res)
		return -1;

	const char *str = reinterpret_cast<const char *>(res->Data());
	size_t size = res->Size();

	_program = ParseAST(str, size, _log, _up);

	return _program ? 0 : -1;
}

int Scratch3::Run()
{
	if (!_program)
		return -1;

	return -1;
}

int Scratch3::Suspend()
{
	if (!_program)
		return -1;

	return -1;
}

int Scratch3::Resume()
{
	if (!_program)
		return -1;

	return -1;
}

int Scratch3::Stop()
{
	if (!_program)
		return -1;

	return -1;
}

int Scratch3::Wait(unsigned long ms)
{
	if (!_program)
		return -1;

	return -1;
}

Scratch3::Scratch3(Loader *loader, Scratch3_LogCallback log, void *up) :
	_log(log),
	_up(up),
	_loader(loader),
	_program(nullptr) {}

Scratch3::~Scratch3() { delete _loader; }
