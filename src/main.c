#define _GNU_SOURCE

#include <ctype.h>
#include <fcntl.h>
#include <filehandler.h>
#include <ftw.h>
#include <libgen.h>
#include <mkdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../config.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

typedef enum { INCLUDE } directive_e;

typedef struct {
  unsigned int offset;
  unsigned int length;
} key_match_t;

typedef struct {
  directive_e type;
  void *operands;
} directive_t;

directive_t *find_directive(char *content, key_match_t *match);
key_match_t *find_next_key(char *buffer);
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

    if (i == strlen(buffer) - 1) {
      free(match);
      return NULL;
    }
  }

  char *subbuffer = buffer + match->offset;
  for (size_t i = 0; i < strlen(subbuffer) - 1; i++) {
    if (subbuffer[i] == '}' && subbuffer[i + 1] == '}')
      match->length = i + 2;

    if (i == strlen(buffer) - 1) {
      printf("Unterminated Key\n");
      free(match);
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

      char *temp_buffer;
      asprintf(&temp_buffer, "%s", *buffer);

      free(*buffer);
      asprintf(buffer,
               "%.*s%s%s\n",
               match->offset,
               temp_buffer,
               partial_content,
               temp_buffer + match->offset + match->length);

      free(temp_buffer);
    }

    free(directive);
    free(match);
  }
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

  free(buffer);

  fclose(in);
  fclose(out);

  free(inpath);
  free(outpath);
}

int
copy_recursively(const char *fpath,
                 const struct stat *sb,
                 int typeflag,
                 struct FTW *ftwbuf)
{
  (void) sb;
  (void) ftwbuf;

  const char *path = fpath + strlen(DIRECTORY) + 1;
  char *output_path = NULL;
  asprintf(&output_path, "%s/%s", OUTPUT, path);

  if (typeflag == FTW_D) {
    mkdir(output_path, 0700);
    return FTW_CONTINUE;
  }

  if (typeflag != FTW_F)
    return FTW_CONTINUE;

  FILE *in = fopen(fpath, "r");
  size_t size = fsize(in);
  fclose(in);

  int in_fd = open(fpath, O_RDONLY);
  int out_fd = open(output_path, O_WRONLY | O_CREAT, 0700);

  sendfile(out_fd, in_fd, 0, size);

  close(in_fd);
  close(out_fd);

  return FTW_CONTINUE;
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
