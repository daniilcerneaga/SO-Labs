#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/* The same descriptor setup as: ls -l > out.txt. */
int main(void)
{
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        close(1);
        int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) { perror("open"); exit(1); }
        fprintf(stderr, "open gave descriptor %d\n", fd);
        execlp("ls", "ls", "-l", (char *)NULL);
        perror("exec");
        exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(pid, &status, 0); } while (waited < 0 && errno == EINTR);
    if (waited < 0) { perror("waitpid"); return 1; }
    if (!WIFEXITED(status)) {
        fprintf(stderr, "redirect: child did not exit normally\n");
        return 1;
    }
    if (WEXITSTATUS(status) != 0)
        return WEXITSTATUS(status);
    printf("done: the listing is in out.txt\n");
    return 0;
}
