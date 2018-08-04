/**
 ** strutil.h
 ** General-purpose string manipulation utilities.
 **
 ** Copyright (C) 2001-2006 Cosmin Truta.
 **
 ** This software is distributed under the same licensing and warranty
 ** terms as OptiPNG.  Please see the attached LICENSE for more info.
 **
 ** TO DO: Implement the corresponding wide-char functions.
 **/


#ifndef STRUTIL_H
#define STRUTIL_H

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************/
/* Auto-configuration                                                        */
/*****************************************************************************/


#if defined(UNIX) || defined(unix) || defined(__unix) || defined(__unix__)
#  ifndef HAVE_STRINGS_H
#    define HAVE_STRINGS_H
#  endif
#  ifndef HAVE_STRCASECMP
#    define HAVE_STRCASECMP
#  endif
#  ifndef HAVE_STRNCASECMP
#    define HAVE_STRNCASECMP
#  endif
#endif

#if defined(WIN32) || defined(_WIN32) || defined(_MSC_VER)
#  ifndef HAVE__STRICMP
#    define HAVE__STRICMP
#  endif
#  ifndef HAVE__STRNICMP
#    define HAVE__STRNICMP
#  endif
#  ifndef HAVE__STRLWR
#    define HAVE__STRLWR
#  endif
#  ifndef HAVE__STRUPR
#    define HAVE__STRUPR
#  endif
#endif

#if defined(__TURBOC__)
#  ifndef HAVE_STRICMP
#    define HAVE_STRICMP
#  endif
#  ifndef HAVE_STRNICMP
#    define HAVE_STRNICMP
#  endif
#  ifndef HAVE_STRLWR
#    define HAVE_STRLWR
#  endif
#  ifndef HAVE_STRUPR
#    define HAVE_STRUPR
#  endif
#endif

#include <stddef.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif


/*****************************************************************************/
/* Definitions and prototypes                                                */
/*                                                                           */
/* It's unfortunate that ISO C has tolower and toupper,                      */
/* but it does not have stricmp, strnicmp, strlower or strupper.             */
/*****************************************************************************/


/**
 * Compares two strings without case sensitivity.
 **/
#if defined(HAVE_STRCASECMP)
#  define string_case_cmp(str1, str2) strcasecmp(str1, str2)
#elif defined(HAVE_STRICMP)
#  define string_case_cmp(str1, str2) stricmp(str1, str2)
#elif defined(HAVE__STRICMP)
#  define string_case_cmp(str1, str2) _stricmp(str1, str2)
#elif defined(HAVE_STRCMPI)
#  define string_case_cmp(str1, str2) strcmpi(str1, str2)
#else
int string_case_cmp(const char *str1, const char *str2);
#endif


/**
 * Compares portions of two strings without case sensitivity.
 **/
#if defined(HAVE_STRNCASECMP)
#  define string_num_case_cmp(str1, str2, num) strncasecmp(str1, str2, num)
#elif defined(HAVE_STRNICMP)
#  define string_num_case_cmp(str1, str2, num) strnicmp(str1, str2, num)
#elif defined(HAVE__STRNICMP)
#  define string_num_case_cmp(str1, str2, num) _strnicmp(str1, str2, num)
#elif defined(HAVE_STRNCMPI)
#  define string_num_case_cmp(str1, str2, num) strncmpi(str1, str2, num)
#else
int string_num_case_cmp(const char *str1, const char *str2, size_t num);
#endif


/**
 * Converts the letters in a string to lowercase.
 **/
#if defined(HAVE_STRLWR)
#  define string_lower(str) strlwr(str)
#elif defined(HAVE__STRLWR)
#  define string_lower(str) _strlwr(str)
#else
char *string_lower(char *str);
#endif


/**
 * Converts the letters in a string to uppercase.
 **/
#if defined(HAVE_STRUPR)
#  define string_upper(str) strupr(str)
#elif defined(HAVE__STRUPR)
#  define string_upper(str) _strupr(str)
#else
char *string_upper(char *str);
#endif


/**
 * Checks if "prefix" is a prefix of "str", with case sensitivity.
 * @return  0  if "prefix" is a prefix of "str", and has at least
 *             "minlen" characters; otherwise:
 *         -1  if "str" is lexicographically smaller than "prefix";
 *          1  if "str" is lexicographically bigger than "prefix".
 **/
int string_prefix_cmp(const char *str, const char *prefix, size_t minlen);


/**
 * Checks if "prefix" is a prefix of "str", without case sensitivity.
 * @return  0  if "prefix" is a prefix of "str", and has at least
 *             "minlen" characters; otherwise:
 *         -1  if "str" is lexicographically smaller than "prefix";
 *          1  if "str" is lexicographically bigger than "prefix".
 **/
int string_prefix_case_cmp(const char *str, const char *prefix, size_t minlen);


/**
 * Skips the leading space characters.
 * The function returns the first non-space character in str.
 ** C++ ALERT
 * The following prototypes should be declared in C++:
 *  char *string_skip_spaces(char *str);
 *  const char *string_skip_spaces(const char *str);
 **/
char *string_skip_spaces(const char *str);
char *string_break_if(const char *str, int (*char_predicate)(int));


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* STRUTIL_H */
