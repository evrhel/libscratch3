#include "core.hpp"

#include <lysys/lysys.hpp>

#include "resource.hpp"

S3_EXTERN_C S3_EXPORT Scratch3 *Scratch3_Create(Scratch3Info *info) { return new Scratch3(info); }
S3_EXTERN_C S3_EXPORT void Scratch3_Destroy(Scratch3 *scratch3) { delete scratch3; }
S3_EXTERN_C S3_EXPORT int Scratch3_LoadProject(Scratch3 *S, const char *filename) { return S->LoadProject(filename); }
S3_EXTERN_C S3_EXPORT int Scratch3_LoadProjectFromMemory(Scratch3 *S, const char *data, size_t size) { return S->LoadProjectFromMemory(data, size); }
S3_EXTERN_C S3_EXPORT int Scratch3_LoadProjectFromDir(Scratch3 *S, const char *dirname) { return S->LoadProjectFromDir(dirname); }
S3_EXTERN_C S3_EXPORT int Scratch3_Run(Scratch3 *S) { return -1; }
S3_EXTERN_C S3_EXPORT void Scratch3_Stop(Scratch3 *S) { }
S3_EXTERN_C S3_EXPORT int Scratch3_Wait(Scratch3 *S, unsigned long ms) { return -1; }

int Scratch3::LoadProject(const char *filename)
{
	if (_loader)
		return -1;

	_loader = CreateArchiveLoader(filename);
	if (!_loader)
		return -1;

	printf("\033[1mLoading archive \033[33;1m`%s`\033[0m\n", filename);
	return Load();
}

int Scratch3::LoadProjectFromMemory(const char *data, size_t size)
{
	return -1;
}

int Scratch3::LoadProjectFromDir(const char *dirname)
{
	if (_loader)
		return -1;

	_loader = CreateDirectoryLoader(dirname);
	if (!_loader)
		return -1;

	printf("\033[1mLoading directory \033[33;1m`%s`\033[0m\n", dirname);
	return Load();
}

int Scratch3::Run()
{
	return -1;
}

void Scratch3::Stop()
{

}

int Scratch3::Wait(unsigned long ms)
{
	return -1;
}

Scratch3::Scratch3(Scratch3Info *info) :
	_loader(nullptr),
	_program(nullptr)
{
	ls_init(NULL);
}

Scratch3::~Scratch3()
{
	delete _loader;
	ls_shutdown();
}

int Scratch3::Load()
{
	if (_program)
		return -1;

	double start = ls_time64();

	Resource *res = _loader->Find("project.json");
	if (!res)
	{
		printf("\033[31;1mCould not find project.json\033[0m\n");
		return -1;
	}

	const char *str = reinterpret_cast<const char *>(res->Data());
	size_t size = res->Size();
	std::vector<Message> log;

	_program = ParseAST(str, size, &log);

	/*if (log.size() > 0)
	{
		for (auto &msg : log)
		{
			if (msg.type == MessageType_Error)
				printf("\033[31;1mERRO:\033[0m %s\n", msg.message.c_str());
			else if (msg.type == MessageType_Warning)
				printf("\033[33;1mWARN:\033[0m %s\n", msg.message.c_str());
			else if (msg.type == MessageType_Info)
				printf("\033[32;1mINFO:\033[0m %s\n", msg.message.c_str());
		}
	}*/

	double elapsed = ls_time64() - start;
	if (elapsed < 1.0)
		printf("Finished in %g sec\n", round(elapsed * 1000.0) / 1000.0);

	if (!_program)
		return -1;

	Visitor *dump = CreateDumpVisitor();
	_program->Accept(dump);

	return 0;
}
