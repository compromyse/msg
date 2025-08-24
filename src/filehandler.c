/*
 * msg
 * Copyright (C) 2025  Raghuram Subramani <raghus2247@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
  char *buffer = calloc(size, sizeof(char));

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
