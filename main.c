/*
Copyright (c) 2019 Felipe Ferreira da Silva

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use of
this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim
     that you wrote the original software. If you use this software in a
     product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define PATH_MAX_LENGTH 4096

struct pattern {
	uint8_t *bytes;
	uint64_t occurrences;
};

struct context {
	uint8_t print_help;
	uint8_t file_input_path[PATH_MAX_LENGTH];
	uint8_t file_output_path[PATH_MAX_LENGTH];
	FILE *file_input;
	FILE *file_output;
	uint8_t *pattern_prefix;
	uint32_t pattern_prefix_length;
	uint32_t pattern_prefix_spacing;
	uint8_t *pattern_sufix;
	uint32_t pattern_sufix_length;
	uint32_t pattern_sufix_spacing;
	uint16_t pattern_length;
	uint32_t patterns_count;
	struct pattern *patterns;
};

uint8_t bytes_has_newline(uint8_t *bytes, uint32_t size)
{
	uint8_t result;
	uint32_t i;
	result = 0;
	i = 0;
	while (i < size && result == 0) {
		if (bytes[i] == '\n') {
			result = 1;
		}
		i = i + 1;
	}
	return result;
}

static void add_pattern(struct context *context, uint8_t *pattern)
{
	uint32_t i;
	uint8_t pattern_found;
	pattern_found = 0;
	i = 0;
	while (i < context->patterns_count && pattern_found == 0) {
		if (memcmp(pattern, context->patterns[i].bytes, context->pattern_length) == 0) {
			context->patterns[i].occurrences = context->patterns[i].occurrences + 1;
			pattern_found = 1;
		}
		i = i + 1;
	}
	if (pattern_found == 0) {
		context->patterns_count = context->patterns_count + 1;
		if (context->patterns == NULL) {
			context->patterns = malloc(sizeof(struct pattern) * context->patterns_count);
		} else {
			context->patterns = realloc(context->patterns, sizeof(struct pattern) * context->patterns_count);
		}
		context->patterns[context->patterns_count - 1].bytes = malloc(context->pattern_length);
		memcpy(context->patterns[context->patterns_count - 1].bytes, pattern, context->pattern_length);
		context->patterns[context->patterns_count - 1].occurrences = 1;
	}
}

uint8_t count_patterns(struct context *context)
{
	uint8_t success;
	uint32_t position;
	uint32_t size;
	uint32_t read_length;
	uint8_t *bytes;
	uint32_t count;
	uint8_t end;
	uint8_t in_header;
	success = 1;
	read_length = context->pattern_prefix_length
		+ context->pattern_prefix_spacing
		+ context->pattern_length
		+ context->pattern_sufix_spacing
		+ context->pattern_sufix_length;
	context->file_input = fopen((char *)context->file_input_path, "r");
	if (context->file_input == NULL) {
		printf("Failed to open the input file \"%s\".\n", (char *)context->file_input_path);
		success = 0;
		goto done;
	}
	fseek(context->file_input, 0, SEEK_END);
	size = ftell(context->file_input);
	fseek(context->file_input, 0, SEEK_SET);
	bytes = malloc(read_length + 1);
	if (bytes == NULL) {
		success = 0;
		goto closefile;
	}
	end = 0;
	in_header = 0;
	count = 0;
	bytes[read_length] = 0;
	position = ftell(context->file_input);
	while (end == 0) {
		uint8_t byte;
		if (fread(&byte, 1, 1, context->file_input) == 1) {
			if (byte == '>') {
				in_header = 1;
			}
			if (in_header == 1) {
				if (byte == '\n') {
					in_header = 0;
				}
			} else {
				if (byte != '\n') {
					bytes[count] = tolower(byte);
					count = count + 1;
				}
			}
		} else {
			end = 1;
		}
		if (count == read_length && in_header == 0) {
			uint8_t valid;
			count = 0;
			valid = 1;
			if (context->pattern_prefix_length > 0) {
				if (memcmp(bytes, context->pattern_prefix, context->pattern_prefix_length) != 0) {
					valid = 0;
				}
			}
			if (context->pattern_sufix_length > 0) {
				if (memcmp(bytes + context->pattern_prefix_length + context->pattern_prefix_spacing + context->pattern_length + context->pattern_sufix_spacing, context->pattern_sufix, context->pattern_sufix_length) != 0) {
					valid = 0;
				}
			}
			if (valid == 1) {
				if (context->pattern_prefix_length > 0 && context->pattern_sufix_length > 0) {
					printf("\rCounting patterns of length %u with prefix \"%s\" and sufix \"%s\" (%06.2f %%) \"%s\".",
						context->pattern_length,
						context->pattern_prefix,
						context->pattern_sufix,
						100.0 * ((double)position / (double)size),
						bytes);
				} else if (context->pattern_prefix_length > 0) {
					printf("\rCounting patterns of length %u with prefix \"%s\" (%06.2f %%) \"%s\".",
						context->pattern_length,
						context->pattern_prefix,
						100.0 * ((double)position / (double)size),
						bytes);
				} else if (context->pattern_sufix_length > 0) {
					printf("\rCounting patterns of length %u with sufix \"%s\" (%06.2f %%) \"%s\".",
						context->pattern_length,
						context->pattern_sufix,
						100.0 * ((double)position / (double)size),
						bytes);
				} else {
					printf("\rCounting patterns of length %u (%06.2f %%) \"%s\".",
						context->pattern_length,
						100.0 * ((double)position / (double)size),
						bytes);
				}
				fflush(stdout);
				add_pattern(context, bytes + context->pattern_prefix_length + context->pattern_prefix_spacing);
			}
		}
		fseek(context->file_input, position + 1, SEEK_SET);
		position = ftell(context->file_input);
	}
	printf("\n");
	free(bytes);
closefile:
	fclose(context->file_input);
done:
	return success;
}

static uint8_t write_patterns(struct context *context)
{
	uint8_t success;
	uint32_t i;
	uint8_t *pattern;
	success = 1;
	context->file_output = fopen((char *)context->file_output_path, "w");
	if (context->file_output == NULL) {
		printf("Failed to open the output file \"%s\".\n", (char *)context->file_output_path);
		success = 0;
		goto done;
	}
	pattern = malloc(context->pattern_length + 1);
	pattern[context->pattern_length] = 0;
	i = 0;
	while (i < context->patterns_count) {
		memcpy(pattern, context->patterns[i].bytes, context->pattern_length);
		fprintf(context->file_output, "%s\t%lu\n", pattern, context->patterns[i].occurrences);
		i = i + 1;
	}
	free(pattern);
	fclose(context->file_output);
done:
	return success;
}

static void print_help(void)
{
	puts("SUPC (Sequence Unique Patterns Counter)");
	puts("Created by Felipe Ferreira da Silva.");
	puts("");
	puts("Usage:");
	puts("  supc [1] [2] [3] [4] [5]");
	puts("");
	puts(" Argument 1: path to input file.");
	puts(" Argument 2: path to output file.");
	puts(" Argument 3: pattern prefix.");
	puts(" Argument 4: prefix spacing. Must be \"0\" if there is no prefix.");
	puts(" Argument 5: pattern sufix.");
	puts(" Argument 6: sufix spacing. Must be \"0\" if there is no sufix.");
	puts(" Argument 7: pattern length (not counting the prefix and sufix).");
}

int main(int arguments_count, char **arguments)
{
	uint8_t success;
	int32_t status;
	struct context *context;
	success = 1;
	context = malloc(sizeof(struct context));
	if (context == NULL) {
		success = 0;
		goto done;
	}
	memset(context, 0, sizeof(struct context));
	/* Arguments */
	if (arguments_count == 8) {
		strncat((char *)context->file_input_path, arguments[1], PATH_MAX_LENGTH - 1);
		strncat((char *)context->file_output_path, arguments[2], PATH_MAX_LENGTH - 1);
		context->pattern_prefix_length = strlen(arguments[3]);
		context->pattern_prefix = malloc(context->pattern_prefix_length + 1);
		strncat((char *)context->pattern_prefix, arguments[3], context->pattern_prefix_length);
		context->pattern_prefix[context->pattern_prefix_length] = 0;
		context->pattern_prefix_spacing = atoi(arguments[4]);
		context->pattern_sufix_length = strlen(arguments[5]);
		context->pattern_sufix = malloc(context->pattern_sufix_length + 1);
		strncat((char *)context->pattern_sufix, arguments[5], context->pattern_sufix_length);
		context->pattern_sufix[context->pattern_sufix_length] = 0;
		context->pattern_sufix_spacing = atoi(arguments[6]);
		context->pattern_length = atoi(arguments[7]);
	} else {
		context->print_help = 1;
	}
	/* Process */
	if (context->print_help == 1) {
		print_help();
		goto freecontext;
	}
	printf("  Prefix: \"%s\"\n", context->pattern_prefix);
	printf("  Prefix length: %u\n", context->pattern_prefix_length);
	printf("  Prefix spacing: %u\n", context->pattern_prefix_spacing);
	printf("  Sufix: \"%s\"\n", context->pattern_sufix);
	printf("  Sufix length: %u\n", context->pattern_sufix_length);
	printf("  Sufix spacing: %u\n", context->pattern_sufix_spacing);
	printf("  Pattern length: %u\n", context->pattern_length);
	count_patterns(context);
	write_patterns(context);
freecontext:
	free(context);
done:
	if (success == 1) {
		status = EXIT_SUCCESS;
	} else {
		status = EXIT_FAILURE;
	}
	return status;
}
