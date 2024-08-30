#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

#include <lysys/lysys.hpp>

#include <scratch3/scratch3.h>

static void *ReadFile(const char *file, size_t *size)
{
	ls_handle fh;
	void *data;
	size_t len;
	int rc;

	fh = ls_open(file, LS_FILE_READ, 0, LS_OPEN_EXISTING);
	if (!fh)
	{
		ls_perror("ls_open");
		return nullptr;
	}

	struct ls_stat st;
	rc = ls_stat(file, &st);
	if (rc == -1)
	{
		ls_perror("ls_stat");
		ls_close(fh);
		return nullptr;
	}

	data = malloc(st.size);
	if (!data)
	{
		printf("Failed to allocate memory\n");
		ls_close(fh);
		return nullptr;
	}

	len = ls_read(fh, data, st.size);

	if (len == -1)
	{
		ls_perror("ls_read");
		ls_close(fh);
		free(data);
		return nullptr;
	}

	ls_close(fh);

	*size = len;
	return data;
}

static void Usage()
{
	printf("Usage: scratch3 [options...] <project>\n\n");
	printf("Options:\n");
	printf("  -h, --help                 Show this message\n");
	printf("  -v, --version              Show version\n");
	printf("  -Og, -O0, -O1, -O2, -Oa    Set optimization level, default -O2\n");
	printf("  -Oh                        Show optimization help\n");
	printf("  -c, --compile              Only compile project\n");
	printf("  -o, --out <file>           Specify binary output file\n");
	printf("  -d, --debug                Enable live debugging\n");
	printf("  -F, --framerate            Set framerate\n");
	printf("  -W, --width <width>        Set window width\n");
	printf("  -H, --height <height>      Set window height\n");
	printf("  -r, --resizable            Set window resizable\n");
	printf("  -s, --stream               Stream resources instead of loading all at once\n");
	printf("  -f, --fullscreen           Set fullscreen\n");
	printf("  -b, --borderless           Set borderless\n");
	printf("  -a, --free-aspect          Don't lock aspect ratio\n");
	printf("  -u, --suspend              Suspend VM on start\n");
}

static void Version()
{
	printf("scratch3 1.0\n");
}

static void OptimizationHelp()
{
	printf("Optimization levels:\n");
	printf("  -O0    No optimization\n");
	printf("  -O1    Basic optimization\n");
	printf("  -O2    Full optimization\n");
	printf("  -Og    Debug optimization\n");
	printf("  -Oa    Aggressive optimization. Identifies common subexpressions and\n");
	printf("         replaces them with a single instruction. This may cause some\n");
	printf("         projects to behave differently.\n");
}

struct Options
{
	char *file = nullptr;
	int optimization = 2;
	bool onlyCompile = false;
	char *out = nullptr;
	bool debugCompile = false;
	bool liveDebug = false;
	int framerate = -1;
	int width = -1;
	int height = -1;
	bool resizable = false;
	bool stream = false;
	bool fullscreen = false;
	bool borderless = false;
	bool freeAspectRatio = false;
	bool suspend = false;

	void Parse(int argc, char *argv[])
	{
		for (int i = 1; i < argc; i++)
		{
			char *arg = argv[i];
			if (!strcmp(arg, "--help"))
			{
				Usage();
				exit(0);
			}
			else if (!strcmp(arg, "--version"))
			{
				Version();
				exit(0);
			}
			else if (!strcmp(arg, "--compile"))
				onlyCompile = true;
			else if (!strcmp(arg, "--out") || !strcmp(arg, "-o"))
			{
				if (i + 1 >= argc)
				{
					fprintf(stderr, "Missing argument for --out\n");
					exit(1);
				}
				out = argv[++i];
			}
			else if (!strcmp(arg, "--debug"))
				liveDebug = true;
			else if (!strcmp(arg, "--framerate") || !strcmp(arg, "-F"))
			{
				if (i + 1 >= argc)
				{
					fprintf(stderr, "Missing argument for --framerate\n");
					exit(1);
				}
				framerate = atoi(argv[++i]);
			}
			else if (!strcmp(arg, "--width") || !strcmp(arg, "-W"))
			{
				if (i + 1 >= argc)
				{
					fprintf(stderr, "Missing argument for --width\n");
					exit(1);
				}
				width = atoi(argv[++i]);
			}
			else if (!strcmp(arg, "--height") || !strcmp(arg, "-H"))
			{
				if (i + 1 >= argc)
				{
					fprintf(stderr, "Missing argument for --height\n");
					exit(1);
				}
				height = atoi(argv[++i]);
			}
			else if (!strcmp(arg, "--resizable"))
				resizable = true;
			else if (!strcmp(arg, "--stream"))
				stream = true;
			else if (!strcmp(arg, "--fullscreen"))
				fullscreen = true;
			else if (!strcmp(arg, "--borderless"))
				borderless = true;
			else if (!strcmp(arg, "--free-aspect"))
				freeAspectRatio = true;
			else if (!strcmp(arg, "--suspend"))
				suspend = true;
			else if (!strcmp(arg, "-Og"))
			{
				optimization = 0;
				debugCompile = true;
			}
			else if (!strcmp(arg, "-O0"))
				optimization = 0;
			else if (!strcmp(arg, "-O1"))
				optimization = 1;
			else if (!strcmp(arg, "-O2"))
				optimization = 2;
			else if (!strcmp(arg, "-Oa"))
				optimization = 1000;
			else if (!strcmp(arg, "-Oh"))
			{
				OptimizationHelp();
				exit(0);
			}
			else if (arg[0] == '-')
			{
				arg++;
				while (*arg)
				{
					char c = *arg++;
					switch (c)
					{
					case 'h':
						Usage();
						exit(0);
					case 'v':
						Version();
						exit(0);
					case 'c':
						onlyCompile = true;
						break;
					case 'd':
						liveDebug = true;
						break;
					case 'r':
						resizable = true;
						break;
					case 's':
						stream = true;
						break;
					case 'f':
						fullscreen = true;
						break;
					case 'b':
						borderless = true;
						break;
					case 'a':
						freeAspectRatio = true;
						break;
					case 'u':
						suspend = true;
						break;
					case 'o':
					case 'F':
					case 'W':
					case 'H':
					case 'O':
						printf("Cannot use -%c in this context\n", c);
						exit(1);
					default:
						printf("Unknown option: -%c\n", c);
						exit(1);
					}
				}
			}
			else
			{
				file = arg;
				break;
			}
		}
	}
};

