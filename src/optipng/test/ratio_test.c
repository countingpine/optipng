/*
 * ratio_test.c
 * Test for ratio.
 *
 * Copyright (C) 2008-2017 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "ratio.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if !defined(OPNG_ULLONG_MAX) || !defined(OPNG_ULLONG_C)
#error Missing opng_ullong_t macros
#endif

#if OPNG_ULLONG_MAX != ~OPNG_ULLONG_C(0)
#error Incorrect opng_ullong_t macros
#endif


static int num_tests = 0;
static int num_errors = 0;

#define U32_MAX 0xffffffffUL
#define ULL_MAX OPNG_ULLONG_MAX

#define BUFFER_SIZE 64

typedef int (*ulconv_func_t)(char *buffer, size_t buffer_size,
                             const struct opng_ulratio *ratio);
typedef int (*ullconv_func_t)(char *buffer, size_t buffer_size,
                              const struct opng_ullratio *ratio);

static int
do_conv(ulconv_func_t ulconv, ullconv_func_t ullconv,
        char *buffer, size_t buffer_size,
        opng_ullong_t num, opng_ullong_t denom)
{
    char replica_buffer[BUFFER_SIZE];
    struct opng_ulratio ulratio;
    struct opng_ullratio ullratio;
    int ulresult, ullresult;

    ullratio.num = num;
    ullratio.denom = denom;
    ullresult = ullconv(buffer, buffer_size, &ullratio);
    if (ullresult >= 0 && strlen(buffer) != (size_t)ullresult)
    {
        fprintf(stderr, "Can't handle incorrect output strings\n.");
        abort();
    }

#if OPNG_ULLONG_MAX > ULONG_MAX
    /* Replicate the result only if the input can be narrowed correctly. */
    if ((unsigned long)num != num || (unsigned long)denom != denom)
        return ullresult;
#endif

    ulratio.num = (unsigned long)num;
    ulratio.denom = (unsigned long)denom;
    ulresult = ulconv(replica_buffer, buffer_size, &ulratio);
    if (ulresult != ullresult ||
        (ulresult >= 0 && strcmp(replica_buffer, buffer) != 0))
    {
        fprintf(stderr, "Can't handle inconsistent output strings.\n");
        abort();
    }

    return ullresult;
}

static void
test_conv(ulconv_func_t ulconv, ullconv_func_t ullconv,
          char *buffer, size_t buffer_size,
          opng_ullong_t num, opng_ullong_t denom)
{
    char check_buffer[BUFFER_SIZE];
    size_t exact_buffer_size;
    int conv_result;

    conv_result = do_conv(ulconv, ullconv, buffer, buffer_size, num, denom);
    if (conv_result <= 0)
    {
        fprintf(stderr, "Can't test; a larger buffer is required.\n");
        abort();
    }

    exact_buffer_size = (size_t)conv_result + 1;
    if (do_conv(ulconv, ullconv,
                check_buffer, exact_buffer_size,
                num, denom) != conv_result)
    {
        fprintf(stderr, "Can't handle buffers of exact size.\n");
        abort();
    }

    if (do_conv(ulconv, ullconv,
                check_buffer, exact_buffer_size - 1,
                num, denom) >= 0 ||
        do_conv(ulconv, ullconv, check_buffer, 1, num, denom) >= 0 ||
        do_conv(ulconv, ullconv, check_buffer, 0, num, denom) >= 0)
    {
        fprintf(stderr, "Can't handle buffers of insufficient size.\n");
        abort();
    }
}

