/*
 * FILE             : dc.h
 * PROJECT          : A5 - The Histogram System
 * By               : Anthony Moronta - 8819338, Brayan Rojas - 8829975
 * FIRST VERSION    : April 06, 2025
 * DESCRIPTION      : Header file for Data Consumer. Defines function prototypes
 *                    and global variables used in dc.c.
 */

#ifndef DC_H
#define DC_H

#include "../../Common/shared_defs.h"

void lock();
void unlock();
void printHistogram();
void shutdownHandler(int sig);

#endif

