#include <dirent.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "template.h"

#define PARTIAL_DIRECTORY "partials"
#define RESOURCES_FILE ".resources"

Engine *e;

static int
filter(const struct dirent *entry)
{
	if (entry->d_type == DT_REG) {

		char *dot = strrchr(entry->d_name, '.');
		if (dot && !strcmp(dot, ".html"))
			return 1;

		return 0;
	} else if (entry->d_type != DT_DIR)
		return 0;

	/* TODO: Recursively search subdirs, unless they're present in resources */
	return 0;
}

void
engine_init(char *working_directory, char *output_directory)
{
	e = calloc(1, sizeof(Engine));

	e->wd = working_directory;
	e->od = output_directory;

	engine_parse_resources();

	e->n_base_files = scandir(e->wd, &e->base_files, filter, alphasort);
}

void
engine_exit(void)
{
	free(e->wd);
	free(e->od);
	free(e);
}

void
engine_parse_resources(void)
{
	char *full_path;
	asprintf(&full_path, "%s/" RESOURCES_FILE, e->wd);

	FILE *f = fopen(full_path, "r");
	if (f == NULL) {
		printf("Unable to open file: %s\n", full_path);
		engine_exit();
		exit(EXIT_FAILURE);
	}

	/* TODO: Implement read file function */
	char *output = template_ingest_file(f);
	e->resources = malloc(1);

	char *token = strtok(output, "\n");
	int i;
	for (i = 0; token != NULL; i++) {
		reallocarray(e->resources, i + 1, sizeof(char *));
		e->resources[i] = token;
		token = strtok(NULL, "\n");
	}
	free(output);

	e->n_resources = i;
}

static void
parse_base_file(const struct dirent *file)
{
	char *full_path;
	asprintf(&full_path, "%s/%s", e->wd, file->d_name);

	FILE *f = fopen(full_path, "r");
	if (f == NULL) {
		printf("Unable to open file: %s\n", full_path);
		engine_exit();
		exit(EXIT_FAILURE);
	}

	char *output = template_ingest_file(f);
	fclose(f);

	char *output_path;
	asprintf(&output_path, "%s/%s", e->od, file->d_name);

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
engine_parse_base_files(void)
{
	for (int i = 0; i < e->n_base_files; i++)
		parse_base_file(e->base_files[i]);
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

	char *output = template_ingest_file(f);

	return output;
}
