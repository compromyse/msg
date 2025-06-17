#define _GNU_SOURCE

#include <ctype.h>
#include <libgen.h>
#include <mkdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

typedef enum { CONTENT, INCLUDE } directive_e;

typedef struct {
  unsigned int offset;
  unsigned int length;
} key_match_t;

typedef struct {
  directive_e type;
  void *operands;
} directive_t;

char *fcontent(FILE *f, unsigned int size);
directive_t *find_directive(char *content, key_match_t *match);
key_match_t *find_next_key(char *buffer);
unsigned int fsize(FILE *f);
void handle_file(const char *path);
void ingest(char **buffer);

char *base_pre;
char *base_post;

key_match_t *
find_next_key(char *buffer)
{
  key_match_t *match = calloc(1, sizeof(key_match_t));

  for (size_t i = 0; i < strlen(buffer) - 1; i++) {
    if (buffer[i] == '{' && buffer[i + 1] == '{')
      match->offset = i;

    if (i == strlen(buffer) - 1)
      return NULL;
  }

  char *subbuffer = buffer + match->offset;
  for (size_t i = 0; i < strlen(buffer) - 1; i++) {
    if (subbuffer[i] == '}' && subbuffer[i + 1] == '}')
      match->length = i + 2;

    if (i == strlen(buffer) - 1) {
      printf("Unterminated Key\n");
      return NULL;
    }
  }

  return match;
}

directive_t *
find_directive(char *content, key_match_t *match)
{
  directive_t *directive = (directive_t *) calloc(1, sizeof(directive_t));

  char *buffer = content + match->offset;
  unsigned int n = 0;

  for (size_t i = 0; i < match->length; i++)
    switch (buffer[i]) {
    case '{':
    case ' ':
    case '\t':
    case '\n':
      n++;
      break;

    default:
      goto found_start;
    }

  return NULL;

found_start:
  if (strncmp(buffer + n, "include", strlen("include")) == 0) {
    directive->type = INCLUDE;

    char *operand = NULL;
    for (size_t i = n + strlen("include");
         i < match->length - strlen("include");
         i++)
      if (isalnum(buffer[i])) {
        sscanf(buffer + i, "%ms\"", &operand);
        operand[strlen(operand) - 1] = '\0';
        break;
      }

    asprintf((char **) &directive->operands, "%s", operand);
    free(operand);
  }

  return directive;
}

void
ingest(char **buffer)
{
  key_match_t *match;

  while (true) {
    match = find_next_key(*buffer);
    if (match == NULL)
      break;

    directive_t *directive = find_directive(*buffer, match);
    if (directive == NULL)
      break;

    if (directive->type == INCLUDE) {
      char *operand = (char *) directive->operands;
      char *partial_path;
      asprintf(&partial_path, "%s/%s/%s", DIRECTORY, PARTIALS, operand);

      FILE *f = fopen(partial_path, "r");
      unsigned int size = fsize(f);
      char *partial_content = fcontent(f, size);

      asprintf(buffer,
               "%.*s%s%s\n",
               match->offset,
               *buffer,
               partial_content,
               *buffer + match->offset + match->length);
    }

    free(directive);
    free(match);
  }
}

unsigned int
fsize(FILE *f)
{
  unsigned int current = ftell(f);

  fseek(f, 0, SEEK_END);
  unsigned int s = ftell(f);
  fseek(f, current, SEEK_SET);

  return s + 1;
}

char *
fcontent(FILE *f, unsigned int size)
{
  char *buffer = (char *) calloc(size, sizeof(char));

  fseek(f, 0, SEEK_SET);
  int bytesread = fread(buffer, sizeof(char), size, f);
  if (bytesread < 0)
    return NULL;

  return buffer;
}

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

  fclose(in);
  fclose(out);

  free(inpath);
  free(outpath);

  free(buffer);
}

int
main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  FILE *base = fopen(DIRECTORY "/" BASE_TEMPLATE, "r");

  unsigned int size = fsize(base);
  char *contents = fcontent(base, size);

  key_match_t *match = find_next_key(contents);
  asprintf(&base_pre, "%.*s", match->offset, contents);
  asprintf(&base_post,
           "%.*s",
           size - match->offset - match->length,
           contents + match->offset + match->length);

  free(contents);
  fclose(base);

  mkdir(OUTPUT, 0700);

  char **x;
  char *filepath;

  for (x = (char **) html_resources; *x != NULL; x++) {
    asprintf(&filepath, "%s.html", *x);
    handle_file(filepath);
    free(filepath);
  }

  free(base_pre);
  free(base_post);
  return EXIT_SUCCESS;
}
