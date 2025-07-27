#ifndef __FILEHANDLER_H
#define __FILEHANDLER_H

#include <list.h>
#include <stdio.h>

char *fcontent(FILE *f, unsigned int size);
unsigned int fsize(FILE *f);
list_t *enumfilesindir(char *path);

#endif
