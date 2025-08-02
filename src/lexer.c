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

#include <ctype.h>
#include <lexer.h>
#include <list.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

list_t *
lex(char *buffer)
{
  list_t *directives = list_create(sizeof(directive_t));
  size_t current_offset = 0;

  while (true) {
    key_match_t *match = find_next_key(buffer, 0);
    if (match == NULL)
      break;

    directive_t *directive = find_directive(buffer, match);
    /* TODO: Handle unknown directive */
    if (directive == NULL)
      break;

    current_offset += match->length + match->offset;

    if (current_offset != 0) {
      char *raw_content = strndup(buffer, match->offset);

      directive_t *raw_directive = malloc(sizeof(directive_t));

      raw_directive->type = _RAW;
      raw_directive->operands = raw_content;
      list_add(directives, raw_directive);

      free(raw_directive);
    }

    buffer += match->offset + match->length;

    list_add(directives, directive);

    free(directive);
    free(match);
  }

  if (strlen(buffer) > 0) {
    char *raw_content = strdup(buffer);

    directive_t *raw_directive = malloc(sizeof(directive_t));

    raw_directive->type = _RAW;
    raw_directive->operands = raw_content;
    list_add(directives, raw_directive);

    free(raw_directive);
  }

  return directives;
}

key_match_t *
find_next_key(char *buffer, size_t skip)
{
  key_match_t *match = calloc(1, sizeof(key_match_t));
  size_t count = 0;

  for (size_t i = 0; i < strlen(buffer); i++) {
    if (buffer[i] == '{' && buffer[i + 1] == '{') {
      count++;

      if (count > skip) {
        match->offset = i;
        break;
      }
    }

    if (i == strlen(buffer) - 1) {
      free(match);
      return NULL;
    }
  }

  char *subbuffer = buffer + match->offset;
  for (size_t i = 0; i < strlen(subbuffer); i++) {
    if (subbuffer[i] == '}' && subbuffer[i + 1] == '}') {
      match->length = i + 2;
      break;
    }

    if (i == strlen(buffer) - 1) {
      printf("Unterminated Key\n");
      free(match);
      return NULL;
    }
  }

  return match;
}

void
lexer_handle_include(directive_t *directive,
                     key_match_t *match,
                     char *buffer,
                     size_t n)
{
  directive->type = INCLUDE;

  char *operand = NULL;
  for (size_t i = n + strlen("include"); i < match->length - n; i++)
    if (isalnum(buffer[i])) {
      sscanf(buffer + i, "%ms\"", &operand);
      operand[strlen(operand) - 1] = '\0';
      break;
    }

  directive->operands = operand;
}

void
lexer_handle_contentfor(directive_t *directive,
                        key_match_t *match,
                        char *buffer,
                        char *content,
                        size_t n)
{
  directive->type = CONTENTFOR;
  directive->operands = NULL;
  contentfor_operand_t *operands = malloc(sizeof(contentfor_operand_t));

  for (size_t i = n + strlen("contentfor"); i < match->length; i++)
    if (isalnum(buffer[i])) {
      sscanf(buffer + i, "%ms\"", &operands->key);
      operands->key[strlen(operands->key) - 1] = '\0';
      break;
    }

  buffer = content + match->length + match->offset;

  key_match_t *new_match;

  while (true) {
    new_match = find_next_key(buffer, 0);
    if (new_match == NULL) {
      printf("Cannot find endcontent\n");
      free(new_match);
      directive_delete(directive);
      /* TODO: Handle early returns */
      return;
    }

    directive_t *new_directive = find_directive(buffer, new_match);
    if (new_directive == NULL) {
      printf("Cannot find directive: %.*s\n",
             new_match->length,
             buffer + new_match->offset);
      directive_delete(new_directive);
      free(new_match);
      directive_delete(directive);
      return;
    }

    if (new_directive->type == ENDCONTENT) {
      directive_delete(new_directive);
      break;
    }

    directive_delete(new_directive);
    free(new_match);
  }

  operands->content = strndup(buffer, new_match->offset);
  operands->length
      = match->offset + match->length + new_match->offset + new_match->length;
  free(new_match);

  directive->operands = operands;
}

