#define _GNU_SOURCE

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

int
fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  (void) sb;

  const char *basename = fpath + ftwbuf->base;

  /* printf("%s\n", basename); */
  if (typeflag == FTW_D) {
    if (!strcmp(basename, partials_directory)
        || !strcmp(basename, assets_directory))
      return FTW_SKIP_SUBTREE;

    return FTW_CONTINUE;
  }

  const char *path = fpath + strlen(directory) + 1;

  printf("%s\n", fpath);
  return FTW_CONTINUE;
}

int
main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  nftw(directory, fn, 64, FTW_PHYS | FTW_ACTIONRETVAL);

  return EXIT_SUCCESS;
}
