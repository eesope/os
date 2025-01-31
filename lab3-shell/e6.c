#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>   // for kill()
#include <sys/wait.h>

int main() {
    pid_t rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // 자식 프로세스: 오래 실행
        printf("[Child] PID: %d, Sleeping 30 seconds...\n", (int)getpid());
        sleep(30);
        printf("[Child] Finished sleep\n");
        exit(0);
    } else {
        // 부모 프로세스
        printf("[Parent] Child PID = %d\n", rc);
        // 예: 5초 후 자식 종료
        sleep(5);
        printf("[Parent] Killing child...\n");
        kill(rc, SIGTERM); // 자식 프로세스 종료 요청
        // 자식이 종료될 때까지 대기
        wait(NULL);
        printf("[Parent] Child reaped. Exiting.\n");
    }
    return 0;
}
