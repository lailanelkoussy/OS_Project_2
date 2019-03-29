/**
 * Simple shell interface program.
 *
 * Operating System Concepts - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#define MAX_LINE 80
#define MAX_TOKS 40
#define DELIMITERS " \t"


int parseString(char *line, char ***argv);

int main(void) {
    char **args;    /* command line (of 80) has max of 40 arguments */
    int should_run = 1;
    char input[MAX_LINE];
    int n, i;
    pid_t pid;
    while (should_run) {
        printf("osh> ");
        fflush(stdout);
        //getting input
        scanf("%[^\n]%*c", input);

        //parsing the input into args

        /* splits the input line into separate words */
        n = parseString(input, &args); // number of strings read
        //forking

        pid = fork();

        if (pid < 0) { //could not fork
            fprintf(stderr, "Fork Failed");
            return -1;
        } else if (pid == 0) { //we are in the child process
            execvp(args[0], args);
        } else { //parent process
            if (*args[n - 1] == '&')
                wait(NULL);

        }

        should_run = 0;
    }

    return 0;
}

int parseString(char *line, char ***argv) {

    char *buffer;
    int argc;

    buffer = (char *) malloc(strlen(line) * sizeof(char));
    strcpy(buffer, line);
    (*argv) = (char **) malloc(MAX_TOKS * sizeof(char **));

    argc = 0;
    (*argv)[argc++] = strtok(buffer, DELIMITERS);
    while ((((*argv)[argc] = strtok(NULL, DELIMITERS)) != NULL) &&
           (argc < MAX_TOKS))
        ++argc;

    return argc;
}
