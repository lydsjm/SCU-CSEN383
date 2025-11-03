#ifndef PAGING_H
#define PAGING_H

#define DEBUG 0

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define Total_PROCESS 300
#define TotaL_DURATION 60
#define PROCss_DuraTN 5
#define Toal_PAGE 100

#define Intializing_PAGELIst Intializing_PGELIst
#define PageWhoAreFree       PageWhoRFree
#define MEMinPAGES           MEMinPGES
#define PageDisplaying       PageeDisplayingg

extern int Simulation_CLOCK;
extern int *PAGINGOPtns;

typedef struct {
    int pid;            // process id
    int PGECNTER;       // page count
    int TIMEARR;        // arrival time
    int DRUTION;        // service duration
    int PGCRR;          // current page number
} process;

typedef struct page {
    int pid;            // process id
    int PGENUMBER;      // page number
    struct page* next;  // pointer to the next page

    float FTBOUGHT;     // time brought in
    float LONE;         // last accessed time
    int CNTER;          // reference counter
} page;

typedef struct {
    page* HDL;          // head pointer to the list of pages
} LISTOFPAGES;

// pages which are free
int PageWhoRFree(LISTOFPAGES*,int);

//Existing pages in memory
int MEMinPGES(LISTOFPAGES*,int,int);

page* PAGEfrreeeg(LISTOFPAGES*);

// freeing memortyss
void MEMRFree(LISTOFPAGES*,int);

//initializing all pages
void Intializing_PGELIst(LISTOFPAGES*);

/// displaying all pages
void PageeDisplayingg(LISTOFPAGES*);

// generate next page number randomly
int PGNUMNXT(int,int);

// comparision between arrival time
int CMP_ARRtime(const void* ,const void*);

// Find a specific page belonging to a process
page* IDwhozPGrFree(LISTOFPAGES*,int,int);

void FIFO_FUNCTION(LISTOFPAGES*);
void LRU_FUNCTION(LISTOFPAGES*);
void LFU_FUNCTION(LISTOFPAGES*);
void MFU_FUNCTION(LISTOFPAGES*);
void R_FUNCTION(LISTOFPAGES*);

#endif
