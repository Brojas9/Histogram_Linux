/*
 * FILE             : dp1.c
 * PROJECT          : A5 - The Histogram System
 * By               : Anthony Moronta - 8819338, Brayan Rojas - 8829975
 * FIRST VERSION    : April 06, 2025
 * DESCRIPTION      : DataProducer-1 is the first data producer. It creates shared memory and semaphore,
 *                    initializes the circular buffer, forks DataProducer-2, and writes 20 random letters
 *                    (A-T) every 2 seconds into the shared buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <time.h>
#include "../Common/shared_defs.h"
#include "../inc/dp1.h"

int shmID = -1, semID = -1;
SharedData *shared = NULL;


/*  -- Function Header Comment
  Name    : cleanup
  Purpose : Handles the SIGINT signal by detaching the shared memory and printing
            a shutdown message before the program exits.
  Inputs  : sig - the signal number received (typically SIGINT)
  Outputs : A shutdown message printed to the console
  Returns : void
*/
void cleanup(int sig) {
    if (shared) shmdt(shared);
    printf("\nDP-1 exiting.\n");
    exit(0);
}


/*  -- Function Header Comment
  Name    : lock
  Purpose : Performs a semaphore wait (P operation) to lock access to the shared memory,
            ensuring that only one process can modify it at a time.
  Inputs  : None
  Outputs : None
  Returns : Nothing (void)
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
  Purpose : Performs a semaphore signal (V operation) to unlock access to the shared memory,
            allowing other processes to access it.
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
  Name    : main
  Purpose : Entry point for DP-1. It creates and initializes shared memory and a semaphore,
            forks the DP-2 process, and writes 20 random letters (A–T) into the shared buffer
            every 2 seconds.
  Inputs  : None
  Outputs : Writes to the shared memory buffer; displays error messages to the console if issues occur
  Returns : int
*/
int main() {

    // Register the cleanup function to handle SIGINT (Ctrl+C)
    if (signal(SIGINT, cleanup) == SIG_ERR) {

        perror("[DP-1] signal registration failed");
        exit(1);
    }


    // Seed the random number generator with the current time
    srand(time(NULL));

    // Create a new shared memory segment
    shmID = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmID < 0) { perror("shmget"); exit(1); }

    // Attach the shared memory segment to this process
    shared = (SharedData *)shmat(shmID, NULL, 0);
    if (shared == (void *) -1) { perror("shmat"); exit(1); }

    // Initialize buffer and indices
    memset(shared->buffer, 0, BUFFER_SIZE);
    shared->readIndex = 0;
    shared->writeIndex = 0;

    // Create semaphore
    key_t semKey = ftok(FTOK_PATH, FTOK_SEM_ID);
    if (semKey == -1) { perror("ftok"); exit(1); }

    semID = semget(semKey, 1, IPC_CREAT | 0666);
    if (semID < 0) { perror("semget"); exit(1); }

    if (semID < 0) { perror("semget"); exit(1); }
    semctl(semID, 0, SETVAL, 1);

    // Fork a child process to run DataProducer2 (DP-2)
    pid_t pid = fork();
    if (pid == 0) {
        char shmStr[10];
        snprintf(shmStr, sizeof(shmStr), "%d", shmID);
        execl("./DataProducer2/bin/dp2", "dp2", shmStr, NULL);
        perror("execl DP-2");
        exit(1);
    }

    // Main loop: write 20 random letters (A-T) into the buffer every 2 seconds
    while (1) {

        // Lock the semaphore to enter critical section
        lock();

        // Calculate available space in the circular buffer
        int space = (shared->readIndex > shared->writeIndex) ?
                    shared->readIndex - shared->writeIndex - 1 :
                    BUFFER_SIZE - (shared->writeIndex - shared->readIndex);

        // Write up to 20 letters depending on available space
        int count = (space >= 20) ? 20 : space;
        for (int i = 0; i < count; i++) {
            shared->buffer[shared->writeIndex] = (char)(LETTER_OFFSET + rand() % LETTER_RANGE);
            shared->writeIndex = (shared->writeIndex + 1) % BUFFER_SIZE;
        }

        unlock(); // Unlock the semaphore
        sleep(2); // Sleep for 2 seconds before writing again
    }

    return 0;
}

