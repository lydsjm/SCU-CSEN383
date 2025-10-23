#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int cmp_arrival_then_name(const void *a, const void *b) // comparison function for qsort
{
    const Proc *p = (const Proc *)a;
    const Proc *q = (const Proc *)b;

    if (p->arrival != q->arrival) // compare arrival times
        return p->arrival - q->arrival;

    return (int)p->name - (int)q->name; // compare names
}

int earliest_not_started(Proc *P, int n) // find the index of the job that has not started yet with the earliest arrival time
{
    int idx = -1;
    for (int i = 0; i < n; i++)
    {
        if (P[i].start == -1)
        {
            if (idx == -1 ||
                P[i].arrival < P[idx].arrival ||
                (P[i].arrival == P[idx].arrival && P[i].name < P[idx].name))
            {
                idx = i;
            }
        }
    }

    return idx;
}

void make_workload(Proc *P, int n, unsigned int seed) // create a workload of n processes with given seed
{
    srand(seed);
    for (int i = 0; i < n; i++)
    {
        P[i].name = (char)('A' + i);
        P[i].arrival = rand() % MAX_QUANTA;      // Random arrival time between 0 and MAX_QUANTA-1
        P[i].service = (rand() % 10) + 1; // Random service time between 1 and 10
        P[i].priority = (rand() % 4) + 1; // Random priority between 1 and 4
        P[i].start = -1;
        P[i].finish = -1;
        P[i].remaining = P[i].service;
        P[i].counted = false;
    }

    qsort(P, n, sizeof(Proc), cmp_arrival_then_name); // sort by arrival time, then name
}

void run_algo(const Algo *algo, int N, const unsigned int seeds[5])
{
    double sum_avg_tat = 0, sum_avg_wt = 0, sum_avg_rpt = 0, sum_tp = 0;

    for (int r = 0; r < WORKLOADS; r++) // # of runs/workloads with different seeds
    {
        Proc *jobs = (Proc *)malloc(sizeof(Proc) * N);
        make_workload(jobs, N, seeds[r]);

        printf("===== RUN %d (seed=%u, jobs=%d) - %s =====\n", r + 1, seeds[r], N, algo->full_name);
        printf("Created processes (sorted by arrival):\n");
        printf("Name  Arr  Serv  Prio\n");
        for (int i = 0; i < N; i++)
        {
            printf("  %c    %3d   %3d    %d\n", jobs[i].name, jobs[i].arrival, jobs[i].service, jobs[i].priority);
        }

        Stats st;
        char *timeline = NULL;
        algo->run_one(algo, jobs, N, &st, &timeline);

        printf("\nTime chart (100+ quanta, '-' = idle):\n%s\n", timeline);
        printf("\nPer-run stats over processes that actually ran (start < 100):\n");
        printf("  Completed: %d\n", st.completed);
        printf("  Elapsed quanta: %d\n", st.elapsed);
        printf("  Avg Turnaround (TAT): %.3f\n", st.avg_tat);
        printf("  Avg Waiting   (WT):  %.3f\n", st.avg_wt);
        printf("  Avg Response  (RPT): %.3f\n", st.avg_rpt);
        printf("  Throughput (completed/elapsed): %.4f\n", st.throughput);
        printf("  Max consecutive idle quanta (info): %d\n", st.max_idle_streak);

        sum_avg_tat += st.avg_tat;
        sum_avg_wt += st.avg_wt;
        sum_avg_rpt += st.avg_rpt;
        sum_tp += st.throughput;

        free(timeline);
        free(jobs);
        printf("\n");
    }

    printf("===== 5-RUN AVERAGES (%s) =====\n", algo->name);
    printf("Avg of Avg TAT: %.3f\n", sum_avg_tat / 5.0);
    printf("Avg of Avg WT : %.3f\n", sum_avg_wt / 5.0);
    printf("Avg of Avg RPT: %.3f\n", sum_avg_rpt / 5.0);
    printf("Avg Throughput: %.4f\n", sum_tp / 5.0);

    printf("=================================\n");
}

