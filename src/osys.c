/*
 * osys.c - system extensions
 *
 * Copyright (C) 2003-2004 Cosmin Truta.
 * This program is open-source software.  See LICENSE for more details.
 */

#if defined UNIX || defined unix
# define OSYS_UNIX
#endif

#if defined OSYS_UNIX || defined __GNUC__
# include <unistd.h>
#endif

#if defined _POSIX_VERSION
# define OSYS_POSIX
# ifndef OSYS_UNIX
#  define OSYS_UNIX
# endif
#endif

#if defined MSDOS || defined __MSDOS__
# define OSYS_DOS
#endif

#if defined WIN32 || defined _WIN32 || defined __WIN32__
# define OSYS_WIN32
#endif

#ifndef OSYS_SLASH
# define OSYS_SLASH "/"
#endif

#ifndef OSYS_DOT
# define OSYS_DOT "."
#endif


#include <stddef.h>
#include <stdio.h>
#include <string.h>
#ifdef OSYS_UNIX
# include <sys/types.h>
# include <sys/stat.h>
# include <utime.h>
#endif
#ifdef OSYS_WIN32
# include <windows.h>
#endif

#include "osys.h"


/**
 * Creates a backup file name.
 * On success, returns buffer.
 * On error, returns NULL.
 **/
char *osys_bak_nam(char *buffer, size_t bufsize, const char *fname)
{
#if defined OSYS_DOS

#error osys_bak_nam for DOS is not implemented

#else

    if (strlen(fname) + sizeof(OSYS_DOT "bak") > bufsize)
        return NULL;
    strcpy(buffer, fname);
    strcat(buffer, OSYS_DOT "bak");
    return buffer;

#endif
}


/**
 * Copies the file mode and the time stamp of the file
 * named by destname into the file named by srcname.
 * On success, returns 0.
 * On error, sets the global variable errno and returns -1.
 **/
int osys_attr_cpy(const char *destname, const char *srcname)
{
#if defined OSYS_UNIX

    struct stat sbuf;
    int /* mode_t */ mode;
    struct utimbuf utbuf;

    if (stat(srcname, &sbuf) != 0)
        return -1;

    mode = (int)sbuf.st_mode;
    utbuf.actime = sbuf.st_atime;
    utbuf.modtime = sbuf.st_mtime;

    if (utime(destname, &utbuf) == 0 && chmod(destname, mode) == 0)
        return 0;
    else
        return -1;

#elif defined OSYS_WIN32

    HANDLE hFile;
    FILETIME ftLastWrite;
    BOOL result;

    hFile = CreateFile(srcname, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;
    result = GetFileTime(hFile, NULL, NULL, &ftLastWrite);
    CloseHandle(hFile);
    if (!result)
        return -1;

    hFile = CreateFile(destname, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;
    result = SetFileTime(hFile, NULL, NULL, &ftLastWrite);
    CloseHandle(hFile);
    if (!result)
        return -1;

    return 0;

    /* TODO: Copy file access mode. */

#else

    /* Do nothing. */
    return 0;

#endif
}
