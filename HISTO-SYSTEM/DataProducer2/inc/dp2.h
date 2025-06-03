/*
 * FILE             : dp2.h
 * PROJECT          : A5 - The Histogram System
 * By               : Anthony Moronta - 8819338, Brayan Rojas - 8829975
 * FIRST VERSION    : April 06, 2025
 * DESCRIPTION      : Header file for Data Producer 2. Defines function prototypes
 *                    and global variables used in dp2.c.
 */

#ifndef DP2_H
#define DP2_H

#include "../../Common/shared_defs.h"

void cleanup(int sig);
void lock();
void unlock();

#endif

