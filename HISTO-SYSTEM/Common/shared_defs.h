/*
 * FILE             : shared_defs.h
 * PROJECT          : A5 - The Histogram System
 * By               : Anthony Moronta - 8819338, Brayan Rojas - 8829975
 * FIRST VERSION    : April 06, 2025
 * DESCRIPTION      : This file defines shared constants and data structures used by DP-1, DP-2, and DC.
 *                    It includes the structure for the shared memory buffer, buffer size, and letter range.
 *                    It also defines the constants for using ftok() to generate a portable key for semaphores,
 *                    helping all processes use the same semaphore key without hardcoding it.
 */
#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H

#include <sys/types.h>

#define SHM_KEY 1234
#define FTOK_PATH "."      // Current directory; could be any existing file path
#define FTOK_SEM_ID 'S'    // Unique project ID for semaphore


#define BUFFER_SIZE 256
#define LETTER_RANGE 20 // A-T

// Histogram letters: 'A' = index 0, ..., 'T' = index 19
#define LETTER_OFFSET 'A'

// Structure stored in shared memory
typedef struct {
    char buffer[BUFFER_SIZE];
    int readIndex;
    int writeIndex;
} SharedData;

#endif

