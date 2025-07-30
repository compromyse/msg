/*
 * msg
 * Copyright (C) 2025  Raghuram Subramani <raghus2247@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <template.h>
#include <util.h>

extern msg_t *msg;
template_t *base_template;

void
handle_file(const char *path)
{
  char *inpath;
  char *outpath;

  asprintf(&inpath, "%s/%s", msg->base_directory, path);

  char *dot = strrchr(inpath, '.');
  if (dot && strcmp(dot, ".md") == 0) {
    asprintf(&outpath,
             "%s/%.*s.html",
             msg->output_directory,
             (int) strlen(path) - 3,
             path);
  } else {
    asprintf(&outpath, "%s/%s", msg->output_directory, path);
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
    list_t *content_headers = engine_ingest(&buffer);
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
run(void)
{
  struct stat sb;
  if (stat(msg->base_directory, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
    printf("%s does not exist.\n", msg->base_directory);
    return EXIT_FAILURE;
  }

  char *config_path;
  asprintf(&config_path, "%s/%s", msg->base_directory, CONFIG_FILE);
  config_t *config = config_fetch_and_parse(config_path);
  free(config_path);

  if (config == NULL)
    return EXIT_FAILURE;

  base_template = template_create(BASE_TEMPLATE);

  int err = mkdir(msg->output_directory, 0700);
  if (err != 0 && errno != EEXIST) {
    perror("mkdir");
    return EXIT_FAILURE;
  }

  list_t *static_ = get_wrapped(list_find_corresponding_value_from_ptr_wrapper(
      config->keys, config->array_values, "static"));

  if (static_ == NULL) {
    printf("Could not find resources in config.cfg\n");
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < static_->size; i++) {
    ptr_wrapper_t *value = list_get(static_, i);
    char *path = NULL;
    asprintf(&path, "%s/%s", msg->base_directory, (char *) value->ptr);

    struct stat path_stat;
    stat(path, &path_stat);

    /* TODO: Error handling */
    if (S_ISREG(path_stat.st_mode))
      copy_recursively(path, NULL, FTW_F, NULL);
    else if (S_ISDIR(path_stat.st_mode))
      nftw(path, copy_recursively, 64, FTW_PHYS | FTW_ACTIONRETVAL);

    free(path);
  }

  list_t *resources
      = get_wrapped(list_find_corresponding_value_from_ptr_wrapper(
          config->keys, config->array_values, "resources"));

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

  template_delete(base_template);
  config_delete(config);

  return EXIT_SUCCESS;
}
