#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

struct child {
  struct child *next;
  char path[];
};

static int failed;

static char *
basename(char *path)
{
  char *name = path;
  for(char *p = path; *p; p++)
    if(*p == '/' && p[1])
      name = p + 1;
  return name;
}

static struct child *
make_child(char *path, char *part)
{
  uint prefix = strlen(path);
  struct child *child = malloc(sizeof(*child) + prefix + strlen(part) + 2);
  if(child == 0){
    fprintf(2, "find: out of memory\n");
    failed = 1;
    return 0;
  }
  child->next = 0;
  strcpy(child->path, path);
  if(part[0]){
    child->path[prefix] = '/';
    strcpy(child->path + prefix + 1, part);
  }
  return child;
}

static void
walk(char *path, char *name, int show_inode)
{
  // An explicit DFS stack avoids the small xv6 C stack and nested open dirs.
  struct child *pending = make_child(path, "");
  while(pending){
    struct child *current = pending;
    pending = current->next;
    struct stat st;
    if(stat(current->path, &st) < 0){
      fprintf(2, "find: cannot stat %s\n", current->path);
      failed = 1;
      free(current);
      continue;
    }
    if(strcmp(basename(current->path), name) == 0){
      if(show_inode)
        printf("%s: inode %u\n", current->path, st.ino);
      else
        printf("%s\n", current->path);
    }
    if(st.type == T_DIR){
      int fd = open(current->path, O_RDONLY);
      if(fd < 0){
        fprintf(2, "find: cannot open %s\n", current->path);
        failed = 1;
      } else {
        struct dirent entry;
        int n;
        while((n = read(fd, &entry, sizeof(entry))) == sizeof(entry)){
          if(entry.inum == 0)
            continue;
          char part[DIRSIZ + 1];
          memmove(part, entry.name, DIRSIZ);
          part[DIRSIZ] = '\0';
          if(strcmp(part, ".") == 0 || strcmp(part, "..") == 0)
            continue;
          struct child *child = make_child(current->path, part);
          if(child){
            child->next = pending;
            pending = child;
          }
        }
        if(n != 0){
          fprintf(2, "find: incomplete directory read: %s\n", current->path);
          failed = 1;
        }
        close(fd);
      }
    }
    free(current);
  }
}

int
main(int argc, char *argv[])
{
  if((argc != 3 && argc != 4) ||
     (argc == 4 && strcmp(argv[3], "-n") != 0)){
    fprintf(2, "usage: find <dir> <name> [-n]\n");
    exit(1);
  }
  // Normalise a trailing slash so basename comparison also works on the root.
  int length = strlen(argv[1]);
  while(length > 1 && argv[1][length - 1] == '/')
    argv[1][--length] = '\0';
  walk(argv[1], argv[2], argc == 4);
  exit(failed);
}
