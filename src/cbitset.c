/**
 ** cbitset.c -- Simple C routines for bitset handling.
 **
 ** Copyright (C) 2001-2003 Cosmin Truta.
 **
 ** This software is distributed under the same licensing and warranty
 ** terms as OptiPNG. Please see the attached LICENSE for more info.
 **/


#include <stddef.h>
#include <limits.h>
#include "cbitset.h"


#undef IS_GRAPH
#define IS_GRAPH(_ch)  \
    ((unsigned char)(_ch) > ' ')

#undef SKIP_NON_GRAPH
#define SKIP_NON_GRAPH(_str, _i)  \
    { while ((_str)[i] != 0 && !IS_GRAPH((_str)[_i])) ++_i; }


/**
 * Counts the number of elements in a bitset.
 **/
int bitset_count(bitset set)
{
    int result = 0;
    unsigned int i;

    if (!BITSET_IS_VALID(set))
        return -1;
    for (i = 0; i < BITSET_SIZE; ++i)
        if (BITSET_GET(set, i))
            ++result;
    return result;
}


/**
 * Converts a string to a bitset.
 **/
bitset string_to_bitset(const char *str);
/* not implemented */


/**
 * Converts a bitset to a string.
 **/
char *bitset_to_string(bitset set, char *str_buf)
{
    int i = BITSET_SIZE - 1;
    unsigned int j = 0;

    if (!BITSET_IS_VALID(set) || str_buf == NULL)
        return NULL;

    while (i > 0 && !BITSET_GET(set, i))
        --i;
    while (i >= 0)
    {
        /* C ALERT ** (cond ? '1' : '0') is int instead of char */
        str_buf[j] = (char)(BITSET_GET(set, i) ? '1' : '0');
        --i;
        ++j;
    }
    str_buf[j] = 0;
    return str_buf;
}


/**
 * Converts a text to a bitset.
 **/
bitset text_to_bitset(const char *text)
{
    bitset result = BITSET_EMPTY;
    unsigned int num1, num2, i, j;
    int is_range;

    if (text == NULL)
        return BITSET_EMPTY;

    for (i = 0; ; ++i)
    {
        SKIP_NON_GRAPH(text, i);
        if (text[i] == 0)
            return result;

        num1 = UINT_MAX;  /* unassigned */
        num2 = 0;
        is_range = 0;
        while ((text[i] >= '0' && text[i] <= '9') || (text[i] == '-'))
        {
            if (text[i] == '-')  /* range */
            {
                is_range = 1;
                if (num1 == UINT_MAX)
                    num1 = 0;  /* default */
                num2 = BITSET_SIZE - 1;  /* default */
                ++i;
            }
            else  /* number */
            {
                for (num2 = 0; text[i] >= '0' && text[i] <= '9'; ++i)
                {
                    num2 = 10 * num2 + (text[i] - '0');
                    if (num2 > BITSET_SIZE)  /* overflow protection */
                        num2 = BITSET_SIZE;
                }
                if (!is_range)
                    num1 = num2;
            }
            SKIP_NON_GRAPH(text, i);
        }

        if (num2 >= BITSET_SIZE)  /* overflow protection */
            num2 = BITSET_SIZE - 1;
        for (j = num1; j <= num2; ++j)
            BITSET_SET(result, j);

        if (text[i] == 0)
            return result;
        if (text[i] != ',' && text[i] != ';')  /* not a separator */
            return BITSET_INVALID;
    }
}


/**
 * Converts a bitset to a text.
 **/
char *bitset_to_text(bitset set, char *text_buf);
/* not implemented */
