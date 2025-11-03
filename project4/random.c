#include "page.h"
#include <stdlib.h> // rand()

// RANDOM: evict a uniformly random occupied frame.
void R_FUNCTION(LISTOFPAGES* pl) {
    // count occupied frames
    int occ = 0;
    for (page* it = pl->HDL; it; it = it->next) {
        if (it->pid != -1) occ++;
    }
    if (occ == 0) return; // nothing to evict

    int k = rand() % occ;  // pick k-th occupied

    page* victim = NULL;
    for (page* it = pl->HDL; it; it = it->next) {
        if (it->pid == -1) continue;
        if (k == 0) { victim = it; break; }
        k--;
    }
    if (!victim) return; // safety

    if (DEBUG==1)
        printf("EVICTED (RANDOM) :: p[%03d] c:%02d last:%0.2f first:%0.2f\n",
               victim->pid, victim->CNTER, victim->LONE, victim->FTBOUGHT);

    victim->pid       = -1;
    victim->PGENUMBER = -1;
}
