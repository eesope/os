#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// 예: N을 프로그램 인자로 받음
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        exit(1);
    }
    int N = atoi(argv[1]);

    for (int i = 0; i < N; i++) {
        pid_t rc = fork();
        if (rc < 0) {
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        // 부모든 자식이든, fork 후 전체 프로세스 수가 2배가 되어 다음 루프 수행
        // i < N이면 또 fork 호출...
    }

    // 여기까지 오면 2^N개 프로세스들이 각각 이 지점에 도달
    // 각 프로세스는 자신의 PID 출력 후 종료
    printf("PID %d says hello\n", (int)getpid());

    // 부모든 자식이든, 모두 wait()를 호출할 경우
    // 자기 자식이 있으면 회수, 자식이 없으면 -1 반환
    while (wait(NULL) > 0) {
        // 기다릴 자식이 없을 때까지 대기
    }

    return 0;
}
