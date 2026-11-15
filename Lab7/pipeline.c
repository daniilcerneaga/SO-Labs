#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

static int collect(pid_t pid)
{
    int status;
    pid_t waited;
    do { waited = waitpid(pid, &status, 0); } while (waited < 0 && errno == EINTR);
    if (waited < 0) { perror("waitpid"); return 1; }
    return !WIFEXITED(status) || WEXITSTATUS(status) != 0;
}

/* The same process/descriptor setup as: ls /etc | wc -l. */
int main(void)
{
    int p[2];
    if (pipe(p) < 0) { perror("pipe"); return 1; }
    pid_t producer = fork();
    if (producer < 0) { perror("fork"); close(p[0]); close(p[1]); return 1; }
    if (producer == 0) {
        close(1);
        if (dup(p[1]) < 0) { perror("dup stdout"); exit(1); }
        close(p[0]);
        close(p[1]);
        execlp("ls", "ls", "/etc", (char *)NULL);
        perror("exec ls");
        exit(127);
    }
    pid_t consumer = fork();
    if (consumer < 0) {
        perror("fork"); close(p[0]); close(p[1]); collect(producer); return 1;
    }
    if (consumer == 0) {
        close(0);
        if (dup(p[0]) < 0) { perror("dup stdin"); exit(1); }
        close(p[0]);
        close(p[1]);
        execlp("wc", "wc", "-l", (char *)NULL);
        perror("exec wc");
        exit(127);
    }
    close(p[0]);                         /* parent uses neither end */
    close(p[1]);
    int failed = collect(producer);
    failed |= collect(consumer);
    return failed;
}
