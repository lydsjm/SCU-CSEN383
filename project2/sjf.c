#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int pick_sjf(Proc *P, int n, int now) // pick the index of the job with the shortest service time among those that have arrived and not started yet
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
                if (P[i].service < P[best].service) // shorter service time
                    best = i;
                else if (P[i].service == P[best].service)
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