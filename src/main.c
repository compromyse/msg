#define _GNU_SOURCE

#include <copy.h>
#include <engine.h>
#include <filehandler.h>
#include <ftw.h>
#include <lexer.h>
#include <libgen.h>
#include <mkdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <template.h>

#include "../config.h"

void handle_file(const char *path);

char *base_pre;
char *base_post;

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
  FILE *out = fopen(outpath, "w");

  if (in == NULL) {
    printf("Failed to open %s\n", inpath);
    return;
  }

  if (out == NULL) {
    printf("Failed to open %s\n", outpath);
    return;
  }

  unsigned int size = fsize(in);
  char *buffer = fcontent(in, size);

  if (dot && strcmp(dot, ".md") == 0) {
    MMIOT *doc = mkd_string(buffer, size, 0);
    fprintf(out, "%s", base_pre);
    markdown(doc, out, 0);
    fprintf(out, "%s", base_post);
  } else {
    if (strlen(buffer) != 0)
      ingest(&buffer);
    fprintf(out, "%s%s%s", base_pre, buffer, base_post);
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

  struct stat sb;
  if (stat(DIRECTORY, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
    printf("%s does not exist.\n", DIRECTORY);
    return EXIT_FAILURE;
  }

  template_initialize(&base_pre, &base_post);

  mkdir(OUTPUT, 0700);
  nftw(
      DIRECTORY "/" ASSETS, copy_recursively, 64, FTW_PHYS | FTW_ACTIONRETVAL);

  char **x;
  char *filepath;

  for (x = (char **) html_resources; *x != NULL; x++) {
    asprintf(&filepath, "%s.html", *x);
    handle_file(filepath);
    free(filepath);
  }

  for (x = (char **) md_resources; *x != NULL; x++) {
    asprintf(&filepath, "%s.md", *x);
    handle_file(filepath);
    free(filepath);
  }

  free(base_pre);
  free(base_post);

  return EXIT_SUCCESS;
}
