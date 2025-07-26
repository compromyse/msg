#ifndef __ENGINE_H
#define __ENGINE_H

#include <lexer.h>
#include <list.h>

#define PARTIALS "partials"

list_t *ingest(char **buffer);
void handle_include(char **buffer, key_match_t *match, directive_t *directive);
void handle_contentfor(char **buffer,
                       key_match_t *match,
                       directive_t *directive,
                       list_t *content_headers);

#endif
