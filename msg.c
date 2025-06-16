#define _GNU_SOURCE

#include <ftw.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

typedef struct {
  unsigned int offset;
  unsigned int length;
} key_match_t;

char *fcontent(FILE *f, unsigned int size);
key_match_t *find_next_key(char *content);
unsigned int fsize(FILE *f);
void handle_file(const char *path);
void ingest(char *buffer);

key_match_t *
find_next_key(char *content)
{
  char *buffer = content;

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

void
ingest(char *buffer)
{
  key_match_t *match;

  while (true) {
    match = find_next_key(buffer);
    if (match == NULL)
      break;

    buffer += match->offset;
    printf("%.*s", match->length, buffer);

    free(match);

    exit(0);
  }
}

unsigned int
fsize(FILE *f)
{
  unsigned int current = ftell(f);

  fseek(f, 0, SEEK_END);
  unsigned int s = ftell(f);
  fseek(f, current, SEEK_SET);

  return s;
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
  asprintf(&inpath, "%s/%s", directory, path);
  asprintf(&outpath, "%s/%s", output_directory, path);

  FILE *in = fopen(inpath, "r");
  FILE *out = fopen(outpath, "w");

  unsigned int size = fsize(in);
  char *buffer = fcontent(in, size);

  ingest(buffer);

  fwrite(buffer, size, sizeof(char), out);
  fclose(in);
  fclose(out);

  free(buffer);
}

int
fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  (void) sb;

  const char *basename = fpath + ftwbuf->base;
  const char *path = fpath + strlen(directory) + 1;
  char *output_path = NULL;
  asprintf(&output_path, "%s/%s", output_directory, path);

  if (typeflag == FTW_D) {
    if (!strcmp(basename, partials_directory)
        || !strcmp(basename, assets_directory))
      return FTW_SKIP_SUBTREE;

    mkdir(output_path, 0700);

    return FTW_CONTINUE;
  }

  handle_file(path);

  return FTW_CONTINUE;
}

int
main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  mkdir(output_directory, 0700);
  nftw(directory, fn, 64, FTW_PHYS | FTW_ACTIONRETVAL);

  return EXIT_SUCCESS;
}
