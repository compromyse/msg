#include <filehandler.h>
#include <stdio.h>
#include <stdlib.h>

char *
fcontent(FILE *f, unsigned int size)
{
  char *buffer = (char *) calloc(size, sizeof(char));

  fseek(f, 0, SEEK_SET);
  int bytesread = fread(buffer, sizeof(char), size, f);
  if (bytesread < 0)
    return NULL;

  return buffer;
}

unsigned int
fsize(FILE *f)
{
  unsigned int current = ftell(f);

  fseek(f, 0, SEEK_END);
  unsigned int s = ftell(f);
  fseek(f, current, SEEK_SET);

  return s + 1;
}
