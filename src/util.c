#include <ctype.h>
#include <string.h>
#include <util.h>

char *
trim(char *s)
{
  int i;

  while (isspace(*s))
    s++; // skip left side white spaces
  for (i = strlen(s) - 1; (isspace(s[i])); i--)
    ; // skip right side white spaces
  s[i + 1] = '\0';

  return s;
}
