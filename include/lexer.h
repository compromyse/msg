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

#ifndef __LEXER_H
#define __LEXER_H

#include <list.h>

#define DIRECTIVE_IS(key) strncmp(buffer + n, key, strlen(key)) == 0

typedef enum {
  _RAW,
  INCLUDE,
  CONTENT,
  CONTENTFOR,
  ENDCONTENT,
  BODY,
  EACHDO,
  ENDEACHDO,
  PUT,
  PUTPAGE
} directive_e;

typedef struct {
  unsigned int offset;
  unsigned int length;
} key_match_t;

typedef struct {
  directive_e type;
  void *operands;
} directive_t;

/* TODO: strlen(content) instead of length? */
typedef struct {
  char *key;
  char *content;
  size_t length;
} contentfor_operand_t;

typedef struct {
  char *source;
  char *key;
  char *content;
  size_t length;
} eachdo_operands_t;

list_t *lex(char *buffer);
directive_t *find_directive(char *content, key_match_t *match);
key_match_t *find_next_key(char *buffer, size_t skip);
char *find_contentfor_value(list_t *content_headers, char *key);

void directive_delete(directive_t *directive);

void lexer_handle_include(directive_t *directive,
                          key_match_t *match,
                          char *buffer,
                          size_t n);
void lexer_handle_contentfor(directive_t *directive,
                             key_match_t *match,
                             char *buffer,
                             char *content,
                             size_t n);
void lexer_handle_content(directive_t *directive,
                          key_match_t *match,
                          char *buffer,
                          size_t n);
void lexer_handle_eachdo(directive_t *directive,
                         key_match_t *match,
                         char *buffer,
                         size_t n);
void lexer_handle_putpage(directive_t *directive,
                          key_match_t *match,
                          char *buffer,
                          size_t n);

#endif
