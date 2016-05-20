#include "vector_memory.h"

void* vm_element_get(vm_t* v)    
{
  int i;
  void* return_val = 0;

  if (v->num_allocated_elements == v->num_elements) return return_val;    

  for (i = 0; i < v->num_elements; i++)
  {
    if (!v->allocated_elements[i]) 
    {
      return_val = &(v->elements[v->element_size * i]);
      v->allocated_elements[i] = true;
      v->num_allocated_elements++;
      break;
    }
  }

  return return_val;
}


// Generic function to return a pointer of a memory element from the active vector
// to the inactive vector 
void vm_element_release(vm_t* v, void* element)
{
  int i;

  for (i = 0; i < v->num_elements; i++)
  {
    if ( (&(v->elements[v->element_size * i]) == element) && v->allocated_elements[i] ) 
    {
      // Mark element as not allocated
      v->allocated_elements[i] = false;
      v->num_allocated_elements--;
      break;
    }
  }
}

// Generic function to return a pointer of a memory element from the active vector
// to the inactive vector 
void vm_release_all(vm_t* v)
{
  int i;

  for (i = 0; i < v->num_elements; i++)
  {
    // Mark element as not allocated
    v->allocated_elements[i] = false;
  }
  v->num_allocated_elements = 0;
}


//Iterator
//Return element at index
//and update index to next element
void* vm_iterate(vm_t* v, size_t* index)
{
  void* ret_val = NULL;

  if (!index || !v) return NULL;

  // First Call to iterator
  if (*index == 0)
  {
    for ( ; *index < v->num_elements; (*index)++)
    {
      if (v->allocated_elements[*index]) break;
    }
  }

  // Return NULL if iterator reach end of list
  if (*index >= v->num_elements) return NULL;

  // Check that index is poiting to an allocated
  // element and set the return value
  if (v->allocated_elements[*index])
  {
    ret_val = &(v->elements[v->element_size * *index]);
  }

  // Update index to point to the next element
  // after returning the last element set 
  // index to num_elements
  for (++(*index) ; *index < v->num_elements; (*index)++)
  {
    if (v->allocated_elements[*index]) break;
  }

  return ret_val;
}
