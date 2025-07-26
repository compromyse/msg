#ifndef __COPY_H
#define __COPY_H

#include <ftw.h>
#include <sys/stat.h>

#define OUTPUT "dist"
#define DIRECTORY "compromyse.xyz"

typedef struct FTW FTW;

int copy_recursively(const char *fpath,
                     const struct stat *sb,
                     int typeflag,
                     FTW *ftwbuf);

#endif
