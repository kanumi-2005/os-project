struct sysinfo {
  uint64 freemem;   // amount of free memory (bytes)
  uint64 nproc;     // number of process

#ifdef LOAD_AVG
  uint64 loads[3];
#endif
};

#ifdef LOAD_AVG

#define HZ 10 // Based on start.c:timerinit

// Based on linux loadavg.h
#define FSHIFT    11          /* nr of bits of precision */
#define FIXED_1   (1<<FSHIFT) /* 1.0 as fixed-point */
#define LOAD_FREQ (5*HZ+1)    /* 5 sec intervals */
#define EXP_1     1884        /* 1/exp(5sec/1min) as fixed-point */
#define EXP_5     2014        /* 1/exp(5sec/5min) */
#define EXP_15    2037        /* 1/exp(5sec/15min) */

#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

#endif // LOAD_AVG
