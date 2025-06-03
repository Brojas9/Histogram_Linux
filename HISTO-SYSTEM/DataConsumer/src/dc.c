/*
 * FILE             : dc.c
 * PROJECT          : A5 - The Histogram System
 * By               : Anthony Moronta - 8819338, Brayan Rojas - 8829975
 * FIRST VERSION    : April 06, 2025
 * DESCRIPTION      : The Data Consumer reads letters (A-T) from the shared buffer, updates a histogram,
 *                    and prints it every 10 seconds. It sleeps for 2 seconds between reads.
 *                    On SIGINT, it signals DP-1 and DP-2 to terminate, drains the buffer, and exits.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "../Common/shared_defs.h"
#include "../inc/dc.h"

int shmID = -1, semID = -1;
SharedData *shared = NULL;
int histogram[LETTER_RANGE] = {0};
int secondsPassed = 0;
pid_t dp1_pid = -1, dp2_pid = -1;
volatile sig_atomic_t alarmTriggered = 0;

/*  -- Function Header Comment
  Name    : alarmHandler
  Purpose : Sets a flag (alarmTriggered) whenever the SIGALRM signal is received.
            This flag is used in the main loop to trigger timed execution.
  Inputs  : sig - signal number received (should be SIGALRM)
  Outputs : Sets global flag
  Returns : void
*/
void alarmHandler(int sig) {
    alarmTriggered = 1;
}



/*  -- Function Header Comment
  Name    : lock
  Purpose : Performs a semaphore wait (P operation) to safely lock access to the shared memory,
            preventing other processes from writing while the consumer reads data.
  Inputs  : None
  Outputs : None
  Returns : void
*/
void lock() {
    struct sembuf op = {0, -1, 0};
    if (semop(semID, &op, 1) == -1) {
        perror("[DC] semop (lock) failed");
        exit(1);
    }
}


/*  -- Function Header Comment
  Name    : unlock
  Purpose : Performs a semaphore signal (V operation) to release the lock on shared memory,
            allowing other processes to access it after the consumer finishes reading.
  Inputs  : None
  Outputs : None
  Returns : void
*/
void unlock() {
    struct sembuf op = {0, 1, 0};
    if (semop(semID, &op, 1) == -1) {
        perror("[DC] semop (unlock) failed");
        exit(1);
    }
}


/*  -- Function Header Comment
  Name    : printHistogram
  Purpose : Displays the current letter frequency histogram using a visual format. 
            Each letter (A–T) is shown with a three-digit count followed by symbols:
            '*' for hundreds, '+' for tens, and '-' for units.
  Inputs  : None
  Outputs : Prints the histogram to the console
  Returns : void
*/
void printHistogram() {

    system("clear");// Clears the terminal screen before printing

    printf("\n--- HISTOGRAM ---\n");

    for (int i = 0; i < LETTER_RANGE; i++) {
        int count = histogram[i];
        char letter = (char)(LETTER_OFFSET + i);

        // Print label like A-058
        printf("%c-%03d ", letter, count);

        // Calculate number of symbols
        int stars = count / 100;
        int pluses = (count % 100) / 10;
        int dashes = count % 10;

        // Print symbols for each level
        for (int j = 0; j < stars; j++) printf("*");
        for (int j = 0; j < pluses; j++) printf("+");
        for (int j = 0; j < dashes; j++) printf("-");

        printf("\n");
    }

    printf("------------------\n");
}



