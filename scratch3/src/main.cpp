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

	S = Scratch3_Create(file, NULL, NULL);
	if (!S)
	{
		fprintf(stderr, "Failed to create instance\n");
		ls_shutdown();
		return 1;
	}

	rc = Scratch3_Compile(S);
	if (rc == -1)
	{
		fprintf(stderr, "Failed to compile project\n");

		Scratch3_Destroy(S);
		ls_shutdown();
		return 1;
	}

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

	Scratch3_Destroy(S);
	ls_shutdown();

	return 0;
}
