#ifndef __ENGINE_H
#define __ENGINE_H

#include "stdio.h"

typedef struct {
	char *wd; /* Working Directory */
	char *od; /* Output Directory */
	char **resources;
	size_t n_resources;
	struct dirent **base_files;
	size_t n_base_files;
} Engine;

void engine_init(char *working_directory, char *output_directory);
void engine_exit(void);

void engine_parse_resources(void);
void engine_parse_base_files(void);
void engine_recreate_output_directory(void);

char *engine_fetch_partial_content(char *partial_name);

#endif
