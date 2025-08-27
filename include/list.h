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

#ifndef __LIST_H
#define __LIST_H

#include <stddef.h>
#include <stdint.h>

#define START_SIZE 10
#define INCREMENT_BY 5

typedef struct {
    size_t element_size;
    size_t max;
    size_t size;
    uint8_t *elements;
} list_t;

list_t *list_create(size_t element_size);
void list_add(list_t *list, void *element);
void list_wrap_and_add(list_t *list, void *element);
void *list_get(list_t *list, size_t i);
void list_delete(list_t *list);
void *list_find_corresponding_value_from_ptr_wrapper(list_t *keys,
                                                     list_t *values,
                                                     char *key);

#endif
