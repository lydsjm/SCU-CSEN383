#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* pick job with MIN remaining time among arrived & unfinished.
   If allow_new_start==false (t >= MAX_QUANTA), do NOT consider jobs that never started. */
int pick_srt(Proc *P, int n, int now, bool allow_new_start)
{
    int best = -1;
    for (int i = 0; i < n; i++)
    {
        if (P[i].remaining > 0 && P[i].arrival <= now)
        {
            if (!allow_new_start && P[i].start == -1)
                continue; // cannot first-start after 99
            if (best == -1)
                best = i;
            else
            {
                if (P[i].remaining < P[best].remaining)
                    best = i;
                else if (P[i].remaining == P[best].remaining)
                {
                    if (P[i].arrival < P[best].arrival)
                        best = i;
                    else if (P[i].arrival == P[best].arrival && P[i].name < P[best].name)
                        best = i;
                }
            }
        }
    }
    return best;
}

void run_one_srt(const Algo *algo, Proc *jobs, int n, Stats *outStats, char **timeline_out)
{
    int t = 0;
    int running = -1;
    int idle_streak = 0, max_idle = 0;

    int cap = 256, len = 0;
    char *timeline = (char *)malloc(cap);

    while (1)
    {
        bool allow_new_start = (t < MAX_QUANTA);

        // Choose the job with the shortest remaining time (preemptive)
        int cand = pick_srt(jobs, n, t, allow_new_start);

        // Enforce "≤2 consecutive idle" like your SJF
        if (cand == -1 && allow_new_start && idle_streak >= 2)
        {
            int j = earliest_not_started(jobs, n);
            if (j != -1)
            {
                if (jobs[j].arrival > t)
                    jobs[j].arrival = t; // pull forward
                cand = pick_srt(jobs, n, t, allow_new_start);
            }
        }

        // Preempt / dispatch at quantum boundary
        if (cand != -1)
        {
            if (jobs[cand].start == -1)
            {
                // first dispatch allowed only before 100
                if (allow_new_start)
                {
                    jobs[cand].start = t;
                    jobs[cand].counted = true;
                }
                else
                {
                    // cannot start new jobs after 99
                    cand = -1;
                }
            }
        }

        if (cand != -1)
            running = cand;

        // Ensure timeline buffer
        if (len + 2 >= cap)
        {
            cap *= 2;
            timeline = (char *)realloc(timeline, cap);
        }

        if (running != -1)
        {
            timeline[len++] = jobs[running].name;
            idle_streak = 0;

            jobs[running].remaining -= 1;
            if (jobs[running].remaining == 0)
            {
                jobs[running].finish = t + 1;
                running = -1; // will re-pick next tick
            }
        }
        else
        {
            timeline[len++] = '-';
            idle_streak++;
            if (idle_streak > max_idle)
                max_idle = idle_streak;
        }
        t++;

        // termination checks (same pattern as your other files)
        bool any_unfinished_started = false;
        for (int i = 0; i < n; i++)
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
            for (int i = 0; i < n; i++)
                if (!jobs[i].counted && jobs[i].start == -1)
                {
                    any_not_started = true;
                    break;
                }
        }

        if (t >= MAX_QUANTA && running == -1 && !any_unfinished_started)
            break;
        if (t < MAX_QUANTA && running == -1 && !any_unfinished_started && !any_not_started)
            break;
    }

    timeline[len] = '\0';
    *timeline_out = timeline;

    // stats
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