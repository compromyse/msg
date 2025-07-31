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

#include <limits.h>
#include <msg.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#define BUFFER_SIZE ((sizeof(struct inotify_event) + NAME_MAX + 1) * 4096)

msg_t *msg;
bool stop = false;

void
signal_handler(int x)
{
  (void) x;
  stop = true;
}

void
usage(char *program)
{
  printf("Usage: %s [-h] [-w] [-v] [-o <output>] <directory>\n", program);
  printf("\t-h         : Help\n");
  printf("\t-w         : Watch working directory for changes\n");
  printf("\t-v         : Verbose\n");
  printf("\t-o <output>: Output directory\n");
  printf("\t<directory>: Working directory\n");
}

void
config(void)
{
  printf("Base Directory: %s\n", msg->base_directory);
  printf("Output Directory: %s\n\n", msg->output_directory);
}

int
main(int argc, char **argv)
{
  printf("msg: The Minimal Static Site Generator\n\n");

  bool watch = false;

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
  if (!watch) {
    free(msg);
    return r;
  }

  signal(SIGKILL, signal_handler);
  signal(SIGINT, signal_handler);

  int fd = inotify_init1(IN_NONBLOCK);
  if (fd < 0) {
    perror("inotify");
    return EXIT_FAILURE;
  }

  int wd = inotify_add_watch(
      fd, msg->base_directory, IN_MODIFY | IN_CREATE | IN_DELETE);
  if (wd < 0) {
    perror("inotify_add_watch");
    return EXIT_FAILURE;
  }

  char *buffer = malloc(BUFFER_SIZE);

  while (!stop) {
    size_t i = 0;
    size_t length = read(fd, buffer, BUFFER_SIZE);
    if (length == 0) {
    }

    if (length < 0) {
      perror("read");
      return EXIT_FAILURE;
    }

    char *p;
    for (p = buffer; p < buffer + length;) {
      struct inotify_event *event = (struct inotify_event *) p;

      if (event->len) {
        switch (event->mask) {
        case IN_MODIFY:
          printf("\n\n");
          run();
          break;

        default:
          break;
        }
      }
      p += sizeof(struct inotify_event) + event->len;
    }
  }

  free(buffer);
  free(msg);
  return r;
}
