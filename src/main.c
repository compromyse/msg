#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config.h"
#include <engine.h>

int
main(int argc, char **argv)
{
	char *working_directory;
	if (argc < 2) {
		working_directory = getcwd(NULL, 0);
	} else {
		asprintf(&working_directory, "%s", argv[1]);
	}

	char *output_directory;
	asprintf(&output_directory, "%s/" OUTPUT_DIRECTORY, working_directory);

	engine_init(working_directory, output_directory);
	engine_run();
	engine_exit();

	return EXIT_SUCCESS;
}
