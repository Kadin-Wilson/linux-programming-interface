#define _XOPEN_SOURCE 500
#include <ftw.h>

#include <stdio.h>
#include <stdlib.h>

#define INITIAL_LIST_SIZE 100

typedef struct {
    size_t sz;
    size_t ct;
} size_count;

typedef struct {
    size_t len;
    size_t cap;
    size_count d[];
} file_list;

file_list *new_list() {
    file_list *l = malloc(INITIAL_LIST_SIZE * sizeof(size_count) + sizeof *l);
    l->len = 0;
    l->cap = INITIAL_LIST_SIZE;
    return l;
}

file_list *grow_list(file_list *l) {
    l = realloc(l, l->cap * 2 * sizeof(size_count) + sizeof *l);
    l->cap *= 2;
    return l;
}

file_list *insert_size(file_list *l, size_t sz) {
    if (l->len == l->cap)
        l = grow_list(l);

    if (l->len == 0) {
        l->d[0].sz = sz;
        l->d[0].ct = 1;
        l->len++;
        return l;
    }

    size_t b = 0;
    size_t e = l->len;
    while (b < e) {
        size_t m = b + (e - b) / 2;
        if (l->d[m].sz >= sz)
            e = m;
        else
            b = m + 1;
    }

    if (b == l->len) {
        l->d[b].sz = sz;
        l->d[b].ct = 1;
        l->len++;
        return l;
    }

    if (l->d[b].sz == sz) {
        l->d[b].ct++;
        return l;
    }

    for (size_t i = l->len - 1; i > b; i--) {
        l->d[i + 1] = l->d[i];
    }
    l->d[b + 1] = l->d[b];
    l->d[b].sz = sz;
    l->d[b].ct = 1;
    l->len++;

    return l;
}

file_list *list;

int get_file_size(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_NS) {
        return 0;
    }
    list = insert_size(list, sb->st_blocks);
    return 0;
}

const char *size_unit(size_t size) {
    static char s[100];
    static const char *suffix[4] = { "KB", "MB", "GB", "TB" };

    double sz = size >> 1;

    int i = 0;
    while (sz > 1000 & i < 3) {
        i++;
        sz /= 1000;
    }

    sprintf(s, "%7.4lg%s", sz, suffix[i]);

    return s;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fputs("must supply exactly one directory path\n", stderr);
        return EXIT_FAILURE;
    }

    list = new_list();

    if (nftw(argv[1], get_file_size, 10, FTW_PHYS) == -1) {
        free(list);
        fputs("failed to walk file tree\n", stderr);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < list->len; i++) {
        printf("%s %zu\n", size_unit(list->d[i].sz), list->d[i].ct);
    }

    size_t sum = 0;
    size_t count = 0;
    for (size_t i = 0; i < list->len; i++) {
        sum += list->d[i].sz * list->d[i].ct;
        count += list->d[i].ct;
    }
    printf("%9s\n%s\n", "avg", size_unit(sum / count));

    free(list);

    return EXIT_SUCCESS;
}
