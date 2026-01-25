#ifndef DYNAMIC_ARRAY_H

#define DYNAMIC_ARRAY_H


#include <stdlib.h>


typedef struct{
  void *data;
  size_t element_size;
  size_t length;
  size_t capacity;
}DynamicArray;

void init_array(DynamicArray *arr, size_t element_size, size_t initial_capacity);
void add_element(DynamicArray *arr, void* element);
void *get_element(DynamicArray *arr, size_t index);
void free_array(DynamicArray *arr);




#endif
