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

int main(int argc, char* argv[]) {
	char line[MAX_INPUT_SIZE];            
	char **tokens;              
	int i;

	while (1) {
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		fflush(stdout);  // prevent over buffering

		if (scanf("%[^\n]", line) == EOF) {
			break;
		}
		getchar();  // consume \n
		/* END: TAKING INPUT */

		if (strlen(line) == 0) {  // if empty line
			continue;
		}

		/* Tokenize the command line */
		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		if (tokens[0] == NULL) { // if no command
			free(tokens);
			continue;
		}

		/* cd command? -> exec parent process */
		if (strcmp(tokens[0], "cd") == 0) {
			if (tokens[1] == NULL) {
				fprintf(stderr, "Shell: Incorrect command\n");
			} 
			else if (strcmp(tokens[1], "..") == 0) {
				int ret = chdir("..");
				if (ret != 0) {
					fprintf(stderr, "Shell: Incorrect command\n");
				}

			}
			else {
				int ret = chdir(tokens[1]);
				if (ret != 0) {
					fprintf(stderr, "Shell: Incorrect command\n");
				}

			}
		}

		/* not cd command -> child process */
		else {
			pid_t pid = fork();
			if (pid < 0) {
				fprintf(stderr, "Fork failed\n");
			}
			else if (pid == 0) {
				/* to exec /usr/bin commands */
				if (execvp(tokens[0], tokens) == -1) {
					fprintf(stderr, "Shell: Incorrect command\n");
				}
				exit(1);; // child process fin.
			}
			else { // parent process
				int status;
				waitpid(pid, &status, 0);
			}

		}

		// Freeing the allocated memory	
		for(i = 0; tokens[i] != NULL; i++) {
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
