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
        // 자식 프로세스
        printf("[Child] Before exec\n");

        // 예: 존재하지 않는 명령어를 실행해보기 위해 주석 처리
        // execlp("ls", "ls", NULL);

        // 잘못된 exec -> 실패 시 그 다음 코드가 실행됨
        execlp("some_non_existing_cmd", "some_non_existing_cmd", NULL);
        
        // exec가 실패하면 아래 메시지 출력됨
        printf("[Child] After exec (exec failed)\n");
        exit(1);
    } else {
        wait(NULL);
    }
    return 0;
}
