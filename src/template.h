#ifndef _TEMPLATE_H
#define _TEMPLATE_H

#include "stdio.h"

typedef struct {
	int offset;
	int length;
} TemplateResult;

char *template_ingest_file(FILE *f);
TemplateResult *template_find_keys(char *content);

#endif
