#include "common.h"
#include <stdio.h>
#include <stdlib.h>

void run_one_hpfp(const Algo *algo, Proc *jobs, int n, Stats *outStats, char **timeline_out)
{
    int t = 0;
    int idle_streak = 0, max_idle = 0;

    int cap = 256, len = 0;
    char *timeline = (char *)malloc(cap);

    int next_rr_pos[4]; // next position to check for RR within each priority level (1-4)
    for (int p = 1; p <= 4; p++) next_rr_pos[p-1] = 0;

    while (1)
    {
        bool allow_new_start = (t < MAX_QUANTA); // whether we can start new jobs (t < 100)

        // pick candidate with highest priority among arrived & unfinished jobs
        int cand = -1;
        int cand_prio = -1;
        for (int pr = 1; pr <= 4; pr++) // from highest priority (1) to lowest (4)
        {
            // check if any job of this priority is available
            bool any = false;
            for (int i = 0; i < n; i++)
            {
                if (jobs[i].priority == pr && jobs[i].arrival <= t && jobs[i].remaining > 0)
                {
                    any = true;
                    break;
                }
                else if (jobs[i].arrival > t)
                {
                    break; // jobs sorted by arrival time, so no need to check further
                }
            }
            if (!any) continue; // no job of this priority is available

            // find next job for RR starting at next_rr_pos[pr-1]
            int start = next_rr_pos[pr - 1] % n;
            int found = -1;
            for (int offset = 0; offset < n; offset++)
            {
                int idx = (start + offset) % n;
                if (jobs[idx].priority == pr && 
                    jobs[idx].arrival <= t && 
                    jobs[idx].remaining > 0 &&
                    (allow_new_start || jobs[idx].start != -1)) // if not allowing new starts, skip jobs that never started
                {
                    found = idx;
                    break;
                }
            }
            if (found != -1) // found a job of this priority
            {
                cand = found;
                cand_prio = pr;
                break; // no need to check lower priorities
            }
        }

        if (cand == -1 && t < MAX_QUANTA && idle_streak >= 2)
        {
            int j = earliest_not_started(jobs, n);
            if (j != -1)
            {
                if (jobs[j].arrival > t) // jump forward in time
                    t = jobs[j].arrival;
                
                continue; // re-pick candidate at this time
            }
        }

        if (len + 2 >= cap) // expand timeline buffer if needed
        {
            cap *= 2;
            timeline = (char *)realloc(timeline, cap);
        }

        if (cand != -1)
        {
            
            if (jobs[cand].start == -1) // first time running this job
            {
                jobs[cand].start = t;
                jobs[cand].counted = true;
            }

            timeline[len++] = jobs[cand].name; // add job to the timeline
            idle_streak = 0; // reset idle streak

            jobs[cand].remaining -= 1; // decrement remaining time (1 quantum)
            next_rr_pos[cand_prio - 1] = (cand + 1) % n; // next RR position for this priority level

            if (jobs[cand].remaining == 0) // if job finished
            {
                jobs[cand].finish = t + 1;
            }
        }
        else // idle
        {
            timeline[len++] = '-';
            idle_streak++;
            if (idle_streak > max_idle) max_idle = idle_streak;
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

        if (t >= MAX_QUANTA && !any_unfinished_started) // if all jobs are finished after time MAX_QUANTA
            break;
        if (t < MAX_QUANTA && !any_unfinished_started && !any_not_started) // if all started jobs are finished and no more jobs will arrive
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