#include "common.h"
#include <stdio.h>
#include <stdlib.h>

int pick_hpfnp(Proc *P, int n, int now) // pick the index of the job with the highest priority (lowest number) among those that have arrived and not started yet
{
    int best = -1;
    for (int i = 0; i < n; i++)
    {
        if (P[i].start == -1 && P[i].arrival <= now) // not started and has arrived
        {
            if (best == -1) // first candidate
                best = i;
            else // compare with current best
            {
                if (P[i].priority < P[best].priority) // higher priority (lower number)
                    best = i;
                else if (P[i].priority == P[best].priority)
                {
                    if (P[i].arrival < P[best].arrival) // earlier arrival time
                        best = i;
                    else if (P[i].arrival == P[best].arrival && P[i].name < P[best].name) // alphabetically earlier name
                        best = i;
                }
            }
        }
    }
    return best;
}