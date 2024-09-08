#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "engine.h"

#define OUTPUT_DIRECTORY ".out"

int
main(int argc, char **argv)
{
	char *working_directory;
	if (argc < 2) {
		// working_directory = getcwd(NULL, 0);
		asprintf(&working_directory, ".");
	} else {
		asprintf(&working_directory, "%s", argv[1]);
	}

	char *output_directory;
	asprintf(&output_directory, "%s/" OUTPUT_DIRECTORY, working_directory);

	engine_init(working_directory, output_directory);

	engine_recreate_output_directory();
	engine_parse_hypertext_content();

	engine_exit();
	return EXIT_SUCCESS;
}
