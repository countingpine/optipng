/**
 ** strtest.c
 ** Unit test driver for strutil.
 **
 ** Copyright (C) 2001-2006 Cosmin Truta.
 **
 ** This software is distributed under the same licensing and warranty
 ** terms as OptiPNG.  Please see the attached LICENSE for more info.
 **/


#include <stdio.h>
#include <stdlib.h>
#include "strutil.h"


static char buf1[256], buf2[256], buf3[256];


static void help(void)
{
    printf(
        "Usage:\n"
        "\tstrtest [routine]\n"
        "Routines:\n"
        "\tstring_case_cmp, string_num_case_cmp,\n"
        "\tstring_lower, string_upper,\n"
        "\tstring_prefix_cmp, string_prefix_case_cmp\n"
        "\tstring_prefix_min_cmp, string_prefix_min_case_cmp\n"
        "\tstring_suffix_cmp, string_suffix_case_cmp\n"
    );
    exit(EXIT_FAILURE);
}


static void test_string_case_cmp(void)
{
    unsigned int num;

    for ( ; ; )
    {
        printf("Enter string #1: ");
        gets(buf1);
        if (buf1[0] == 0)
            return;
        printf("Enter string #2: ");
        gets(buf2);
        printf("Enter num: ");
        gets(buf3);
        sscanf(buf3, "%u", &num);
        printf("string_case_cmp(\"%s\", \"%s\") = %d\n",
            buf1, buf2, string_case_cmp(buf1, buf2));
        printf("string_num_case_cmp(\"%s\", \"%s\", %d) = %d\n",
            buf1, buf2, num, string_num_case_cmp(buf1, buf2, num));
    }
}


static void test_string_lower_upper(void)
{
    for ( ; ; )
    {
        printf("Enter string : ");
        gets(buf1);
        if (buf1[0] == 0)
            return;
        printf("string_lower = %s\n", string_lower(buf1));
        printf("string_upper = %s\n", string_upper(buf1));
    }
}


static void test_string_prefix_cmp(void)
{
    unsigned int min_len;

    for ( ; ; )
    {
        printf("Enter string #1: ");
        gets(buf1);
        if (buf1[0] == 0)
            return;
        printf("Enter string #2: ");
        gets(buf2);
        printf("Enter minimum length: ");
        gets(buf3);
        sscanf(buf3, "%u", &min_len);
        printf("string_prefix_cmp(\"%s\", \"%s\") = %d\n",
            buf1, buf2,
            string_prefix_cmp(buf1, buf2));
        printf("string_prefix_case_cmp(\"%s\", \"%s\") = %d\n",
            buf1, buf2,
            string_prefix_case_cmp(buf1, buf2));
        printf("string_prefix_min_cmp(\"%s\", \"%s\", %d) = %d\n",
            buf1, buf2, min_len,
            string_prefix_min_cmp(buf1, buf2, min_len));
        printf("string_prefix_min_case_cmp(\"%s\", \"%s\", %d) = %d\n",
            buf1, buf2, min_len,
            string_prefix_min_case_cmp(buf1, buf2, min_len));
        printf("string_suffix_cmp(\"%s\", \"%s\") = %d\n",
            buf1, buf2,
            string_suffix_cmp(buf1, buf2));
        printf("string_suffix_case_cmp(\"%s\", \"%s\") = %d\n",
            buf1, buf2,
            string_suffix_case_cmp(buf1, buf2));
    }
}


int main(int argc, char *argv[])
{
    char *routine;

    if (argc <= 1)
        help();

    routine = argv[1];

    if (strcmp(routine, "string_case_cmp") == 0
          || strcmp(routine, "string_num_case_cmp") == 0)
        test_string_case_cmp();
    else if (strcmp(routine, "string_lower") == 0
          || strcmp(routine, "string_upper") == 0)
        test_string_lower_upper();
    else if (strcmp(routine, "string_prefix_cmp") == 0
          || strcmp(routine, "string_prefix_case_cmp") == 0
          || strcmp(routine, "string_prefix_min_cmp") == 0
          || strcmp(routine, "string_prefix_min_case_cmp") == 0
          || strcmp(routine, "string_suffix_cmp") == 0
          || strcmp(routine, "string_suffix_case_cmp") == 0)
        test_string_prefix_cmp();
    else
        help();

    return EXIT_SUCCESS;
}
