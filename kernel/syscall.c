#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if(copyinstr(p->pagetable, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void
argint(int n, int *ip)
{
  *ip = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);
extern uint64 sys_trace(void);
extern uint64 sys_sysinfo(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_trace]   sys_trace,
[SYS_sysinfo] sys_sysinfo,
};

static char *syscall_names[] = {
  [SYS_fork]    "fork",
  [SYS_exit]    "exit",
  [SYS_wait]    "wait",
  [SYS_pipe]    "pipe",
  [SYS_read]    "read",
  [SYS_kill]    "kill",
  [SYS_exec]    "exec",
  [SYS_fstat]   "fstat",
  [SYS_chdir]   "chdir",
  [SYS_dup]     "dup",
  [SYS_getpid]  "getpid",
  [SYS_sbrk]    "sbrk",
  [SYS_sleep]   "sleep",
  [SYS_uptime]  "uptime",
  [SYS_open]    "open",
  [SYS_write]   "write",
  [SYS_mknod]   "mknod",
  [SYS_unlink]  "unlink",
  [SYS_link]    "link",
  [SYS_mkdir]   "mkdir",
  [SYS_close]   "close",
  [SYS_trace]   "trace",
  [SYS_sysinfo] "sysinfo",
};

#ifdef TRACE_ARG
enum argtype {
  ARG_INT,
  ARG_PTR,
  ARG_STR,
  ARG_ARGV,   // for exec
};

struct syscall_argdesc {
  int nargs;
  enum argtype types[4];
};

static struct syscall_argdesc syscall_args[] = {
  [SYS_fork] = {
    .nargs = 0,
  },
  [SYS_exit] = {
    .nargs = 1,
    .types = { ARG_INT },
  },
  [SYS_wait] = {
    .nargs = 1,
    .types = { ARG_PTR },
  },
  [SYS_pipe] = {
    .nargs = 1,
    .types = { ARG_PTR },
  },
  [SYS_read] = {
    .nargs = 3,
    .types = { ARG_INT, ARG_PTR, ARG_INT },
  },
  [SYS_kill] = {
    .nargs = 1,
    .types = { ARG_INT },
  },
  [SYS_exec] = {
    .nargs = 2,
    .types = { ARG_STR, ARG_ARGV },   // ⭐ exec đặc biệt
  },
  [SYS_fstat] = {
    .nargs = 1,
    .types = { ARG_PTR },
  },
  [SYS_chdir] = {
    .nargs = 1,
    .types = { ARG_STR },
  },
  [SYS_dup] = {
    .nargs = 1,
    .types = { ARG_INT },
  },
  [SYS_getpid] = {
    .nargs = 0,
  },
  [SYS_sbrk] = {
    .nargs = 1,
    .types = { ARG_INT },
  },
  [SYS_sleep] = {
    .nargs = 1,
    .types = { ARG_INT },
  },
  [SYS_uptime] = {
    .nargs = 0,
  },
  [SYS_open] = {
    .nargs = 2,
    .types = { ARG_STR, ARG_INT },
  },
  [SYS_write] = {
    .nargs = 3,
    .types = { ARG_INT, ARG_STR, ARG_INT },
  },
  [SYS_mknod] = {
    .nargs = 3,
    .types = { ARG_STR, ARG_INT, ARG_INT },
  },
  [SYS_unlink] = {
    .nargs = 1,
    .types = { ARG_STR },
  },
  [SYS_link] = {
    .nargs = 2,
    .types = { ARG_STR, ARG_STR },
  },
  [SYS_mkdir] = {
    .nargs = 1,
    .types = { ARG_STR },
  },
  [SYS_close] = {
    .nargs = 1,
    .types = { ARG_INT },
  },
  [SYS_trace] = {
    .nargs = 1,
    .types = { ARG_INT },
  },
  [SYS_sysinfo] = {
    .nargs = 1,
    .types = { ARG_PTR },
  }
};

static void
trace_syscall_args(struct proc *p, int num, uint64 *args)
{
  printf("(");

  struct syscall_argdesc *desc = &syscall_args[num];

  for (int i = 0; i < desc->nargs; i++) {
    uint64 a = args[i];

    switch (desc->types[i]) {

    case ARG_INT:
      printf("%d", (int)a);
      break;

    case ARG_PTR:
      printf("%p", (void*)a);
      break;

    case ARG_STR: {
      char buf[64];
      if (fetchstr(a, buf, sizeof(buf)) >= 0)
        printf("\"%s\"", buf);
      else
        printf("%p", (void*)a);
      break;
    }

    case ARG_ARGV: {
      uint64 uarg;
      char buf[64];

      printf("[");
      for (int j = 0; j < MAXARG; j++) {
        if (fetchaddr(a + j * sizeof(uint64), &uarg) < 0)
          break;
        if (uarg == 0)
          break;

        if (j > 0)
          printf(", ");

        if (fetchstr(uarg, buf, sizeof(buf)) >= 0)
          printf("\"%s\"", buf);
        else
          printf("%p", (void*)uarg);
      }
      printf("]");
      break;
    }
    }

    if (i + 1 < desc->nargs)
      printf(", ");
  }

  printf(")");
}
#endif

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
#ifdef TRACE_ARG
    uint64 args[4];
    for (int i = 0; i < 4; i++)
      args[i] = argraw(i);
#endif

    int is_exec = (num == SYS_exec);

    // exec is handled specially because it replaces the process address space,
    // so its arguments must be fetched before invoking the system call.
    if (is_exec && (p->tracemask & (1 << num))) {
      printf("%d: syscall %s",
             p->pid,
             syscall_names[num]);

#ifdef TRACE_ARG
      trace_syscall_args(p, num, args);
#endif
    }

    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0
    int ret = syscalls[num]();
    p->trapframe->a0 = ret;

    if (p->tracemask & (1 << num)) {

      if (!is_exec) {
      printf("%d: syscall %s",
             p->pid,
             syscall_names[num]);

#ifdef TRACE_ARG
        trace_syscall_args(p, num, args);
#endif
      }

      // trace return value
      printf(" -> %d\n", ret);
    }

  } else {
    printf("%d %s: unknown sys call %d\n",
           p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
