#include <ctype.h>
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

void
remove_spaces(char *str)
{
  int count = 0;

  for (int i = 0; str[i]; i++)
    if (!isspace(str[i]))
      str[count++] = str[i];
  str[count] = '\0';
}
