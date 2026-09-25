#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX_ARGS 64

void print_error(void) {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int parse_args(char *cmd, char **args) {
    int n = 0;
    char *token;

    while ((token = strsep(&cmd, " \t\n")) != NULL) {
        if (*token == '\0') {
            continue;   // роздільники підряд
        }
        if (n == MAX_ARGS - 1) {
            return -1;  // забагато аргументів
        }
        args[n++] = token;
    }
    args[n] = NULL;
    return n;
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

        char *args[MAX_ARGS];
        int argn = parse_args(line, args);
        if (argn == -1) {
            print_error();
            continue;
        }
        if (strcmp(args[0], "exit") == 0) {
            if (argn != 1) {
                print_error();
                continue;
            }
            break;
        }

        for (int i = 0; i < argn; i++) {
            printf("arg[%d] = '%s'\n", i, args[i]);  // debug
        }
    }

    free(line);
    if (input != stdin) {
        fclose(input);
    }
    exit(0);
}