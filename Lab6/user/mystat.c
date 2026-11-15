#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static char *
typename(int type)
{
  switch(type){
  case T_DIR: return "directory";
  case T_FILE: return "file";
  case T_DEVICE: return "device";
  default: return "unknown";
  }
}

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "usage: mystat <path> ...\n");
    exit(1);
  }
  int failed = 0;
  for(int i = 1; i < argc; i++){
    struct stat st;
    if(stat(argv[i], &st) < 0){
      fprintf(2, "mystat: cannot stat %s\n", argv[i]);
      failed = 1;
      continue;
    }
    printf("%s: %s, inode %u, links %d, size %lu bytes\n",
           argv[i], typename(st.type), st.ino, st.nlink, st.size);
  }
  exit(failed);
}
