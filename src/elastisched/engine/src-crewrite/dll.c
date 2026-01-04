#include "dll.h"

typedef struct dll_node dll_node;

struct dll_node {
    dll_node* prev;
    dll_node* next;
    void* value;
};

struct dll {
    dll_node* head;
    dll_node* tail;
    size_t size;
};

dll* mk_dll() {
    dll* deque = malloc(sizeof(dll));
    if (!deque) return NULL;

    deque->head = NULL;
    deque->tail = NULL;
    deque->size = 0;

    return deque;
}

void dll_free(dll* deque, void (*free_fn)(void *)) {
    if (!deque) return;
    dll_node* curr = deque->head;
    while (curr != NULL) {
        dll_node* temp = curr->next;
        if (free_fn) free_fn(curr->value);
        free(curr);
        curr = temp;
    }
    
    free(deque);
}

void* dll_head(dll* deque) {
    if (!deque->head) return NULL;
    return deque->head->value;
}

void* dll_tail(dll* deque) {
    if (!deque->tail) return NULL;
    return deque->tail->value;
}

void dll_append(dll* deque, void* e) {
    dll_node* new_tail = malloc(sizeof(dll_node));
    if (!new_tail) return;
    new_tail->value = e;
    if (!deque->head) {
        deque->head = new_tail;
        deque->tail = new_tail;
        new_tail->prev = NULL;
        new_tail->next = NULL;
    } else {
        new_tail->prev = deque->tail;
        new_tail->next = NULL;
        deque->tail = new_tail;
    }
    deque->size++;
    return;
}

void dll_prepend(dll* deque, void* e) {
    dll_node* new_head = malloc(sizeof(dll_node));
    if (!new_head) return;
    new_head->value = e;
    if (!deque->head) {
        deque->head = new_head;
        deque->tail = new_head;
        new_head->prev = NULL;
        new_head->next = NULL;
    } else {
        new_head->prev = NULL;
        new_head->next = deque->head;
        deque->head = new_head;
    }
    deque->size++;
    return;
}

void* dll_popleft(dll* deque) {
    if (deque->size == 0) return NULL;

    dll_node* temp = deque->head;
    void* value = temp->value;

    if (deque->size == 1) {
        deque->head = NULL;
        deque->tail = NULL;
    } else {
        deque->head = deque->head->next;
        deque->head->next->prev = NULL;
    }

    deque->size--;
    free(temp);
    return value;
}

void* dll_popright(dll* deque) {
    if (deque->size == 0) return NULL;

    dll_node* temp = deque->tail;
    void* value = temp->value;

    if (deque->size == 1) {
        deque->head = NULL;
        deque->tail = NULL;
    }
    else {
        deque->tail = deque->tail->prev;
        deque->tail->prev->next = NULL;
    }

    deque->size--;
    free(temp);
    return value;
}

size_t dll_size(dll* deque) {
    return deque->size;
}