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

#include <config.h>
#include <filehandler.h>
#include <list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

static void
parse_nested_block(char **buffer, list_t *nested_config_values)
{
    (*buffer)++;
    char *raw_block = strsep(buffer, "]");

    config_t *config = config_parse(raw_block);
    list_wrap_and_add(nested_config_values, config);
}

static void
parse_array(char **buffer, list_t *array_values)
{
    (*buffer)++;
    list_t *l = list_create(sizeof(ptr_wrapper_t));
    char *raw_array = strsep(buffer, "}");

    char *value = strsep(&raw_array, DELIM_ARRAY);
    while (value != NULL) {
        list_wrap_and_add(l, strdup(trim(value)));
        value = strsep(&raw_array, DELIM_ARRAY);
    }

    list_wrap_and_add(array_values, l);
}

static void
parse_simple_string(char **buffer, list_t *values)
{
    char *value = trim(strsep(buffer, "\n"));
    list_wrap_and_add(values, strdup(value));
}

config_t *
config_parse(char *content)
{
    list_t *keys = list_create(sizeof(ptr_wrapper_t));
    list_t *values = list_create(sizeof(ptr_wrapper_t));
    list_t *array_values = list_create(sizeof(ptr_wrapper_t));
    list_t *nested_config_values = list_create(sizeof(ptr_wrapper_t));

    char *buffer = strdup(content);
    /* For free() */
    char *x = buffer;

    char *key = trim(strsep(&buffer, DELIM));

    while (buffer != NULL) {
        buffer = ltrim(buffer);
        list_wrap_and_add(keys, strdup(key));

        if (*buffer == '{') {
            parse_array(&buffer, array_values);

            list_wrap_and_add(values, NULL);
            list_wrap_and_add(nested_config_values, NULL);
        } else if (*buffer == '[') {
            parse_nested_block(&buffer, nested_config_values);

            list_wrap_and_add(values, NULL);
            list_wrap_and_add(array_values, NULL);
        } else {
            parse_simple_string(&buffer, values);

            list_wrap_and_add(array_values, NULL);
            list_wrap_and_add(nested_config_values, NULL);
        }

        key = trim(strsep(&buffer, DELIM));
    }

    free(x);

    config_t *config = malloc(sizeof(config_t));
    config->keys = keys;
    config->values = values;
    config->array_values = array_values;
    config->nested_config_values = nested_config_values;
    return config;
}

void
config_delete(config_t *config)
{
    for (size_t i = 0; i < config->keys->size; i++) {
        ptr_wrapper_t *wrapper;

        wrapper = list_get(config->keys, i);
        if (wrapper->ptr != NULL)
            free(wrapper->ptr);

        wrapper = list_get(config->values, i);
        if (wrapper->ptr != NULL)
            free(wrapper->ptr);

        wrapper = list_get(config->nested_config_values, i);
        if (wrapper->ptr != NULL)
            config_delete(wrapper->ptr);

        list_t *l = unwrap(list_get(config->array_values, i));
        if (l != NULL) {
            for (size_t y = 0; y < l->size; y++) {
                wrapper = list_get(l, y);
                free(wrapper->ptr);
            }
            list_delete(l);
        }
    }

    list_delete(config->keys);
    list_delete(config->values);
    list_delete(config->array_values);
    list_delete(config->nested_config_values);
    free(config);
}

config_t *
config_fetch_and_parse(char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        printf("Could not open %s\n", path);
        return NULL;
    }

    size_t s = fsize(f);
    char *content = fcontent(f, s);
    fclose(f);

    char *p = strstr(content, "---");
    if (p != NULL)
        content[p - content] = '\0';

    config_t *config = config_parse(content);
    free(content);

    return config;
}
