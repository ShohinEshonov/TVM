#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "dynamic_array.h"


void init_array(DynamicArray *arr, size_t element_size, size_t initial_capacity)
{
  arr->element_size = element_size;
  arr->capacity = initial_capacity;
  arr->length = 0;
  arr->data = malloc(arr->capacity * element_size);
  if(arr->data == NULL)
  {
    perror("Cannot allocate dynamic array");
    exit(1);
  }
}

static void resize_array(DynamicArray *arr)
{
  size_t new_capacity = arr->capacity * 2;
  void *new_data = realloc(arr->data, new_capacity*arr->element_size);
  if(new_data == NULL)
  {
    perror("Cannot resize dynamic array");
    free(arr->data);
    exit(1);
  }

  arr->capacity = new_capacity;
  arr->data = new_data;
  
}


void add_element(DynamicArray *arr, void* element)
{
  if(arr->length == arr->capacity)
  {
    resize_array(arr);
  }

  void *dst = (char *) arr->data + arr->length * arr->element_size;

  memcpy(dst, element, arr->element_size);
  arr->length++;
}

void *get_element(DynamicArray *arr, size_t index)
{
  if(index >= arr->length)
  {
    fprintf(stderr,"Index out of bounds");
    exit(1);
  }

  return (char*) arr->data + index * arr->element_size;
}



void free_array(DynamicArray *arr)
{
  free(arr->data);
  arr->data = NULL;
  arr->length = 0;
  arr->capacity = 0;
  arr->element_size = 0;
}






