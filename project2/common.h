// common.h

#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

#define MAX_JOBS 26 // maximum number of jobs (A-Z)
#define MAX_QUANTA 100 // maximum number of quanta to simulate
#define WORKLOADS 5 // number of workloads to run per algorithm

struct Algo; // forward declaration

typedef struct
{
    char name;  // name/ID of the process (A, B, C, ...)
    int arrival; // arrival time (0-99)
    int service; // service time (runtime) (1-10)
    int priority; // priority (1-4)
    int start;   // start time (-1 if not started yet)
    int finish;  // finish time
    int remaining; // remaining time
    bool counted;
} Proc;

typedef struct
{
    double avg_tat, avg_wt, avg_rpt, throughput; // turnaround, waiting, response, throughput
    int completed; // number of completed processes
    int elapsed; // total elapsed time
    int max_idle_streak;
} Stats;

typedef struct Algo
{
    enum { FCFS, SJF, SRT, RR, HPFNP, HPFP } type;
    const char *name;
    const char *full_name;
    void (*run_one)(const struct Algo *algo, Proc *jobs, int n, Stats *outStats, char **timeline_out);
    int (*pick)(Proc *P, int n, int now);

} Algo;

int cmp_arrival_then_name(const void *a, const void *b);
int earliest_not_started(Proc *P, int n);
void make_workload(Proc *P, int n, unsigned int seed);
int pick_fcfs(Proc *P, int n, int now);
int pick_sjf(Proc *P, int n, int now);
int pick_srt(Proc *P, int n, int now, bool allow_new_start);
// int pick_rr(Proc *P, int n, int now);
int pick_hpfnp(Proc *P, int n, int now);
int pick_hpfp(Proc *P, int n, int now);
void run_algo(const Algo *algo, int N, const unsigned int seeds[5]);
void run_one_algo(const Algo *algo, Proc *jobs, int n, Stats *outStats, char **timeline_out);
void run_one_srt(const Algo *algo, Proc *jobs, int n, Stats *outStats, char **timeline_out);
void run_one_rr(const Algo *algo, Proc *jobs, int n, Stats *outStats, char **timeline_out);
void run_one_hpfp(const Algo *algo, Proc *jobs, int n, Stats *outStats, char **timeline_out);

#endif // COMMON_H