/**
 ** wildargs.c
 ** Automatic wildcard expansion in the command line.
 ** This module is for non-Unix environments, such as Win32.
 **
 ** NOT COPYRIGHTED by Cosmin Truta, 2003.
 **/

/* The following code is inspired from MinGW32 by Colin Peters. */
#if defined WIN32 || defined _WIN32 || defined _MSC_VER
int _dowildcard = 1;
#endif

/* The following code is inspired from BMP2PNG by MIYASAKA Masaru. */
#if defined __WIN32__ && defined __BORLANDC__
#include <wildargs.h>
typedef void _RTLENTRY (* _RTLENTRY _argv_expand_fn)(char *, _PFN_ADDARG);
typedef void _RTLENTRY (* _RTLENTRY _wargv_expand_fn)(wchar_t *, _PFN_ADDARG);
_argv_expand_fn  _argv_expand_ptr  = _expand_wild;
_wargv_expand_fn _wargv_expand_ptr = _wexpand_wild;
#endif
