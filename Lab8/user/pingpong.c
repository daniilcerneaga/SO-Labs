#include "kernel/types.h"
#include "user/user.h"

static void
fail(char *message)
{
  fprintf(2, "pingpong: %s\n", message);
  exit(1);
}

int
main(void)
{
  int p2c[2], c2p[2];
  char byte = 'p';
  if(pipe(p2c) < 0)
    fail("pipe");
  if(pipe(c2p) < 0){
    close(p2c[0]); close(p2c[1]);
    fail("pipe");
  }
  int pid = fork();
  if(pid < 0){
    close(p2c[0]); close(p2c[1]);
    close(c2p[0]); close(c2p[1]);
    fail("fork");
  }
  if(pid == 0){
    close(p2c[1]); close(c2p[0]);
    if(read(p2c[0], &byte, 1) != 1)
      fail("read ping");
    close(p2c[0]);
    printf("%d: received ping\n", getpid());
    if(write(c2p[1], &byte, 1) != 1)
      fail("write pong");
    close(c2p[1]);
    exit(0);
  }
  close(p2c[0]); close(c2p[1]);
  if(write(p2c[1], &byte, 1) != 1)
    fail("write ping");
  close(p2c[1]);
  if(read(c2p[0], &byte, 1) != 1 || byte != 'p')
    fail("read pong");
  close(c2p[0]);
  printf("%d: received pong\n", getpid());
  int status;
  if(wait(&status) != pid || status != 0)
    fail("child status");
  exit(0);
}
