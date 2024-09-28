#ifndef __OPS_H
#define __OPS_H

typedef enum { INCLUDE } OPERATION;

typedef struct {
	OPERATION op;
	void *op_result;
} OpsOperation;

typedef struct {
	char *operand;
} OpsIncludeOperands;

#include <template.h>

OpsIncludeOperands *ops_fetch_include_operands(char *content);

char *ops_handle_include(OpsOperation *operation, TemplateMatch *match,
						 char *buffer);

#endif
