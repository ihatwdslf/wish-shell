#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_error(void) {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    int interactive = 1;

    if (argc > 2) {
        print_error();
        exit(1);
    }

    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            print_error();
            exit(1);
        }
        interactive = 0;
    }

    char *line = NULL;  // буфер
    size_t cap = 0;     // розмір буфера

    while (1) {
        if (interactive) {
            printf("wish> ");
            fflush(stdout);
        }

        ssize_t len = getline(&line, &cap, input);
        if (len == -1) {
            break;      // EOF
        }

        printf("you typed: %s", line);  // debug
    }

    free(line);
    if (input != stdin) {
        fclose(input);
    }
    exit(0);
}