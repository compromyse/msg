#include <stdio.h>
#include <stdlib.h>

#include <util.h>

char *
read_file(char *path)
{
	FILE *f = fopen(path, "r");
	if (f == NULL) {
		printf("Unable to open file: %s\n", path);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = calloc(1, fsize + 1);
	fread(buffer, fsize, 1, f);

	fclose(f);
	return buffer;
}
