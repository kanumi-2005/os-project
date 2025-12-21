#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/syscall.h"

int
main(int argc, char *argv[])
{

  printf("== trace test start ==\n");

  // Trace một số syscall quan trọng
  // fork, exec, write, open, close, exit
  trace((1 << SYS_fork) |
        (1 << SYS_exec) |
        (1 << SYS_write) |
        (1 << SYS_open) |
        (1 << SYS_close) |
        (1 << SYS_exit));
  // write
  write(1, "hello trace\n", 12);

  // open / close
  int fd = open("README", 0);
  if (fd >= 0) {
    close(fd);
  }

  // fork
  int pid = fork();
  if (pid == 0) {
    // child
    char *args[] = { "echo", "hi", "from", "exec", 0 };
    exec("echo", args);

    // nếu exec fail
    printf("exec failed\n");
    exit(1);
  } else {
    wait(0);
  }

  trace(0);
  printf("== trace test end ==\n");
  exit(0);
}

