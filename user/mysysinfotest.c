#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

#define N_CHILD 5
#define MEMALLOC (4096 * 8)
#define N_LOAD_CHILD 50
#define BURN_TIME 30

#ifdef LOAD_AVG
static void
print_load(uint64 load)
{
  printf("%d.%d%d",
         LOAD_INT(load),
         LOAD_FRAC(load) / 10,
         LOAD_FRAC(load) % 10);
}

static void
print_loads(uint64 *loads)
{
  printf("Load avg   : ");
  print_load(loads[0]);
  printf(" ");
  print_load(loads[1]);
  printf(" ");
  print_load(loads[2]);
}
#endif

static void
print_freemem(uint64 freemem)
{
  printf("Free memory: %d bytes", freemem);
}

static void
print_nproc(uint64 nproc)
{
  printf("Processes  : %d", nproc);
}

static void
print_sysinfo(struct sysinfo *si)
{
  print_freemem(si->freemem);
  printf("\n");
  print_nproc(si->nproc);
  printf("\n");
#ifdef LOAD_AVG
  print_loads(si->loads);
  printf("\n");
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

  print_sysinfo(&si);

  printf("\nSpawning CPU freemem & nproc (%d processes)...\n", N_CHILD);

  // Tạo tiến trình con
  for (int i = 0; i < N_CHILD; i++) {
    int pid = fork();
    if (pid == 0) {
      int *ptr = (int *)malloc(MEMALLOC);
      int t = 0;
      while(t < BURN_TIME*10) {
        for (volatile int j = 0; j < 1000000; j++); // burn CPU
        t++;
      }
      free(ptr);
      exit(0);
    } else {
      printf("After forking %d times: ", i + 1);
        if (sysinfo(&si) < 0) {
          printf("sysinfo failed\n");
          exit(1);
        }
        print_freemem(si.freemem);
	printf(", ");
        print_nproc(si.nproc);
	printf("\n");
    }
  }
  for (int i = 0; i < N_CHILD; i++) {
    wait(0);
    printf("After waiting %d child processes: ", i + 1);
    if (sysinfo(&si) < 0) {
      printf("sysinfo failed\n");
      exit(1);
    }
    print_freemem(si.freemem);
    printf(", ");
    print_nproc(si.nproc);
    printf("\n");
  }

  printf("All child processes exited. Final sysinfo:\n");
  if (sysinfo(&si) == 0) {
    print_sysinfo(&si);
  }

#ifdef LOAD_AVG
  printf("\nSpawning CPU loads (%d processes)...\n", N_LOAD_CHILD);
  for (int i = 0; i < N_LOAD_CHILD; i++) {
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

  for (int i = 0; i < N_LOAD_CHILD / 5 * 2; i++) {
    sleep(50);
    if (sysinfo(&si) < 0) {
      printf("sysinfo failed\n");
      exit(1);
    }
    printf("After waiting %d sec: ", (i+1)*5);
    print_loads(si.loads);
    printf("\n");
  }

  for (int i = 0; i < N_LOAD_CHILD; i++) {
    wait(0);
  }

  printf("All child processes exited. Final sysinfo:\n");
  if (sysinfo(&si) == 0) {
    print_sysinfo(&si);
  }
#endif

  exit(0);
}
