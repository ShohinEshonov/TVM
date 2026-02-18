#ifndef FORMAT_H

#define FORMAT_H


#include <stdint.h>

#define MAGIC "TVM"
#define MAGIC_SIZE 3
#define HEADER_SIZE (MAGIC_SIZE + sizeof(uint16_t))


typedef struct{
  char magic[3];
  uint16_t prog_size;
}Header;


#endif
