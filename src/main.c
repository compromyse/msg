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

#include <ftw.h>
#include <limits.h>
#include <list.h>
#include <msg.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <util.h>

#define BUFFER_SIZE ((sizeof(struct inotify_event) + NAME_MAX + 1) * 4096)

msg_t *msg;
bool stop = false;
bool watch = false;
int inotify_fd;

list_t *directory_names;
list_t *wds;

void
add_directory_to_watch(const char *directory)
{
  int wd = inotify_add_watch(
      inotify_fd, directory, IN_MODIFY | IN_CREATE | IN_DELETE);
  if (wd < 0)
    perror("inotify_add_watch");

  list_add(wds, &wd);
  list_wrap_and_add(directory_names, strdup(directory));
}

int
watch_directory(const char *fpath,
                const struct stat *sb,
                int typeflag,
                struct FTW *ftwbuf)
{
  (void) sb;
  (void) ftwbuf;

  if (typeflag != FTW_D)
    goto exit;

  add_directory_to_watch(fpath);

exit:
  return FTW_CONTINUE;
}

void
signal_handler(int x)
{
  (void) x;
  stop = true;
}

void
usage(char *program)
{
  printf("Usage: %s [-h] [-w] [-v] [-o <output>] <directory>\n"
         "\t-h         : Help\n"
         "\t-w         : Watch working directory for changes\n"
         "\t-v         : Verbose\n"
         "\t-o <output>: Output directory\n"
         "\t<directory>: Working directory\n",
         program);
}

void
config(void)
{
  printf("Base Directory: %s\n"
         "Output Directory: %s\n"
         "Verbose: %s\n"
         "Watching: %s\n\n",
         msg->base_directory,
         msg->output_directory,
         msg->verbose ? "true" : "false",
         watch ? "true" : "false");
}

int
main(int argc, char **argv)
{
  printf("msg: The Minimal Static Site Generator\n\n");

  int opt;
  msg = malloc(sizeof(msg_t));
  msg->base_directory = ".";
  msg->output_directory = "dist";
  msg->verbose = false;

  while ((opt = getopt(argc, argv, "o:hvw")) != -1) {
    switch (opt) {
    case 'o':
      msg->output_directory = optarg;
      break;
    case 'w':
      watch = true;
      break;
    case 'v':
      msg->verbose = true;
      break;
    case 'h':
    default:
      usage(argv[0]);
      return EXIT_SUCCESS;
    }
  }

  if (optind < argc)
    msg->base_directory = argv[optind];

  config();

  int r = run();
  if (!watch || r != EXIT_SUCCESS) {
    free(msg);
    return r;
  }

  signal(SIGKILL, signal_handler);
  signal(SIGINT, signal_handler);

  inotify_fd = inotify_init1(IN_NONBLOCK);
  if (inotify_fd < 0) {
    perror("inotify");
    return EXIT_FAILURE;
  }

  wds = list_create(sizeof(int));
  directory_names = list_create(sizeof(ptr_wrapper_t));
  nftw(msg->base_directory, watch_directory, 64, FTW_PHYS | FTW_ACTIONRETVAL);

  char *buffer = malloc(BUFFER_SIZE);

  while (!stop) {
    size_t i = 0;
    size_t length = read(inotify_fd, buffer, BUFFER_SIZE);
    if (length == 0) {
      printf("Reading from inotify fd failed");
    }

    if (length < 0) {
      perror("read");
      return EXIT_FAILURE;
    }

    char *p;
    for (p = buffer; p < buffer + length;) {
      struct inotify_event *event = (struct inotify_event *) p;

      if (event->len) {
        if (event->mask & IN_MODIFY) {
          printf("\n\n");
          r = run();
        } else if (event->mask & IN_CREATE && event->mask & IN_ISDIR) {
          for (size_t i = 0; i < wds->size; i++) {
            int *wd = list_get(wds, i);

            if (*wd == event->wd) {
              char *directory_name;
              char *name = get_wrapped(list_get(directory_names, i));
              asprintf(&directory_name, "%s/%s", name, event->name);

              add_directory_to_watch(directory_name);

              free(directory_name);
              break;
            }
          }
        }
      }
      p += sizeof(struct inotify_event) + event->len;
    }
  }

  for (size_t i = 0; i < wds->size; i++) {
    char *name = get_wrapped(list_get(directory_names, i));
    free(name);
  }
  list_delete(wds);
  list_delete(directory_names);

  free(buffer);
  free(msg);
  return r;
}
