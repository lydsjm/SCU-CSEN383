#include "page.h"


void MFU_FUNCTION(LISTOFPAGES* pl) {
    page* it = pl->HDL;
    page* victim = NULL;

    while (it) {
        if (it->pid != -1) { // only consider occupied frames
            if (!victim) {
                victim = it;
            } else {
                if (it->CNTER > victim->CNTER) {
                    victim = it;
                } else if (it->CNTER == victim->CNTER) {
                    if (it->LONE < victim->LONE) {
                        victim = it;
                    } else if (it->LONE == victim->LONE &&
                               it->FTBOUGHT < victim->FTBOUGHT) {
                        victim = it;
                    }
                }
            }
        }
        it = it->next;
    }

    if (!victim) return; // nothing to evict (shouldn't happen when full)

    if (DEBUG==1)
        printf("EVICTED (MFU) :: p[%03d] c:%02d last:%0.2f first:%0.2f\n",
               victim->pid, victim->CNTER, victim->LONE, victim->FTBOUGHT);

    // mark frame free 
    victim->pid       = -1;
    victim->PGENUMBER = -1;

}
