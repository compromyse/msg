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

#include <config.h>
#include <lexer.h>
#include <list.h>

/*
 * Simply just there to return content headers and config.
 *
 * content_headers: List of contentfor operands (CONTENTFOR directive operands)
 * config: Config of the ingested file
 */
typedef struct {
    list_t *content_headers;
    config_t *config;
} engine_t;

/*
 * An atom is a single iteration of an EACHDO.
 */
typedef struct {
    char *content;
    int priority;
} atom_t;

engine_t *engine_ingest(char **buffer);
void engine_delete(engine_t *engine);

void handle_include(char **buffer, key_match_t *match, directive_t *directive);
void handle_contentfor(char **buffer,
                       key_match_t *match,
                       directive_t *directive,
                       list_t *content_headers);

/* EACHDO */
void handle_eachdo(char **buffer, key_match_t *match, directive_t *directive);
void handle_page_source(list_t *atoms,
                        eachdo_operands_t *operands,
                        list_t *directives,
                        config_t *config);

#endif
