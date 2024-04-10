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

	Resource *res = _loader->Find("project.json");
	if (!res)
		return -1;

	const char *str = reinterpret_cast<const char *>(res->Data());
	size_t size = res->Size();
	std::vector<Message> log;

	_program = ParseAST(str, size, &log);

	if (log.size() > 0)
	{
		printf("Errors:\n");
		for (auto &msg : log)
		{
			if (msg.type == MessageType_Error)
				printf("Error: %s\n", msg.message.c_str());
			else if (msg.type == MessageType_Warning)
				printf("Warning: %s\n", msg.message.c_str());
		}
	}

	if (!_program)
	{
		printf("Failed to parse\n");
		return -1;
	}

	printf("Parsed without errors\n");

	Visitor *dump = CreateDumpVisitor();
	_program->Accept(dump);

	return 0;
}
