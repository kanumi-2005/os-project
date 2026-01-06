#include "kernel/types.h"
#include "user/user.h"


void  __attribute__((noreturn)) primes(int fd_write, int fd_read)
{
  close(fd_write);

  int prime = 0;
  int number = 0;

  if (read(fd_read, &prime, 4) == 0)
    exit(0);

  printf("prime %d\n", prime);

  int fd_parent2child[2];
  pipe(fd_parent2child);

  int pid = fork();

  if (pid < 0)
  {
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }

  if (pid > 0)
  {
    close(fd_parent2child[0]);

    while (read(fd_read, &number, 4))
    {
      if (number % prime == 0)
        continue;

      write(fd_parent2child[1], &number, 4);
    }

    close(fd_read);
    close(fd_parent2child[1]);
    wait(0);
    exit(0);
  }

  close(fd_read);
  primes(fd_parent2child[1], fd_parent2child[0]);
  exit(0);
}


int main()
{

  int fd[2];
  pipe(fd);

  int pid = fork();

  if (pid < 0)
  {
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }

  if (pid > 0)
  {
    close(fd[0]);

    for (int i = 2; i <= 280; ++i)
      write(fd[1], &i, 4);

    close(fd[1]);

    wait(0);
    exit(0);
  }

  primes(fd[1], fd[0]);
}
