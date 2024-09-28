#include <stdlib.h>
#include <string.h>

#include <engine.h>
#include <ops.h>
#include <template.h>

OpsIncludeOperands *
ops_fetch_include_operands(char *content)
{
	char *operand;
	sscanf(content, "{{ include \"%ms }}", &operand);

	/* TODO: Cleaner way of extracting the operand */
	char *end = strchr(operand, '"');
	if (end)
		*end = '\0';

	OpsIncludeOperands *op_operands = calloc(1, sizeof(OpsIncludeOperands));
	op_operands->operand = operand;

	return op_operands;
}

char *
ops_handle_include(OpsOperation *operation, TemplateMatch *match, char *buffer)
{
	OpsIncludeOperands *include_operands
		= (OpsIncludeOperands *) operation->op_result;

	char *operand_content
		= engine_fetch_partial_content(include_operands->operand);

	char *output
		= calloc(strlen(buffer) - match->length + strlen(operand_content) + 1,
				 sizeof(char));

	strncpy(output, buffer, match->offset);
	strcat(output, operand_content);
	strcat(output, buffer + match->offset + match->length);

	return output;
}
