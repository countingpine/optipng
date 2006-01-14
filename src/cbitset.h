/**
 ** bitset.h -- Simple C routines for bitset handling.
 **
 ** Copyright (C) 2001-2003 Cosmin Truta.
 **
 ** This software is distributed under the same licensing and warranty
 ** terms as OptiPNG. Please see the attached LICENSE for more info.
 **/


#ifndef CBITSET_H
#define CBITSET_H


/**
 * The bitset type.
 **/
typedef int bitset;


/**
 * Macros for bitset handling.
 **/
#define BITSET_SIZE     (8 * sizeof(int) - 1)
#define BITSET_EMPTY    (0)
#define BITSET_INVALID  (-1)

#define BITSET_IS_EMPTY(_set)  \
    ((_set) == 0)
#define BITSET_IS_VALID(_set)  \
    ((_set) >= 0)

#define BITSET_GET(_set, _item)   \
    (((1 << (_item)) & (_set)) != 0)
#define BITSET_RESET(_set, _item) \
    ((_set) &= ~(1 << (_item)))
#define BITSET_SET(_set, _item)   \
    ((_set) |= (1 << (_item)))


/**
 * Counts the number of elements in a bitset.
 * If the input is invalid, the function returns -1.
 **/
int bitset_count(bitset set);


/**
 * Converts a string to a bitset.
 * A valid input may contain only the characters '0' and '1'.
 * The leading and trailing non-printable characters (including ' ')
 * are ignored.
 * If the input is invalid, the function returns BITSET_INVALID.
 **/
bitset string_to_bitset(const char *str);


/**
 * Converts a bitset to a string.
 * If the input is valid, the function return str_buf.
 * Otherwise, it returns NULL.
 **/
char *bitset_to_string(bitset set, char *str_buf);


/**
 * Converts a text to a bitset.
 * A valid input may contain digits and the separators '-', ',', ';',
 * and it must match the regular expression:
 *  "[-0-9,;]*"
 * The following examples assume BITSET_SIZE == 15:
 *  ""        => 000000000000000
 *  "0-2,4-5" => 000000000110111
 *  "-3,5,7-" => 111111110101111
 *  "9-,6"    => 111111001000000
 *  "8-4"     => 000000000000000
 *  "-"       => 111111111111111
 * The non-printable characters (including ' ') are ignored.
 * If the input is invalid, the function returns BITSET_INVALID.
 **/
bitset text_to_bitset(const char *text);


/**
 * Converts a bitset to a text.
 * If the input is valid, the function return text_buf.
 * Otherwise, it returns NULL.
 * The space allocated for text_buf must be large enough to hold the
 * returned string, including the terminating null character.
 **/
char *bitset_to_text(bitset set, char *text_buf);


#endif  /* CBITSET_H */
