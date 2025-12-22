#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

#define N_CHILD 50
#define BURN_TIME 30

static void
print_load(uint64 load)
{
#ifdef LOAD_AVG
  printf("%d.%d%d",
         LOAD_INT(load),
         LOAD_FRAC(load) / 10,
         LOAD_FRAC(load) % 10);
#else
  printf("N/A");
#endif
}

int
main(void)
{
  struct sysinfo si;

  printf("=== sysinfo test ===\n");

  if (sysinfo(&si) < 0) {
    printf("sysinfo failed\n");
    exit(1);
  }

  printf("Free memory: %d bytes\n", si.freemem);
  printf("Processes  : %d\n", si.nproc);

#ifdef LOAD_AVG
  printf("Load avg   : ");
  print_load(si.loads[0]);
  printf(" ");
  print_load(si.loads[1]);
  printf(" ");
  print_load(si.loads[2]);
  printf("\n");
#endif

  printf("\nSpawning CPU load (%d processes)...\n", N_CHILD);

  // Tạo tiến trình con
  for (int i = 0; i < N_CHILD; i++) {
    int pid = fork();
    if (pid == 0) {
      int t = 0;
      while(t < BURN_TIME*10) {
        for (volatile int j = 0; j < 1000000; j++); // burn CPU
        t++;
      }
      exit(0);
    }
  }

  for (int i = 0; i < N_CHILD / 5 * 2; i++) {
    sleep(50);
    if (sysinfo(&si) < 0) {
      printf("sysinfo failed\n");
      exit(1);
    }
    printf("After %d sec: load = ", (i+1)*5);
    print_load(si.loads[0]);
    printf(" ");
    print_load(si.loads[1]);
    printf(" ");
    print_load(si.loads[2]);
    printf("\n");
  }

  // Wait tất cả con
  for (int i = 0; i < N_CHILD; i++) {
    wait(0);
  }

  printf("All child processes exited. Final loadavg:\n");
  if (sysinfo(&si) == 0) {
    print_load(si.loads[0]); printf(" ");
    print_load(si.loads[1]); printf(" ");
    print_load(si.loads[2]); printf("\n");
  }

  exit(0);
}
