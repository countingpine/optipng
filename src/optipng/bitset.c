/*
 * bitset.c
 * Plain old bitset data type.
 *
 * Copyright (C) 2001-2017 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "bitset.h"

#include <ctype.h>
#include <errno.h>
#include <stddef.h>


/*
 * Returns the minimum of two given values.
 */
#define opng__MIN__(a, b) \
    ((a) < (b) ? (a) : (b))

/*
 * Returns the maximum of two given values.
 */
#define opng__MAX__(a, b) \
    ((a) > (b) ? (a) : (b))

/*
 * Spans the given pointer past the elements that satisfy the given predicate.
 * E.g., opng__SPAN__(str, isspace) moves str past the leading whitespace.
 */
#define opng__SPAN__(ptr, predicate) \
    { \
        while ((predicate)(*(ptr))) \
            ++(ptr); \
    }


/*
 * Counts the number of elements in a bitset.
 */
unsigned int
opng_bitset_count(opng_bitset_t set)
{
    unsigned int result;

    /* Apply Wegner's method. */
    result = 0;
    while (set != 0)
    {
        set &= (set - 1);
        ++result;
    }
    return result;
}

/*
 * Finds the first element in a bitset.
 */
int
opng_bitset_find_first(opng_bitset_t set)
{
    int i;

    for (i = 0; i <= OPNG_BITSET_ELT_MAX; ++i)
    {
        if (opng_bitset_test(set, i))
            return i;
    }
    return -1;
}

/*
 * Finds the next element in a bitset.
 */
int
opng_bitset_find_next(opng_bitset_t set, int elt)
{
    int i;

    for (i = opng__MAX__(elt, -1) + 1; i <= OPNG_BITSET_ELT_MAX; ++i)
    {
        if (opng_bitset_test(set, i))
            return i;
    }
    return -1;
}

/*
 * Finds the last element in a bitset.
 */
int
opng_bitset_find_last(opng_bitset_t set)
{
    int i;

    for (i = OPNG_BITSET_ELT_MAX; i >= 0; --i)
    {
        if (opng_bitset_test(set, i))
            return i;
    }
    return -1;
}

/*
 * Finds the previous element in a bitset.
 */
int
opng_bitset_find_prev(opng_bitset_t set, int elt)
{
    int i;

    for (i = opng__MIN__(elt, OPNG_BITSET_ELT_MAX + 1) - 1; i >= 0; --i)
    {
        if (opng_bitset_test(set, i))
            return i;
    }
    return -1;
}

/*
 * Parses a rangeset string and converts the result to a bitset.
 */
int
opng_strparse_rangeset_to_bitset(opng_bitset_t *out_set,
                                 const char *rangeset_str,
                                 opng_bitset_t mask_set)
{
    opng_bitset_t result;
    const char *ptr;
    int state;
    int num, num1, num2;
    int err_invalid, err_range;

    result = OPNG_BITSET_EMPTY;
    ptr = rangeset_str;
    state = 0;
    err_invalid = err_range = 0;
    num1 = num2 = -1;

    for ( ; ; )
    {
        opng__SPAN__(ptr, isspace);
        switch (state)
        {
        case 0:  /* "" */
        case 2:  /* "N-" */
            /* Expecting number; go to next state. */
            if (*ptr >= '0' && *ptr <= '9')
            {
                num = 0;
                do
                {
                    num = 10 * num + (*ptr - '0');
                    if (num > OPNG_BITSET_ELT_MAX)
                    {
                        num = OPNG_BITSET_ELT_MAX;
                        err_range = 1;
                    }
                    ++ptr;
                } while (*ptr >= '0' && *ptr <= '9');
                if (!opng_bitset_test(mask_set, num))
                    err_range = 1;
                if (state == 0)
                    num1 = num;
                num2 = num;
                ++state;
                continue;
            }
            break;
        case 1:  /* "N" */
            /* Expecting range operator; go to next state. */
            if (*ptr == '-')
            {
                ++ptr;
                num2 = OPNG_BITSET_ELT_MAX;
                ++state;
                continue;
            }
            break;
        }

        if (state > 0)  /* "N", "N-" or "N-N" */
        {
            /* Store the partial result; go to state 0. */
            if (num1 <= num2)
            {
                opng_bitset_set_range(&result, num1, num2);
                result &= mask_set;
            }
            else
            {
                /* Incorrect range operands. */
                err_range = 1;
            }
            state = 0;
        }

        if (*ptr == ',' || *ptr == ';')
        {
            /* Separator: continue the loop. */
            ++ptr;
            continue;
        }
        else if (*ptr == '-')
        {
            /* Unexpected range operator: invalidate and exit the loop. */
            err_invalid = 1;
            break;
        }
        else
        {
            /* Unexpected character or end of string: exit the loop. */
            break;
        }
    }

    opng__SPAN__(ptr, isspace);
    if (*ptr != '\0')
    {
        /* Unexpected trailing character: invalidate. */
        err_invalid = 1;
    }

    if (err_invalid)
    {
        /* Invalid input error. */
#ifdef EINVAL
        errno = EINVAL;
#endif
        *out_set = OPNG_BITSET_EMPTY;
        return -1;
    }
    else if (err_range)
    {
        /* Range error. */
#ifdef ERANGE
        errno = ERANGE;
#endif
        *out_set = OPNG_BITSET_FULL;
        return -1;
    }
    else
    {
        /* Success. */
        *out_set = result;
        return 0;
    }
}

/*
 * Formats a bitset using the rangeset string representation.
 */
size_t
opng_strformat_bitset_as_rangeset(char *out_buf,
                                  size_t out_buf_size,
                                  opng_bitset_t bitset);
/* TODO */
