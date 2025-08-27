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
#include <list.h>
#include <msg.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <util.h>

msg_t *msg;
bool stop = false;
bool regen = false;

void
signal_handler(int x)
{
    (void) x;
    stop = true;
}

void
usage(char *program)
{
    printf("Usage: %s [-h] [-r] [-v] [-o <output>] <directory>\n"
           "\t-h         : Help\n"
           "\t-v         : Verbose\n"
           "\t-r         : Regenerate every 500ms\n"
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
           "Regenerating: %s\n\n",
           msg->base_directory,
           msg->output_directory,
           msg->verbose ? "true" : "false",
           regen ? "true" : "false");
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

    while ((opt = getopt(argc, argv, "o:hvr")) != -1) {
        switch (opt) {
        case 'o':
            msg->output_directory = optarg;
            break;
        case 'r':
            regen = true;
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

    int r = run(true);
    if (!regen || r != EXIT_SUCCESS) {
        free(msg);
        return r;
    }

    signal(SIGINT, signal_handler);

    while (!stop) {
        printf(".");
        fflush(stdout);
        r = run(false);
        msleep(500);
    }

    free(msg);
    return r;
}
