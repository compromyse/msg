#ifndef _TEMPLATE_H
#define _TEMPLATE_H

#include "stdio.h"

#define MAXIMUM_OPERATION_NAME_SIZE 16

typedef enum { INCLUDE } OPERATION;

typedef struct {
	int offset;
	int length;
} TemplateMatch;

typedef struct {
	TemplateMatch *matches;
	size_t n_matches;
} TemplateMatches;

typedef struct {
	char *operand;
} TemplateIncludeOperands;

typedef struct {
	OPERATION op;
	void *op_result;
} TemplateOperationResult;

char *template_ingest_file(FILE *f);

TemplateMatches *template_find_keys(char *content);
TemplateOperationResult *template_parse_key(char *content);

char *template_produce_output(TemplateOperationResult *result,
							  TemplateMatch match, char *buffer);

#endif