static std::string GetName(const char *path)
{
	std::string name;
	size_t cb = ls_basename(path, nullptr, 0);
	if (cb == 0)
		return path; // no basename

	name.resize(cb - 1);
	ls_basename(path, &name[0], name.size() + 1);
	return name;
}

static void ExportCompiled(Scratch3 *S, Options &opts)
{
	std::string out;
	if (!opts.out)
	{
		std::string dir, name;

		dir.resize(ls_dirname(opts.file, nullptr, 0) - 1);
		ls_dirname(opts.file, &dir[0], dir.size() + 1);

		name.resize(ls_basename(opts.file, nullptr, 0) - 1);
		ls_basename(opts.file, &name[0], name.size() + 1);

		size_t ext = name.find_last_of('.');
		if (ext != std::string::npos)
			name = name.substr(0, ext);

		out = dir + LS_PATH_SEP_STR + name + ".csb3";
	}
	else
		out = opts.out;

	size_t binaryLen;
	const void *prog = Scratch3GetProgram(S, &binaryLen);

	ls_handle file = ls_open(out.c_str(), LS_FILE_WRITE, 0, LS_CREATE_ALWAYS);
	if (!file)
	{
		printf("Failed to open output file\n");
		exit(1);
	}

	ls_write(file, prog, binaryLen);

	ls_close(file);

	printf("Wrote binary to `%s`\n", out.c_str());

	Scratch3Destroy(S);
}

int main(int argc, char *argv[])
{
	Scratch3 *S;
	int rc;

	if (argc < 2)
	{
		Usage();
		exit(1);
	}

	Options opts;
	opts.Parse(argc, argv);	

	if (!opts.file)
	{
		printf("Missing project file\n");
		exit(1);
	}

	S = Scratch3Create();
	if (!S)
	{
		printf("Failed to create instance\n");
		exit(1);
	}

	Scratch3SetLog(S, Scratch3GetStdoutLog(), SCRATCH3_SEVERITY_INFO, nullptr);

	size_t size;
	void *data = ReadFile(opts.file, &size);
	if (!data)
	{
		printf("Failed to read project\n");
		exit(1);
	}

	rc = Scratch3Load(S, GetName(opts.file).c_str(), data, size);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		printf("Failed to load project: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	Scratch3CompilerOptions compileOptions;
	memset(&compileOptions, 0, sizeof(compileOptions));
	compileOptions.debug = opts.debugCompile ? 1 : 0;
	compileOptions.optimization = opts.optimization;

	rc = Scratch3Compile(S, &compileOptions);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		printf("Failed to compile project: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	if (opts.onlyCompile)
	{
		ExportCompiled(S, opts);
		return 0;
	}

	Scratch3VMOptions vmOptions;
	memset(&vmOptions, 0, sizeof(vmOptions));
	vmOptions.debug = opts.liveDebug;
	vmOptions.framerate = opts.framerate;
	vmOptions.suspend = opts.suspend;
	vmOptions.width = opts.width;
	vmOptions.height = opts.height;
	vmOptions.resizable = opts.resizable;
	vmOptions.stream = opts.stream;
	vmOptions.fullscreen = opts.fullscreen;
	vmOptions.borderless = opts.borderless;
	vmOptions.freeAspectRatio = opts.freeAspectRatio;

	rc = Scratch3VMInit(S, &vmOptions);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		printf("Failed to initialize VM: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	rc = Scratch3VMStart(S);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		printf("Failed to start VM: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	while ((rc = Scratch3VMUpdate(S) == 0)); // loop until VM terminates

	Scratch3Destroy(S);

	return 0;
}
