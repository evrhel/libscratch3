#include <cstdio>

#include <scratch3/scratch3.h>
#include <lysys/lysys.hpp>

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

	ls_init(NULL);

	struct ls_stat st;
	if (ls_stat(file, &st) != 0)
	{
		fprintf(stderr, "Could not find project file\n");
		ls_shutdown();
		return 1;
	}

	bool isDir = st.type == LS_FT_DIR;

	Scratch3Info info{};

	S = Scratch3_Create(&info);
	if (!S)
	{
		fprintf(stderr, "Failed to create Scratch3 instance\n");
		ls_shutdown();
		return 1;
	}

	if (isDir)
		rc = Scratch3_LoadProjectFromDir(S, file);
	else
		rc = Scratch3_LoadProject(S, file);

	if (rc == -1)
	{
		fprintf(stderr, "Failed to load project\n");
		Scratch3_Destroy(S);
		ls_shutdown();
		return 1;
	}

	double start = ls_time64();

	rc = Scratch3_Run(S);
	if (rc == -1)
	{
		fprintf(stderr, "Failed to run project\n");

		Scratch3_Destroy(S);
		ls_shutdown();
		return 1;
	}

	rc = Scratch3_Wait(S, -1);
	if (rc == -1)
	{
		fprintf(stderr, "Failed to wait for project to finish\n");
		Scratch3_Destroy(S);
		ls_shutdown();
		return 1;
	}

	double end = ls_time64();

	printf("Project finished in %.2f seconds\n", end - start);

	Scratch3_Destroy(S);
	ls_shutdown();

	return 0;
}
