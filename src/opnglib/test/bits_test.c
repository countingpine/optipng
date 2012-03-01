/*
 * bits_test.c
 * Test for optk/bits.h
 *
 * Copyright (C) 2001-2011 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "optk/bits.h"

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
check_bits(optk_bits_t value)
{
    optk_bits_t empty, full;
    optk_bits_t flipped1, flipped2, flipped3, flipped4;
    int empty_is_empty, full_is_full;
    int check_test_all, check_test_any;
    int i;

    empty = full = flipped1 = value;
    optk_bits_reset_range(&empty, OPTK_BITS_ELT_MIN, OPTK_BITS_ELT_MAX);
    optk_bits_set_range(&full, OPTK_BITS_ELT_MIN, OPTK_BITS_ELT_MAX);
    optk_bits_flip_range(&flipped1, OPTK_BITS_ELT_MIN, OPTK_BITS_ELT_MAX);
    assert(empty == 0);
    assert(full == (optk_bits_t)~0);
    assert(flipped1 == ~value);

    empty_is_empty =
        !optk_bits_test_any_in_range(empty,
                                     OPTK_BITS_ELT_MIN, OPTK_BITS_ELT_MAX);
    full_is_full =
        optk_bits_test_all_in_range(full,
                                    OPTK_BITS_ELT_MIN, OPTK_BITS_ELT_MAX);
    assert(empty_is_empty);
    assert(full_is_full);

    flipped4 = flipped3 = flipped2 = flipped1;
    for (i = OPTK_BITS_ELT_MIN; i <= OPTK_BITS_ELT_MAX; ++i)
    {
        optk_bits_flip(&flipped1, i);
        optk_bits_flip_range(&flipped2, i, i);
        if (optk_bits_test(flipped3, i))
        {
            optk_bits_reset(&flipped3, i);
            optk_bits_reset_range(&flipped4, i, i);
            check_test_all = !optk_bits_test_all_in_range(flipped3, i, i);
            check_test_any = !optk_bits_test_any_in_range(flipped4, i, i);
        }
        else
        {
            optk_bits_set(&flipped3, i);
            optk_bits_set_range(&flipped4, i, i);
            check_test_all = optk_bits_test_all_in_range(flipped3, i, i);
            check_test_any = optk_bits_test_any_in_range(flipped4, i, i);
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
dump_bits(optk_bits_t value)
{
    unsigned int forward_count, reverse_count;
    int i;

    printf("count: %u\n", optk_bits_count(value));
    printf("iteration:");
    forward_count = 0;
    for (i = optk_bits_find_first(value);
         i >= 0;
         i = optk_bits_find_next(value, i))
    {
        ++forward_count;
        printf(" %d", i);
        assert(i >= OPTK_BITS_ELT_MIN && i <= OPTK_BITS_ELT_MAX);
        assert(optk_bits_test(value, i));
    }
    printf("\nreverse iteration:");
    reverse_count = 0;
    for (i = optk_bits_find_last(value);
         i >= 0;
         i = optk_bits_find_prev(value, i))
    {
        ++reverse_count;
        printf(" %d", i);
        assert(i >= OPTK_BITS_ELT_MIN && i <= OPTK_BITS_ELT_MAX);
        assert(optk_bits_test(value, i));
    }
    printf("\n");
    assert(optk_bits_count(value) == forward_count);
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
    optk_bits_t set;
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
        set = optk_rangeset_string_to_bits(buf, &end_idx);
        saved_errno = errno;
        if (buf[end_idx] != 0)
            printf("...%s\n", &buf[end_idx]);
        check_bits(set);
        dump_bits(set);
        dump_error(saved_errno);
        printf("\n");
    }
}
