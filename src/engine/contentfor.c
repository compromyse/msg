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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
handle_contentfor(char **buffer,
                  key_match_t *match,
                  directive_t *directive,
                  list_t *content_headers)
{
  contentfor_operand_t *operand = directive->operands;
  contentfor_operand_t new_operand
      = { .content = strdup(operand->content), .key = strdup(operand->key) };
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
           temp_buffer + strlen(operand->content));

  free(temp_buffer);
}