static void
test(opng_ullong_t num, opng_ullong_t denom,
     const char *expected_factor_string, const char *expected_percent_string)
{
    char buffer_factor[BUFFER_SIZE], buffer_percent[BUFFER_SIZE];
    int success;

    success = 1;

    test_conv(opng_ulratio_to_factor_string, opng_ullratio_to_factor_string,
              buffer_factor, BUFFER_SIZE,
              num, denom);
    if (strcmp(buffer_factor, expected_factor_string) != 0)
        success = 0;

    test_conv(opng_ulratio_to_percent_string, opng_ullratio_to_percent_string,
              buffer_percent, BUFFER_SIZE,
              num, denom);
    if (strcmp(buffer_percent, expected_percent_string) != 0)
        success = 0;

    ++num_tests;
    if (success)
    {
        printf("Passed: "
               "%" OPNG_LLONG_FORMAT_PREFIX "u / "
               "%" OPNG_LLONG_FORMAT_PREFIX "u\n",
               num, denom);
    }
    else
    {
        ++num_errors;
        printf("FAILED: "
               "%" OPNG_LLONG_FORMAT_PREFIX "u / "
               "%" OPNG_LLONG_FORMAT_PREFIX "u, "
               "result: (%s %s), expected: (%s %s)\n",
               num, denom,
               buffer_factor, buffer_percent,
               expected_factor_string, expected_percent_string);
    }
}

