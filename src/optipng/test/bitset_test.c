/*
 * bitset_test.c
 * Test for bitset.
 *
 * Copyright (C) 2001-2017 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "bitset.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>


static int num_tests = 0;
static int num_errors = 0;

#define EMPTY   OPNG_BITSET_EMPTY
#define FULL    OPNG_BITSET_FULL
#define ELT_MIN OPNG_BITSET_ELT_MIN
#define ELT_MAX OPNG_BITSET_ELT_MAX

#define TEST_BOOL(expr) \
    test_int_impl("!!(" #expr ")", !!(expr), 1)

#define TEST_INT(expr, expected) \
    test_int_impl(#expr, expr, expected)

#define TEST_BITSET(expr, expected) \
    test_bitset_impl(#expr, expr, expected)

static void
test_int_impl(const char *expr_str, int result, int expected)
{
    ++num_tests;
    if (result != expected)
    {
        ++num_errors;
        printf("FAILED: %s, result: %d, expected: %d\n",
               expr_str, result, expected);
    }
}

static void
test_bitset_impl(const char *expr_str,
                 opng_bitset_t result, opng_bitset_t expected)
{
    ++num_tests;
    if (result != expected)
    {
        ++num_errors;
        printf("FAILED: %s, result: 0x%lx, expected: 0x%lx\n",
               expr_str, (unsigned long)result, (unsigned long)expected);
    }
}

static void
test_bitset_const()
{
    printf("Testing: bitset constants\n");

    TEST_BITSET(EMPTY, 0);
    TEST_BITSET(FULL,  ~EMPTY);

    TEST_INT(ELT_MIN, 0);
    TEST_INT(ELT_MAX, (int)OPNG_BITSIZEOF(opng_bitset_t) - 1);
    TEST_INT(ELT_MAX, (int)(sizeof(opng_bitset_t) * CHAR_BIT) - 1);
}

static void
test_bitset_obs()
{
    printf("Testing: bitset observers\n");

    TEST_INT(opng_bitset_test(EMPTY,  ELT_MIN), 0);
    TEST_INT(opng_bitset_test(EMPTY,  ELT_MAX), 0);
    TEST_INT(opng_bitset_test(FULL,   ELT_MIN), 1);
    TEST_INT(opng_bitset_test(FULL,   ELT_MAX), 1);
    TEST_INT(opng_bitset_test(0x0002, ELT_MIN), 0);
    TEST_INT(opng_bitset_test(0x0ff0, ELT_MAX), 0);
    TEST_INT(opng_bitset_test(EMPTY,  1),       0);
    TEST_INT(opng_bitset_test(FULL,   1),       1);
    TEST_INT(opng_bitset_test(0x0002, 1),       1);
    TEST_INT(opng_bitset_test(0x0ff0, 1),       0);

    TEST_INT(opng_bitset_test_all_in_range(EMPTY,  ELT_MIN, ELT_MAX), 0);
    TEST_INT(opng_bitset_test_all_in_range(FULL,   ELT_MIN, ELT_MAX), 1);
    TEST_INT(opng_bitset_test_all_in_range(0x0002, ELT_MIN, ELT_MAX), 0);
    TEST_INT(opng_bitset_test_all_in_range(0x0ff0, ELT_MIN, ELT_MAX), 0);
    TEST_INT(opng_bitset_test_all_in_range(EMPTY,  1,       1),       0);
    TEST_INT(opng_bitset_test_all_in_range(FULL,   1,       1),       1);
    TEST_INT(opng_bitset_test_all_in_range(0x0002, 1,       1),       1);
    TEST_INT(opng_bitset_test_all_in_range(0x0ff0, 1,       1),       0);

    TEST_INT(opng_bitset_test_any_in_range(EMPTY,  ELT_MIN, ELT_MAX), 0);
    TEST_INT(opng_bitset_test_any_in_range(FULL,   ELT_MIN, ELT_MAX), 1);
    TEST_INT(opng_bitset_test_any_in_range(0x0002, ELT_MIN, ELT_MAX), 1);
    TEST_INT(opng_bitset_test_any_in_range(0x0ff0, ELT_MIN, ELT_MAX), 1);
    TEST_INT(opng_bitset_test_any_in_range(EMPTY,  1,       1),       0);
    TEST_INT(opng_bitset_test_any_in_range(FULL,   1,       1),       1);
    TEST_INT(opng_bitset_test_any_in_range(0x0002, 1,       1),       1);
    TEST_INT(opng_bitset_test_any_in_range(0x0ff0, 1,       1),       0);

    TEST_INT(opng_bitset_count(EMPTY),  0);
    TEST_INT(opng_bitset_count(FULL),   OPNG_BITSIZEOF(opng_bitset_t));
    TEST_INT(opng_bitset_count(0x0002), 1);
    TEST_INT(opng_bitset_count(0x0ff0), 8);
}

static void
test_bitset_mod()
{
    opng_bitset_t set;

    printf("Testing: bitset modifiers\n");

    set = 0x0002;
    opng_bitset_set(&set, 1);
    opng_bitset_set(&set, 2);
    TEST_BITSET(set, 0x06);
    opng_bitset_reset(&set, 2);
    opng_bitset_reset(&set, 3);
    TEST_BITSET(set, 0x0002);
    opng_bitset_flip(&set, 3);
    opng_bitset_flip(&set, 1);
    TEST_BITSET(set, 0x08);

    set = 0x0ff0;
    opng_bitset_set_range(&set, 0, 7);
    TEST_BITSET(set, 0x0fff);
    opng_bitset_set_range(&set, ELT_MIN, ELT_MAX);
    TEST_BITSET(set, FULL);
    set = 0x0ff0;
    opng_bitset_reset_range(&set, 0, 7);
    TEST_BITSET(set, 0x0f00);
    opng_bitset_reset_range(&set, ELT_MIN, ELT_MAX);
    TEST_BITSET(set, EMPTY);
    set = 0x0ff0;
    opng_bitset_flip_range(&set, 0, 7);
    TEST_BITSET(set, 0x0f0f);
    opng_bitset_flip_range(&set, ELT_MIN, ELT_MAX);
    TEST_BITSET(set, FULL ^ 0x0f0f);
}

static void
test_bitset_find_forward_impl(opng_bitset_t bitset)
{
    opng_bitset_t check_bitset;
    int check_count;
    int elt, prev_elt;

    check_bitset = EMPTY;
    check_count = 0;
    prev_elt = -1;
    for (elt = opng_bitset_find_first(bitset);
         elt >= 0;
         elt = opng_bitset_find_next(bitset, elt))
    {
        if (elt < ELT_MIN || elt > ELT_MAX || elt <= prev_elt)
        {
            /* Fail. */
            TEST_INT(check_count, -1);
            return;
        }
        opng_bitset_set(&check_bitset, elt);
        ++check_count;
        prev_elt = elt;
    }
    TEST_BITSET(check_bitset, bitset);
    TEST_INT(check_count, opng_bitset_count(bitset));
}

