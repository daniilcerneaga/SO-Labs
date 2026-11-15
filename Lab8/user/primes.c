#include "kernel/types.h"
#include "user/user.h"

// Each process filters multiples of one prime, then feeds the next stage.
static void __attribute__((noreturn))
filter(int input)
{
  int prime, value;
  int n = read(input, &prime, sizeof(prime));
  if(n == 0){
    close(input);
    exit(0);
  }
  if(n != sizeof(prime)){
    close(input);
    exit(1);
  }
  printf("prime %d\n", prime);
  int next[2];
  if(pipe(next) < 0)
    exit(1);
  int pid = fork();
  if(pid < 0)
    exit(1);
  if(pid == 0){
    close(input);
    close(next[1]);
    filter(next[0]);
  }
  close(next[0]);
  while((n = read(input, &value, sizeof(value))) == sizeof(value)){
    if(value % prime != 0 && write(next[1], &value, sizeof(value)) != sizeof(value))
      exit(1);
  }
  close(input);
  close(next[1]);
  int status;
  wait(&status);
  exit(n == 0 ? status : 1);
}

int
main(void)
{
  int p[2];
  if(pipe(p) < 0)
    exit(1);
  int pid = fork();
  if(pid < 0)
    exit(1);
  if(pid == 0){
    close(p[1]);
    filter(p[0]);
  }
  close(p[0]);
  for(int value = 2; value <= 35; value++)
    if(write(p[1], &value, sizeof(value)) != sizeof(value))
      exit(1);
  close(p[1]);
  int status;
  wait(&status);
  exit(status);
}
