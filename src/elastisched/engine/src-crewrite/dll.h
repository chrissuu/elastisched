#ifndef ELASTISCHED_DLL_H
#define ELASTISCHED_DLL_H

#include <stddef.h>

typedef struct dll_node dll_node;
typedef struct dll dll;

dll* mk_dll();
void dll_free(dll* deque, void (*free_fn)(void* e));

dll_node* dll_head(dll* deque);
dll_node* dll_tail(dll* deque);
void dll_append(dll* deque, void* e);
void dll_prepend(dll* deque, void* e);
void* dll_popleft(dll* deque);
void* dll_popright(dll* deque);

void dll_remove(dll* deque, dll_node* node);
dll_node* dll_next(dll_node* node);
dll_node* dll_prev(dll_node* node);
void* dll_node_get_value(dll_node* node);

size_t dll_size(dll* deque);

#endif