static void
test_bitset_find_reverse_impl(opng_bitset_t bitset)
{
    opng_bitset_t check_bitset;
    int check_count;
    int elt, prev_elt;

    check_bitset = EMPTY;
    check_count = 0;
    prev_elt = INT_MAX;
    for (elt = opng_bitset_find_last(bitset);
         elt >= 0;
         elt = opng_bitset_find_prev(bitset, elt))
    {
        if (elt < ELT_MIN || elt > ELT_MAX || elt >= prev_elt)
        {
            /* Fail. */
            TEST_INT(check_count, -1);
            return;
        }
        opng_bitset_set(&check_bitset, elt);
        ++check_count;
        prev_elt = elt;
    }
    TEST_BITSET(check_bitset, bitset);
    TEST_INT(check_count, opng_bitset_count(bitset));
}

static void
test_bitset_iter()
{
    printf("Testing: bitset iterators\n");

    TEST_INT(opng_bitset_find_first(EMPTY),  -1);
    TEST_INT(opng_bitset_find_first(FULL),   ELT_MIN);
    TEST_INT(opng_bitset_find_first(0x0002), 1);
    TEST_INT(opng_bitset_find_first(0x0ff0), 4);

    TEST_INT(opng_bitset_find_next(EMPTY,  0),       -1);
    TEST_INT(opng_bitset_find_next(FULL,   0),       1);
    TEST_INT(opng_bitset_find_next(0x0002, 0),       1);
    TEST_INT(opng_bitset_find_next(0x0ff0, 0),       4);
    TEST_INT(opng_bitset_find_next(EMPTY,  1),       -1);
    TEST_INT(opng_bitset_find_next(FULL,   1),       2);
    TEST_INT(opng_bitset_find_next(0x0002, 1),       -1);
    TEST_INT(opng_bitset_find_next(0x0ff0, 1),       4);
    TEST_INT(opng_bitset_find_next(EMPTY,  2),       -1);
    TEST_INT(opng_bitset_find_next(FULL,   2),       3);
    TEST_INT(opng_bitset_find_next(0x0002, 2),       -1);
    TEST_INT(opng_bitset_find_next(0x0ff0, 2),       4);
    TEST_INT(opng_bitset_find_next(EMPTY,  ELT_MAX), -1);
    TEST_INT(opng_bitset_find_next(FULL,   ELT_MAX), -1);
    TEST_INT(opng_bitset_find_next(0x0002, ELT_MAX), -1);
    TEST_INT(opng_bitset_find_next(0x0ff0, ELT_MAX), -1);

    TEST_INT(opng_bitset_find_last(EMPTY),  -1);
    TEST_INT(opng_bitset_find_last(FULL),   ELT_MAX);
    TEST_INT(opng_bitset_find_last(0x0002), 1);
    TEST_INT(opng_bitset_find_last(0x0ff0), 11);

    TEST_INT(opng_bitset_find_prev(EMPTY,  0),       -1);
    TEST_INT(opng_bitset_find_prev(FULL,   0),       -1);
    TEST_INT(opng_bitset_find_prev(0x0002, 0),       -1);
    TEST_INT(opng_bitset_find_prev(0x0ff0, 0),       -1);
    TEST_INT(opng_bitset_find_prev(EMPTY,  1),       -1);
    TEST_INT(opng_bitset_find_prev(FULL,   1),       0);
    TEST_INT(opng_bitset_find_prev(0x0002, 1),       -1);
    TEST_INT(opng_bitset_find_prev(0x0ff0, 1),       -1);
    TEST_INT(opng_bitset_find_prev(EMPTY,  2),       -1);
    TEST_INT(opng_bitset_find_prev(FULL,   2),       1);
    TEST_INT(opng_bitset_find_prev(0x0002, 2),       1);
    TEST_INT(opng_bitset_find_prev(0x0ff0, 2),       -1);
    TEST_INT(opng_bitset_find_prev(EMPTY,  ELT_MAX), -1);
    TEST_INT(opng_bitset_find_prev(FULL,   ELT_MAX), ELT_MAX - 1);
    TEST_INT(opng_bitset_find_prev(0x0002, ELT_MAX), 1);
    TEST_INT(opng_bitset_find_prev(0x0ff0, ELT_MAX), 11);

    test_bitset_find_forward_impl(EMPTY);
    test_bitset_find_forward_impl(FULL);
    test_bitset_find_forward_impl(0x0002);
    test_bitset_find_forward_impl(0x0ff0);

    test_bitset_find_reverse_impl(EMPTY);
    test_bitset_find_reverse_impl(FULL);
    test_bitset_find_reverse_impl(0x0002);
    test_bitset_find_reverse_impl(0x0ff0);
}

