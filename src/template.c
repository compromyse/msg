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
#include <dirent.h>
#include <engine.h>
#include <filehandler.h>
#include <lexer.h>
#include <list.h>
#include <mkdio.h>
#include <msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <template.h>
#include <util.h>

extern msg_t *msg;

list_t *keys;
list_t *templates;

static void
delete_components(template_t *template)
{
  for (size_t i = 0; i < template->components->size; i++) {
    directive_t *directive = list_get(template->components, i);
    switch (directive->type) {
    case _RAW:
    case CONTENT:
      free(directive->operands);
      break;

    default:
      break;
    }
  }
  list_delete(template->components);
}

void
template_initialize(void)
{
  keys = list_create(sizeof(ptr_wrapper_t));
  templates = list_create(sizeof(template_t));

  char *template_directory;
  asprintf(&template_directory, "%s/%s", msg->base_directory, TEMPLATES);
  DIR *dir = opendir(template_directory);

  if (dir == NULL) {
    printf("Could not open %s\n", template_directory);
    return;
  }
  free(template_directory);

  struct dirent *f;
  while ((f = readdir(dir)) != NULL) {
    if (f->d_type != DT_REG)
      continue;

    template_t *t = template_create(f->d_name);
    list_wrap_and_add(keys, strdup(f->d_name));
    list_add(templates, t);

    free(t);
  }

  closedir(dir);
}

void
template_clean(void)
{
  for (size_t i = 0; i < keys->size; i++) {
    template_t *template = list_get(templates, i);
    delete_components(template);

    ptr_wrapper_t *wrapper = list_get(keys, i);
    free(wrapper->ptr);
  }

  list_delete(keys);
  list_delete(templates);
}

template_t *
template_create(char *template_name)
{
  template_t *template = malloc(sizeof(template_t));

  char *path;
  asprintf(&path, "%s/%s/%s", msg->base_directory, TEMPLATES, template_name);
  FILE *base = fopen(path, "r");
  free(path);

  unsigned int size = fsize(base);
  char *buffer = fcontent(base, size);
  fclose(base);

  engine_t *engine = engine_ingest(&buffer);
  engine_delete(engine);
  template->components = lex(buffer);

  free(buffer);
  return template;
}

void
template_delete(template_t *template)
{
  delete_components(template);
  free(template);
}

void
template_write(engine_t *engine, FILE *f, void *doc, bool is_markdown)
{
  template_t *template = template
      = list_find_corresponding_value_from_ptr_wrapper(
          keys, templates, "base.html");

  if (engine != NULL && engine->config != NULL) {
    char *template_name
        = get_wrapped(list_find_corresponding_value_from_ptr_wrapper(
            engine->config->keys, engine->config->values, "template"));

    if (template_name != NULL) {
      template = list_find_corresponding_value_from_ptr_wrapper(
          keys, templates, template_name);

      if (template == NULL) {
        printf("Could not find template %s\n", template_name);
        return;
      }
    }
  }

  for (size_t i = 0; i < template->components->size; i++) {
    directive_t *match = list_get(template->components, i);

    switch (match->type) {
    case _RAW:
      fprintf(f, "%s", (char *) match->operands);
      break;

    case CONTENT: {
      /* TODO: handle this gracefully */
      if (!is_markdown) {
        char *content
            = find_contentfor_value(engine->content_headers, match->operands);
        fprintf(f, "%s", content);
      }
      break;
    }

    case BODY: {
      if (is_markdown) {
        markdown(doc, f, 0);
      } else {
        fprintf(f, "%s", (char *) doc);
      }

      break;
    }

    /* TODO: Handle this gracefully */
    default:
      break;
    }
  }
}
