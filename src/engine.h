#ifndef __ENGINE_H
#define __ENGINE_H

typedef struct {
	char *wd; /* Working Directory */
	char *od; /* Output Directory */
	struct dirent **base_files;
	int n_base_files;
} Engine;

void engine_init(char *working_directory, char *output_directory);
void engine_exit(void);

void engine_parse_base_files(void);
void engine_recreate_output_directory(void);

char *engine_fetch_partial_content(char *partial_name);

#endif
