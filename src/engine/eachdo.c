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

#include <engine.h>
#include <filehandler.h>
#include <msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

extern msg_t *msg;

void
handle_eachdo(char **buffer, key_match_t *match, directive_t *directive)
{
  eachdo_operands_t *operands = directive->operands;

  engine_t *engine = engine_ingest(&operands->content);
  engine_delete(engine);
  list_t *directives = lex(operands->content);

#ifdef DEBUG
  printf("KEY: %s\n", operands->key);
  printf("CONTENT: %s\n", operands->content);
#endif

  char *path;

  asprintf(&path, "%s/%s", msg->base_directory, trim(operands->key));
  list_t *files = enumfilesindir(path);
  free(path);

  if (files == NULL) {
    printf("Could not find key %s\n", trim(operands->key));
    free(operands);
    return;
  }

  list_t *atoms = list_create(sizeof(ptr_wrapper_t));
  size_t length = 1;

  for (size_t i = 0; i < files->size; i++) {
    ptr_wrapper_t *file_wrp = list_get(files, i);
    asprintf(&path,
             "%s/%s/%s",
             msg->base_directory,
             trim(operands->key),
             (char *) file_wrp->ptr);

    int len = strlen(path);
    char *comparable = &path[len - strlen("index.html")];
    if (strcmp(comparable, "index.html") == 0) {
      free(path);
      free(file_wrp->ptr);
      continue;
    }

    config_t *config = config_fetch_and_parse(path);

    for (size_t i = 0; i < directives->size; i++) {
      directive_t *_directive = list_get(directives, i);
      switch (_directive->type) {
      case _RAW: {
        list_wrap_and_add(atoms, strdup(_directive->operands));
        length += strlen(_directive->operands);
        break;
      }

      case PUT: {
        ptr_wrapper_t *key_wrp
            = list_find_corresponding_value_from_ptr_wrapper(
                config->keys, config->values, trim(_directive->operands));

        if (key_wrp != NULL) {
          list_wrap_and_add(atoms, strdup(key_wrp->ptr));
          length += strlen(key_wrp->ptr);
        }

        break;
      }

      default:
        /* TODO: Handle this */
        break;
      }
    }

    config_delete(config);
    free(file_wrp->ptr);
  }

  list_delete(files);

  char *content = calloc(length, sizeof(char));

  for (size_t i = 0; i < atoms->size; i++) {
    ptr_wrapper_t *wrapper = list_get(atoms, i);
    strcat(content, (char *) wrapper->ptr);
    free(wrapper->ptr);
  }

  char *temp_buffer = strdup(*buffer);

  free(*buffer);
  asprintf(buffer,
           "%.*s%s%s\n",
           match->offset,
           temp_buffer,
           content,
           temp_buffer + operands->length);

  for (size_t i = 0; i < directives->size; i++) {
    directive_t *_directive = list_get(directives, i);
    free(_directive->operands);
  }
  list_delete(directives);
  list_delete(atoms);
  free(content);
  free(temp_buffer);
}
