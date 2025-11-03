#include <ctype.h>
#include "page.h"

int main(int Argument1, char *Argument2[])
{

    int TimeStamp = 0; // simulator timestamp
    int pageHits = 0;
    int pageMisses = 0;

    // paging options (MB)
    int *PGCoptn = malloc(sizeof(int) * 4);
    PGCoptn[0] = 5;
    PGCoptn[1] = 11;
    PGCoptn[2] = 17;
    PGCoptn[3] = 31;

    // pointer to the Page
    page *Page_pointer;

    void (*AlgoFunction)(LISTOFPAGES *);
    if (Argument1 != 2)
    {
        printf("####################################################################\n");
        printf("Usage: %s <algorithm>\n", Argument2[0]);
        printf("Algorithms to run : fifo, lru, lfu, mfu or random.\n");
        printf("####################################################################\n");
        return -1;
    } // checking which function to call
    if (strcmp(Argument2[1], "fifo") == 0)
    {
        AlgoFunction = FIFO_FUNCTION;
    }
    else if (strcmp(Argument2[1], "lru") == 0)
    {
        AlgoFunction = LRU_FUNCTION;
    }
    else if (strcmp(Argument2[1], "lfu") == 0)
    {
        AlgoFunction = LFU_FUNCTION;
    }
    else if (strcmp(Argument2[1], "mfu") == 0)
    {
        AlgoFunction = MFU_FUNCTION;
    }
    else if (strcmp(Argument2[1], "random") == 0)
    {
        AlgoFunction = R_FUNCTION;
    }
    else
    {
        printf("####################################################################\n");
        printf("Algorithms to run : fifo, lru, lfu, mfu or random.\n");
        printf("####################################################################\n");

        return -1;
    }

    int swappingInProcess = 0; // counting number of processes swapped in

    srand(0);
    int i;
    for (i = 0; i < 5; i++)
    {
        printf("------======= Running Simulator   %d ========-------\n", i + 1);

        LISTOFPAGES pl;
        Intializing_PAGELIst(&pl);
        process Q[Total_PROCESS];

        int i;
        for (i = 0; i < Total_PROCESS; i++)
        {
            Q[i].pid = i;
            Q[i].PGECNTER = PGCoptn[rand() % 4];
            Q[i].TIMEARR = rand() % 60;
            Q[i].DRUTION = rand() % PROCss_DuraTN; // service duration between 1 to 5 seconds
            Q[i].PGCRR = 0;                        // all processes begin with page 0
        }

        qsort(Q, Total_PROCESS, sizeof(process), CMP_ARRtime); // sort by arrival time

        int index = 0; // index to the start of process queue
        for (TimeStamp = 0; TimeStamp < TotaL_DURATION; TimeStamp++)
        {

            // looking for new process at start of every second
            while (index < Total_PROCESS && Q[index].TIMEARR <= TimeStamp)
            {

                // To check at least four pages
                if (PageWhoAreFree(&pl, 4))
                {
                    // if its present, then bring it in the memory
                    page *p = PAGEfrreeeg(&pl); // get a free page
                    p->pid = Q[index].pid;
                    p->PGENUMBER = Q[index].PGCRR;
                    p->FTBOUGHT = 1.0 * TimeStamp;
                    p->CNTER = 1;
                    p->LONE = TimeStamp;
                    printf("Page::: %d for process id::: %d brought in at ::: %f\n", Q[index].PGCRR, Q[index].pid, p->FTBOUGHT);
                    swappingInProcess++;
                    index++;
                }
                else
                    break;
            }

            // Update the page table every second
            int i;
            for (i = 0; i < 10; i++)
            {
                int j;
                for (j = 0; j < index; j++)
                    if (Q[j].DRUTION > 0) // process is still running
                    {
                        Q[j].PGCRR = PGNUMNXT(Q[j].PGCRR, Q[j].PGECNTER); // get next page number

                        if (MEMinPAGES(&pl, Q[j].pid, Q[j].PGCRR)) // page is already in memory
                        {

                            Page_pointer = IDwhozPGrFree(&pl, Q[j].pid, Q[j].PGCRR); // get pointer to that page
                            if (Page_pointer == NULL)
                            {
                                printf("Theres is bug , got null : pid %d page:: %d\n", Q[j].pid, Q[j].PGCRR);
                                return -1;
                            }

                            Page_pointer->CNTER++;
                            Page_pointer->LONE = TimeStamp;

                            // page hit
                            pageHits++;
                            continue;
                        }

                        // if we are here then that means we refered a page which is not there in memory. So we need to bring it in.
                        // page miss
                        pageMisses++;

                        page *pageeeg = PAGEfrreeeg(&pl); // get a free page
                        if (!pageeeg)
                        {
                            printf("####################################################################\n");

                            printf("Memory is full, Page list:\n");
                            PageDisplaying(&pl); // display current pages

                            AlgoFunction(&pl); // call the replacement algorithm
                            PageDisplaying(&pl); // display pages after replacement

                            pageeeg = PAGEfrreeeg(&pl); // get a free page
                        }
                        pageeeg->pid = Q[j].pid;
                        pageeeg->PGENUMBER = Q[j].PGCRR;
                        pageeeg->FTBOUGHT = TimeStamp + (0.1 * i); // slight offset to avoid ties
                        pageeeg->LONE = TimeStamp + (0.1 * i);   // last accessed time
                        pageeeg->CNTER = 1;
                        printf("Page::: %d for process %d brought in at %f\n", Q[j].PGCRR, Q[j].pid, pageeeg->FTBOUGHT);
                        swappingInProcess++;
                    }
            }

            // Update the page table every second
            int j;
            for (j = 0; j < index; j++)
                if (Q[j].DRUTION > 0) // process is still running
                {
                    Q[j].DRUTION--;
                    if (Q[j].DRUTION == 0) // process is done
                    {
                        printf("####################################################################\n");
                        printf("Process id %d is done. Memory is getting free ... \n", Q[j].pid);
                        printf("####################################################################\n");
                        MEMRFree(&pl, Q[j].pid);
                    }
                }
            usleep(900);
        }
    }

    // ...

    printf("Page hit rate: %.2f%%\n", ((float)pageHits / (pageHits + pageMisses)) * 100);
    printf("Page miss rate: %.2f%%\n", ((float)pageMisses / (pageHits + pageMisses)) * 100);
    printf("Average number of processes that were successfully swapped in %d\n", (swappingInProcess / 5));

    return 0;
}
