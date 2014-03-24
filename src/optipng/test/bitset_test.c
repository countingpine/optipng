/*
 * bitset_test.c
 * Test for bitset.h
 *
 * Copyright (C) 2001-2014 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "bitset.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>


char *
my_strrtrim(char *str)
{
    char *ptr;
    char *end;

    end = str;
    for (ptr = str; *ptr != 0; ++ptr)
    {
        if (!isspace(*ptr))
            end = ptr + 1;
    }
    *end = (char)0;
    return str;
}

void
check_bits(opng_bitset_t value)
{
    opng_bitset_t empty, full;
    opng_bitset_t flipped1, flipped2, flipped3, flipped4;
    int empty_is_empty, full_is_full;
    int check_test_all, check_test_any;
    int i;

    empty = full = flipped1 = value;
    opng_bitset_reset_range(&empty, OPNG_BITSET_ELT_MIN, OPNG_BITSET_ELT_MAX);
    opng_bitset_set_range(&full, OPNG_BITSET_ELT_MIN, OPNG_BITSET_ELT_MAX);
    opng_bitset_flip_range(&flipped1, OPNG_BITSET_ELT_MIN, OPNG_BITSET_ELT_MAX);
    assert(empty == 0);
    assert(full == (opng_bitset_t)~0);
    assert(flipped1 == ~value);

    empty_is_empty = !opng_bitset_test_any_in_range(empty,
                                                    OPNG_BITSET_ELT_MIN,
                                                    OPNG_BITSET_ELT_MAX);
    full_is_full = opng_bitset_test_all_in_range(full,
                                                 OPNG_BITSET_ELT_MIN,
                                                 OPNG_BITSET_ELT_MAX);
    assert(empty_is_empty);
    assert(full_is_full);

    flipped4 = flipped3 = flipped2 = flipped1;
    for (i = OPNG_BITSET_ELT_MIN; i <= OPNG_BITSET_ELT_MAX; ++i)
    {
        opng_bitset_flip(&flipped1, i);
        opng_bitset_flip_range(&flipped2, i, i);
        if (opng_bitset_test(flipped3, i))
        {
            opng_bitset_reset(&flipped3, i);
            opng_bitset_reset_range(&flipped4, i, i);
            check_test_all = !opng_bitset_test_all_in_range(flipped3, i, i);
            check_test_any = !opng_bitset_test_any_in_range(flipped4, i, i);
        }
        else
        {
            opng_bitset_set(&flipped3, i);
            opng_bitset_set_range(&flipped4, i, i);
            check_test_all = opng_bitset_test_all_in_range(flipped3, i, i);
            check_test_any = opng_bitset_test_any_in_range(flipped4, i, i);
        }
        assert(flipped1 == flipped2);
        assert(flipped2 == flipped3);
        assert(flipped3 == flipped4);
        assert(check_test_all);
        assert(check_test_any);
    }
    assert(flipped1 == value);
}

void
dump_bits(opng_bitset_t value)
{
    unsigned int forward_count, reverse_count;
    int i;

    printf("count: %u\n", opng_bitset_count(value));
    printf("iteration:");
    forward_count = 0;
    for (i = opng_bitset_find_first(value);
         i >= 0;
         i = opng_bitset_find_next(value, i))
    {
        ++forward_count;
        printf(" %d", i);
        assert(i >= OPNG_BITSET_ELT_MIN && i <= OPNG_BITSET_ELT_MAX);
        assert(opng_bitset_test(value, i));
    }
    printf("\nreverse iteration:");
    reverse_count = 0;
    for (i = opng_bitset_find_last(value);
         i >= 0;
         i = opng_bitset_find_prev(value, i))
    {
        ++reverse_count;
        printf(" %d", i);
        assert(i >= OPNG_BITSET_ELT_MIN && i <= OPNG_BITSET_ELT_MAX);
        assert(opng_bitset_test(value, i));
    }
    printf("\n");
    assert(opng_bitset_count(value) == forward_count);
    assert(forward_count == reverse_count);
}

void
dump_error(int err)
{
    if (err == EINVAL)
        printf("error: EINVAL\n");
    else if (err == ERANGE)
        printf("error: ERANGE\n");
    else if (err != 0)
        printf("error: errno == %d\n", err);
}

int
main()
{
    char buf[256];
    size_t end_idx;
    opng_bitset_t set;
    int saved_errno;

    for ( ; ; )
    {
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            return 0;

        my_strrtrim(buf);
        if (buf[0] == 0)
            continue;

        printf("%s\n", buf);

        errno = 0;
        set = opng_rangeset_string_to_bitset(buf, &end_idx);
        saved_errno = errno;
        if (buf[end_idx] != 0)
            printf("...%s\n", &buf[end_idx]);
        check_bits(set);
        dump_bits(set);
        dump_error(saved_errno);
        printf("\n");
    }
}
