#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "engine.h"
#include "string.h"
#include "template.h"

#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array)[0]))

char *
template_ingest_file(FILE *f)
{
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = calloc(1, fsize);
	fread(buffer, fsize, 1, f);

	char *output;

	TemplateMatches *matches = template_find_keys(buffer);
	for (size_t i = 0; i < matches->n_matches; i++) {
		TemplateMatch match = matches->matches[i];
		TemplateOperationResult *result
			= template_parse_key(strndup(buffer + match.offset, match.length));
		output = template_produce_output(result, match, buffer);
		/* TODO: Free TemplateMatches */
	}

	return output;
}

TemplateMatches *
template_find_keys(char *content)
{
	char *buffer = content;

	regex_t regex;
	regcomp(&regex, "{{[^}]*}}", 0);

	regmatch_t pmatch[1];
	regoff_t offset, length;

	TemplateMatch *matches = malloc(1);
	int n_matches = 0;

	int file_offset = 0;
	int ret;
	while (true) {
		ret = regexec(&regex, buffer, ARRAY_SIZE(pmatch), pmatch, 0);
		if (ret == REG_NOMATCH) {
			break;
		}

		offset = pmatch[0].rm_so;
		length = pmatch[0].rm_eo - pmatch[0].rm_so;

		n_matches++;
		realloc(matches, n_matches * sizeof(TemplateMatch));

		matches[n_matches - 1].length = length;
		matches[n_matches - 1].offset = file_offset + offset;

		file_offset = offset + length;
		buffer += file_offset;
	}
	regfree(&regex);

	if (n_matches == 0) {
		free(matches);
		return NULL;
	}

	TemplateMatches *out = calloc(1, sizeof(TemplateMatches));
	out->n_matches = n_matches;
	out->matches = matches;

	return out;
}

TemplateOperationResult *
template_parse_key(char *content)
{
	TemplateOperationResult *result = malloc(sizeof(TemplateOperationResult));

	char *operation;
	sscanf(content, "{{ %ms", &operation);

	if (strcmp(operation, "include") == 0)
		result->op = INCLUDE;
	else {
		printf("Unknown operation: %s in \"%s\"", operation, content);
		/* TODO: Clean exit. */
		exit(1);
	}
	free(operation);

	switch (result->op) {
	case INCLUDE: {
		char *operand;
		sscanf(content, "{{ include \"%ms }}", &operand);

		/* TODO: Cleaner way of extracting the operand */
		char *end = strchr(operand, '"');
		if (end)
			*end = '\0';
		TemplateIncludeOperands *op_operands
			= malloc(sizeof(TemplateIncludeOperands));
		op_operands->operand = operand;

		result->op_result = op_operands;
	} break;
	}

	return result;
}

char *
template_produce_output(TemplateOperationResult *result, TemplateMatch match,
						char *buffer)
{
	switch (result->op) {
	case INCLUDE: {
		TemplateIncludeOperands *include_operands
			= (TemplateIncludeOperands *) result->op_result;

		char *operand_content
			= engine_fetch_partial_content(include_operands->operand);
		/* printf("%s\n", operand_content); */

		char *output = calloc(strlen(buffer) - match.length
								  + strlen(operand_content) + 1,
							  sizeof(char));

		strncpy(output, buffer, match.offset);
		strcat(output, operand_content);
		strcat(output, buffer + match.offset + match.length);

		return output;
	} break;
	}
}
