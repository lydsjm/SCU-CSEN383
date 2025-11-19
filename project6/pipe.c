#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>

#define BUFFER_SIZE 100
#define READ_END 0
#define WRITE_END 1
#define NUM_PIPES 5
#define PIPE_DURATION 30


fd_set set_fds, active_fds;
FILE *log_file;
char read_buffer[BUFFER_SIZE];
char input_buffer[BUFFER_SIZE-10];
int pipes[NUM_PIPES][2];
int is_timeout = 0;
int msg_counter = 1;
struct timeval start_timeval;
struct itimerval count_down_timer;
time_t start_time;

// read from pipe
void readFromPipe(int *pipe_descriptor, int end_of_pipe, int pipe_index)
{
    (void)pipe_descriptor;  // explicitly mark as unused
    if (!is_timeout)
    {
        // close(pipe_descriptor[WRITE_END]);
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        // time elapsed
        float elapsed_time = (float)((current_time.tv_sec - start_timeval.tv_sec) + ((current_time.tv_usec - start_timeval.tv_usec) / 1000000.));
        // read from pipe end to buffer
        ssize_t n = read(end_of_pipe, read_buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            // EOF or error, nothing to log
            close(end_of_pipe);
            FD_CLR(end_of_pipe, &active_fds);
            return;
        }
        read_buffer[n] = '\0';
        if (pipe_index == 4)
        {
            fprintf(log_file, "%6.3f: %s", elapsed_time, read_buffer);
        }
        else
        {
            fprintf(log_file, "%6.3f: %s\n", elapsed_time, read_buffer);
        }
        fflush(log_file);
    }
}
// write into pipe
void writeToPipe(int *pipe_descriptor)
{
    if (is_timeout) {
        return;   // timer expired, don't bother writing
    }
    size_t len = strlen(read_buffer);
    if (len >= BUFFER_SIZE) {
        len = BUFFER_SIZE - 1;   // safety clamp
    }
    // write into the WRITE end of this pipe
    write(pipe_descriptor[WRITE_END], read_buffer, len);
}
// interrupt signal
void alarmSignal(int sig)
{
    (void)sig;
    is_timeout = 1;  // Set timeout flag to signal that 30 seconds have elapsed
}

int main()
{
    log_file = fopen("output.txt", "w"); // open log file
    time(&start_time); // Record the start time
    count_down_timer.it_value.tv_sec = PIPE_DURATION; // timeout duration
    count_down_timer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &count_down_timer, NULL);
    gettimeofday(&start_timeval, NULL);
    signal(SIGALRM, alarmSignal); // alarm signal handler
    srand(time(NULL));
    int random_seed;
    FD_ZERO(&active_fds);
    int i, selected_pipe;
    pid_t process_id;
    int max_fd = -1;  
    pid_t child_pids[NUM_PIPES];
    // child processes and their associated pipes
    for (i = 0; i < NUM_PIPES; i++)
    {
       // Create the pipe
       if (pipe(pipes[i]) == -1) {
           perror("pipe");
           exit(1);
       }
       // Fork a child process
       process_id = fork();
       if (process_id < 0) {
        perror("fork");
        exit(1);
       }
       // Child process
       if (process_id == 0) {
        // Child shouldn't use the read end of the pipe
        close(pipes[i][READ_END]);
        break;
       }
       // Parent process
        child_pids[i] = process_id;
        close(pipes[i][WRITE_END]);              // parent only reads
        FD_SET(pipes[i][READ_END], &active_fds); // add to select set
        if (pipes[i][READ_END] > max_fd) {
            max_fd = pipes[i][READ_END];
        }
    }
        // Main loop
    while (!is_timeout)
    {
        // Parent Process
        if (process_id > 0)
        {
            fd_set temp_fds = active_fds;
            struct timeval tv;
            tv.tv_sec  = 1;      // check once per second
            tv.tv_usec = 0;
            int ready = select(max_fd + 1, &temp_fds, NULL, NULL, &tv);
            if (ready < 0) {
                if (is_timeout) break;  // interrupted by alarm
                perror("select");
                break;
            }
            if (ready == 0) {
                // no data ready this second, loop again
                continue;
            }
            // Check which pipes have data and read them
            for (int k = 0; k < NUM_PIPES; k++) {
                int fd = pipes[k][READ_END];
                if (FD_ISSET(fd, &temp_fds)) {
                    // use provided helper so we get timestamped logging
                    readFromPipe(pipes[k], fd, k);
                }
            }
        }
        // Child Process
        else
        {
            if (is_timeout) break;  // stop generating messages after 30s
            if (i == 4)
            {
                printf("Input for Child 5:: ");
                if (fgets(input_buffer, BUFFER_SIZE, stdin) == NULL) {
                    // EOF or error, just stop this child
                    break;
                }
                // Calculate child's timestamp
                struct timeval child_time;
                gettimeofday(&child_time, NULL);
                float child_elapsed = (float)((child_time.tv_sec - start_timeval.tv_sec) +
                                    ((child_time.tv_usec - start_timeval.tv_usec) / 1000000.));
                // Strip newline from input
                input_buffer[strcspn(input_buffer, "\n")] = '\0';
                snprintf(read_buffer, BUFFER_SIZE,"%6.3f: Child 5: %.70s\n", child_elapsed, input_buffer);
                writeToPipe(pipes[i]);
            }
            else
            {
                struct timeval child_time;
                gettimeofday(&child_time, NULL);
                float child_elapsed = (float)((child_time.tv_sec - start_timeval.tv_sec) +
                                    ((child_time.tv_usec - start_timeval.tv_usec) / 1000000.));

                snprintf(read_buffer, BUFFER_SIZE,
                        "%6.3f: Child %d Message %d  ", child_elapsed, i + 1, msg_counter++);
                writeToPipe(pipes[i]);
                usleep(100);
                sleep(rand() % 3); // random sleep duration (0, 1, or 2 seconds)
            }
        }
    }
    fclose(log_file); // close log file
    exit(0);
}
