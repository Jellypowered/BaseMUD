/***************************************************************************
 * Exploration tracking system - tracks rooms visited by player characters
 * Adapted from snippet by (unknown author) and optimized for dynamic buffers
 ***************************************************************************/

#ifndef __ROM_EXPLORE_H
#define __ROM_EXPLORE_H

#include <stdio.h>
#include "merc.h"

/* Function prototypes */
void explore_set_bit (char *explored, int size, int index);
int explore_get_bit (char *explored, int size, int index);
int explore_roomcount (CHAR_T *ch);
void explore_fwrite_rle (char *explored, int size, FILE *fp);
void explore_fread_rle (char **explored, int *size, FILE *fp);

#endif
