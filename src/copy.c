#define _GNU_SOURCE

#include <copy.h>
#include <fcntl.h>
#include <filehandler.h>
#include <ftw.h>
#include <main.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

extern msg_t *msg;

int
copy_recursively(const char *fpath,
                 const struct stat *sb,
                 int typeflag,
                 struct FTW *ftwbuf)
{
  (void) sb;
  (void) ftwbuf;

  const char *path = fpath + strlen(msg->base_directory) + 1;
  char *output_path = NULL;
  asprintf(&output_path, "%s/%s", OUTPUT, path);

  if (typeflag == FTW_D) {
    mkdir(output_path, 0700);
    return FTW_CONTINUE;
  }

  if (typeflag != FTW_F)
    return FTW_CONTINUE;

  FILE *in = fopen(fpath, "r");
  size_t size = fsize(in);
  fclose(in);

  int in_fd = open(fpath, O_RDONLY);
  int out_fd = open(output_path, O_WRONLY | O_CREAT, 0700);

  sendfile(out_fd, in_fd, 0, size);

  free(output_path);
  close(in_fd);
  close(out_fd);

  return FTW_CONTINUE;
}
