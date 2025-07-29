#define _GNU_SOURCE

#include <dirent.h>
#include <filehandler.h>
#include <list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

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

list_t *
enumfilesindir(char *path)
{
  DIR *d;
  struct dirent *dir;

  d = opendir(path);
  if (!d)
    return NULL;

  list_t *l = list_create(sizeof(ptr_wrapper_t *));

  while ((dir = readdir(d)) != NULL)
    if (dir->d_type == DT_REG)
      list_wrap_and_add(l, strdup(dir->d_name));

  closedir(d);
  return l;
}
