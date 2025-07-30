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

#ifndef __ENGINE_H
#define __ENGINE_H

#include <lexer.h>
#include <list.h>

list_t *engine_ingest(char **buffer);
void handle_include(char **buffer, key_match_t *match, directive_t *directive);
void handle_contentfor(char **buffer,
                       key_match_t *match,
                       directive_t *directive,
                       list_t *content_headers);
void handle_eachdo(char **buffer, key_match_t *match, directive_t *directive);

#endif
