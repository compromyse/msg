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
  PUT
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
  unsigned int length;
} contentfor_operand_t;

typedef struct {
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
void lexer_handle_put(directive_t *directive,
                      key_match_t *match,
                      char *buffer,
                      size_t n);

#endif
