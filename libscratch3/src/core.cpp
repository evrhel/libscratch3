#include "core.hpp"

#include <lysys/lysys.hpp>

#include "resource.hpp"
#include "ast/ast.hpp"
#include "vm/vm.hpp"

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

	// Get type of input file
	rc = ls_stat(path, &st);
	if (rc == -1)
		return nullptr;

	// Create loader
	Loader *loader = nullptr;
	if (st.type = LS_FT_FILE)
		loader = CreateArchiveLoader(path);
	else if (st.type == LS_FT_DIR)
		loader = CreateDirectoryLoader(path);

	if (!loader)
		return nullptr;

	std::string name;
	char buf[1024];
	size_t len;

	len = ls_basename(path, buf, sizeof(buf));
	if (len == -1)
		name = path;
	else
		name = std::string(buf, len);

	// Create the instance
	return new Scratch3(name, loader, log, up);
}

SCRATCH3_EXTERN_C SCRATCH3_EXPORT
void Scratch3_Destroy(Scratch3 *scratch3)
{
	delete scratch3;
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

	_program = Retain(ParseAST(str, size, _log, _up));

	return _program ? 0 : -1;
}

int Scratch3::Run()
{
	if (!_program || _vm)
		return -1;

	_vm = new VirtualMachine();

	int rc = _vm->Load(_program, _programName, _loader);
	if (rc == -1)
	{
		delete _vm;
		_vm = nullptr;
		return -1;
	}

	_vm->VMStart();

	return 0;
}

int Scratch3::Suspend()
{
	if (!_vm)
		return -1;

	return -1;
}

int Scratch3::Resume()
{
	if (!_vm)
		return -1;

	return -1;
}

int Scratch3::Stop()
{
	if (!_vm)
		return -1;
	_vm->VMTerminate();
	return 0;
}

int Scratch3::Wait(unsigned long ms)
{
	int rc;

	if (!_vm)
		return 0;
	
	rc = _vm->VMWait(ms);
	if (rc == 0)
	{
		delete _vm;
		_vm = nullptr;
	}
	
	return rc;
}

Scratch3::Scratch3(const std::string &programName, Loader *loader, Scratch3_LogCallback log, void *up) :
	_log(log),
	_up(up),
	_loader(loader),
	_program(nullptr),
	_programName(programName),
	_vm(nullptr) {}

Scratch3::~Scratch3()
{
	if (_vm)
		delete _vm;

	Release(_program);
	delete _loader;
}
