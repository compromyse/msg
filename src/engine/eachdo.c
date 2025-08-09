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

/*
 * A generic function that accepts keys, corresponding values and the
 * directives. It appends to the buffer the corresponding value of the key for
 * each PUT in the directives.
 *
 * NOTE: The keys and values must be wrapped.
 *
 * content: A pointer to the buffer that is appended to
 * directives: A list of the EACHDO directives (only _RAW and PUT)
 * keys, values: A hash of PUT keys and their corresponding values
 */
static void
write_eachdo_iteration(char **content,
                       list_t *directives,
                       list_t *keys,
                       list_t *values)
{
  for (size_t i = 0; i < directives->size; i++) {
    directive_t *_directive = list_get(directives, i);
    switch (_directive->type) {
    case _RAW: {
      *content = realloc(*content,
                         strlen(*content) + strlen(_directive->operands) + 1);
      strcat(*content, _directive->operands);
      break;
    }

    case PUT: {
      char *key = unwrap(list_find_corresponding_value_from_ptr_wrapper(
          keys, values, trim(_directive->operands)));

      if (key != NULL) {
        *content = realloc(*content, strlen(*content) + strlen(key) + 1);
        strcat(*content, key);
      }

      break;
    }

    default:
      /* TODO: Handle this */
      break;
    }
  }
}

/*
 * Handles EACHDOs on files. Fetches each file from a given directory
 * (operands->key) and parses each of its configs before handing off the
 * config->keys and config->values to write_eachdo_iteration.
 *
 * operands: Operands to the EACHDO call
 * directives: List of all directives (from lexing the EACHDO's content block)
 * content: A pointer to the buffer that is appended to
 */
static void
handle_file_source(eachdo_operands_t *operands,
                   list_t *directives,
                   char **content)
{
  char *path;
  asprintf(&path, "%s/%s", msg->base_directory, trim(operands->key));
  list_t *files = enumfilesindir(path);
  free(path);

  if (files == NULL) {
    printf("Could not find key %s\n", trim(operands->key));
    free(operands);
    return;
  }

  for (size_t i = 0; i < files->size; i++) {
    char *file_path = unwrap(list_get(files, i));
    asprintf(&path,
             "%s/%s/%s",
             msg->base_directory,
             trim(operands->key),
             file_path);

    int len = strlen(path);
    char *comparable = &path[len - strlen("index.html")];
    if (strcmp(comparable, "index.html") == 0) {
      free(path);
      free(file_path);
      continue;
    }

    config_t *config = config_fetch_and_parse(path);

    write_eachdo_iteration(content, directives, config->keys, config->values);

    config_delete(config);
    free(file_path);
    free(path);
  }
  list_delete(files);
}

/*
 * Handles EACHDO calls. Given a pointer to the buffer, it replaces the EACHDO
 * call along with its content block and ENDEACHDO call with the fetched
 * content.
 *
 * buffer: Pointer to the buffer that is modified
 * match: Pointer to the key match
 * directive: Pointer to the directive struct
 */
void
handle_eachdo(char **buffer, key_match_t *match, directive_t *directive)
{
  eachdo_operands_t *operands = directive->operands;

  engine_t *engine = engine_ingest(&operands->content);
  engine_delete(engine);
  list_t *directives = lex(operands->content);

  char *content = calloc(1, sizeof(char));

  if (!strcmp(operands->source, "resources"))
    handle_file_source(operands, directives, &content);
  else {
    printf("Unknown source: %s\n", operands->source);
    /* TODO: handle this gracefully */
    return;
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
  free(content);
  free(temp_buffer);
}
