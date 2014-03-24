/*
 * wildargs.c
 * Automatic command-line wildcard expansion for environments that
 * are not based on the Un*x shell.
 *
 * Copyright (C) 2003-2014 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

/*
 * Automatic wildcard expansion for Microsoft Visual C++.
 */
#ifdef _MSC_VER
#if defined _WIN32 || defined _WIN64
/* The following line is inspired from MinGW32 by Colin Peters. */
int _dowildcard = 1;
#endif
#endif

/*
 * Automatic wildcard expansion for Borland C++.
 */
#ifdef __BORLANDC__
#if defined _WIN32 || defined __WIN32__ || defined _WIN64 || defined __WIN64__
/* The following lines are inspired from BMP2PNG by MIYASAKA Masaru. */
#include <wildargs.h>
typedef void _RTLENTRY (* _RTLENTRY _argv_expand_fn)(char *, _PFN_ADDARG);
typedef void _RTLENTRY (* _RTLENTRY _wargv_expand_fn)(wchar_t *, _PFN_ADDARG);
_argv_expand_fn _argv_expand_ptr = _expand_wild;
_wargv_expand_fn _wargv_expand_ptr = _wexpand_wild;
#endif
#endif
