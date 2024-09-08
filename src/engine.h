#ifndef __ENGINE_H
#define __ENGINE_H

#include "stdio.h"
typedef struct {
	char *wd; /* Working Directory */
	char *od; /* Output Directory */
	char **excludes;
	size_t n_excludes;
	char **hypertext_content;
	int n_hypertext_content;
	char **non_hypertext_content;
	int n_non_hypertext_content;
} Engine;

void engine_init(char *working_directory, char *output_directory);
void engine_exit(void);

void engine_hydrate_files(char *directory);

void engine_parse_excludes(void);
void engine_parse_hypertext_content(void);
void engine_recreate_output_directory(void);

char *engine_fetch_partial_content(char *partial_name);

#endif
