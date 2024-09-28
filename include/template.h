#ifndef _TEMPLATE_H
#define _TEMPLATE_H

#define MAXIMUM_OPERATION_NAME_SIZE 16

typedef struct {
	int offset;
	int length;
} TemplateMatch;

#include <ops.h>

char *template_ingest(char *buffer);

TemplateMatch *template_find_next_key(char *content);
OpsOperation *template_parse_key(char *content);

char *template_produce_output(OpsOperation *operation, TemplateMatch *match,
							  char *buffer);

#endif
