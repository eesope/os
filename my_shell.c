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
			add_bg_pid(pid);
            printf("Shell: background process started (PID=%d)\n", pid);
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


int main(int argc, char* argv[]) {
    char line[MAX_INPUT_SIZE];            
    char **tokens;              

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
