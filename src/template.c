#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "template.h"

#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array)[0]))

char *
template_ingest_file(FILE *f)
{
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(fsize + 1);
	fread(buffer, fsize, 1, f);

	template_find_keys(buffer);

	return buffer;
}

TemplateResult *
template_find_keys(char *content)
{
	char *buffer = content;

	regex_t regex;
	regcomp(&regex, "{{[^}]*}}", 0);

	regmatch_t pmatch[2];
	regoff_t offset, length;

	TemplateResult *results = malloc(1);
	int n_results = 0;

	int file_offset = 0;
	int ret;
	while (true) {
		ret = regexec(&regex, buffer, ARRAY_SIZE(pmatch), pmatch, 0);
		if (ret == REG_NOMATCH) {
			break;
		}

		offset = pmatch[0].rm_so;
		length = pmatch[0].rm_eo - pmatch[0].rm_so;

		n_results++;
		realloc(results, n_results * sizeof(TemplateResult));

		/* TODO: Pass substrings to result. */
		results[n_results - 1].length = length;
		results[n_results - 1].offset = file_offset + offset;

		file_offset = offset + length;
		buffer += file_offset;
	}
	regfree(&regex);

	if (n_results == 0) {
		free(results);
		return NULL;
	}

	/* for (int i = 0; i < n_results; i++)
		printf("%.*s\n", results[i].length, content + results[i].offset); */

	return results;
}
