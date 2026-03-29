#ifndef FILE_HANDLER_H

#define FILE_HANDLER_H

#include <stdio.h>
#include <stdbool.h>
#include "format.h" 

bool check_extension(char *filename, char *extension);
Header read_header(FILE *file);
FILE * open_file(char *file_name);
uint8_t *read_file(FILE *input_file, uint16_t *prog_size);

#endif
