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
#include <util.h>

extern msg_t *msg;

/*
 * First reads the config, if present. Then streams each directive and parses
 * it, modifying the buffer. At the end, all applicable directives in the
 * buffer are parsed and evaluated.
 *
 * ENDEACHDO, BODY, CONTENT, _RAW, etc. are not applicable,
 * for varying reasons. For example, ENDEACHDO isn't really meant to be parsed.
 * It's only there to define the end of the EACHDO content block.
 */
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
            printf("Unknown directive: %.*s\n",
                   match->length,
                   *buffer + match->offset);

            break;
        }

        switch (directive->type) {
        case INCLUDE:
            handle_include(buffer, match, directive);
            break;
        case CONTENTFOR:
            handle_contentfor(
                buffer, match, directive, engine->content_headers);
            break;
        case EACHDO: {
            eachdo_operands_t *operands = directive->operands;
            /* TODO: Don't handle page source only if a template is currently
             * being parsed */
            if (!strcmp(operands->source, "page")) {
                skip++;
                break;
            }

            handle_eachdo(buffer, match, directive);
            break;
        }

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

/*
 * Frees the provided engine_t's children before freeing the structure itself.
 */
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
