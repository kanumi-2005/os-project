#include "kernel/types.h"
#include "user/user.h"

const char *usage =
"xargs COMMAND [INITIAL-ARGS]\n"
"Run COMMAND with arguments INITIAL-ARGS and more arguments read from input\n";


int main(int argc, char *argv[])
{
  if (argc < 2)
    fprintf(2, "Usage:\n%s\n", usage);

  char **new_argv = malloc(sizeof(void *) * (argc + 1));
  for (int i = 1; argv[i]; ++i)
	  new_argv[i - 1] = argv[i];
  char buf[512];
  new_argv[argc] = 0;
  new_argv[argc - 1] = buf;

  while (*gets(buf, 512) != 0)
  {
    int len = strlen(buf);
    if (buf[len - 1] != '\n' && buf[len - 1] != '\r')
    {
      fprintf(2, "%s: input line too long\n", argv[0]);
    }

    buf[len - 1] = 0;

    int pid = fork();
    if (pid < 0)
    {
      fprintf(2, "%s: fork failed\n", argv[0]);
      exit(1);
    }

    if (pid == 0)
    {
      exec(argv[1], new_argv);
      exit(1);
    }

    wait(0);
  }

  free(new_argv);
  exit(0);
}
