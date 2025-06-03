/*
 * FILE             : dp2.c
 * PROJECT          : A5 - The Histogram System
 * By               : Anthony Moronta - 8819338, Brayan Rojas - 8829975
 * FIRST VERSION    : April 06, 2025
 * DESCRIPTION      : DataProducer-2 is the second data producer. It receives the shared memory ID from DataProducer-1,
 *                    forks and launches the DC (data consumer), then writes one random letter (A-T)
 *                    every 1/20 of a second into the shared buffer.
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
#include <sys/types.h>
#include <sys/wait.h>
#include "../Common/shared_defs.h"
#include "../inc/dp2.h"

int shmID = -1, semID = -1;
SharedData *shared = NULL;
pid_t parentPID;


/*  -- Function Header Comment
  Name    : cleanup
  Purpose : Handles the SIGINT signal by detaching the shared memory and 
            printing a message before the program exits.
  Inputs  : sig - the signal number received (expected to be SIGINT)
  Outputs : A message to the console indicating the program is exiting
  Returns : Void
*/
void cleanup(int sig) {

    if (shared) shmdt(shared);
    printf("\nDP-2 exiting.\n");

    exit(0);
}


/*  -- Function Header Comment
  Name    : lock
  Purpose : Locks access to the shared memory by performing a semaphore wait (P operation).
            This prevents other processes from modifying the shared memory at the same time.
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
  Purpose : Unlocks access to the shared memory by performing a semaphore signal (V operation).
            This allows other processes to safely access the shared memory.
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
  Purpose : Entry point for DP-2. Attaches to shared memory, launches the data consumer process,
            and writes one random letter (A–T) into the buffer every 1/20 of a second.
  Inputs  : argc - number of command-line arguments
            argv - array of argument strings (expects shared memory ID at argv[1])
  Outputs : Writes to shared memory buffer; prints errors to the console if any occur
  Returns : int
*/
int main(int argc, char *argv[]) {

    // Check if the user provided the shared memory ID as a command-line argument
    if (argc != 2) {

        fprintf(stderr, "Usage: %s <shmID>\n", argv[0]);
        exit(1); // Exit if missing
    }

    // Register cleanup function for Ctrl+C
    if (signal(SIGINT, cleanup) == SIG_ERR) {
        perror("[DP-1] signal registration failed");
        exit(1);
    }

    srand(time(NULL));    // Seed the random number generator

    shmID = atoi(argv[1]);  // Convert the shmID from string to integer
    parentPID = getppid();  // Get the PID of the parent process (DP-1)
    pid_t selfPID = getpid(); // Get the PID of this process (DP-2)

    // Fork and launch DC
    pid_t pid = fork();
    if (pid == 0) {

        // Prepare the arguments for dc: shared memory ID and PIDs
        char shmStr[10], pid1Str[10], pid2Str[10];
        snprintf(shmStr, sizeof(shmStr), "%d", shmID);
        snprintf(pid1Str, sizeof(pid1Str), "%d", parentPID);
        snprintf(pid2Str, sizeof(pid2Str), "%d", selfPID);

        // Execute the dc program with the given arguments
        execl("./DataConsumer/bin/dc", "dc", shmStr, pid1Str, pid2Str, NULL);
        perror("execl DC"); // If execl fails, print error and exit

        exit(1);
    }

    // Attach to shared memory
    shared = (SharedData *)shmat(shmID, NULL, 0);
    if (shared == (void *) -1) { perror("shmat"); exit(1); }

    // Access existing semaphore
    key_t semKey = ftok(FTOK_PATH, FTOK_SEM_ID);
    if (semKey == -1) { perror("ftok"); exit(1); }

    semID = semget(semKey, 1, 0);
    if (semID < 0) { perror("semget"); exit(1); }

    if (semID < 0) { perror("semget"); exit(1); }

    // Infinite loop to write random letters (A-T) into the shared buffer
    while (1) {

        // Lock semaphore before accessing shared memory
        lock();

        // Calculate the next position to write
        int nextIndex = (shared->writeIndex + 1) % BUFFER_SIZE;

        // Only write if the buffer is not full
        if (nextIndex != shared->readIndex) {

            // Write a random letter (A-T) to the buffer
            shared->buffer[shared->writeIndex] = (char)(LETTER_OFFSET + rand() % LETTER_RANGE);
            // Move write index forward
            shared->writeIndex = nextIndex;
        }

        unlock(); // Unlock semaphore
        usleep(50000); // Wait for 1/20th second
    }

    return 0;
}

