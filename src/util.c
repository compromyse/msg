#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

char *
ltrim(char *s)
{
  while (isspace(*s))
    s++;
  return s;
}

char *
rtrim(char *s)
{
  char *back = s + strlen(s);
  while (isspace(*--back))
    ;
  *(back + 1) = '\0';
  return s;
}

char *
trim(char *s)
{
  return rtrim(ltrim(s));
}

ptr_wrapper_t *
wrap_ptr(void *ptr)
{
  ptr_wrapper_t *wrapper = malloc(sizeof(ptr_wrapper_t));
  wrapper->ptr = ptr;
  return wrapper;
}

void *
get_wrapped(ptr_wrapper_t *wrapper)
{
  return wrapper->ptr;
}
