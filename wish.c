#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char *line = NULL;  // буфер
    size_t cap = 0;     // розмір буфера

    while (1) {
        printf("wish> ");
        fflush(stdout);

        ssize_t len = getline(&line, &cap, stdin);
        if (len == -1) {
            break;      // EOF
        }

        printf("you typed: %s", line);  // debug
    }

    free(line);
    exit(0);
}