/*  -- Function Header Comment
  Name    : shutdownHandler
  Purpose : Handles the SIGINT signal sent to the Data Consumer. It terminates both producer
            processes, drains remaining data from the shared buffer, prints the final histogram,
            and releases all shared memory and semaphore resources before exiting.
  Inputs  : sig - the signal number received (typically SIGINT)
  Outputs : Console messages indicating shutdown progress and the final histogram
  Returns : void
*/
void shutdownHandler(int sig) {

    printf("\n[DC] SIGINT received. Shutting down...\n");

    if (dp1_pid != -1 && kill(dp1_pid, SIGINT) == -1) {

        perror("[DC] Failed to send SIGINT to DP-1");
    }


    if (dp2_pid != -1 && kill(dp2_pid, SIGINT) == -1) {

        perror("[DC] Failed to send SIGINT to DP-2");
    }

    // Drain the buffer
    lock();
    while (shared->readIndex != shared->writeIndex) {

        char letter = shared->buffer[shared->readIndex];
        int idx = letter - LETTER_OFFSET;
        if (idx >= 0 && idx < LETTER_RANGE) histogram[idx]++;
        shared->readIndex = (shared->readIndex + 1) % BUFFER_SIZE;
    }
    unlock();

    printHistogram();

    shmdt(shared);
    shmctl(shmID, IPC_RMID, NULL);
    semctl(semID, 0, IPC_RMID);

    printf("Shazam !!\n");
    exit(0);
}


/*  -- Function Header Comment
  Name    : main
  Purpose : Entry point for the Data Consumer (DC). It attaches to shared memory and semaphore,
            reads characters written by the producers, and updates a histogram of letter counts.
            It also handles periodic printing and responds to SIGINT for graceful shutdown.
  Inputs  : argc - number of command-line arguments (expects 4)
            argv - array of argument strings: <shmID> <dp1PID> <dp2PID>
  Outputs : Console messages for histogram updates and shutdown status
  Returns : int
*/
int main(int argc, char *argv[]) {

    // Expecting 3 arguments
    if (argc != 4) {

        fprintf(stderr, "Usage: %s <shmID> <dp1PID> <dp2PID>\n", argv[0]);
        exit(1);
    }

    // Register the signal handler for SIGINT (Ctrl+C)
    if (signal(SIGINT, shutdownHandler) == SIG_ERR) {

        perror("[DC] signal registration failed");
        exit(1);

    }

    // Parse arguments: shared memory ID and the PIDs of DP-1 and DP-2
    shmID = atoi(argv[1]);
    dp1_pid = atoi(argv[2]);
    dp2_pid = atoi(argv[3]);


    // Attach to the shared memory segment
    shared = (SharedData *)shmat(shmID, NULL, 0);
    if (shared == (void *) -1) { 
        perror("shmat"); 
        exit(1); 
    }


    // Access the semaphore created by the producer
    key_t semKey = ftok(FTOK_PATH, FTOK_SEM_ID);
    if (semKey == -1) { 
        perror("ftok"); 
        exit(1); 
    }

    semID = semget(semKey, 1, 0);
    if (semID < 0) {
        perror("semget");   
        exit(1); 
    }


    // Start main processing loop
        // Register SIGALRM once
    if (signal(SIGALRM, alarmHandler) == SIG_ERR) {
        perror("[DC] signal(SIGALRM) registration failed");
        exit(1);
    }

    // Set timer for SIGALRM every 2 seconds
    struct itimerval timer;
    timer.it_value.tv_sec = 2;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 2;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("[DC] setitimer failed");
        exit(1);
    }

    // Main logic loop
    while (1) {
        while (!alarmTriggered) {
            pause(); // Wait for timer signal
        }

        alarmTriggered = 0;

        // Check if DP2 is alive
        if (kill(dp2_pid, 0) == -1 && errno == ESRCH) {
            fprintf(stderr, "[DC] Warning: DP2 process has terminated unexpectedly.\n");
            shutdownHandler(SIGINT);
        }

        lock();

        while (shared->readIndex != shared->writeIndex) {
            char letter = shared->buffer[shared->readIndex];
            int idx = letter - LETTER_OFFSET;
            if (idx >= 0 && idx < LETTER_RANGE) histogram[idx]++;
            shared->readIndex = (shared->readIndex + 1) % BUFFER_SIZE;
        }

        unlock();

        secondsPassed += 2;
        if (secondsPassed >= 10) {
            printHistogram();
            secondsPassed = 0;
        }
    }

    return 0;
}