void
lexer_handle_eachdo(directive_t *directive,
                    key_match_t *match,
                    char *buffer,
                    size_t n)
{
  directive->type = EACHDO;
  eachdo_operands_t *operands = malloc(sizeof(eachdo_operands_t));

  operands->key = strndup(buffer + n + strlen("eachdo"),
                          match->length - n - strlen("eachdo") - 2);

  buffer += match->length;
  key_match_t *new_match;
  size_t skip = 0;

  while (true) {
    new_match = find_next_key(buffer, skip);
    if (new_match == NULL) {
      printf("Cannot find endeachdo\n");
      free(new_match);
      directive_delete(directive);
      /* TODO: Handle early returns */
      return;
    }

    directive_t *new_directive = find_directive(buffer, new_match);
    if (new_directive == NULL) {
      printf("Cannot find directive: %.*s\n",
             new_match->length,
             buffer + new_match->offset);
      free(new_match);
      directive_delete(directive);
      return;
    }

    if (new_directive->type == ENDEACHDO) {
      directive_delete(new_directive);
      break;
    }

    directive_delete(new_directive);
    free(new_match);

    skip++;
  }

  operands->content = strndup(buffer, new_match->offset);
  operands->length
      = match->offset + match->length + new_match->offset + new_match->length;
  free(new_match);

  directive->operands = operands;
}

void
lexer_handle_put(directive_t *directive,
                 key_match_t *match,
                 char *buffer,
                 size_t n)
{
  directive->type = PUT;

  /* TODO: Use this for include and contentfor too instead of sscanf() */
  directive->operands = strndup(buffer + n + strlen("put"),
                                match->length - n - strlen("put") - 2);
}

void
lexer_handle_putpage(directive_t *directive,
                     key_match_t *match,
                     char *buffer,
                     size_t n)
{
  directive->type = PUTPAGE;
  directive->operands = strndup(buffer + n + strlen("putpage"),
                                match->length - n - strlen("putpage") - 2);
}

void
lexer_handle_content(directive_t *directive,
                     key_match_t *match,
                     char *buffer,
                     size_t n)
{
  directive->type = CONTENT;

  char *operand = NULL;
  for (size_t i = n + strlen("content"); i < match->length - n; i++)
    if (isalnum(buffer[i])) {
      sscanf(buffer + i, "%ms\"", &operand);
      operand[strlen(operand) - 1] = '\0';
      break;
    }

  directive->operands = operand;
}

directive_t *
find_directive(char *content, key_match_t *match)
{
  directive_t *directive;

  char *buffer = content + match->offset;
  size_t n = 0;

  for (size_t i = 0; i < match->length; i++)
    if (isspace(buffer[i]) || buffer[i] == '{')
      n++;
    else
      goto found_start;

  return NULL;

found_start:
  directive = (directive_t *) calloc(1, sizeof(directive_t));

  if (DIRECTIVE_IS("endcontent")) {
    directive->type = ENDCONTENT;
    directive->operands = NULL;
  } else if (DIRECTIVE_IS("endeachdo")) {
    directive->type = ENDEACHDO;
    directive->operands = NULL;
  } else if (DIRECTIVE_IS("body")) {
    directive->type = BODY;
    directive->operands = NULL;
  } else if (DIRECTIVE_IS("include")) {
    lexer_handle_include(directive, match, buffer, n);
  } else if (DIRECTIVE_IS("contentfor")) {
    lexer_handle_contentfor(directive, match, buffer, content, n);
  } else if (DIRECTIVE_IS("content")) {
    lexer_handle_content(directive, match, buffer, n);
  } else if (DIRECTIVE_IS("eachdo")) {
    lexer_handle_eachdo(directive, match, buffer, n);
  } else if (DIRECTIVE_IS("putpage")) {
    lexer_handle_putpage(directive, match, buffer, n);
  } else if (DIRECTIVE_IS("put")) {
    lexer_handle_put(directive, match, buffer, n);
  } else {
    directive_delete(directive);
    directive = NULL;
  }

  return directive;
}

char *
find_contentfor_value(list_t *content_headers, char *key)
{
  for (size_t i = 0; i < content_headers->size; i++) {
    contentfor_operand_t *operand = list_get(content_headers, i);

    if (strcmp(key, operand->key) == 0)
      return operand->content;
  }

  return NULL;
}

void
directive_delete(directive_t *directive)
{
  switch (directive->type) {
  case EACHDO: {
    eachdo_operands_t *operands = directive->operands;
    free(operands->content);
    free(operands->key);
    free(operands);
    break;
  }
  case CONTENTFOR: {
    contentfor_operand_t *operands = directive->operands;
    free(operands->content);
    free(operands->key);
    free(operands);
    break;
  }
  case PUT:
  case CONTENT:
  case INCLUDE:
    if (directive->operands != NULL)
      free(directive->operands);
    break;
  default:
    break;
  }

  free(directive);
}
