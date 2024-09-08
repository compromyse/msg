#include <dirent.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "template.h"

#define PARTIAL_DIRECTORY "partials"
#define EXCLUDE_FILE ".exclude"

Engine *e;

void
engine_init(char *working_directory, char *output_directory)
{
	e = calloc(1, sizeof(Engine));

	e->wd = working_directory;
	e->od = output_directory;

	engine_parse_excludes();
	engine_hydrate_files(e->wd);

	for (int i = 0; i < e->n_hypertext_content; i++)
		puts(e->hypertext_content[i]);
}

void
engine_exit(void)
{
	free(e->wd);
	free(e->od);
	free(e);
}

void
engine_hydrate_files(char *directory)
{
	DIR *d = opendir(directory);
	if (d == NULL) {
		printf("Unable to open directory: %s\n", directory);
		engine_exit();
		exit(EXIT_FAILURE);
	}

	e->hypertext_content = NULL;
	e->non_hypertext_content = NULL;
	e->n_hypertext_content = 0;
	e->n_non_hypertext_content = 0;

	struct dirent *entry;
	while ((entry = readdir(d)) != NULL) {
		if (!strcmp(entry->d_name, "."))
			continue;
		if (!strcmp(entry->d_name, ".."))
			continue;
		for (size_t i = 0; i < e->n_excludes; i++)
			if (!strcmp(entry->d_name, e->excludes[i]))
				continue;

		char *path;
		asprintf(&path, "%s/%s", directory, entry->d_name);

		if (entry->d_type == DT_REG) {
			char *dot = strrchr(entry->d_name, '.');
			if (dot && !strcmp(dot, ".html")) {
				char **temp = reallocarray(e->hypertext_content,
										   e->n_hypertext_content + 1,
										   sizeof(char **));
				e->hypertext_content = temp;

				e->hypertext_content[e->n_hypertext_content] = path;
				e->n_hypertext_content++;
			} else {
				char **temp = reallocarray(e->non_hypertext_content,
										   e->n_non_hypertext_content + 1,
										   sizeof(char **));
				e->non_hypertext_content = temp;

				e->non_hypertext_content[e->n_non_hypertext_content] = path;
				e->n_non_hypertext_content++;
			}
		} else if (entry->d_type == DT_DIR) {
			char *subdir_path;
			asprintf(&subdir_path, "%s/%s", directory, entry->d_name);
			engine_hydrate_files(subdir_path);
		}
	}
}

void
engine_parse_excludes(void)
{
	char *full_path;
	asprintf(&full_path, "%s/" EXCLUDE_FILE, e->wd);

	FILE *f = fopen(full_path, "r");
	if (f == NULL) {
		printf("Unable to open file: %s\n", full_path);
		engine_exit();
		exit(EXIT_FAILURE);
	}

	/* TODO: Implement read file function */
	char *output = template_ingest_file(f);
	e->excludes = NULL;

	char *token = strtok(output, "\n");
	int i;
	for (i = 0; token != NULL; i++) {
		char **temp = reallocarray(e->excludes, i + 1, sizeof(char *));
		e->excludes = temp;

		char *atoken = calloc(1, strlen(token) + 1);
		strcpy(atoken, token);
		e->excludes[i] = atoken;
		token = strtok(NULL, "\n");
	}
	free(output);

	e->n_excludes = i;
}

static void
parse_base_file(char *file_path)
{
	FILE *f = fopen(file_path, "r");
	if (f == NULL) {
		printf("Unable to open file: %s\n", file_path);
		engine_exit();
		exit(EXIT_FAILURE);
	}

	char *output = template_ingest_file(f);
	fclose(f);

	char *output_path;
	asprintf(&output_path, "%s/%s", e->od, file_path);

	FILE *out = fopen(output_path, "w");
	if (out == NULL) {
		printf("Unable to open file: %s\n", output_path);
		engine_exit();
		exit(EXIT_FAILURE);
	}

	fprintf(out, output);
	fclose(out);
}

void
engine_parse_hypertext_content(void)
{
	for (int i = 0; i < e->n_hypertext_content; i++)
		parse_base_file(e->hypertext_content[i]);
}

static int
unlink_cb(const char *fpath, const struct stat *sb, int typeflag,
		  struct FTW *ftwbuf)
{
	(void) sb;
	(void) typeflag;
	(void) ftwbuf;

	int rv = remove(fpath);
	if (rv)
		perror(fpath);
	return rv;
}

void
engine_recreate_output_directory(void)
{
	nftw(e->od, unlink_cb, 64,
		 FTW_DEPTH | FTW_PHYS); /* Recursively delete old output directory */
	mkdir(e->od, 0777);
}

char *
engine_fetch_partial_content(char *partial_name)
{
	char *full_path;
	asprintf(&full_path, "%s/" PARTIAL_DIRECTORY "/%s", e->wd, partial_name);

	FILE *f = fopen(full_path, "r");
	if (f == NULL) {
		printf("Unable to open file: %s\n", full_path);
		engine_exit();
		exit(EXIT_FAILURE);
	}

	return template_ingest_file(f);
}
