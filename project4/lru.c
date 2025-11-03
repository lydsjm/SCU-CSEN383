#include "page.h"

// LRU algorithm
void LRU_FUNCTION(LISTOFPAGES* pl)
{
    // Start with the first page in the list as the initial candidate for eviction
    page* page_iterator = pl->HDL;
    page* page_to_evict = pl->HDL;

    // Iterate through all pages in memory to find the one to evict
    while (page_iterator) {
        // Evict the page that was least recently used (smallest LONE)
        if (page_iterator->LONE < page_to_evict->LONE) {
            page_to_evict = page_iterator;
        }
        page_iterator = page_iterator->next;
    }

    // Eviction details
    if (DEBUG==1) {
        printf("EVICTED (LRU): p[%03d] page:%02d last_accessed:%0.2f first_brought_in:%0.2f\n",
        page_to_evict->pid, page_to_evict->PGENUMBER, page_to_evict->LONE, page_to_evict->FTBOUGHT);
    }

    // Evict the page by resetting its process ID and page number
    // Make the frame available
    page_to_evict->pid = -1;
    page_to_evict->PGENUMBER = -1;
}
