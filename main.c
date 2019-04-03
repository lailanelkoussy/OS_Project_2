/**
 * Simple shell interface program.
 *
 * Student: Laila N ElKoussy
 * ID# : 900160812
 *
 * Note: Some of this code has been taken from the textbook. Any other source of code is mentioned in the comments
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80
#define MAX_TOKS 40
#define READ_END 0
#define WRITE_END 1
#define DELIMITERS " \t"


int parseString(char *line, char ***argv);

void exec(char **arr, int n);

int main(void) {
    char **args, **last;    /* command line (of 80) has max of 40 arguments */
    int should_run = 1;
    char input[MAX_LINE] = "", lastin[MAX_LINE] /*previous instruction */, comp[3] = "!!";
    int n, i, m;
    pid_t pid;

    while (should_run) {
        printf("osh> ");
        fflush(stdout);

        for (i = 0; i < MAX_LINE; i++) //transferring input onto history buffer
            lastin[i] = input[i];

        //getting input
        scanf("%[^\n]%*c", input);

        if (!(strcmp(input, "exit")))//exit
            should_run = 0;

        //parsing the input into args
        n = parseString(input, &args); // number of strings read

        if (n == 0)
            should_run = 0;

        //forking
        if ((strcmp(input, "\n") != 0) && should_run)
            pid = fork();

        if (pid < 0) { //could not fork
            fprintf(stderr, "Fork Failed");
            return -1;
        } else if (pid == 0) { //we are in the child process
            if (!(strcmp(input, comp))) { //if we get a history command
                if (!strcmp(lastin, ""))
                    printf("No commands in history. \n");
                else {
                    m = parseString(lastin, &last);
                    printf("%s \n", lastin);
                    //execvp(last[0], last);
                    exec(last, m);
                }

            } else
                //execvp(args[0], args);
                exec(args, n);
        } else { //parent process
            if (strcmp(args[n - 1], "&") != 0)
                waitpid(pid, NULL, 0);

        }

    }

    return 0;
}

int parseString(char *line, char ***argv) { //function obtained off of cs.nyu.edu

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

void exec(char **arr, int n) {
    int i = n, j, fd, k, pipeloc, l;
    int pipes = 0;
    FILE *fp;
    pid_t pid;
    int fs[2];

    if (!strcmp(arr[n - 1], "&")) { //removing the &
        arr[n - 1] = 0;
        i--;
    }
    for (k = 0; k < i; k++) {
        if (!strcmp(arr[k], "|")) {
            pipes = 1;
            pipeloc = k;
            arr[pipeloc] = 0;
        }
    }

    if (pipes) {

        /* create the pipe */
        if (pipe(fs) == -1) {
            fprintf(stderr, "Pipe failed");
            exit(-1);
        }
        /* fork a child process */
        pid = fork();
        if (pid < 0) { /* error occurred */
            fprintf(stderr, "Fork Failed");
            exit(-1);
        }
        if (pid > 0) { //parent
            waitpid(pid, NULL, 0);
            /* close the unused end of the pipe */
            close(fs[WRITE_END]);
            /* read from the pipe */
            dup2(fs[READ_END], STDIN_FILENO);

            //  printf("3arr[0]:\n %s\n", arr[0]);

            char **args2;
            int c = 0;
            for (l = pipeloc + 1; arr[l] != NULL; l++) //removing the instruction at the beginning of the array
            {
                args2[c] = arr[l];
                c++;
            }
            args2[c] = arr[l];

            /* close the read end of the pipe */
            close(fs[READ_END]);
            execvp(args2[0], args2);
            printf("Unable to execute command \n");
            exit(-1);


        } else {
            //child process
            // close the unused end of the pipe
            close(fs[READ_END]);
            // write to the pipe
            dup2(fs[WRITE_END], STDOUT_FILENO);

            /* close the write end of the pipe */
            close(fs[WRITE_END]);

            execvp(arr[0], arr);
            printf("Unable to execute command \n");
            exit(-1);
        }

    } else if (i > 1) {
        if (!strcmp(arr[i - 2], "<")) { //reading from a file

            fp = fopen(arr[i - 1], "r");
            arr[i - 1] = 0;
            arr[i - 2] = 0;
            fd = fileno(fp);
            dup2(fd, STDIN_FILENO);
            execvp(arr[0], arr);
            printf("Unable to execute command \n");
            exit(-1);

        } else if (!strcmp(arr[i - 2], ">")) { //writing to a file

            fp = fopen(arr[i - 1], "a");
            arr[i - 1] = 0;
            arr[i - 2] = 0;
            fd = fileno(fp);
            dup2(fd, STDOUT_FILENO);
            execvp(arr[0], arr);
            printf("Unable to execute command \n");
            exit(-1);

        }

        execvp(arr[0], arr);
        printf("Unable to execute command \n");
        exit(-1);

    }
    execvp(arr[0], arr);
    printf("Unable to execute command \n");
    exit(-1);

}
