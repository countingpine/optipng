/**
 ** cbittest.c -- Test driver for the "cbitset" routines.
 **
 ** Copyright (C) 2001-2003 Cosmin Truta.
 **
 ** This software is distributed under the same licensing and warranty
 ** terms as OptiPNG. Please see the attached LICENSE for more info.
 **/


#include <stdio.h>
#include "cbitset.h"


int main()
{
    char in_buf[256], out_buf[256];
    
    for ( ;; )
    {
        printf("Enter bitset text: ");
        gets(in_buf);
        if (in_buf[0] == 0)
            return 0;
        printf("%s\n", bitset_to_string(text_to_bitset(in_buf), out_buf));
    }
}
