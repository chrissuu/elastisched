#include <stdlib.h>

typedef struct dll_node dll_node;
typedef struct dll dll;

dll* mk_dll();
void dll_free(dll* deque, void (*free_fn)(void* e));

dll_node* dll_head(dll* deque);
dll_node* dll_tail(dll* deque);
void dll_append(dll* deque, void* e);
void dll_prepend(dll* deque, void* e);
dll_node* dll_popleft(dll* deque);
dll_node* dll_popright(dll* deque);

void dll_remove(dll* deque, dll_node* node);
dll_node* dll_next(dll_node* node);
dll_node* dll_prev(dll_node* node);
void* dll_node_get_value(dll_node* node);

size_t dll_size(dll* deque);
