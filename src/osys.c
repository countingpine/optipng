/*
 * osys.c - system extensions
 *
 * Copyright (C) 2003-2006 Cosmin Truta.
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

#if defined OS2 || defined OS_2 || defined __OS2__
# define OSYS_OS2
#endif

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined _WIN32_WCE
# define OSYS_WIN32
#endif

#if defined WIN64 || defined _WIN64 || defined __WIN64__
# define OSYS_WIN64
#endif

#if /* defined _WINDOWS || */ defined OSYS_WIN32 || defined OSYS_WIN64
# define OSYS_WINDOWS
#endif

#define OSYS_FNAME_CHR_SLASH '/'
#define OSYS_FNAME_CHR_DOT   '.'
#define OSYS_FNAME_STR_SLASH "/"
#define OSYS_FNAME_STR_DOT   "."

#if defined OSYS_DOS || defined OSYS_OS2 || defined OSYS_WINDOWS
# define OSYS_FNAME_IGN_CASE 1
#else  /* OSYS_UNIX and possibly others */
# define OSYS_FNAME_IGN_CASE 0
#endif


#include <stddef.h>
#include <stdio.h>
#include <string.h>
#ifdef OSYS_UNIX
# include <sys/types.h>
# include <sys/stat.h>
# include <utime.h>
#endif
#ifdef OSYS_WINDOWS
# include <windows.h>
#endif

#include "osys.h"


/**
 * Copies the file mode and the time stamp of the file
 * named by destname into the file named by srcname.
 * On success, returns 0.
 * On error, sets the global variable errno and returns -1.
 **/
int osys_fattr_cpy(const char *destname, const char *srcname)
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

#elif defined OSYS_WINDOWS

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


/**
 * Creates a backup file name.
 * On success, returns buffer.
 * On error, returns NULL.
 **/
char *osys_fname_mkbak(char *buffer, size_t bufsize, const char *fname)
{
    if (strlen(fname) + sizeof(OSYS_FNAME_STR_DOT "bak") > bufsize)
        return NULL;

#if defined OSYS_DOS

    return osys_fname_chext(buffer, bufsize, fname, OSYS_FNAME_STR_DOT "bak");

#else

    strcpy(buffer, fname);
    strcat(buffer, OSYS_FNAME_STR_DOT "bak");
    return buffer;

#endif
}


/**
 * Creates a file name by changing the extension of a given file name.
 * The new extension can be the empty string, indicating that the new
 * file name has no extension.  Otherwise, it must begin with the
 * extension separator (usually '.').
 * On success, returns buffer.
 * On error, returns NULL.
 **/
char *osys_fname_chext(char *buffer, size_t bufsize,
    const char *oldname, const char *newext)
{
    size_t i, pos;

    if (newext[0] != OSYS_FNAME_CHR_DOT)
        return NULL;  /* invalid argument */
    for (i = 0, pos = (size_t)(-1); oldname[i] != 0; ++i)
    {
        if (i >= bufsize)
            return NULL;  /* overflow */
        if ((buffer[i] = oldname[i]) == OSYS_FNAME_CHR_DOT)
            pos = i;
    }
    if (i > pos)
        i = pos;  /* go back only if oldname has an extension */
    for ( ;; ++i, ++newext)
    {
        if (i >= bufsize)
            return NULL;  /* overflow */
        if ((buffer[i] = *newext) == 0)
            return buffer;  /* success */
    }
}


/**
 * Compares one file name to another.
 * It returns a value (less than, equal to, or greater than 0)
 * based on the result of comparing name1 to name2.
 * The comparison may or may not be case sensitive, depending on
 * the operating system.
 **/
int osys_fname_cmp(const char *name1, const char *name2)
{
#if OSYS_FNAME_IGN_CASE
# ifdef OSYS_WINDOWS
    return lstrcmpi(name1, name2);
# else
    return stricmp(name1, name2);
# endif
#else
    return strcmp(name1, name2);
#endif
}


/**
 * Reads a block of data from the specified file offset.
 * The file-position indicator is saved and restored after reading.
 * The file buffer is flushed before and after reading.
 * On success, the function returns the number of bytes read.
 * On error, it returns 0.
 **/
size_t osys_fread_at(FILE *stream, long offset, int whence,
    void *block, size_t blocksize)
{
    fpos_t pos;
    size_t result;

    if (fflush(stream) != 0 || block == NULL || blocksize == 0
            || fgetpos(stream, &pos) != 0)
        return 0;
    if (fseek(stream, offset, whence) == 0)
        result = fread(block, 1, blocksize, stream);
    else
        result = 0;
    if (fflush(stream) != 0) result = 0;
    if (fsetpos(stream, &pos) != 0) result = 0;
    if (fflush(stream) != 0) result = 0;
    return result;
}


/**
 * Writes a block of data at the specified file offset.
 * The file-position indicator is saved and restored after writing.
 * The file buffer is flushed before and after writing.
 * On success, the function returns the number of bytes written.
 * On error, it returns 0.
 **/
size_t osys_fwrite_at(FILE *stream, long offset, int whence,
    const void *block, size_t blocksize)
{
    fpos_t pos;
    size_t result;

    if (fflush(stream) != 0 || block == NULL || blocksize == 0
            || fgetpos(stream, &pos) != 0)
        return 0;
    if (fseek(stream, offset, whence) == 0)
        result = fwrite(block, 1, blocksize, stream);
    else
        result = 0;
    if (fflush(stream) != 0) result = 0;
    if (fsetpos(stream, &pos) != 0) result = 0;
    if (fflush(stream) != 0) result = 0;
    return result;
}
