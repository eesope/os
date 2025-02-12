#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BG_PROCESSES 64

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
 * Accept user input keep it in line buffer.
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

/*
* enum for command types
*/ 
typedef enum {
    CMD_NONE,
    CMD_CD,
    CMD_EXIT,
    CMD_OTHER
} CommandType;

/* 
* Parse tokens[0] and return enum value
*/
CommandType parse_cmd (char *cmd) {
    if (cmd == NULL) {
        return CMD_NONE;
    }
    if (strcmp(cmd, "cd") == 0) {
        return CMD_CD;
    }
    if (strcmp(cmd, "exit") == 0) {
        return CMD_EXIT;
    }
    return CMD_OTHER;
}

/**
 * is there & ? 
 * -> yes, put flag 1 (background) 
 * -> no, put flag 0 (foreground)
 */
int is_bg_sig(char **tokens) {
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

void handle_cd(char **tokens) {
    if (tokens[1] == NULL) {
        fprintf(stderr, "Shell: Incorrect command\n");
    } 
    else if (strcmp(tokens[1], "..") == 0) {
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

// Data structure for background processes
pid_t bg_pids[MAX_BG_PROCESSES];
int bg_count = 0;

/*
* Count the pid of background processes in the array.
*/ 
void add_bg_pid (pid_t pid) {
	if (bg_count < MAX_BG_PROCESSES) {
		bg_pids[bg_count++] = pid;
	}
	else {
		fprintf(stderr, "Too many background processes for this computer.\n");
	}
}

void rm_bg_pid (pid_t pid) {
	for(int i = 0; i < bg_count; i++) {
		if (bg_pids[i] == pid) {
			bg_pids[i] = bg_pids[bg_count - 1];
			bg_count--;
			break;
		}
	}
}

/**
 * Clean zombie up
 */
void reap_zombie() {
    int status;
    pid_t wpid;

    // WNOHANG: non-block; if no zombie child just pass
    while ((wpid = waitpid(-1, &status, WNOHANG)) > 0) {
		rm_bg_pid(wpid);
        printf("Shell: Background process finished (PID=%d)\n", wpid);
    }
}

void sig_handler (int signum) {
    // printf("\nsignal#: %d\n", signum);
    switch (signum) {
        case 2:
            // terminate current foreground process
            exit(0);
    }    
}

char **split_by_delim(char *line, const char *delim, int *num_cmds) {
    char **commands = malloc(MAX_NUM_TOKENS * sizeof(char *));
    int count = 0;
    char *token = strtok(line, delim);

    while (token != NULL && count < MAX_NUM_TOKENS) {
        commands[count++] = strdup(token); // duplicate str arr
        token = strtok(NULL, delim);
    }
    commands[count] = NULL;
    if (num_cmds)
        *num_cmds = count;
    return commands;
}

/**
 * fork() -> execvp() to lookup user/bin
 * act upon is_bg? 0 (no &) | 1 (yes &)
 */
void execute_command(char **tokens, int is_bg) {
    pid_t child_pid = fork();
    // printf("0 child pid: %d\n", child_pid);
    // printf("0 parent pid: %d\n", getpid());

    if (child_pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return;
    }
    else if (child_pid == 0) {
        // actual executing cmd

        // this process listens signal
        signal(SIGINT, SIG_DFL);

        if (execvp(tokens[0], tokens) == -1) { // execute and checkup
            fprintf(stderr, "Shell: Incorrect command\n");
        }
        _exit(0);
    }
    else {
        // if successfully forked @1st line of function
        // check whether bg process or not 

        if (is_bg == 0) {
            // foreground

            // mk child PGID independent on foreground
            tcsetpgrp(STDIN_FILENO, child_pid);
            // STDIN_FILENO == terminal

            int status;
            waitpid(child_pid, &status, 0);

            // return shell to parent
            tcsetpgrp(STDIN_FILENO, getpgrp()); 

        } else {
            // background
            setpgid(child_pid, child_pid);
			add_bg_pid(child_pid);
            printf("Shell: background process started (PID=%d)\n", child_pid);
        }
    }
}

void execute_serial(char *line) {
    int num_cmds = 0;
    char **cmds = split_by_delim(line, "&&", &num_cmds);
    for (int i = 0; i < num_cmds; i++) {
        char **tokens = tokenize(cmds[i]);
        // serial foreground -> is_bg = 0
        execute_command(tokens, 0);
        
        for (int j = 0; tokens[j] != NULL; j++) {
            free(tokens[j]);
        }
        free(tokens);
        free(cmds[i]);

        // if interrupted by ctrl+c 
        if (SIGINFO == 2) {
            break;
        }
    }
    free(cmds);
}

void execute_parallel(char *line) {
    int num_cmds = 0;
    char **cmds = split_by_delim(line, "&&&", &num_cmds);
    pid_t pids[MAX_NUM_TOKENS];
    for (int i = 0; i < num_cmds; i++) {
        char **tokens = tokenize(cmds[i]);
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Fork failed\n");
        } else if (pid == 0) {
            setpgid(0, 0);
            signal(SIGINT, SIG_DFL);
            if (execvp(tokens[0], tokens) == -1) {
                fprintf(stderr, "Shell: Incorrect command\n");
            }
            _exit(0);
        } else {
            pids[i] = pid;
        }
        for (int j = 0; tokens[j] != NULL; j++) {
            free(tokens[j]);
        }
        free(tokens);
        free(cmds[i]);
    }
    free(cmds);
    // Wait for all parallel commands to finish
    for (int i = 0; i < num_cmds; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    tcsetpgrp(STDIN_FILENO, getpgrp());
}


/**
 * Drive the program. 
*/ 
int main(int argc, char* argv[]) {
    char line[MAX_INPUT_SIZE];            
    char **tokens;

    // signal ignored in parent process
    signal(SIGINT, SIG_IGN);
    
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
        // pared one separate command
        tokens = tokenize(line);

        if (strstr(line, "&&&") != NULL) {
            execute_parallel(line);
        } 
        else if (strstr(line, "&&") != NULL) {
            execute_serial(line);
        } 
        else { // single command execution
            tokens = tokenize(line);
            if (tokens[0] == NULL) {
                free(tokens);
                continue;
            }
        }
		// is there & ?
        int is_bg = is_bg_sig(tokens);
        
		// parse command line
		CommandType cmd_type = parse_cmd(tokens[0]);

		switch (cmd_type)
		{
		case CMD_NONE:
			break;

		case CMD_CD:
			handle_cd(tokens);
			break;

		case CMD_EXIT:
			for (int i = 0; i < bg_count; i++) {
				kill(bg_pids[i], SIGTERM);
			}
			free(tokens);	
			reap_zombie();
			goto END_OF_LOOP;

		case CMD_OTHER:
		default:
			execute_command(tokens, is_bg);
			break;
		}

		int i;
        // dismiss tokens
        for(i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }

	END_OF_LOOP:
    reap_zombie();
    return 0;
}
