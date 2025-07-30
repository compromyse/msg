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

#ifndef __TEMPLATE_H
#define __TEMPLATE_H

#include <list.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  list_t *components;
} template_t;

template_t *template_create(char *template_name);
void template_delete(template_t *template);
void template_write(template_t *template,
                    list_t *content_headers,
                    FILE *f,
                    void *doc,
                    bool is_markdown);

#endif
