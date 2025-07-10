#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

void
xstrcat(char *s1, char *s2)
{
  size_t a = strlen(s1);
  size_t b = strlen(s2);
  size_t size_ab = a + b + 1;

  s1 = realloc(s1, size_ab);

  memcpy(s1 + a, s2, b + 1);
}
