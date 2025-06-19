#ifndef __FILEHANDLER_H
#define __FILEHANDLER_H

#include <stdio.h>

typedef struct {

} filehandler_t;

char *fcontent(FILE *f, unsigned int size);
unsigned int fsize(FILE *f);

#endif
