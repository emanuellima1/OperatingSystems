#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include "builtin.h"

#define MAX_WORDS 50
#define MAX_CWD_LENGTH 200
#define MAX_PROMPT_LENGTH 250

void split(char *, char **);
void handler();
void handler_parent();

int main()
{
    char* command, * arg, * user, cwd[MAX_CWD_LENGTH], prompt[MAX_PROMPT_LENGTH];
    char* ptrv[MAX_WORDS];
    int wstatus, err;
    pid_t child;

    struct sigaction act1, act2;
    act1.sa_handler = handler;
    act2.sa_handler = handler_parent;
    act1.sa_flags = act2.sa_flags = 0;
    sigemptyset(&(act1.sa_mask));
    sigemptyset(&(act2.sa_mask));
    sigaction(SIGINT, &act1, NULL);

    using_history();

    user = getlogin();
    getcwd(cwd, MAX_CWD_LENGTH);
    snprintf(prompt, MAX_PROMPT_LENGTH, "{%s@%s} ", user, cwd);

    while ((command = readline(prompt))) {
        add_history(command);

        if (!strncmp(command, "mkdir", 5)) {
            arg = &(command[6]);
            make_dir(arg);
        } else if (!strncmp(command, "kill -9", 7)) {
            arg = &(command[8]);
            kill_9(atoi(arg));
        } else if (!strncmp(command, "ln -s", 5)) {
            arg = &(command[6]);
            ln_s(arg);
        } else if (!strncmp(command, "cd", 2)) {
            arg = &(command[3]);
            cd(arg);
        }

        else {
            split(command, ptrv);
            if ((child = fork())) {
                sigaction(SIGINT, &act2, NULL);
                wait(&wstatus);
                sigaction(SIGINT, &act1, NULL);
            } else {
                if ((err = execvp(command, ptrv)) == -1) {
                    printf("Não foi possível executar o comando\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        user = getlogin();
        getcwd(cwd, MAX_CWD_LENGTH);
        snprintf(prompt, MAX_PROMPT_LENGTH, "{%s@%s} ", user, cwd);
        free(command);
    }
    printf("\n");

    return EXIT_SUCCESS;
}


void split(char* string, char ** ptrv)
{
    int k = 0, flag = 1;
    for (int i = 0; string[i] != '\0' && k < MAX_WORDS; i++) {
        if (string[i] == ' ') {
            string[i] = '\0';
            flag = 1;
        }
        else if (flag) {
            ptrv[k] = string + i;
            flag = 0;
            k++;
        }
    }
    ptrv[k] = NULL;
}

void handler() {
    printf("\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void handler_parent() {
  printf("\n");
}
