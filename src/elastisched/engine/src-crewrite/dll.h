#include <stdlib.h>

typedef struct dll dll;

dll* mk_dll();
void dll_free(dll* deque, void (*free_fn)(void* e));

void* dll_head(dll* deque);
void* dll_tail(dll* deque);
void dll_append(dll* deque, void* e);
void dll_prepend(dll* deque, void* e);
void* dll_popleft(dll* deque);
void* dll_popright(dll* deque);
size_t dll_size(dll* deque);
