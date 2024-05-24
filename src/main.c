#include "parser.h"
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

char *file_read_text(const char *filename)
{
	FILE *file = fopen(filename, "rb");

	if (!file) {
		printf("Error: Could not open file %s.\n", filename);
		exit(EXIT_FAILURE);
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *buffer = malloc(length + 1);

	if (!buffer) {
		printf("Error: Could not allocate buffer for file %s.\n", filename);
		exit(EXIT_FAILURE);
	}

	fread(buffer, 1, length, file);
	buffer[length] = 0;
	fclose(file);
	return buffer;
}

void file_free_text(char *text)
{
	free(text);
}

void compile(const char *path)
{
	char *source = file_read_text(path);
	parse_program(source);
	file_free_text(source);
}

extern void print_file(void);
int main(int argc, char **argv)
{
	compile("./tests/01sample.ob0");
	print_file();
	printf("Done compiling\n");

	return 0;
}
