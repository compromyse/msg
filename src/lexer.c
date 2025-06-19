#define _GNU_SOURCE

#include <ctype.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

key_match_t *
find_next_key(char *buffer)
{
  key_match_t *match = calloc(1, sizeof(key_match_t));

  for (size_t i = 0; i < strlen(buffer) - 1; i++) {
    if (buffer[i] == '{' && buffer[i + 1] == '{')
      match->offset = i;

    if (i == strlen(buffer) - 1) {
      free(match);
      return NULL;
    }
  }

  char *subbuffer = buffer + match->offset;
  for (size_t i = 0; i < strlen(subbuffer) - 1; i++) {
    if (subbuffer[i] == '}' && subbuffer[i + 1] == '}')
      match->length = i + 2;

    if (i == strlen(buffer) - 1) {
      printf("Unterminated Key\n");
      free(match);
      return NULL;
    }
  }

  return match;
}

directive_t *
find_directive(char *content, key_match_t *match)
{
  directive_t *directive = (directive_t *) calloc(1, sizeof(directive_t));

  char *buffer = content + match->offset;
  unsigned int n = 0;

  for (size_t i = 0; i < match->length; i++)
    switch (buffer[i]) {
    case '{':
    case ' ':
    case '\t':
    case '\n':
      n++;
      break;

    default:
      goto found_start;
    }

  return NULL;

found_start:
  if (strncmp(buffer + n, "include", strlen("include")) == 0) {
    directive->type = INCLUDE;

    char *operand = NULL;
    for (size_t i = n + strlen("include");
         i < match->length - strlen("include");
         i++)
      if (isalnum(buffer[i])) {
        sscanf(buffer + i, "%ms\"", &operand);
        operand[strlen(operand) - 1] = '\0';
        break;
      }

    asprintf((char **) &directive->operands, "%s", operand);
    free(operand);
  }

  return directive;
}
