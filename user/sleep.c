#include "kernel/types.h"
#include "user/user.h"

const char *usage =
"Usage: sleep NUMBER\n"
"Pause for NUMBER seconds, where NUMBER is an positive integer.\n";

int is_number(const char *s)
{
  if (s == 0) return 0;
  if (*s == 0) return 0;

  int i = 0;
  if (s[0] == '+' || s[0] == '-')
    ++i;

  for (; s[i]; ++i)
  {
    if (s[i] < '0' || s[i] > '9')
      return 0;
  }

  return 1;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(2, "Usage:\n%s\n", usage);
    exit(1);
  }

  if (!is_number(argv[1]))
  {
    fprintf(2, "%s: invalid argument (not a number): %s\n", argv[0], argv[1]);
    fprintf(2, "Usage:\n%s\n", usage);
    exit(1);
  }

  int nsec = atoi(argv[1]);

  if (nsec <= 0)
  {
    fprintf(2, "%s: invalid argument (non-positive): %s\n", argv[0], argv[1]);
    fprintf(2, "Usage:\n%s\n", usage);
    exit(1);
  }

  sleep(nsec * 10);

  exit(0);
}
