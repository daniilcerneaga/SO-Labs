#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* Delay waitpid so the terminated child remains visible as a zombie. */
int main(void)
{
    printf("parent: my PID is %d\n", getpid());
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        printf("child:  my PID is %d, my parent is %d\n", getpid(), getppid());
        fflush(stdout);
        execlp("ls", "ls", "-l", (char *)NULL);
        perror("exec");
        exit(127);
    }
    int status;
    sleep(20);
    pid_t waited;
    do { waited = waitpid(pid, &status, 0); } while (waited < 0 && errno == EINTR);
    if (waited < 0) { perror("waitpid"); return 1; }
    if (!WIFEXITED(status)) {
        fprintf(stderr, "parent: child %d did not exit normally\n", pid);
        return 1;
    }
    printf("parent: child %d ended with status %d\n", pid, WEXITSTATUS(status));
    return 0;
}
