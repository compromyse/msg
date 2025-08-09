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

extern msg_t *msg;

/*
 * Handles INCLUDE calls. Given a pointer to the buffer, it replaces the
 * INCLUDE call with the operand file's content.
 *
 * buffer: Pointer to the buffer that is modified
 * match: Pointer to the key match
 * directive: Pointer to the directive struct
 */
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
