/**
 ** cbittest.c
 ** Test driver for the cbitset routines.
 **
 ** Copyright (C) 2001-2006 Cosmin Truta.
 **
 ** This software is distributed under the same licensing and warranty
 ** terms as OptiPNG.  Please see the attached LICENSE for more info.
 **/


#include <ctype.h>
#include <stdio.h>
#include "cbitset.h"


#define SKIP_SPACES(_str)  \
    { while (isspace(*(_str))) ++(_str); }


void fput_bitset(bitset_t value, FILE *stream)
{
    char buf[BITSET_SIZE + 1];

    fputs(bitset_to_string(value, buf, sizeof(buf)), stream);
    if (BITSET_GET_OVERFLOW(value))
        fputs(" (overflow)", stream);
}


int main()
{
    char buf[256];
    char *ptr;
    bitset_t set;

    for ( ; ; )
    {
        /* Input text */
        fputs("Enter bitset text: ", stdout);
        fflush(stdout);
        fgets(buf, sizeof(buf), stdin);
        fflush(stdin);

        if (buf[0] == 0 || buf[0] == '\n')
            return 0;

        /* Test string_to_bitset() */
        set = string_to_bitset(buf, &ptr);
        SKIP_SPACES(ptr);
        if (*ptr == 0)
        {
            fputs("string_to_bitset(text) = ", stdout);
            fput_bitset(set, stdout);
            fputs("\n", stdout);
        }

        /* Test bitset_parse() */
        fputs("bitset_parse(text) = ", stdout);
        if (bitset_parse(buf, &set) == 0)
        {
            fput_bitset(set, stdout);
            fputs("\n", stdout);
        }
        else
            fputs("error\n", stdout);

        fputs("\n", stdout);
    }
}
