#ifndef _vector_memory_h
#define _vector_memory_h

#include <stddef.h>
#include <stdbool.h>

typedef struct 
{ 
  char* elements;
  bool* allocated_elements;

  size_t num_elements;
  size_t element_size;
  size_t num_allocated_elements;    
} vm_t;

// Allocate a union of an array of char of size _NUM_ELEMENTS * _ELEMENT_SIZE
// the dummy int is needed to force the compiler to place the array on a 32 bit boundary
#define ALLOC_ARRAY(_NAME, _ELEMENT_SIZE, _NUM_ELEMENTS) \
            static union {char _c[(_ELEMENT_SIZE) * (_NUM_ELEMENTS)]; int _dummy_int; } _NAME;



#define VM_ALLOC(_NAME, _ELEMENT_SIZE, _NUM_ELEMENTS) \
               ALLOC_ARRAY(_vm_el_ ## _NAME, (_ELEMENT_SIZE), (_NUM_ELEMENTS)); \
               ALLOC_ARRAY(_vm_al_el_ ## _NAME, (sizeof(bool)), (_NUM_ELEMENTS)); \
               vm_t _NAME = {(char*)&_vm_el_ ## _NAME, \
                             (bool*)&_vm_al_el_ ## _NAME, \
                              _NUM_ELEMENTS, \
                              _ELEMENT_SIZE, \
                             0};


void* vm_element_get(vm_t* v);
void vm_element_release(vm_t* v, void* element);
void* vm_iterate(vm_t* v, size_t* index);
void vm_release_all(vm_t* v);

#endif

