#ifndef __LEXER_H
#define __LEXER_H

#include <list.h>

typedef enum {
  _RAW,
  INCLUDE,
  CONTENT,
  CONTENTFOR,
  ENDCONTENT,
  BODY
} directive_e;

typedef struct {
  unsigned int offset;
  unsigned int length;
} key_match_t;

typedef struct {
  directive_e type;
  void *operands;
} directive_t;

typedef struct {
  char *key;
  char *content;
  unsigned int length;
} contentfor_operand_t;

list_t *lex_file(char *buffer);
directive_t *find_directive(char *content, key_match_t *match);
key_match_t *find_next_key(char *buffer);

#endif
