#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "page.h"

// Initialize all pages
void Intializing_PGELIst(LISTOFPAGES* pl) {
    pl->HDL = malloc(sizeof(page));
    page* it = pl->HDL;
    int i;
    for (i = 0; i < Toal_PAGE; i++) {
        it->pid = -1;
        it->PGENUMBER = -1;
        it->next = NULL;
        if (i < Toal_PAGE - 1) {  // safer than hardcoded 99
            it->next = malloc(sizeof(page));
            it = it->next;
        }
    }
}

// Display all pages
void PageeDisplayingg(LISTOFPAGES* pl) {
    page* it = pl->HDL;
    int cnt = 0;
    while (it) {
        printf(it->pid > 0 ? "%03d " : " .  ", it->pid);
        cnt++;
        if ((cnt % 10) == 0) printf("\n");
        it = it->next;
    }
    printf("\n");
}

// Check if enough free pages exist
int PageWhoRFree(LISTOFPAGES* pl, int CNTER) {
    page* it = pl->HDL;
    while (it) {
        if (it->pid == -1) {  // page not being used by any process
            CNTER--;
        }
        if (!CNTER) return 1;
        it = it->next;
    }
    return 0;
}

// Check if a process already has a page in memory
int MEMinPGES(LISTOFPAGES* pl, int pid, int PGENUMBER) {
    page* it = pl->HDL;
    while (it) {
        if (it->pid == pid && it->PGENUMBER == PGENUMBER) return 1;
        it = it->next;
    }
    return 0;
}

// Find the first free page
page* PAGEfrreeeg(LISTOFPAGES* pl) {
    page* it = pl->HDL;
    while (it) {
        if (it->pid == -1) return it;
        it = it->next;
    }
    return NULL;
}

// Free all pages belonging to a given process
void MEMRFree(LISTOFPAGES* pl, int pid) {
    page* it = pl->HDL;
    while (it) {
        if (it->pid == pid) {
            it->pid = -1;
            it->PGENUMBER = -1;
        }
        it = it->next;
    }
}

// Generate the next page number randomly
int PGNUMNXT(int curr_page_no, int max_page_size) {
    int x = rand() % 10;
    if (x < 7) {
        x = curr_page_no + (rand() % 3) - 1;
        if (x < 0) x = 0;  // safety check
    } else {
        x = rand() % max_page_size;
        while (abs(x - curr_page_no) <= 1)
            x = rand() % max_page_size;
    }
    return x;
}

// Find a specific page belonging to a process
page* IDwhozPGrFree(LISTOFPAGES* pl, int pid, int PGENUMBER) {
    page* it = pl->HDL;
    while (it) {
        if (it->pid == pid && it->PGENUMBER == PGENUMBER) return it;
        it = it->next;
    }
    return NULL;
}

// Compare processes by arrival time (for sorting)
int CMP_ARRtime(const void* a, const void* b) {
    return ((process*)a)->TIMEARR - ((process*)b)->TIMEARR;
}

// --------------------------
// Test main() function
// --------------------------
// int main() {
//     LISTOFPAGES list;

//     // Initialize the page list
//     Intializing_PGELIst(&list);

//     printf("Initial page table:\n");
//     PageeDisplayingg(&list);

//     // Example check for free pages
//     if (PageWhoRFree(&list, 5))
//         printf("There are at least 5 free pages.\n");
//     else
//         printf("Not enough free pages.\n");

//     return 0;
// }