void run_one_algo(const Algo *algo, Proc *jobs, int n, Stats *outStats, char **timeline_out) // run one workload
{
    int t = 0;
    int running = -1;
    int finished_count = 0;
    int idle_streak = 0, max_idle = 0;

    int cap = 256, len = 0;
    char *timeline = (char *)malloc(cap);

    while (1)
    {
        if (running == -1)
        {
            int cand = algo->pick(jobs, n, t);
            if (cand == -1 && t < MAX_QUANTA && idle_streak >= 2) // if idle for 2 or more quanta, advance time to next arrival
            {
                int j = earliest_not_started(jobs, n);
                if (j != -1)
                {
                    if (jobs[j].arrival > t)
                        jobs[j].arrival = t;
                    cand = pick_sjf(jobs, n, t);
                }
            }

            if (cand != -1 && t < MAX_QUANTA) // found a candidate to run
            {
                jobs[cand].start = t;
                jobs[cand].counted = true;
                jobs[cand].remaining = jobs[cand].service;
                running = cand; // start running it
            }
        }

        if (len + 2 >= cap) // expand timeline buffer if needed
        {
            cap *= 2;
            timeline = (char *)realloc(timeline, cap);
        }
        if (running != -1) // running a job
        {
            timeline[len++] = jobs[running].name; // add job to the timeline
            idle_streak = 0; // reset idle streak

            jobs[running].remaining -= 1; // decrement remaining time
            
            if (jobs[running].remaining == 0) // if job finished
            {
                jobs[running].finish = t + 1;
                running = -1;
                finished_count++;
            }
        }
        else // idle
        {
            timeline[len++] = '-';
            idle_streak++;
            if (idle_streak > max_idle)
                max_idle = idle_streak;
        }
        t++;

        // check termination conditions
        bool any_unfinished_started = false;
        for (int i = 0; i < n; i++) // check if any started job is unfinished
        {
            if (jobs[i].counted && jobs[i].finish == -1)
            {
                any_unfinished_started = true;
                break;
            }
        }
        bool any_not_started = false;
        if (t < MAX_QUANTA)
        {
            for (int i = 0; i < n; i++) // check if any job has not started yet
                if (!jobs[i].counted && jobs[i].start == -1)
                {
                    any_not_started = true;
                    break;
                }
        }

        if (t >= MAX_QUANTA && running == -1 && !any_unfinished_started) // if all jobs are finished after time MAX_QUANTA
            break;
        if (t < MAX_QUANTA && running == -1 && !any_unfinished_started && !any_not_started) // if all started jobs are finished and no more jobs will arrive
            break;
    }

    timeline[len] = '\0';
    *timeline_out = timeline;

    double sum_tat = 0, sum_wt = 0, sum_rpt = 0;
    int counted_jobs = 0;
    for (int i = 0; i < n; i++)
    {
        if (jobs[i].counted)
        {
            int tat = jobs[i].finish - jobs[i].arrival;
            int wt = tat - jobs[i].service;
            int rpt = jobs[i].start - jobs[i].arrival;
            sum_tat += tat;
            sum_wt += wt;
            sum_rpt += rpt;
            counted_jobs++;
        }
    }

    outStats->completed = counted_jobs;
    outStats->elapsed = t;
    outStats->max_idle_streak = max_idle;
    if (counted_jobs > 0)
    {
        outStats->avg_tat = sum_tat / counted_jobs;
        outStats->avg_wt = sum_wt / counted_jobs;
        outStats->avg_rpt = sum_rpt / counted_jobs;
    }
    else
    {
        outStats->avg_tat = outStats->avg_wt = outStats->avg_rpt = 0.0;
    }
    outStats->throughput = (t > 0) ? ((double)counted_jobs / (double)t) : 0.0;
}