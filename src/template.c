#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <engine.h>
#include <ops.h>
#include <template.h>

#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array)[0]))

char *
template_ingest(char *buffer)
{
	TemplateMatch *match;
	OpsOperation *operation;

	while (true) {
		match = template_find_next_key(buffer);
		if (match == NULL)
			break;

		operation = template_parse_key(
			strndup(buffer + match->offset, match->length));
		buffer = template_produce_output(operation, match, buffer);

		free(match);
		free(operation);
	}

	return buffer;
}

TemplateMatch *
template_find_next_key(char *content)
{
	char *buffer = content;

	regex_t regex;
	regcomp(&regex, "{{[^}]*}}", 0);

	regmatch_t pmatch[1];
	regoff_t offset, length;

	int file_offset = 0;
	int ret;
	ret = regexec(&regex, buffer, ARRAY_SIZE(pmatch), pmatch, 0);
	if (ret == REG_NOMATCH)
		return NULL;

	TemplateMatch *match = calloc(1, sizeof(TemplateMatch));
	offset = pmatch[0].rm_so;
	length = pmatch[0].rm_eo - pmatch[0].rm_so;

	match->length = length;
	match->offset = file_offset + offset;

	file_offset = offset + length;
	buffer += file_offset;
	regfree(&regex);

	return match;
}

OpsOperation *
template_parse_key(char *content)
{
	OpsOperation *operation = calloc(1, sizeof(OpsOperation));

	char *op;
	sscanf(content, "{{ %ms", &op);

	if (strcmp(op, "include") == 0)
		operation->op = INCLUDE;
	else {
		printf("Unknown operation: %s in \"%s\"", operation, content);
		/* TODO: Clean exit. */
		exit(1);
	}
	free(op);

	switch (operation->op) {
	case INCLUDE: {
		operation->op_result = ops_fetch_include_operands(content);
	} break;
	}

	return operation;
}

char *
template_produce_output(OpsOperation *result, TemplateMatch *match,
						char *buffer)
{
	switch (result->op) {
	case INCLUDE:
		return ops_handle_include(result, match, buffer);
	}
}
