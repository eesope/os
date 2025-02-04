#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/**
 * Splits the string by space and returns the array of tokens.
 */
char **tokenize(char *line) {
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for (i = 0; i < strlen(line); i++) {
        char readChar = line[i];

        if (readChar == ' ' || readChar == '\n' || readChar == '\t') {
            token[tokenIndex] = '\0';
            if (tokenIndex != 0) {
                tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0; 
            }
        }
        else {
            token[tokenIndex++] = readChar;
        }
    }

    free(token);
    tokens[tokenNo] = NULL;
    return tokens;
}

/**
 * Clean zombie up
 */
void reap_zombie() {
    int status;
    pid_t wpid;

    // WNOHANG: non-block; if no zombie child just pass
    while ((wpid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Shell: Background process finished (PID=%d)\n", wpid);
    }
}

/**
 * Accept user input keep it in line buffer 
 */
int read_input(char *line, int size) {
    bzero(line, size);
    printf("$ ");
    fflush(stdout);

    if (scanf("%[^\n]", line) == EOF) {
        return 0;
    }
    getchar();  // consume \n 
    return 1;
}

/**
 * is &? 
 * -> yes, put flag 1 (background) 
 * -> no, put flag 0 (foreground)
 */
int check_background(char **tokens) {
    int last_index = 0;
    while (tokens[last_index] != NULL) {
        last_index++;
    }

    if (last_index > 0 && strcmp(tokens[last_index - 1], "&") == 0) {
        free(tokens[last_index - 1]);
        tokens[last_index - 1] = NULL;
        return 1;
    }
    return 0;
}

/**
 * is cd?
 */
void handle_cd(char **tokens) {
    // tokens[0] = "cd"
    if (tokens[1] == NULL) {
        fprintf(stderr, "Shell: Incorrect command\n");
    } 
    else if (strcmp(tokens[1], "..") == 0) {
        // "cd .."
        if (chdir("..") != 0) {
            fprintf(stderr, "Shell: Incorrect command\n");
        }
    }
    else {
        // cd <directory>
        if (chdir(tokens[1]) != 0) {
            fprintf(stderr, "Shell: Incorrect command\n");
        }
    }
}

/**
 * fork() -> execvp() to lookup user/bin
 * act upon is_bg? 0 (no &) | 1 (yes &)
 */
void execute_command(char **tokens, int is_bg) {
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return;
    }
    else if (pid == 0) {
        // child
        if (execvp(tokens[0], tokens) == -1) {
            fprintf(stderr, "Shell: Incorrect command\n");
        }
        exit(1);
    }
    else {
        // parent
        if (is_bg == 0) {
            // foreground
            int status;
            waitpid(pid, &status, 0);
        } else {
            // background
            printf("Shell: background process started (PID=%d)\n", pid);
        }
    }
}

int main(int argc, char* argv[]) {
    char line[MAX_INPUT_SIZE];            
    char **tokens;              
    int i;

    while (1) {
        reap_zombie();

        if (!read_input(line, sizeof(line))) {
            break;
        }

		// clean up empty command
        if (strlen(line) == 0) {
            continue;
        }

        line[strlen(line)] = '\n'; 
        tokens = tokenize(line);

        if (tokens[0] == NULL) {
            free(tokens);
            continue;
        }

		// is there & ?
        int is_bg = check_background(tokens);

		// is this cd ?
        if (strcmp(tokens[0], "cd") == 0) {
            handle_cd(tokens);
        }

		// other than cd command line
        else {
            execute_command(tokens, is_bg);
        }

        // dismiss tokens
        for(i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }

    reap_zombie();
    return 0;
}
