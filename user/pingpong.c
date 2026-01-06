#include "kernel/types.h"
#include "user/user.h"

int main(void) {
  int fd_parent2child[2];
  int fd_child2parent[2];

  pipe(fd_parent2child);
  pipe(fd_child2parent);

  char buf[1];

  int pid = fork();

  if (pid < 0)
  {
	  fprintf(2,"pingpong: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    close(fd_parent2child[1]);
    close(fd_child2parent[0]);
    read(fd_parent2child[0], buf, 1);
    if (buf[0] == 'p') {
      fprintf(1, "%d: received ping\n", getpid());
      write(fd_child2parent[1], "c", 1);
    }

  } else {
    close(fd_parent2child[0]);
    close(fd_child2parent[1]);
    write(fd_parent2child[1], "p", 1);
    read(fd_child2parent[0], buf, 1);
    if (buf[0] == 'c') {
      fprintf(1, "%d: received pong\n", getpid());
    }
    wait(0);
  }
  exit(0);
}
