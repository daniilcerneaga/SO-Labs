#include "kernel/types.h"
#include "user/user.h"

static void
usage(void)
{
  fprintf(2, "usage: sleep <ticks>\n");
  exit(1);
}

int
main(int argc, char *argv[])
{
  if(argc != 2 || argv[1][0] == '\0')
    usage();
  int ticks = 0;
  for(char *p = argv[1]; *p; p++){
    if(*p < '0' || *p > '9')
      usage();
    int digit = *p - '0';
    if(ticks > (2147483647 - digit) / 10)
      usage();
    ticks = ticks * 10 + digit;
  }
  if(pause(ticks) < 0){
    fprintf(2, "sleep: interrupted\n");
    exit(1);
  }
  exit(0);
}
