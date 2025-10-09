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
    // place node
    mem_node *cn = free_list.next;
    while (cn != &free_list_tail && n > cn) {
        cn = cn->next;
    }
    n->next = cn;
    n->prev = cn->prev;
    cn->prev->next = n;
    cn->prev = n;

    // merge right
    while (n->next != &free_list_tail && (char*)n + n->size >= (char*)n->next) {
        n->next->next->prev = n;
        n->size += n->next->size;
        n->next = n->next->next;
    }

    // merge left
    while (n->prev != &free_list && (char*)n <= (char*)n->prev + n->prev->size) {
        n->prev->size += n->size;
        n->prev->next = n->next;
        n->next->prev = n->prev;
        n = n->prev;
    }
}

void *check_free_list(size_t size) {
    return NULL;
}

int sprint_free_list(char *s) {
    int c = 0;
    for (mem_node *p = &free_list; p != NULL; p = p->next) {
        c += sprintf(
            s + c,
            "\t(%p, %4lld, %p)\n",
            p, 
            (long long)p->size, 
            (char*)p + p->size
        );
    }
    return c;
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

    static char out[10 * 13 * 200];
    char *s = out;

    int order[10] = {3, 7, 9, 0, 2, 1, 5, 8, 4, 6};
    for (int i = 0; i < 10; i++) {
        s += sprintf(
            s,
            "deallocate (%p, %lu):\n", 
            (mem_node*)mem[order[i]] - 1, 
            (order[i] + 1) * 100 + sizeof(mem_node)
        );
        deallocate(mem[order[i]]);
        s += sprint_free_list(s);
    }

    printf("%s", out);

    return EXIT_SUCCESS;
}
