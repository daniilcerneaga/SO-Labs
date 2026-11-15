#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Optional three-stage pipeline: ls /usr/bin | grep x | wc -l. */
int main(void)
{
    int first[2], second[2];
    if (pipe(first) < 0) { perror("pipe"); return 1; }
    if (pipe(second) < 0) {
        perror("pipe"); close(first[0]); close(first[1]); return 1;
    }
    pid_t children[3];
    for (int stage = 0; stage < 3; stage++) {
        children[stage] = fork();
        if (children[stage] < 0) {
            perror("fork");
            close(first[0]); close(first[1]);
            close(second[0]); close(second[1]);
            for (int i = 0; i < stage; i++)
                while (waitpid(children[i], NULL, 0) < 0 && errno == EINTR) {}
            return 1;
        }
        if (children[stage] == 0) {
            if (stage > 0) {
                close(0);
                if (dup(stage == 1 ? first[0] : second[0]) < 0) {
                    perror("dup stdin"); exit(1);
                }
            }
            if (stage < 2) {
                close(1);
                if (dup(stage == 0 ? first[1] : second[1]) < 0) {
                    perror("dup stdout"); exit(1);
                }
            }
            close(first[0]); close(first[1]);
            close(second[0]); close(second[1]);
            if (stage == 0) execlp("ls", "ls", "/usr/bin", (char *)NULL);
            if (stage == 1) execlp("grep", "grep", "x", (char *)NULL);
            if (stage == 2) execlp("wc", "wc", "-l", (char *)NULL);
            perror("exec");
            exit(127);
        }
    }
    close(first[0]); close(first[1]);
    close(second[0]); close(second[1]);
    int failed = 0;
    for (int i = 0; i < 3; i++) {
        int status;
        pid_t waited;
        do { waited = waitpid(children[i], &status, 0); } while (waited < 0 && errno == EINTR);
        if (waited < 0) { perror("waitpid"); return 1; }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) failed = 1;
    }
    return failed;
}
