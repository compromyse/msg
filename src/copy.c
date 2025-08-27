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

#include <copy.h>
#include <fcntl.h>
#include <filehandler.h>
#include <ftw.h>
#include <libgen.h>
#include <msg.h>
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
    asprintf(&output_path, "%s/%s", msg->output_directory, path);

    if (typeflag == FTW_D)
        goto exit;

    if (typeflag != FTW_F)
        goto exit;

    char *temppath = strdup(path);
    char *directory;
    asprintf(&directory, "%s/%s", msg->output_directory, dirname(temppath));
    char *next = calloc(strlen(directory) + 1, sizeof(char));
    strcpy(next, "");

    char *token;
    for (token = strtok(directory, "/"); token != NULL;
         token = strtok(NULL, "/")) {
        if (strcmp(next, "") != 0) {
            strcat(next, "/");
        }

        strcat(next, token);
        mkdir(next, 0700);
    }

    free(temppath);
    free(directory);
    free(next);

    FILE *in = fopen(fpath, "r");
    size_t size = fsize(in);
    fclose(in);

    int in_fd = open(fpath, O_RDONLY);
    int out_fd = open(output_path, O_WRONLY | O_CREAT, 0700);

    sendfile(out_fd, in_fd, 0, size);

    close(in_fd);
    close(out_fd);

exit:
    free(output_path);
    return FTW_CONTINUE;
}
