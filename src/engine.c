#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

#include <stdio.h>

typedef struct {
  char *wd; /* Working Directory */
  struct dirent **base_files;
  int n_base_files;
} Engine;

Engine *e;

void engine_init(void) {
  e = malloc(sizeof(Engine));

  // e->wd = getcwd(NULL, 0);
  e->wd = "site";
}

void engine_exit(void) {
  // free(e->wd);
  free(e);
}

static int filter(const struct dirent *entry) {
  char* dot = strrchr(entry->d_name, '.');
  if (dot && !strcmp(dot, ".html"))
    return 1;

  return 0;
}

void engine_get_all_base_files(void) {
  e->n_base_files = scandir(e->wd, &e->base_files, filter, alphasort);

#if 0
  for (int i = 0; i < e->n_base_files; i++) {
    printf("%s\n", e->base_files[i]->d_name);
  }
#endif
}

static void parse_base_file(const struct dirent *file) {
  printf("Parsing: %s\n", file->d_name);

  char full_path[sizeof(e->wd) + sizeof(file->d_name) + 2] = "";
  strcat(full_path, e->wd);
  strcat(full_path, "/");
  strcat(full_path, file->d_name);

  FILE *f = fopen(full_path, "r");
  if (f == NULL) {
    printf("Unable to open file: %s\n", file->d_name);
    engine_exit();
    exit(EXIT_FAILURE);
  }
}

void engine_parse_base_files(void) {
  for (int i = 0; i < e->n_base_files; i++)
    parse_base_file(e->base_files[i]);
}
