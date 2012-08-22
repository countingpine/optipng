/*
 * integer_test.c
 * Test for optk/integer.h
 *
 * Copyright (C) 2011-2012 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "optk/integer.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>


void
check_signedness(void)
{
    printf("Testing signedness...\n");

    assert((optk_int8_t)(-1)   < 0);
    assert((optk_uint8_t)(-1)  > 0);
    assert((optk_int16_t)(-1)  < 0);
    assert((optk_uint16_t)(-1) > 0);
    assert((optk_int32_t)(-1)  < 0);
    assert((optk_uint32_t)(-1) > 0);
    assert((optk_int64_t)(-1)  < 0);
    assert((optk_uint64_t)(-1) > 0);
}

void
check_sizes(void)
{
    unsigned int i8_bitsize  = sizeof(optk_int8_t)   * CHAR_BIT;
    unsigned int u8_bitsize  = sizeof(optk_uint8_t)  * CHAR_BIT;
    unsigned int i16_bitsize = sizeof(optk_int16_t)  * CHAR_BIT;
    unsigned int u16_bitsize = sizeof(optk_uint16_t) * CHAR_BIT;
    unsigned int i32_bitsize = sizeof(optk_int32_t)  * CHAR_BIT;
    unsigned int u32_bitsize = sizeof(optk_uint32_t) * CHAR_BIT;
    unsigned int i64_bitsize = sizeof(optk_int64_t)  * CHAR_BIT;
    unsigned int u64_bitsize = sizeof(optk_uint64_t) * CHAR_BIT;

    printf("Testing sizes...\n");

    assert(i8_bitsize  == u8_bitsize);
    assert(i16_bitsize == u16_bitsize);
    assert(i32_bitsize == u32_bitsize);
    assert(i64_bitsize == u64_bitsize);

    assert(i8_bitsize  >= 8);
    assert(i16_bitsize >= 16);
    assert(i32_bitsize >= 32);
    assert(i64_bitsize >= 64);

    assert(i16_bitsize >= i8_bitsize);
    assert(i32_bitsize >= i16_bitsize);
    assert(i64_bitsize >= i32_bitsize);
}

void
check_limits(void)
{
    printf("Testing limits...\n");

    assert(OPTK_INT8_MIN   < -OPTK_INT8_C(0x7f));
    assert(OPTK_INT8_MAX   >= OPTK_INT8_C(0x7f));
    assert(OPTK_UINT8_MAX  >= OPTK_INT8_C(0xff));
    assert(OPTK_INT16_MIN  < -OPTK_INT16_C(0x7fff));
    assert(OPTK_INT16_MAX  >= OPTK_INT16_C(0x7fff));
    assert(OPTK_UINT16_MAX >= OPTK_INT16_C(0xffff));
    assert(OPTK_INT32_MIN  < -OPTK_INT32_C(0x7fffffff));
    assert(OPTK_INT32_MAX  >= OPTK_INT32_C(0x7fffffff));
    assert(OPTK_UINT32_MAX >= OPTK_INT32_C(0xffffffff));
    assert(OPTK_INT64_MIN  < -OPTK_INT64_C(0x7fffffffffffffff));
    assert(OPTK_INT64_MAX  >= OPTK_INT64_C(0x7fffffffffffffff));
    assert(OPTK_UINT64_MAX >= OPTK_INT64_C(0xffffffffffffffff));
}

void
check_format_macros(void)
{
#if 0
    /* The format macros for the 8-bit integer types are not reliable. */
    optk_int8_t   d8, i8;
    optk_uint8_t  u8, o8, x8;
#endif
    optk_int16_t  d16, i16;
    optk_uint16_t u16, o16, x16;
    optk_int32_t  d32, i32;
    optk_uint32_t u32, o32, x32;
    optk_int64_t  d64, i64;
    optk_uint64_t u64, o64, x64;
    char buf[16];
    int n;
    const char *scanf_string = "-1 1 3 7 f";
    const char *printf_string = "-1 1 3 7 f F";

    printf("Testing scanf format macros...\n");

    n = sscanf(scanf_string,
               "%" OPTK_SCNd16 " "
               "%" OPTK_SCNi16 " "
               "%" OPTK_SCNu16 " "
               "%" OPTK_SCNo16 " "
               "%" OPTK_SCNx16,
               &d16, &i16, &u16, &o16, &x16);
    assert(n == 5);
    assert(d16 == -1 && i16 == 1 && u16 == 3 && o16 == 7 && x16 == 15);
    n = sscanf(scanf_string,
               "%" OPTK_SCNd32 " "
               "%" OPTK_SCNi32 " "
               "%" OPTK_SCNu32 " "
               "%" OPTK_SCNo32 " "
               "%" OPTK_SCNx32,
               &d32, &i32, &u32, &o32, &x32);
    assert(n == 5);
    assert(d32 == -1 && i32 == 1 && u32 == 3 && o32 == 7 && x32 == 15);
    n = sscanf(scanf_string,
               "%" OPTK_SCNd64 " "
               "%" OPTK_SCNi64 " "
               "%" OPTK_SCNu64 " "
               "%" OPTK_SCNo64 " "
               "%" OPTK_SCNx64,
               &d64, &i64, &u64, &o64, &x64);
    assert(n == 5);
    assert(d64 == -1 && i64 == 1 && u64 == 3 && o64 == 7 && x64 == 15);

    printf("Testing printf format macros...\n");

    snprintf(buf, sizeof(buf),
             "%" OPTK_PRId16 " "
             "%" OPTK_PRIi16 " "
             "%" OPTK_PRIu16 " "
             "%" OPTK_PRIo16 " "
             "%" OPTK_PRIx16 " "
             "%" OPTK_PRIX16,
             d16, i16, u16, o16, x16, x16);
    assert(strcmp(buf, printf_string) == 0);
    snprintf(buf, sizeof(buf),
             "%" OPTK_PRId32 " "
             "%" OPTK_PRIi32 " "
             "%" OPTK_PRIu32 " "
             "%" OPTK_PRIo32 " "
             "%" OPTK_PRIx32 " "
             "%" OPTK_PRIX32,
             d32, i32, u32, o32, x32, x32);
    assert(strcmp(buf, printf_string) == 0);
    snprintf(buf, sizeof(buf),
             "%" OPTK_PRId64 " "
             "%" OPTK_PRIi64 " "
             "%" OPTK_PRIu64 " "
             "%" OPTK_PRIo64 " "
             "%" OPTK_PRIx64 " "
             "%" OPTK_PRIX64,
             d64, i64, u64, o64, x64, x64);
    assert(strcmp(buf, printf_string) == 0);
}

int
main()
{
    check_signedness();
    check_sizes();
    check_limits();
    check_format_macros();
    return 0;
}
