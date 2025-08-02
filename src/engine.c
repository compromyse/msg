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

#include "util.h"
#define _GNU_SOURCE

#include <config.h>
#include <copy.h>
#include <engine.h>
#include <filehandler.h>
#include <lexer.h>
#include <list.h>
#include <msg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern msg_t *msg;

void
handle_include(char **buffer, key_match_t *match, directive_t *directive)
{
  char *operand = directive->operands;
  char *partial_path;
  asprintf(&partial_path, "%s/%s/%s", msg->base_directory, PARTIALS, operand);

  FILE *f = fopen(partial_path, "r");
  if (f == NULL) {
    printf("Could not open: %s\n", partial_path);
    return;
  }
  free(partial_path);

  unsigned int size = fsize(f);
  char *partial_content = fcontent(f, size);
  fclose(f);

  char *temp_buffer = strdup(*buffer);

  free(*buffer);
  asprintf(buffer,
           "%.*s%s%s\n",
           match->offset,
           temp_buffer,
           partial_content,
           temp_buffer + match->offset + match->length);

  free(partial_content);
  free(temp_buffer);
}

void
handle_contentfor(char **buffer,
                  key_match_t *match,
                  directive_t *directive,
                  list_t *content_headers)
{
  contentfor_operand_t *operand = directive->operands;
  contentfor_operand_t new_operand = { .content = strdup(operand->content),
                                       .key = strdup(operand->key),
                                       .length = operand->length };
  list_add(content_headers, &new_operand);

#ifdef DEBUG
  printf("CONTENTFOR: %s\n", operand->key);
  printf("CONTENT: %s\n", operand->content);
#endif

  char *temp_buffer = strdup(*buffer);

  free(*buffer);
  asprintf(buffer,
           "%.*s%s",
           match->offset,
           temp_buffer,
           temp_buffer + operand->length);

  free(temp_buffer);
}

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

    FILE *f = fopen(path, "r");
    free(path);
    size_t size = fsize(f);
    char *content = fcontent(f, size);
    fclose(f);

    config_t *config = NULL;

    char *p = strstr(content, "---");
    if (p != NULL) {
      *p = '\0';
      config = config_parse(content);
    }

    free(content);

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

engine_t *
engine_ingest(char **buffer)
{
  engine_t *engine = malloc(sizeof(engine_t));
  engine->config = NULL;

  char *p = strstr(*buffer, "---");
  if (p != NULL) {
    char *config;
    asprintf(&config, "%.*s\n", (int) (p - *buffer), *buffer);
    engine->config = config_parse(config);
    free(config);

    char *tempbuffer = strdup(p);

    free(*buffer);
    asprintf(buffer, "%s", tempbuffer + strlen("---"));

    free(tempbuffer);
  }

  key_match_t *match;
  engine->content_headers = list_create(sizeof(contentfor_operand_t));
  if (engine->content_headers == NULL) {
    printf("Could not create content_headers\n");
    return NULL;
  }

  size_t skip = 0;
  while (true) {
    match = find_next_key(*buffer, skip);
    if (match == NULL)
      break;

#ifdef DEBUG
    printf("Match: %.*s LENGTH(%d) OFFSET(%d)\n",
           match->length,
           *buffer + match->offset,
           match->length,
           match->offset);
#endif

    directive_t *directive = find_directive(*buffer, match);
    if (directive == NULL) {
      printf(
          "Unknown directive: %.*s\n", match->length, *buffer + match->offset);

      break;
    }

    switch (directive->type) {
    case INCLUDE:
      handle_include(buffer, match, directive);
      break;
    case CONTENTFOR:
      handle_contentfor(buffer, match, directive, engine->content_headers);
      break;
    case EACHDO:
      handle_eachdo(buffer, match, directive);
      break;

    case PUTPAGE:
      /* TODO: handle */
    case PUT:
    case ENDEACHDO:
    case BODY:
    case CONTENT:
    case ENDCONTENT:
    case _RAW:
      skip++;
      break;
    }

    directive_delete(directive);
    free(match);
  }

  return engine;
}

void
engine_delete(engine_t *engine)
{
  if (engine->config != NULL)
    config_delete(engine->config);

  for (size_t i = 0; i < engine->content_headers->size; i++) {
    contentfor_operand_t *operand = list_get(engine->content_headers, i);
    free(operand->content);
    free(operand->key);
  }
  list_delete(engine->content_headers);
  free(engine);
}
