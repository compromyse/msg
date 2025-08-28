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

#ifndef __CONFIG_H
#define __CONFIG_H

#include <list.h>

#define DELIM "="
#define DELIM_ARRAY ","

/*
 * Each config has a list of "top-level keys", with corresponding values.
 * Simple string values are stored in the values array, with array-values being
 * stored in array_values. When a key has an array value, its corresponding
 * entry in values is NULL, and vice versa with simple string values.
 */
typedef struct {
    list_t *keys;
    list_t *values;
    list_t *array_values;
    list_t *nested_config_values;
} config_t;

config_t *config_parse(char *content);
void config_delete(config_t *config);
config_t *config_fetch_and_parse(char *path);

#endif
