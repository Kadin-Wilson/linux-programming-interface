#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

typedef struct node {
    struct node *prev;
    struct node *next;
    size_t size;
} mem_node;

mem_node free_list_tail;

mem_node free_list = {
    .prev = NULL,
    .next = &free_list_tail,
    .size = 0,
};
mem_node free_list_tail = {
    .prev = &free_list,
    .next = NULL,
    .size = 0,
};

void *heap_end = NULL;

void place_node(mem_node *n);

void *check_free_list(size_t size);

void *allocate(size_t size) {
    mem_node *n = check_free_list(size);

    if (n == NULL) {
        n = sbrk(size + sizeof *n);

        if (n == (void*)-1) {
            return NULL;
        }

        heap_end = (void*)n + (size + sizeof *n);
    }

    n->prev = NULL;
    n->next = NULL;
    n->size = size + sizeof *n;

    return n + 1;
}

void deallocate(void *mem) {
    place_node((mem_node*)mem - 1);
}

void place_node(mem_node *n) {
    if (free_list.next == NULL) {
        free_list.next = n;
        free_list_tail.prev = n;
        n->prev = &free_list;
        n->next = &free_list_tail;
        return;
    }

    if ((char*)n + n->size < (char*)free_list.next) { // before head
        n->next = free_list.next;
        n->prev = &free_list;
        free_list.next->prev = n;
        free_list.next = n;
        return;
    }
    if ((char*)n > (char*)free_list_tail.prev + free_list_tail.prev->size) { // after tail
        free_list_tail.prev->next = n;
        n->prev = free_list_tail.prev;
        n->next = &free_list_tail;
        free_list_tail.prev = n;
        return;
    }
    if ((char*)n + n->size <= (char*)free_list.next + free_list.next->size) { // grow head
        n->size += free_list.next->size;
        n->next = free_list.next->next;
        n->prev = &free_list;
        free_list.next->next->prev = n;
        free_list.next = n;
        return;
    }
    if ((char*)n <= (char*)free_list_tail.prev + free_list_tail.prev->size) { // grow tail
        free_list_tail.prev->size += n->size;
        return;
    }

    for (mem_node *cn = free_list.next->next; cn != &free_list_tail; cn = cn->next) {
        if ((char*)n + n->size < (char*)cn) {
            n->next = cn;
            n->prev = cn->prev;
            cn->prev->next = n;
            cn->prev = n;
            return;
        }
        if ((char*)n + n->size <= (char*)cn + cn->size) {
            n->next = cn->next;
            n->prev = cn->prev;
            cn->next->prev = n;
            cn->prev->next = n;
            n->size += cn->size;
            return;
        }
        if ((char*)n <= (char*)cn + cn->size) {
            cn->size += n->size;
            return;
        }
    }

    assert(false); // should be unreachable
}

void *check_free_list(size_t size) {
    return NULL;
}

void print_free_list() {
    for (mem_node *p = &free_list; p != NULL; p = p->next) {
        printf(
            "\t(%p, %4lld, %p)\n", p, 
            (long long)p->size, 
            p + (long long)p->size
        );
    }
}

int main() {
    void *mem[10];

    for (int i = 0; i < 10; i++) {
        mem[i] = allocate(100 * (i + 1));
        if (mem[i] == NULL) {
            printf("allocation %d failed", i + 1);
            exit(EXIT_FAILURE);
        }
    }

    int order[10] = {3, 7, 9, 0, 2, 1, 5, 8, 4, 6};
    for (int i = 0; i < 10; i++) {
        printf(
            "deallocate (%p, %lu):\n", 
            (mem_node*)mem[order[i]] - 1, 
            (order[i] + 1) * 100 + sizeof(mem_node)
        );
        deallocate(mem[order[i]]);
        print_free_list();
    }

    return EXIT_SUCCESS;
}