static void
test_strparse_rangeset_impl(const char *rangeset,
                            opng_bitset_t mask,
                            opng_bitset_t expected_bitset,
                            int expected_errno)
{
    opng_bitset_t bitset;
    int parse_result, expected_parse_result;

    ++num_tests;
    errno = 0;
    expected_parse_result = (expected_errno == 0) ? 0 : -1;
    parse_result = opng_strparse_rangeset_to_bitset(&bitset, rangeset, mask);
    if (parse_result != expected_parse_result ||
        bitset != expected_bitset ||
        errno != expected_errno)
    {
        ++num_errors;
        printf("FAILED: opng_strparse_rangeset_to_bitset(\"%s\", 0x%lx)\n",
               rangeset, (unsigned long)mask);
        printf("        bitset: 0x%lx, expected: 0x%lx\n",
               (unsigned long)bitset, (unsigned long)expected_bitset);
        printf("        result: %d, expected: %d\n",
               parse_result, expected_parse_result);
        printf("        errno: %d, expected: %d\n",
               errno, expected_errno);
    }
}

static void
test_rangeset_parse()
{
    static const opng_bitset_t mask_full    = FULL;
    static const opng_bitset_t mask_byte    = 0x00ff;
    static const opng_bitset_t mask_pattern = 0x5555;
    static const opng_bitset_t mask_empty   = EMPTY;

    printf("Testing: rangeset parser\n");

    test_strparse_rangeset_impl("",            mask_full,    EMPTY,  0);
    test_strparse_rangeset_impl(" ",           mask_byte,    EMPTY,  0);
    test_strparse_rangeset_impl("   ",         mask_pattern, EMPTY,  0);
    test_strparse_rangeset_impl(" ,;  ;, ",    mask_empty,   EMPTY,  0);
    test_strparse_rangeset_impl("0",           mask_full,    0x0001, 0);
    test_strparse_rangeset_impl("0,",          mask_byte,    0x0001, 0);
    test_strparse_rangeset_impl(",0",          mask_pattern, 0x0001, 0);
    test_strparse_rangeset_impl(",0,",         mask_empty,   FULL,   ERANGE);
    test_strparse_rangeset_impl("0-",          mask_full,    FULL,   0);
    test_strparse_rangeset_impl("0-;",         mask_pattern, 0x5555, 0);
    test_strparse_rangeset_impl(";0-",         mask_byte,    0x00ff, 0);
    test_strparse_rangeset_impl(";0-;",        mask_empty,   FULL,   ERANGE);
    test_strparse_rangeset_impl(" 01 ",        mask_full,    0x0002, 0);
    test_strparse_rangeset_impl(" 01,; ",      mask_byte,    0x0002, 0);
    test_strparse_rangeset_impl(" ,;01 ",      mask_pattern, FULL,   ERANGE);
    test_strparse_rangeset_impl(" ,;01,; ",    mask_empty,   FULL,   ERANGE);
    test_strparse_rangeset_impl(" 012 , ;",    mask_full,    0x1000, 0);
    test_strparse_rangeset_impl(", ; 012 ",    mask_byte,    FULL,   ERANGE);
    test_strparse_rangeset_impl(",;012,;",     mask_pattern, 0x1000, 0);
    test_strparse_rangeset_impl("1, 1",        mask_full,    0x0002, 0);
    test_strparse_rangeset_impl("1- 1",        mask_byte,    0x0002, 0);
    test_strparse_rangeset_impl("1 ;2",        mask_full,    0x0006, 0);
    test_strparse_rangeset_impl("1 -2",        mask_byte,    0x0006, 0);
    test_strparse_rangeset_impl("1 - 2",       mask_pattern, FULL,   ERANGE);
    test_strparse_rangeset_impl("2-1",         mask_full,    FULL,   ERANGE);
    test_strparse_rangeset_impl("2-1",         mask_byte,    FULL,   ERANGE);
    test_strparse_rangeset_impl("124",         mask_full,    FULL,   ERANGE);
    test_strparse_rangeset_impl("12-4",        mask_full,    FULL,   ERANGE);
    test_strparse_rangeset_impl("1-2,4",       mask_full,    0x0016, 0);
    test_strparse_rangeset_impl("1-2,4-",      mask_byte,    0x00f6, 0);
    test_strparse_rangeset_impl("1,2,4",       mask_full,    0x0016, 0);
    test_strparse_rangeset_impl("1,2,4-",      mask_pattern, FULL,   ERANGE);
    test_strparse_rangeset_impl("1,2,4",       mask_byte,    0x0016, 0);
    test_strparse_rangeset_impl("1,2-4",       mask_full,    0x001e, 0);
    test_strparse_rangeset_impl("12,4",        mask_full,    0x1010, 0);
    test_strparse_rangeset_impl("12,4",        mask_byte,    FULL,   ERANGE);
    test_strparse_rangeset_impl("2-4,1",       mask_full,    0x001e, 0);
    test_strparse_rangeset_impl("4-2,1",       mask_full,    FULL,   ERANGE);
    test_strparse_rangeset_impl("4-,2-1",      mask_byte,    FULL,   ERANGE);
    test_strparse_rangeset_impl("4-,1-2",      mask_full,    FULL ^ 0x0009, 0);
    test_strparse_rangeset_impl("9999999999",  mask_full,    FULL,   ERANGE);
    test_strparse_rangeset_impl("9999999999-", mask_byte,    FULL,   ERANGE);

    test_strparse_rangeset_impl("-",           mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl(" - ",         mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("--",          mask_byte,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("- -",         mask_empty,   EMPTY, EINVAL);
    test_strparse_rangeset_impl("0--",         mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("0 -- ",       mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("0- -",        mask_byte,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("0 --- ",      mask_empty,   EMPTY, EINVAL);
    test_strparse_rangeset_impl("-0",          mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("-0-",         mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("1k",          mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("1 k",         mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("A1",          mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("0xA1",        mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl(".1",          mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("1.",          mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("1.2",         mask_byte,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("1 2",         mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("2 1",         mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("1--2",        mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("1--2-",       mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("1- -2",       mask_byte,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("1- -2-",      mask_empty,   EMPTY, EINVAL);
    test_strparse_rangeset_impl("-1--2",       mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("-1--2-",      mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("-1- -2",      mask_byte,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("-1- -2-",     mask_empty,   EMPTY, EINVAL);
    test_strparse_rangeset_impl("1-2-4",       mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("1-2-4-",      mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("1-2-4-",      mask_byte,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("-1-2-4",      mask_empty,   EMPTY, EINVAL);
    test_strparse_rangeset_impl("1-4-2,",      mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("1-4-2-,",     mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl("1-4-2-,",     mask_byte,    EMPTY, EINVAL);
    test_strparse_rangeset_impl("-1-4-2,",     mask_empty,   EMPTY, EINVAL);
    test_strparse_rangeset_impl(",4-2-1;",     mask_full,    EMPTY, EINVAL);
    test_strparse_rangeset_impl(",4-2-1-;",    mask_pattern, EMPTY, EINVAL);
    test_strparse_rangeset_impl(",4-2-1-;",    mask_byte,    EMPTY, EINVAL);
    test_strparse_rangeset_impl(",-4-2-1;",    mask_empty,   EMPTY, EINVAL);
    test_strparse_rangeset_impl("-9999999999", mask_full,    EMPTY, EINVAL);
}

static void
run_tests()
{
    test_bitset_const();
    test_bitset_obs();
    test_bitset_mod();
    test_bitset_iter();
    test_rangeset_parse();
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
