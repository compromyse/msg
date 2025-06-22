#ifndef __LEXER_H
#define __LEXER_H

typedef enum { INCLUDE, CONTENTFOR, ENDCONTENT } directive_e;

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
} contentfor_operands_t;

directive_t *find_directive(char *content, key_match_t *match);
key_match_t *find_next_key(char *buffer);

#endif
