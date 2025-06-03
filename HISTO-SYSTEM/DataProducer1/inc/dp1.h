/*
 * FILE             : dp1.h
 * PROJECT          : A5 - The Histogram System
 * By               : Anthony Moronta - 8819338, Brayan Rojas - 8829975
 * FIRST VERSION    : April 06, 2025
 * DESCRIPTION      : Header file for Data Producer 1. Defines function prototypes
 *                    and global variables used in dp1.c.
 */

#ifndef DP1_H
#define DP1_H

#include "../../Common/shared_defs.h"

void cleanup(int sig);
void lock();
void unlock();

#endif

