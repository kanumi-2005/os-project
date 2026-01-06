#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"


int find(char const *dirpath, char const *filename)
{
  int fd;
  struct stat st;
  struct dirent de;
  char buf[512];
  char *p;
  int total = 0;

	if ((fd = open(dirpath, O_RDONLY)) < 0)
    return 0;

  if (strlen(dirpath) + 1 + DIRSIZ + 1 > sizeof(buf))
  {
    close(fd);
    return 0;
  }

  strcpy(buf, dirpath);
  p = buf + strlen(buf);
  *p++ = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de))
  {
    if (de.inum == 0 || strcmp(de.name, ".") == 0
        || strcmp(de.name, "..") == 0)
      continue;

    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;

    if (stat(buf, &st) < 0)
      continue;

    switch (st.type)
    {
    case T_DEVICE:
    case T_FILE:
      if (strcmp(de.name, filename) == 0)
      {
        printf("%s\n", buf);
        total += 1;
      }
      break;

    case T_DIR:
      total += find(buf, filename);
      break;

    default:
      break;
    }
  }

  close(fd);
  return total;
}

const char *usage =
"find DIRECTORY FILENAME\n"
"Find all file with filename FILENAME in directory and sub-directory of DIRECTORY\n";

int main(int argc, char *argv[])
{

  if (argc != 3)
  {
    fprintf(2, "Usage:\n%s\n", usage);
    exit(1);
  }

  struct stat st;

  if (stat(argv[1], &st) < 0 || st.type != T_DIR)
  {
    fprintf(2, "%s: invalid argument (directory): %s\n", argv[0], argv[1]);
    fprintf(2, "Usage:\n%s\n", usage);
    exit(1);
  }

  if (find(argv[1], argv[2]) == 0)
  {
    fprintf(2, "%s: %s: no such file\n", argv[0], argv[2]);
    exit(1);
  }

  exit(0);
}
