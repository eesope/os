#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // 자식 프로세스: ls 실행

        // execlp("ls", "ls", NULL);
        char *args[] = {"ls", NULL};
        char *args[] = {"ls", "-l", NULL}; // flag 있는 경우

        execvp(args[0], args);
        // exec가 성공하면 이후 코드는 실행되지 않음
        // 실패 시 아래가 실행됨
        fprintf(stderr, "exec failed\n");
        exit(1);
    } else {
        // 부모 프로세스
        wait(NULL); // 자식 종료 대기
    }
    return 0;
}
