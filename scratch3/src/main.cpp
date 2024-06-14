#include <cstdio>
#include <cstdlib>
#include <memory>

#include <scratch3/scratch3.h>

static void *ReadFile(const char *file, size_t *size)
{
	FILE *fp;
	void *data;
	size_t len;

	fp = fopen(file, "rb");
	if (!fp)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	data = malloc(len);
	if (!data)
	{
		fclose(fp);
		return nullptr;
	}

	if (fread(data, 1, len, fp) != len)
	{
		fclose(fp);
		free(data);
		return nullptr;
	}

	fclose(fp);

	*size = len;
	return data;
}

static void Usage()
{
	printf("Usage: scratch3 [options...] <project>\n\n");
	printf("Options:\n");
	printf("  -h, --help            Show this message\n");
	printf("  -v, --version         Show version\n");
	printf("  -q, --quiet           Suppress all output\n");
	printf("  -Og, -O0, -O1, -O2    Set optimization level, default -O2\n");
	printf("  -c, --compile         Only compile project\n");
	printf("  -d, --debug           Enable live debugging\n");
	printf("  -F, --framerate       Set framerate\n");
	printf("  -W, --width <width>   Set window width\n");
	printf("  -H, --height <height> Set window height\n");
	printf("  -r, --resizable       Set window resizable\n");
}

static void Version()
{
	printf("scratch3 1.0\n");
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

	const char *file = nullptr;
	bool quiet = false;
	int optimization = 2;
	bool onlyCompile = false;
	bool debugCompile = false;
	bool liveDebug = false;
	int framerate = -1;
	int width = -1, height = -1;
	bool resizable = false;

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
		else if (!strcmp(arg, "--quiet"))
			quiet = true;
		else if (!strcmp(arg, "--compile"))
			onlyCompile = true;
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
				case 'q':
					quiet = true;
					break;
				case 'c':
					onlyCompile = true;
					break;
				case 'd':
					liveDebug = true;
					break;
				case 'r':
					resizable = true;
					break;
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

	if (!file)
	{
		printf("Missing project file\n");
		exit(1);
	}

	if (!quiet)
		printf("Loading project `%s`\n", file);

	S = Scratch3Create();
	if (!S)
	{
		if (!quiet)
			printf("Failed to create instance\n");
		exit(1);
	}

	int severity = quiet ? SCRATCH3_SEVERITY_MAX : SCRATCH3_SEVERITY_INFO;
	Scratch3SetLog(S, Scratch3GetStdoutLog(), severity, nullptr);

	size_t size;
	void *data = ReadFile(file, &size);
	if (!data)
	{
		if (!quiet)
			printf("Failed to read project\n");
		exit(1);
	}

	rc = Scratch3Load(S, file, data, size);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		if (!quiet)
			printf("Failed to load project: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	if (!quiet)
		printf("Compiling project\n");

	Scratch3CompilerOptions compileOptions;
	memset(&compileOptions, 0, sizeof(compileOptions));
	compileOptions.debug = debugCompile ? 1 : 0;
	compileOptions.optimization = optimization;

	rc = Scratch3Compile(S, &compileOptions);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		if (!quiet)
			printf("Failed to compile project: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	Scratch3VMOptions vmOptions;
	memset(&vmOptions, 0, sizeof(vmOptions));
	vmOptions.debug = liveDebug ? 1 : 0;
	vmOptions.framerate = framerate;
	vmOptions.width = width;
	vmOptions.height = height;
	vmOptions.resizable = resizable ? 1 : 0;

	rc = Scratch3VMInit(S, &vmOptions);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		if (!quiet)
			printf("Failed to initialize VM: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	rc = Scratch3VMRun(S);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		if (!quiet)
			printf("Failed to run project: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	rc = Scratch3VMWait(S, -1);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		if (!quiet)
			printf("Failed to wait for project to finish: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	Scratch3Destroy(S);

	return 0;
}
