/*
 * bitset.h
 * Plain old bitset data type.
 *
 * Copyright (C) 2001-2017 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#ifndef OPNG_BITSET_H_
#define OPNG_BITSET_H_

#include <limits.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
 * The bitset type.
 */
typedef unsigned int opng_bitset_t;


/*
 * Bitset constants.
 */
#define OPNG_BITSET_EMPTY 0U
#define OPNG_BITSET_FULL (~0U)


/*
 * The size operator (not restricted to opng_bitset_t).
 */
#define OPNG_BITSIZEOF(object) (sizeof(object) * CHAR_BIT)


/*
 * Bitset element limits.
 */
enum
{
    OPNG_BITSET_ELT_MIN = 0,
    OPNG_BITSET_ELT_MAX = (int)(OPNG_BITSIZEOF(opng_bitset_t) - 1)
};


/*
 * Direct bitset access methods.
 */
#ifdef __cplusplus

inline int
opng_bitset_test(opng_bitset_t set, int elt)
{
    return (set & (1U << elt)) != 0;
}

inline void
opng_bitset_set(opng_bitset_t *set, int elt)
{
    *set |= (1U << elt);
}

inline void
opng_bitset_reset(opng_bitset_t *set, int elt)
{
    *set &= ~(1U << elt);
}

inline void
opng_bitset_flip(opng_bitset_t *set, int elt)
{
    *set ^= (1U << elt);
}

inline opng_bitset_t
opng_bitset__range__(int start_elt, int stop_elt)
{
    return ((1U << (stop_elt - start_elt) << 1) - 1) << start_elt;
}

inline int
opng_bitset_test_all_in_range(opng_bitset_t set, int start_elt, int stop_elt)
{
    return (start_elt <= stop_elt) ?
           ((~set & opng_bitset__range__(start_elt, stop_elt)) == 0) :
           1;
}

inline int
opng_bitset_test_any_in_range(opng_bitset_t set, int start_elt, int stop_elt)
{
    return (start_elt <= stop_elt) ?
           ((set & opng_bitset__range__(start_elt, stop_elt)) != 0) :
           0;
}

inline void
opng_bitset_set_range(opng_bitset_t *set, int start_elt, int stop_elt)
{
    if (start_elt <= stop_elt)
        *set |= opng_bitset__range__(start_elt, stop_elt);
}

inline void
opng_bitset_reset_range(opng_bitset_t *set, int start_elt, int stop_elt)
{
    if (start_elt <= stop_elt)
        *set &= ~opng_bitset__range__(start_elt, stop_elt);
}

inline void
opng_bitset_flip_range(opng_bitset_t *set, int start_elt, int stop_elt)
{
    if (start_elt <= stop_elt)
        *set ^= opng_bitset__range__(start_elt, stop_elt);
}

#else  /* !__cplusplus */

#define opng_bitset_test(set, elt) \
    (((set) & (1U << (elt))) != 0)

#define opng_bitset_set(set, elt) \
    (*(set) |= (1U << (elt)))

#define opng_bitset_reset(set, elt) \
    (*(set) &= ~(1U << (elt)))

#define opng_bitset_flip(set, elt) \
    (*(set) ^= (1U << (elt)))

#define opng_bitset__range__(start_elt, stop_elt) \
    (((1U << ((stop_elt) - (start_elt)) << 1) - 1) << (start_elt))

#define opng_bitset_test_all_in_range(set, start_elt, stop_elt) \
    (((start_elt) <= (stop_elt)) ? \
     (~(set) & opng_bitset__range__(start_elt, stop_elt)) == 0 : \
     1)

#define opng_bitset_test_any_in_range(set, start_elt, stop_elt) \
    (((start_elt) <= (stop_elt)) ? \
     ((set) & opng_bitset__range__(start_elt, stop_elt)) != 0 : \
     0)

#define opng_bitset_set_range(set, start_elt, stop_elt) \
    (*(set) |= ((start_elt) <= (stop_elt)) ? \
               opng_bitset__range__(start_elt, stop_elt) : \
               0U)

#define opng_bitset_reset_range(set, start_elt, stop_elt) \
    (*(set) &= ((start_elt) <= (stop_elt)) ? \
               ~opng_bitset__range__(start_elt, stop_elt) : \
               ~0U)

#define opng_bitset_flip_range(set, start_elt, stop_elt) \
    (*(set) ^= ((start_elt) <= (stop_elt)) ? \
               opng_bitset__range__(start_elt, stop_elt) : \
               0U)

#endif  /* __cplusplus */


/*
 * Counts the number of elements in a bitset.
 *
 * The function returns the number of bits set to 1.
 */
unsigned int
opng_bitset_count(opng_bitset_t set);

/*
 * Finds the first element in a bitset.
 *
 * The function returns the position of the first bit set to 1,
 * or -1 if all bits are set to 0.
 */
int
opng_bitset_find_first(opng_bitset_t set);

/*
 * Finds the next element in a bitset.
 *
 * The function returns the position of the next bit set to 1,
 * or -1 if all the following bits are set to 0.
 */
int
opng_bitset_find_next(opng_bitset_t set, int elt);

/*
 * Finds the last element in a bitset.
 *
 * The function returns the position of the last bit set to 1,
 * or -1 if all bits are set to 0.
 */
int
opng_bitset_find_last(opng_bitset_t set);

/*
 * Finds the previous element in a bitset.
 *
 * The function returns the position of the previous bit set to 1,
 * or -1 if all the preceding bits are set to 0.
 */
int
opng_bitset_find_prev(opng_bitset_t set, int elt);

/*
 * Parses a rangeset string and converts the result to a bitset.
 *
 * A rangeset string is an arbitrary sequence of non-negative integers ("N")
 * and ranges ("M-N" or "M-"), represented in base 10, and separated by comma
 * or semicolon characters. Whitespace is allowed around lexical elements,
 * and is ignored.
 *
 * Here are a few examples, assuming the input mask is 0xffff:
 *  ""         => 0000000000000000
 *  "0"        => 0000000000000001
 *  "0,3,5-7"  => 0000000011101001
 *  "0-3,5,7-" => 1111111110101111
 *  "7-,5"     => 1111111110100000
 *  "7-7"      => 0000000010000000
 *  "7-5"      => 1111111111111111, range error
 *  "99"       => 1111111111111111, range error
 *  "1-2-3"    => 0000000000000000, invalid input error
 *
 * On success, the function sets the output value to the converted bitset.
 * If the input is well-formed, but contains elements or ranges that are not
 * representable within the given mask, the function sets errno to ERANGE,
 * and sets the output value to BITSET_FULL.
 * If the input is ill-formed, the function sets errno to EINVAL, and sets
 * the output value to BITSET_EMPTY.
 *
 * The function returns 0 on success, or -1 on error.
 */
int
opng_strparse_rangeset_to_bitset(opng_bitset_t *out_set,
                                 const char *rangeset_str,
                                 opng_bitset_t mask_set);

/*
 * Formats a bitset using the rangeset string representation.
 *
 * The function returns the length of the rangeset string representation.
 * Upon return, if the output buffer is large enough, it shall contain the
 * rangeset string. Otherwise, the buffer shall remain intact.
 */
size_t
opng_strformat_bitset_as_rangeset(char *out_buf,
                                  size_t out_buf_size,
                                  opng_bitset_t bitset);

/*
 * TODO:
 * opng_wcsparse_rangeset_to_bitset
 * opng_wcsformat_bitset_as_rangeset
 */


#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif  /* OPNG_BITSET_H_ */
