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

int main(int argc, char *argv[])
{
	Scratch3 *S;
	int rc;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: scratch3 <project>\n");
		return 1;
	}

	const char *file = argv[1];

	printf("Loading project `%s`\n", file);

	S = Scratch3Create();
	if (!S)
	{
		fprintf(stderr, "Failed to create instance\n");
		exit(1);
	}

	Scratch3SetLog(S, Scratch3GetStdoutLog(), SCRATCH3_SEVERITY_INFO, nullptr);

	size_t size;
	void *data = ReadFile(file, &size);
	if (!data)
	{
		fprintf(stderr, "Failed to read project\n");
		exit(1);
	}

	rc = Scratch3Load(S, file, data, size);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		fprintf(stderr, "Failed to load project: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	printf("Compiling project\n");

	Scratch3CompilerOptions compileOptions;
	memset(&compileOptions, 0, sizeof(compileOptions));

	rc = Scratch3Compile(S, &compileOptions);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		fprintf(stderr, "Failed to compile project: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	Scratch3VMOptions vmOptions;
	memset(&vmOptions, 0, sizeof(vmOptions));

	rc = Scratch3VMInit(S, &vmOptions);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		fprintf(stderr, "Failed to initialize VM: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	rc = Scratch3VMRun(S);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		fprintf(stderr, "Failed to run project: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	rc = Scratch3VMWait(S, -1);
	if (rc != SCRATCH3_ERROR_SUCCESS)
	{
		fprintf(stderr, "Failed to wait for project to finish: %s\n", Scratch3GetErrorString(rc));
		exit(1);
	}

	Scratch3Destroy(S);

	return 0;
}
