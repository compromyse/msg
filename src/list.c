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

#include <list.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

list_t *
list_create(size_t element_size)
{
    list_t *list = malloc(sizeof(list_t));
    if (list == NULL)
        return NULL;

    list->element_size = element_size;
    list->size = 0;
    list->max = START_SIZE;
    list->elements = (uint8_t *) calloc(list->max, list->element_size);

    return list;
}

void
list_add(list_t *list, void *element)
{
    if (list->size == list->max) {
        list->max += INCREMENT_BY;
        list->elements = (uint8_t *) realloc(list->elements,
                                             list->element_size * list->max);
        if (list->elements == NULL) {
            /* TODO: Handle error */
            printf("Failed to reallocate array\n");
            return;
        }
    }

    void *new_element = list->elements + list->element_size * list->size;
    new_element = memcpy(new_element, element, list->element_size);

    if (new_element == NULL)
        printf("Failed to add a new element\n");

    list->size++;
}

void
list_wrap_and_add(list_t *list, void *element)
{

    ptr_wrapper_t *wrapped = wrap_ptr(element);
    list_add(list, wrapped);
    free(wrapped);
}

void *
list_get(list_t *list, size_t i)
{
    return list->elements + (i * list->element_size);
}

void
list_delete(list_t *list)
{
    free(list->elements);
    free(list);
}

void *
list_find_corresponding_value_from_ptr_wrapper(list_t *keys,
                                               list_t *values,
                                               char *key)
{
    for (size_t i = 0; i < keys->size; i++) {
        ptr_wrapper_t *wrapper = list_get(keys, i);
        if (strcmp(wrapper->ptr, key) == 0) {
            return list_get(values, i);
        }
    }

    return NULL;
}
