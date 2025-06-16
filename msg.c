#define _GNU_SOURCE

#include <ctype.h>
#include <ftw.h>
#include <mkdio.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  regex_t regex;
  regcomp(&regex, "{{[^}]*}}", 0);

  regmatch_t pmatch[1];
  regoff_t offset, length;

  int file_offset = 0;
  int ret;
  ret = regexec(&regex, buffer, ARRAY_SIZE(pmatch), pmatch, 0);
  if (ret == REG_NOMATCH)
    return NULL;

  key_match_t *match = calloc(1, sizeof(key_match_t));
  offset = pmatch[0].rm_so;
  length = pmatch[0].rm_eo - pmatch[0].rm_so;

  match->length = length;
  match->offset = file_offset + offset;

  file_offset = offset + length;
  buffer += file_offset;
  regfree(&regex);

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

  FILE *in = fopen(inpath, "r");
  FILE *out = fopen(outpath, "w");

  unsigned int size = fsize(in);
  char *buffer = fcontent(in, size);

  if (dot && strcmp(dot, ".md") == 0) {
    MMIOT *doc = mkd_string(buffer, size, 0);
    fprintf(out, "%s", base_pre);
    markdown(doc, out, 0);
    fprintf(out, "%s", base_post);
  } else {
    ingest(&buffer);
    fprintf(out, "%s%s%s", base_pre, buffer, base_post);
  }

  fclose(in);
  fclose(out);

  free(buffer);
}

int
fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  (void) sb;

  const char *basename = fpath + ftwbuf->base;
  const char *path = fpath + strlen(DIRECTORY) + 1;
  char *output_path = NULL;
  asprintf(&output_path, "%s/%s", OUTPUT, path);

  if (typeflag == FTW_D) {
    if (strcmp(basename, PARTIALS) == 0 || strcmp(basename, ASSETS) == 0)
      return FTW_SKIP_SUBTREE;

    mkdir(output_path, 0700);

    return FTW_CONTINUE;
  }

  if (typeflag != FTW_F)
    return FTW_CONTINUE;

  if (ftwbuf->level == 1 && strcmp(basename, BASE_TEMPLATE) == 0)
    return FTW_CONTINUE;

  printf("handling: %s\n", path);
  handle_file(path);

  return FTW_CONTINUE;
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
  nftw(DIRECTORY, fn, 64, FTW_PHYS | FTW_ACTIONRETVAL);

  free(base_pre);
  free(base_post);
  return EXIT_SUCCESS;
}
