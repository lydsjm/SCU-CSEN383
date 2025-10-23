#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void run_algo(const Algo *algo, int N, const unsigned int seeds[WORKLOADS]);

static void parse_args(int argc, char **argv, int *N_out, unsigned int seeds[WORKLOADS])
{
    int N = MAX_JOBS;
    if (argc >= 2)
        N = atoi(argv[1]);
    if (N < 1)
        N = 1;
    if (N > MAX_JOBS)
        N = MAX_JOBS;

    unsigned int defaults[WORKLOADS] = {101, 202, 303, 404, 505}; // default seeds (for debugging)
    for (int i = 0; i < WORKLOADS; i++)
    {
        if (2 + i < argc)
            seeds[i] = (unsigned int)strtoul(argv[2 + i], NULL, 10);
        else
            seeds[i] = defaults[i];
    }
    *N_out = N;
}

int main(int argc, char **argv)
{
    int N;
    unsigned int seeds[WORKLOADS];
    parse_args(argc, argv, &N, seeds);

    printf("========== Scheduler Suite ==========\n");
    printf("Jobs per run: %d | Seeds: %u %u %u %u %u\n\n",
           N, seeds[0], seeds[1], seeds[2], seeds[3], seeds[4]);

    printf("----- FCFS -----\n");
    fflush(stdout);
    static const Algo fcfs_algo = { .type = FCFS, .name = "FCFS", .full_name = "FCFS (non-preemptive)", .run_one = run_one_algo, .pick = pick_fcfs };
    run_algo(&fcfs_algo, N, seeds);

    printf("\n----- SJF (non-preemptive) -----\n");
    fflush(stdout);
    static const Algo sjf_algo = { .type = SJF, .name = "SJF", .full_name = "SJF (non-preemptive)", .run_one = run_one_algo, .pick = pick_sjf };
    run_algo(&sjf_algo, N, seeds);

    printf("\n----- SRT (preemptive SJF) -----\n");
    fflush(stdout);
    static const Algo srt_algo = { .type = SRT, .name = "SRT", .full_name = "SRT (preemptive SJF)", .run_one = run_one_srt, .pick = NULL };
    run_algo(&srt_algo, N, seeds);

    printf("\n----- RR (q=1) -----\n");
    fflush(stdout);
    static const Algo rr_algo = { .type = RR, .name = "RR", .full_name = "RR (preemptive) (q=1)", .run_one = run_one_rr, .pick = NULL };
    run_algo(&rr_algo, N, seeds);

    printf("\n----- HPF (highest priority first) (non-preemptive) -----\n");
    fflush(stdout);
    static const Algo hpfnp_algo = { .type = HPFNP, .name = "HPF-NP", .full_name = "HPF (non-preemptive)", .run_one = run_one_algo, .pick = pick_hpfnp };
    run_algo(&hpfnp_algo, N, seeds);

    printf("\n----- HPF (highest priority first) (preemptive) -----\n");
    fflush(stdout);
    static const Algo hpfp_algo = { .type = HPFP, .name = "HPF-P", .full_name = "HPF (preemptive)", .run_one = run_one_hpfp, .pick = NULL };
    run_algo(&hpfp_algo, N, seeds);

    printf("\n========== Done ==========\n");
    return 0;
}