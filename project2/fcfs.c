#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int pick_fcfs(Proc *P, int n, int now) // pick the index of the job with the earliest arrival time among those that have arrived and not started yet
{
    int best = -1;
    for (int i = 0; i < n; i++)
    {
        if (P[i].start == -1 && P[i].arrival <= now) // not started and has arrived
        {
            if (P[i].start == -1 && P[i].arrival <= now) // not started and has arrived
            {
                best = i;
                break;
            }
        }
    }
    return best;
}