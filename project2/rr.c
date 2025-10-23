#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void run_one_rr(const Algo *algo, Proc *jobs, int n, Stats *outStats, char **timeline_out)
{
    int t = 0;
    int finished = 0;
    int idle_streak = 0, max_idle = 0;
    int cap = 256, len = 0;
    char *timeline = (char *)malloc(cap);

    int queue[256];
    int q_front = 0, q_back = 0;

    while (1)
    {
        // Add new arrivals to queue and check all jobs
        // If a job arrives at the current time t, add it to the back of the ready queue
        // Stop adding new jobs after time 99
        if (t < 100)
        {
            for (int i = 0; i < n; i++)
            {
                if (jobs[i].arrival == t)
                {
                    queue[q_back++] = i;
                }
            }
        }

        // Index of the job currently running
        int running_idx = -1; // -1 means none

        // Pick a process from the queue to run
        // If the ready queue isn't empty, get the next job from the front
        if (q_front < q_back)
        {
            running_idx = queue[q_front++]; // Dequeue the job index
        }

        // Dynamically expand the timeline string if it's getting full
        if (len + 2 >= cap)
        {
            cap *= 2;
            timeline = (char *)realloc(timeline, cap);
        }

        // Execute the chosen process or stay idle
        if (running_idx != -1) // If there's a job to run
        {
            timeline[len++] = jobs[running_idx].name; // Record its name in the timeline
            idle_streak = 0;                          // Reset idle counter

            // If this is the job's first time running, record its start time
            if (jobs[running_idx].start == -1)
            {
                jobs[running_idx].start = t;
                jobs[running_idx].counted = true; // Mark it as "ran" for final stats
            }

            jobs[running_idx].remaining -= 1; // Run for one quantum

            // After the process runs
            if (jobs[running_idx].remaining == 0) // If the job is finished
            {
                jobs[running_idx].finish = t + 1; // Record its finish time
                finished++;
            }
            else // If the job isn't finished
            {
                // Put it back at the end of the ready queue
                // It'll wait for its next turn
                queue[q_back++] = running_idx;
            }
        }
        else // If there's no job to run
        {
            timeline[len++] = '-'; // The CPU is idle
            idle_streak++;
            if (idle_streak > max_idle)
                max_idle = idle_streak;
        }

        t++; // Move to the next time quantum

        // Check if it's done
        bool any_unfinished = false;
        for (int i = 0; i < n; i++)
        {
            // A job is unfinished if it started but hasn't finished yet
            if (jobs[i].counted && jobs[i].finish == -1)
            {
                any_unfinished = true;
                break;
            }
        }

        // Done if the ready queue is empty and no jobs are left unfinished
        if (q_front == q_back && !any_unfinished)
        {
            // Make sure no new jobs are scheduled to arrive in the future
            if (t >= 100)
            {
                break;
            }

            // If before t=100, might be idle just waiting for the next job
            // Check if any jobs that haven't started are yet to arrive
            bool future_arrivals = false;
            for (int i = 0; i < n; i++)
            {
                if (jobs[i].start == -1)
                {
                    future_arrivals = true;
                    break;
                }
            }
            if (!future_arrivals)
            {
                break;
            }
        }
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