#define _GNU_SOURCE

#include <config.h>
#include <copy.h>
#include <engine.h>
#include <errno.h>
#include <filehandler.h>
#include <ftw.h>
#include <lexer.h>
#include <libgen.h>
#include <list.h>
#include <mkdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <template.h>

#include "../config.h"

#define ASSETS "assets"

template_t *base_template;

void
handle_file(const char *path)
{
  char *inpath;
  char *outpath;

  asprintf(&inpath, "%s/%s", DIRECTORY, path);

  char *dot = strrchr(inpath, '.');
  if (dot && strcmp(dot, ".md") == 0) {
    asprintf(&outpath, "%s/%.*s.html", OUTPUT, (int) strlen(path) - 3, path);
  } else {
    asprintf(&outpath, "%s/%s", OUTPUT, path);
  }

  char *temp_outpath = strdup(outpath);
  char *directory = dirname(temp_outpath);
  char *next = calloc(strlen(directory) + 1, sizeof(char));
  strcpy(next, "");

  char *token;
  for (token = strtok(directory, "/"); token != NULL;
       token = strtok(NULL, "/")) {
    if (strcmp(next, "") != 0) {
      strcat(next, "/");
    }

    strcat(next, token);
    mkdir(next, 0700);
  }

  free(next);
  free(temp_outpath);

  FILE *in = fopen(inpath, "r");
  if (in == NULL) {
    printf("Failed to open %s\n", inpath);
    return;
  }

  FILE *out = fopen(outpath, "w");
  if (out == NULL) {
    printf("Failed to open %s\n", outpath);
    fclose(in);
    return;
  }

  unsigned int size = fsize(in);
  char *buffer = fcontent(in, size);

  if (dot && strcmp(dot, ".md") == 0) {
    MMIOT *doc = mkd_string(buffer, size, 0);
    template_write(base_template, NULL, out, doc, true);
  } else if (strlen(buffer) != 0) {
    list_t *content_headers = ingest(&buffer);
    template_write(base_template, content_headers, out, buffer, false);
    list_delete(content_headers);
  }

  free(buffer);

  fclose(in);
  fclose(out);

  free(inpath);
  free(outpath);
}

int
main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  FILE *f = fopen("config.cfg", "r");
  size_t s = fsize(f);
  char *content = fcontent(f, s);
  fclose(f);

  config_t *config = config_parse(content);
  free(content);

  struct stat sb;
  if (stat(DIRECTORY, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
    printf("%s does not exist.\n", DIRECTORY);
    return EXIT_FAILURE;
  }

  base_template = template_create();

  int err = mkdir(OUTPUT, 0700);
  if (err != 0 && errno != EEXIST) {
    perror("mkdir");
    return EXIT_FAILURE;
  }

  nftw(
      DIRECTORY "/" ASSETS, copy_recursively, 64, FTW_PHYS | FTW_ACTIONRETVAL);

  list_t *resources = list_find_corresponding_value_from_ptr_wrapper(
      config->keys, config->array_values, "resources");

  if (resources == NULL) {
    printf("Could not find resources in config.cfg\n");
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < resources->size; i++) {
    ptr_wrapper_t *value = list_get(resources, i);
    char *path = value->ptr;
    printf("HANDLING: %s\n", path);
    handle_file(path);
  }

  free(config);

  return EXIT_SUCCESS;
}
