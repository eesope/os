#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// 시그널 핸들러 함수
void handle_sigint(int sig) {
    printf("I will run forever (SIGINT ignored)\n");
    // 핸들러가 끝난 뒤 메인루프로 복귀 → 계속 실행
}

int main() {
    // SIGINT를 handle_sigint로 처리
    signal(SIGINT, handle_sigint);

    while (1) {
        printf("Running... PID = %d\n", (int)getpid());
        sleep(2);
    }
    return 0;
}
