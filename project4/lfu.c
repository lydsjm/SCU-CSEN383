#include "page.h"

// LFU algorithm
void LFU_FUNCTION(LISTOFPAGES* pl) {
    // Start with the first page in the list as the initial candidate for eviction
    page* page_iterator = pl->HDL;
    page* page_to_evict = pl->HDL;

    // Iterate through all pages in memory to find the one to evict
    while (page_iterator) {
        // Evict the page with the lowest reference count (CNTER)
        if (page_iterator->CNTER < page_to_evict->CNTER) {
            page_to_evict = page_iterator;
        }
        // If 2 pages have the same count, the one that was brought into memory first is picked
        // Evict the page that has been in the memory the longest (FIFO tie-breaker)
        // Compare the FTBOUGHT timestamps, and the page with the earlier timestamp (smaller FTBOUGHT value) is chosen
        else if (page_iterator->CNTER == page_to_evict->CNTER) {
            if (page_iterator->FTBOUGHT < page_to_evict->FTBOUGHT) {
                page_to_evict = page_iterator;
            }
        }
        page_iterator = page_iterator->next;
    }

    // Eviction details
    if (DEBUG==1) {
        printf("EVICTED (LFU): p[%03d] page:%02d count:%02d first_brought_in:%02f\n",
        page_to_evict->pid, page_to_evict->PGENUMBER, page_to_evict->CNTER, page_to_evict->FTBOUGHT);
    }

    // Evict the page by resetting its process ID and page number
    // Make the frame available
    page_to_evict->pid = -1;
    page_to_evict->PGENUMBER = -1;
}
