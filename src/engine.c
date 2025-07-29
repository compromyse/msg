#include "util.h"
#define _GNU_SOURCE

#include <config.h>
#include <copy.h>
#include <engine.h>
#include <filehandler.h>
#include <lexer.h>
#include <list.h>
#include <main.h>
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
  list_add(content_headers, operand);

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
  free(operand);
}

void
handle_eachdo(char **buffer, key_match_t *match, directive_t *directive)
{
  eachdo_operands_t *operands = directive->operands;
  char *original_content = operands->content;

  list_t *content_headers = engine_ingest(&operands->content);
  list_delete(content_headers);
  list_t *directives = lex(operands->content);

#ifdef DEBUG
  printf("KEY: %s\n", operands->key);
  printf("CONTENT: %s\n", operands->content);
#endif

  char *path;

  asprintf(&path, "%s/%s", msg->base_directory, operands->key);
  list_t *files = enumfilesindir(path);
  free(path);

  if (files == NULL) {
    printf("Could not find key %s\n", operands->key);
    free(operands);
    return;
  }

  list_t *atoms = list_create(sizeof(ptr_wrapper_t));
  size_t length = 1;

  for (size_t i = 0; i < files->size; i++) {
    ptr_wrapper_t *wrapper = list_get(files, i);
    asprintf(&path,
             "%s/%s/%s",
             msg->base_directory,
             operands->key,
             (char *) wrapper->ptr);

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
      directive_t *directive = list_get(directives, i);
      switch (directive->type) {
      case _RAW:
        list_add(atoms, wrap_ptr(strdup(directive->operands)));
        length += strlen(directive->operands);
        break;

      case PUT: {
        ptr_wrapper_t *wrapper
            = list_find_corresponding_value_from_ptr_wrapper(
                config->keys, config->values, (char *) directive->operands);

        if (wrapper != NULL) {
          list_add(atoms, wrap_ptr(strdup(wrapper->ptr)));
          length += strlen(wrapper->ptr);
        }

        break;
      }

      default:
        /* TODO: Handle this */
        break;
      }
    }

    config_delete(config);
    free(wrapper->ptr);
  }

  list_delete(files);

  char *content = calloc(length, sizeof(char));

  for (size_t i = 0; i < atoms->size; i++) {
    ptr_wrapper_t *wrapper = list_get(atoms, i);
    strcat(content, (char *) wrapper->ptr);
  }

  char *temp_buffer = strdup(*buffer);

  free(*buffer);
  asprintf(buffer,
           "%.*s%s%s\n",
           match->offset,
           temp_buffer,
           content,
           temp_buffer + operands->length);

  for (size_t i = 0; i < atoms->size; i++) {
    ptr_wrapper_t *wrapper = list_get(atoms, i);
    free(wrapper->ptr);
  }
  list_delete(atoms);
  list_delete(directives);
  free(content);
  free(temp_buffer);
  free(operands);
  free(original_content);
}

list_t *
engine_ingest(char **buffer)
{
  /* don't ingest the config if it is present */
  char *p = strstr(*buffer, "---");
  if (p != NULL) {
    strcpy(*buffer, p + strlen("---"));
  }

  key_match_t *match;
  list_t *content_headers = list_create(sizeof(contentfor_operand_t));
  if (content_headers == NULL) {
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
      handle_contentfor(buffer, match, directive, content_headers);
      break;
    case EACHDO:
      handle_eachdo(buffer, match, directive);
      break;

    case PUT:
    case ENDEACHDO:
    case BODY:
    case CONTENT:
    case ENDCONTENT:
    case _RAW:
      skip++;
      break;
    }

    if (directive != NULL)
      free(directive);

    if (match != NULL)
      free(match);
  }

  return content_headers;
}
