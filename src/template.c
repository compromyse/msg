#define _GNU_SOURCE

#include <copy.h>
#include <engine.h>
#include <filehandler.h>
#include <lexer.h>
#include <main.h>
#include <mkdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <template.h>

extern msg_t *msg;

template_t *
template_create(void)
{
  template_t *template = malloc(sizeof(template_t));

  char *path;
  asprintf(&path, "%s/%s", msg->base_directory, BASE_TEMPLATE);
  FILE *base = fopen(path, "r");
  free(path);

  unsigned int size = fsize(base);
  char *buffer = fcontent(base, size);
  fclose(base);

  list_t *content_headers = engine_ingest(&buffer);
  list_delete(content_headers);
  template->components = lex(buffer);

  free(buffer);
  return template;
}

void
template_delete(template_t *template)
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
  free(template);
}

void
template_write(template_t *template,
               list_t *content_headers,
               FILE *f,
               void *doc,
               bool is_markdown)
{
  char *output = malloc(1);
  strcpy(output, "");

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
            = find_contentfor_value(content_headers, match->operands);
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
