#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int *open_files(char **paths, int n, int flags, mode_t mode) {
    if (n < 1) {
        return NULL;
    }

    int *fds = malloc(n * sizeof *fds);

    int *fd = fds;
    while (fd < fds + n) {
        if ((*fd++ = open(*paths++, flags, mode)) < 0) {
            fprintf(stderr, "failed to open: %s\n", *--paths);
            exit(EXIT_FAILURE);
        }
    }

    return fds;
}

void close_files(int *fds, int n) {
    for (int *fd = fds; fd < fds + n; fd++) {
        close(*fd);
    }
    free(fds);
}

int main(int argc, char **argv) {
    int flags = O_WRONLY | O_CREAT | O_APPEND | O_TRUNC;
    mode_t mode = 0666;

    if (getopt(argc, argv, "a") == 'a') {
        flags &= ~O_TRUNC;
    }

    int fc = argc - optind;

    int *fds = open_files(argv + optind, fc, flags, mode);

    char buf[BUFFER_SIZE];
    ssize_t rc;

    while ((rc = read(STDIN_FILENO, &buf, BUFFER_SIZE)) > 0) {
        for (int *fd = fds; fd < fds + fc; fd++) {
            if ((write(*fd, &buf, rc)) != rc) {
                fprintf(stderr, "failed a write\n");
                exit(EXIT_FAILURE);
            }
        }
        if ((write(STDOUT_FILENO, &buf, rc)) != rc) {
            fprintf(stderr, "failed a write\n");
            exit(EXIT_FAILURE);
        }
    }
    if (rc < 0) {
        fprintf(stderr, "failed a read\n");
        exit(EXIT_FAILURE);
    }

    close_files(fds, fc);

    return EXIT_SUCCESS;
}

