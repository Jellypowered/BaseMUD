/***************************************************************************
 * Exploration tracking system - tracks rooms visited by player characters
 * Adapted from snippet by (unknown author)
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "merc.h"
#include "fread.h"
#include "chars.h"

/* Bit patterns for bitpacking */
#define BIT_A 1
#define BIT_B 2
#define BIT_C 4
#define BIT_D 8
#define BIT_E 16
#define BIT_F 32
#define BIT_G 64
#define BIT_H 128

/*
 * explore_bitcount() - returns count of set bits in an 8-bit value
 */
static int explore_bitcount(char c)
{
    int count = 0;
    if (c & BIT_A) count++;
    if (c & BIT_B) count++;
    if (c & BIT_C) count++;
    if (c & BIT_D) count++;
    if (c & BIT_E) count++;
    if (c & BIT_F) count++;
    if (c & BIT_G) count++;
    if (c & BIT_H) count++;
    return count;
}

/*
 * explore_get_bit() - returns 1 if bit is set, 0 otherwise
 */
int explore_get_bit(char *explored, int size, int index)
{
    int bit = 0;
    int byte_index = index / 8;
    
    if (explored == NULL || byte_index >= size)
        return 0;
    
    switch (index % 8)
    {
        case 0: bit = BIT_A; break;
        case 1: bit = BIT_B; break;
        case 2: bit = BIT_C; break;
        case 3: bit = BIT_D; break;
        case 4: bit = BIT_E; break;
        case 5: bit = BIT_F; break;
        case 6: bit = BIT_G; break;
        case 7: bit = BIT_H; break;
        default: bit = 0; break;
    }
    
    return IS_SET(explored[byte_index], bit) ? 1 : 0;
}

/*
 * explore_set_bit() - sets the bit at the given index
 * Dynamically expands buffer if needed
 */
void explore_set_bit(char *explored, int size, int index)
{
    int bit = 0;
    int byte_index = index / 8;
    
    if (explored == NULL || byte_index >= size)
        return;
    
    switch (index % 8)
    {
        case 0: bit = BIT_A; break;
        case 1: bit = BIT_B; break;
        case 2: bit = BIT_C; break;
        case 3: bit = BIT_D; break;
        case 4: bit = BIT_E; break;
        case 5: bit = BIT_F; break;
        case 6: bit = BIT_G; break;
        case 7: bit = BIT_H; break;
        default: bit = 0; break;
    }
    
    SET_BIT(explored[byte_index], bit);
}

/*
 * explore_roomcount() - returns the number of rooms explored by character
 */
int explore_roomcount(CHAR_T *ch)
{
    int count = 0;
    int index;
    
    if (IS_NPC(ch) || ch->pcdata == NULL || ch->pcdata->explored == NULL)
        return 0;
    
    for (index = 0; index < ch->pcdata->explored_size; index++)
        count += explore_bitcount(ch->pcdata->explored[index]);
    
    return count;
}

/*
 * explore_fwrite_rle() - saves the explored buffer using RLE compression
 * Format: RoomRLE <size> <count1> <bit1> <count2> <bit2> ... -1
 */
void explore_fwrite_rle(char *explored, int size, FILE *fp)
{
    int index;
    int bit, count;
    
    if (explored == NULL || size <= 0)
    {
        fprintf(fp, "RoomRLE 0\n");
        return;
    }
    
    fprintf(fp, "RoomRLE %d", size);
    
    bit = explore_get_bit(explored, size, 0);
    count = 0;
    
    for (index = 0; index < (size * 8); index++)
    {
        if (explore_get_bit(explored, size, index) == bit)
        {
            count++;
        }
        else
        {
            fprintf(fp, " %d %d", count, bit);
            count = 1;
            bit = explore_get_bit(explored, size, index);
        }
    }
    
    fprintf(fp, " %d %d -1\n", count, bit);
}

/*
 * explore_fread_rle() - loads the explored buffer using RLE decompression
 */
void explore_fread_rle(char **explored, int *size, FILE *fp)
{
    int index;
    int bit, count, pos;
    char *new_explored;
    int read_size;
    
    read_size = fread_number(fp);
    
    if (read_size <= 0)
    {
        *explored = NULL;
        *size = 0;
        return;
    }
    
    new_explored = calloc(read_size, sizeof(char));
    if (new_explored == NULL)
    {
        *explored = NULL;
        *size = 0;
        return;
    }
    
    index = 0;
    pos = 0;
    
    for (;;)
    {
        count = fread_number(fp);
        if (count < 0)
            break;
        
        bit = fread_number(fp);
        
        do
        {
            if (bit == 1)
                explore_set_bit(new_explored, read_size, index);
            index++;
        } while (index < pos + count);
        
        pos = index;
    }
    
    *explored = new_explored;
    *size = read_size;
}
