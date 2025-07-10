#define _GNU_SOURCE

#include <filehandler.h>
#include <lexer.h>
#include <stdlib.h>
#include <template.h>

#include "../config.h"

template_t *
template_create(void)
{
  template_t *template = malloc(sizeof(template_t));

  FILE *base = fopen(DIRECTORY "/" BASE_TEMPLATE, "r");

  unsigned int size = fsize(base);
  char *buffer = fcontent(base, size);
  fclose(base);

  template->components = lex(buffer);

  return template;
}
