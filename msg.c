#define _GNU_SOURCE

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

unsigned int
fsize(FILE *f)
{
  unsigned int current = ftell(f);

  fseek(f, 0, SEEK_END);
  unsigned int s = ftell(f);
  fseek(f, current, SEEK_SET);

  return s;
}

void
copy_file(const char *path)
{
  char *inpath;
  char *outpath;
  asprintf(&inpath, "%s/%s", directory, path);
  asprintf(&outpath, "%s/%s", output_directory, path);

  FILE *in = fopen(inpath, "r");
  FILE *out = fopen(outpath, "w");

  unsigned int size = fsize(in);
  fseek(in, 0, SEEK_SET);

  char *buffer = (char *) calloc(size, sizeof(char));
  int bytesread = fread(buffer, sizeof(char), size, in);
  if (bytesread < 0)
    return;

  fwrite(buffer, size, 1, out);
  fclose(in);
  fclose(out);

  free(buffer);
}

int
handle(const char *fpath,
       const struct stat *sb,
       int typeflag,
       struct FTW *ftwbuf)
{
  (void) sb;

  const char *basename = fpath + ftwbuf->base;
  const char *path = fpath + strlen(directory) + 1;
  char *output_path = NULL;
  asprintf(&output_path, "%s/%s", output_directory, path);

  if (typeflag == FTW_D) {
    if (!strcmp(basename, partials_directory)
        || !strcmp(basename, assets_directory))
      return FTW_SKIP_SUBTREE;

    mkdir(output_path, 0700);

    return FTW_CONTINUE;
  }

  copy_file(path);

  return FTW_CONTINUE;
}

int
main(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  mkdir(output_directory, 0700);

  nftw(directory, handle, 64, FTW_PHYS | FTW_ACTIONRETVAL);

  return EXIT_SUCCESS;
}
