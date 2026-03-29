#include "format.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool check_extension(char *filename, char *extension) {
  char *dot = strrchr(filename, '.');
  if (!dot)
    return false;
  return strcmp(dot, extension) == 0;
}

Header read_header(FILE *file) {
  Header h;
  fread(h.magic, 1, MAGIC_SIZE, file);
  fread(&h.prog_size, 1, sizeof(uint16_t), file);

  if (memcmp(h.magic, MAGIC, MAGIC_SIZE) != 0) {
    fprintf(stderr, "Invalid format file.");
    exit(1);
  }
  return h;
}

FILE *open_file(char *file_name) {
  FILE *file = fopen(file_name, "rb");
  if (file == NULL) {
    fprintf(stderr, "Provided file doesn't exist.");
    exit(1);
  }
  if (check_extension(file_name, ".bin")) {
    return file;
  } else {
    fprintf(stderr, "Provided file doesn't have .bin extension.");
    exit(1);
  }
}

uint8_t *read_file(FILE *input_file, uint16_t *prog_size) {
  uint8_t *program = malloc(*prog_size);
  fread(program, 1, *prog_size, input_file);
  return program;
}
