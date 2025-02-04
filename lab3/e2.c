#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // for fork(), getpid()
#include <sys/wait.h> // for wait()

int main() {
    pid_t rc = fork(); // fork() 호출

    if (rc < 0) {
        // fork 실패
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // 자식 프로세스
        printf("[Child] PID: %d\n", (int)getpid());
        exit(0); // 자식 종료
    } else {
        // 부모 프로세스
        int status;
        pid_t w = wait(&status); // 자식 종료 대기
        // pid_t w = waitpid(rc, &status, 0); // 특정 자식 종료 대기
        if (w == rc) {
            // 정상적으로 자식을 회수
            printf("[Parent] Child PID reaped: %d\n", (int)w);
        }
        // 부모 프로세스 종료
    }
    return 0;
}