static void
run_tests()
{
    /*
     * (1) num/denom == 0/0
     */
    test(0, 0, "??%", "??%");

    /*
     * (2) num/denom == INFINITY
     */
    test(      1, 0, "INFINITY%", "INFINITY%");
    test(      9, 0, "INFINITY%", "INFINITY%");
    test(U32_MAX, 0, "INFINITY%", "INFINITY%");
    test(ULL_MAX, 0, "INFINITY%", "INFINITY%");

    /*
     * (3) 0 <= num/denom < 99.995% ==> precision = 0.0001
     */
    test(        0,         1,  "0.00%",  "0.00%");  /* = 0% */
    test(        0,   U32_MAX,  "0.00%",  "0.00%");  /* = 0% */
    test(        0,   ULL_MAX,  "0.00%",  "0.00%");  /* = 0% */
    test(        1,   U32_MAX,  "0.00%",  "0.00%");  /* > 0% */
    test(        1,   ULL_MAX,  "0.00%",  "0.00%");  /* > 0% */
    test(        1,     20001,  "0.00%",  "0.00%");  /* < 0.005% */
    test(        1,     20000,  "0.01%",  "0.01%");  /* = 0.005% */
    test(        1,     19999,  "0.01%",  "0.01%");  /* > 0.005% */
    test(        1,     10000,  "0.01%",  "0.01%");  /* = 0.01% */
    test(        1,      4001,  "0.02%",  "0.02%");  /* < 0.025% */
    test(        1,      4000,  "0.03%",  "0.03%");  /* = 0.025% */
    test(        1,      3999,  "0.03%",  "0.03%");  /* > 0.025% */
    test(   199000,    995000, "20.00%", "20.00%");  /* = 20% */
    test(U32_MAX/5,   U32_MAX, "20.00%", "20.00%");  /* = 20% */
    test(ULL_MAX/5,   ULL_MAX, "20.00%", "20.00%");  /* = 20% */
    test(U32_MAX/9, U32_MAX/3, "33.33%", "33.33%");  /* < 33.33...% */
    test(ULL_MAX/9, ULL_MAX/3, "33.33%", "33.33%");  /* < 33.33...% */
    test(       49,        99, "49.49%", "49.49%");  /* = 49.4949...% */
    test(   494949,    999999, "49.49%", "49.49%");  /* = 49.4949...% */
    test(U32_MAX/2,   U32_MAX, "50.00%", "50.00%");  /* < 50% */
    test(ULL_MAX/2,   ULL_MAX, "50.00%", "50.00%");  /* < 50% */
    test(U32_MAX/2, U32_MAX-1, "50.00%", "50.00%");  /* = 50% */
    test(ULL_MAX/2, ULL_MAX-1, "50.00%", "50.00%");  /* = 50% */
    test(    50005,    100000, "50.01%", "50.01%");  /* = 50.005% */
    test(       50,        99, "50.51%", "50.51%");  /* = 50.5050...% */
    test(   505050,    999999, "50.51%", "50.51%");  /* = 50.5050...% */
    test(    99995,    100001, "99.99%", "99.99%");  /* < 99.95% */

    /*
     * (4) 0.995 <= num/denom < INFINITY and force_percent
     * (5) 0.995 <= num/denom < 99.995 ==> precision = 0.01
     */
    test(    99995,    100000,  "1.00x",   "100%");  /* = 99.95% */
    test(U32_MAX-1,   U32_MAX,  "1.00x",   "100%");  /* < 1.0 */
    test(ULL_MAX-1,   ULL_MAX,  "1.00x",   "100%");  /* < 1.0 */
    test(        1,         1,  "1.00x",   "100%");  /* = 1.0 */
    test(  U32_MAX,   U32_MAX,  "1.00x",   "100%");  /* = 1.0 */
    test(  ULL_MAX,   ULL_MAX,  "1.00x",   "100%");  /* = 1.0 */
    test(  U32_MAX, U32_MAX-1,  "1.00x",   "100%");  /* > 1.0 */
    test(  ULL_MAX, ULL_MAX-1,  "1.00x",   "100%");  /* > 1.0 */
    test(    12350,     10001,  "1.23x",   "123%");  /* < 1.235 */
    test(    12350,     10000,  "1.24x",   "124%");  /* = 1.235 */
    test(U32_MAX, U32_MAX/2+1,  "2.00x",   "200%");  /* < 2.0 */
    test(ULL_MAX, ULL_MAX/2+1,  "2.00x",   "200%");  /* < 2.0 */
    test(U32_MAX-1, U32_MAX/2,  "2.00x",   "200%");  /* = 2.0 */
    test(ULL_MAX-1, ULL_MAX/2,  "2.00x",   "200%");  /* = 2.0 */
    test(  U32_MAX, U32_MAX/2,  "2.00x",   "200%");  /* > 2.0 */
    test(  ULL_MAX, ULL_MAX/2,  "2.00x",   "200%");  /* > 2.0 */
    test(  U32_MAX, U32_MAX/6,  "6.00x",   "600%");  /* > 6.0 */
    test(  ULL_MAX, ULL_MAX/6,  "6.00x",   "600%");  /* > 6.0 */
    test(  U32_MAX, U32_MAX/9,  "9.00x",   "900%");  /* > 9.0 */
    test(  ULL_MAX, ULL_MAX/9,  "9.00x",   "900%");  /* > 9.0 */
    test(     1299,       100, "12.99x",  "1299%");  /* = 12.99 */
    test(    12999,      1000, "13.00x",  "1300%");  /* = 12.999 */
    test(U32_MAX,  U32_MAX/99, "99.00x",  "9900%");  /* > 99.0 */
    test(ULL_MAX,  ULL_MAX/99, "99.00x",  "9900%");  /* > 99.0 */
    test(   999950,     10001, "99.99x",  "9999%");  /* < 99.995 */
    test(   999949,     10000, "99.99x",  "9999%");  /* < 99.995 */

    /*
     * (4) 0.995 <= num/denom < INFINITY and force_percent
     * (6) 99.5 <= num/denom < INFINITY ==> precision = 1.0
     */
    test(   999950,     10000,   "100x", "10000%");  /* = 99.995 */
    test(      502,         5,   "100x", "10040%");  /* < 100.5 */
    test(     1004,        10,   "100x", "10040%");  /* < 100.5 */
    test(     1005,        10,   "101x", "10050%");  /* = 100.5 */
    test(      503,         5,   "101x", "10060%");  /* > 100.5 */
    test(     1006,        10,   "101x", "10060%");  /* > 100.5 */
    test(    12399,       100,   "124x", "12399%");  /* = 123.99 */
    test(   123999,      1000,   "124x", "12400%");  /* = 123.999 */
    test(U32_MAX, U32_MAX/999,   "999x", "99900%");  /* > 999.0 */
    test(ULL_MAX, ULL_MAX/999,   "999x", "99900%");  /* > 999.0 */
    test(   999499,      1000,   "999x", "99950%");  /* < 999.5 */
    test(   999500,      1000,  "1000x", "99950%");  /* = 999.5 */
}

int
main()
{
    run_tests();
    if (num_errors != 0)
    {
        printf("** %d/%d tests FAILED **\n", num_errors, num_tests);
        return 1;
    }
    else
    {
        printf("** %d tests passed **\n", num_tests);
        return 0;
    }
}
