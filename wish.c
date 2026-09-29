#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_ARGS 64
#define MAX_PATHS 64
#define MAX_CMDS 64

char *paths[MAX_PATHS]; // список папок для пошуку
int npaths = 0;

// друк єдиного повідомлення про помилку в stderr
void print_error(void)
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// розбиття рядка на слова; повертає кількість слів або -1
int parse_args(char *cmd, char **args)
{
    int n = 0;
    char *token;

    while ((token = strsep(&cmd, " \t\n")) != NULL)
    {
        if (*token == '\0')
        {
            continue; // роздільники підряд
        }
        if (n == MAX_ARGS - 1)
        {
            return -1; // забагато аргументів
        }
        args[n++] = token;
    }
    args[n] = NULL;
    return n;
}

// виділення імені файлу після '>'; 0 при успіху, -1 при помилці
int parse_redirect(char *cmd, char **outfile)
{
    *outfile = NULL;

    char *gt = strchr(cmd, '>');
    if (gt == NULL)
    {
        return 0; // перенаправлення немає
    }
    *gt = '\0';

    char *rest = gt + 1;
    if (strchr(rest, '>') != NULL)
    {
        return -1; // більше одного '>'
    }

    char *files[MAX_ARGS];
    if (parse_args(rest, files) != 1)
    {
        return -1; // немає файлу або файлів кілька
    }
    *outfile = files[0];
    return 0;
}

// пошук програми в папках path; повний шлях записується в full
int find_executable(char *cmd, char *full, size_t size)
{
    for (int i = 0; i < npaths; i++)
    {
        snprintf(full, size, "%s/%s", paths[i], cmd);
        if (access(full, X_OK) == 0)
        {
            return 0;
        }
    }
    return -1;
}

// заміна списку папок path
void set_path(char **args, int argn)
{
    for (int i = 0; i < npaths; i++)
    {
        free(paths[i]);
    }
    npaths = 0;
    for (int i = 1; i < argn && npaths < MAX_PATHS; i++)
    {
        paths[npaths++] = strdup(args[i]);
    }
}

// запуск зовнішньої програми; повертає PID або 0 при помилці
pid_t run_command(char **args, char *outfile)
{
    char path[256];
    if (find_executable(args[0], path, sizeof(path)) == -1)
    {
        print_error();
        return 0;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        print_error();
        return 0;
    }

    if (pid == 0)
    {
        if (outfile != NULL)
        {
            int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                print_error();
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        execv(path, args);
        print_error(); // execv повертається лише при помилці
        exit(1);
    }

    return pid;
}

// обробка однієї команди; повертає PID, 0 або -1 для exit
pid_t execute(char *cmd)
{
    char *outfile;
    if (parse_redirect(cmd, &outfile) == -1)
    {
        print_error();
        return 0;
    }

    char *args[MAX_ARGS];
    int argn = parse_args(cmd, args);
    if (argn == -1)
    {
        print_error();
        return 0;
    }
    if (argn == 0)
    {
        if (outfile != NULL)
        {
            print_error(); // '>' без команди
        }
        return 0;
    }

    if (strcmp(args[0], "exit") == 0)
    {
        if (argn != 1)
        {
            print_error();
            return 0;
        }
        return -1;
    }

    if (strcmp(args[0], "path") == 0)
    {
        set_path(args, argn);
        return 0;
    }

    if (strcmp(args[0], "cd") == 0)
    {
        if (argn != 2 || chdir(args[1]) != 0)
        {
            print_error();
        }
        return 0;
    }

    return run_command(args, outfile);
}

int main(int argc, char *argv[])
{
    FILE *input = stdin;
    int interactive = 1;

    if (argc > 2)
    {
        print_error();
        exit(1);
    }

    if (argc == 2)
    {
        input = fopen(argv[1], "r");
        if (input == NULL)
        {
            print_error();
            exit(1);
        }
        interactive = 0;
    }

    paths[0] = strdup("/bin");
    npaths = 1;

    char *line = NULL; // буфер
    size_t cap = 0;    // розмір буфера
    int done = 0;

    while (!done)
    {
        if (interactive)
        {
            printf("wish> ");
            fflush(stdout);
        }

        ssize_t len = getline(&line, &cap, input);
        if (len == -1)
        {
            break; // EOF
        }

        pid_t pids[MAX_CMDS];
        int npids = 0;

        char *rest = line;
        char *cmd;
        while ((cmd = strsep(&rest, "&")) != NULL)
        {
            pid_t pid = execute(cmd);
            if (pid == -1)
            {
                done = 1; // exit
                break;
            }
            if (pid > 0)
            {
                if (npids < MAX_CMDS)
                {
                    pids[npids++] = pid;
                }
                else
                {
                    waitpid(pid, NULL, 0); // масив заповнений
                }
            }
        }

        for (int i = 0; i < npids; i++)
        {
            waitpid(pids[i], NULL, 0);
        }
    }

    free(line);
    if (input != stdin)
    {
        fclose(input);
    }

    for (int i = 0; i < npaths; i++)
    {
        free(paths[i]);
    }

    exit(0);
}