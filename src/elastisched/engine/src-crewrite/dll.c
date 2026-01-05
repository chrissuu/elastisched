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

dll_node* dll_head(dll* deque) {
    if (!deque->head) return NULL;
    return deque->head;
}

dll_node* dll_tail(dll* deque) {
    if (!deque->tail) return NULL;
    return deque->tail;
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

dll_node* dll_popleft(dll* deque) {
    if (deque->size == 0) return NULL;

    dll_node* temp = deque->head;

    if (deque->size == 1) {
        deque->head = NULL;
        deque->tail = NULL;
    } else {
        deque->head = deque->head->next;
        deque->head->next->prev = NULL;
    }

    deque->size--;
    free(temp);
    return temp;
}

dll_node* dll_popright(dll* deque) {
    if (deque->size == 0) return NULL;

    dll_node* temp = deque->tail;

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
    return temp;
}

size_t dll_size(dll* deque) {
    return deque->size;
}

void dll_remove(dll* deque, dll_node* node) {
    if (node == deque->head) deque->head = deque->head->next;
    if (node == deque->tail) deque->tail = deque->tail->prev;

    if (node->prev) {
        node->prev->next = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }

    return;
}

dll_node* dll_next(dll_node* node) {
    return node->next;
}

dll_node* dll_prev(dll_node* node) {
    return node->prev;
}

void* dll_node_get_value(dll_node* node) {
    return node->value;
}