/*
 * osys.c
 * System extensions.
 *
 * Copyright (C) 2003-2010 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the attached LICENSE for more information.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Auto-configuration.
 */
#if defined UNIX || defined __unix || defined __unix__ || defined __UNIX__ || \
    defined _POSIX_SOURCE || defined _POSIX_C_SOURCE || defined _XOPEN_SOURCE
#  define OSYS_UNIX
/* To be continued. */
#endif

#if defined DOS || defined _DOS || defined __DOS__ || \
    defined MSDOS || defined _MSDOS || defined __MSDOS__
#  define OSYS_DOS
#endif

#if defined OS2 || defined OS_2 || defined __OS2__
#  define OSYS_OS2
#endif

#if defined OSYS_DOS || defined OSYS_OS2
#  define OSYS_DOS_OS2
#endif

#if defined WIN32 || defined _WIN32 || defined _WIN32_WCE || \
    defined __WIN32__ || defined __NT__
#  define OSYS_WIN32
#endif

#if defined WIN64 || defined _WIN64 || defined __WIN64__
#  define OSYS_WIN64
#endif

#if defined WINDOWS || defined OSYS_WIN32 || defined OSYS_WIN64
#  define OSYS_WINDOWS
#endif

#if defined __CYGWIN__ || defined __DJGPP__ || defined __EMX__
#  define OSYS_UNIXISH
#endif

#if defined OSYS_DOS_OS2 || defined OSYS_WINDOWS || defined OSYS_UNIXISH
#  include <io.h>
#endif

#if defined OSYS_UNIX || defined OSYS_UNIXISH || \
    (!defined OSYS_DOS_OS2 && !defined OSYS_WINDOWS && \
     !defined _ANSI_SOURCE && !defined _ANSI_C_SOURCE)
#  include <unistd.h>
#endif

#if defined _POSIX_VERSION || defined _XOPEN_VERSION
#  ifndef OSYS_UNIX
#    define OSYS_UNIX
#  endif
#endif

#if defined OSYS_UNIX || defined OSYS_UNIXISH
#  include <strings.h>
#endif

#if defined OSYS_UNIX || defined OSYS_DOS_OS2 || defined OSYS_WINDOWS
#  include <sys/stat.h>
#  include <sys/types.h>
#  ifdef _MSC_VER
#    include <sys/utime.h>
#  else
#    include <utime.h>
#  endif
#endif

#if defined OSYS_DOS_OS2
#  include <process.h>
#endif

#if defined OSYS_WINDOWS
#  include <windows.h>
#endif

#include "osys.h"


/*
 * More auto-configuration.
 */
#if defined OSYS_DOS_OS2 || defined OSYS_WINDOWS
#  define OSYS_PATH_CHR_SLASH '\\'
#  define OSYS_PATH_STR_SLASH "\\"
#  define OSYS_PATH_STRLIST_SLASH "/\\"
#else
#  define OSYS_PATH_CHR_SLASH '/'
#  define OSYS_PATH_STR_SLASH "/"
#  if defined OSYS_UNIXISH
#    define OSYS_PATH_STRLIST_SLASH "/\\"
#  else
#    define OSYS_PATH_STRLIST_SLASH "/"
#  endif
#endif
#define OSYS_PATH_CHR_DOT '.'
#define OSYS_PATH_STR_DOT "."
#define OSYS_PATH_CHR_QUESTION '?'
#define OSYS_PATH_STR_QUESTION "?"
#define OSYS_PATH_CHR_STAR '*'
#define OSYS_PATH_STR_STAR "*"

#if defined OSYS_DOS_OS2 || defined OSYS_WINDOWS || defined OSYS_UNIXISH
#  define OSYS_PATH_ICASE 1
#  define OSYS_PATH_DOS
#  define OSYS_PATH_IS_DRIVE(ch) \
          (((ch) >= 'A' && (ch) <= 'Z') || ((ch) >= 'a' && (ch) <= 'z'))
#else  /* OSYS_UNIX and others */
#  define OSYS_PATH_ICASE 0
#endif

#ifdef R_OK
#  define OSYS_FTEST_READ R_OK
#else
#  define OSYS_FTEST_READ 4
#endif
#ifdef W_OK
#  define OSYS_FTEST_WRITE W_OK
#else
#  define OSYS_FTEST_WRITE 2
#endif
#ifdef X_OK
#  define OSYS_FTEST_EXEC X_OK
#else
#  define OSYS_FTEST_EXEC 1
#endif
#ifdef F_OK
#  define OSYS_FTEST_FILE F_OK
#else
#  define OSYS_FTEST_FILE 0
#endif


/*
 * Creates a new file path by changing the directory component of
 * a given file path.
 */
char *
osys_path_chdir(char *buffer, size_t bufsize,
                const char *old_path, const char *new_dirname)
{
    const char *path, *ptr;
    size_t dirlen;

    /* Extract file name from old_path. */
    path = old_path;
#ifdef OSYS_PATH_DOS
    if (OSYS_PATH_IS_DRIVE(path[0]) && path[1] == ':')
        path += 2;  /* skip drive name */
#endif
    for ( ; ; )
    {
        ptr = strpbrk(path, OSYS_PATH_STRLIST_SLASH);
        if (ptr == NULL)
            break;
        path = ptr + 1;
    }

    /* Make sure the buffer is large enough. */
    dirlen = strlen(new_dirname);
    if (dirlen + strlen(path) + 2 >= bufsize)
        return NULL;  /* overflow */

    /* Copy the new directory name. Also append a slash if necessary. */
    if (dirlen > 0)
    {
        strcpy(buffer, new_dirname);
#ifdef OSYS_PATH_DOS
        if (dirlen == 2 && buffer[1] == ':' && OSYS_PATH_IS_DRIVE(buffer[0]))
            (void)0;  /* do nothing */
        else
#endif
        if (strchr(OSYS_PATH_STRLIST_SLASH, buffer[dirlen - 1]) == NULL)
            buffer[dirlen++] = OSYS_PATH_CHR_SLASH;  /* append slash to dir */
    }

    /* Append the file name. */
    strcpy(buffer + dirlen, path);
    return buffer;  /* success */
}


/*
 * Creates a new file path by changing the extension component of
 * a given file path.
 */
char *
osys_path_chext(char *buffer, size_t bufsize,
                const char *old_path, const char *new_extname)
{
    size_t i, pos;

    if (new_extname[0] != OSYS_PATH_CHR_DOT)
        return NULL;  /* invalid argument */
    for (i = 0, pos = (size_t)(-1); old_path[i] != 0; ++i)
    {
        if (i >= bufsize)
            return NULL;  /* overflow */
        if ((buffer[i] = old_path[i]) == OSYS_PATH_CHR_DOT)
            pos = i;
    }
    if (i > pos)
        i = pos;  /* go back only if old_path has an extension */
    for ( ; ; ++i, ++new_extname)
    {
        if (i >= bufsize)
            return NULL;  /* overflow */
        if ((buffer[i] = *new_extname) == 0)
            return buffer;  /* success */
    }
}


/*
 * Compares one file path to another.
 */
int
osys_path_cmp(const char *path1, const char *path2)
{
#if defined OSYS_DOS_OS2

    return stricmp(path1, path2);

#elif defined OSYS_WINDOWS

    return lstrcmpiA(path1, path2);

#elif OSYS_PATH_ICASE  /* generic, case-insensitive */

    return strcasecmp(path1, path2);

#else  /* generic, case-sensitive */

    return strcmp(path1, path2);

#endif
}


/*
 * Creates a backup file path.
 */
char *
osys_path_mkbak(char *buffer, size_t bufsize, const char *path)
{
    if (strlen(path) + sizeof(OSYS_PATH_STR_DOT "bak") > bufsize)
        return NULL;

#if defined OSYS_DOS

    return osys_path_chext(buffer, bufsize, path, OSYS_PATH_STR_DOT "bak");

#else  /* OSYS_UNIX and others */

    strcpy(buffer, path);
    strcat(buffer, OSYS_PATH_STR_DOT "bak");
    return buffer;

#endif
}


/*
 * Opens a file and positions it at the specified file offset.
 */
FILE *
osys_fopen_at(const char *path, const char *mode,
              long offset, int whence)
{
    FILE *stream;

    if ((stream = fopen(path, mode)) == NULL)
        return NULL;
    if (offset == 0 && (whence == SEEK_SET || whence == SEEK_CUR))
        return stream;
    if (fseek(stream, offset, whence) != 0)
    {
        fclose(stream);
        return NULL;
    }
    return stream;
}


/*
 * Reads a block of data from the specified file offset.
 */
size_t
osys_fread_at(FILE *stream, long offset, int whence,
              void *block, size_t blocksize)
{
    fpos_t pos;
    size_t result;

    if (fgetpos(stream, &pos) != 0)
        return 0;
    if (fseek(stream, offset, whence) == 0)
        result = fread(block, 1, blocksize, stream);
    else
        result = 0;
    if (fsetpos(stream, &pos) != 0)
        result = 0;
    return result;
}


/*
 * Writes a block of data at the specified file offset.
 */
size_t
osys_fwrite_at(FILE *stream, long offset, int whence,
               const void *block, size_t blocksize)
{
    fpos_t pos;
    size_t result;

    if (fgetpos(stream, &pos) != 0 || fflush(stream) != 0)
        return 0;
    if (fseek(stream, offset, whence) == 0)
        result = fwrite(block, 1, blocksize, stream);
    else
        result = 0;
    if (fflush(stream) != 0)
        result = 0;
    if (fsetpos(stream, &pos) != 0)
        result = 0;
    return result;
}


/*
 * Determines if the accessibility of the specified file path satisfies
 * the specified access mode.
 */
int
osys_ftest(const char *path, const char *mode)
{
    int faccess, freg;

    faccess = freg = 0;
    if (strchr(mode, 'f') != NULL)
        freg = 1;
    if (strchr(mode, 'r') != NULL)
        faccess |= OSYS_FTEST_READ;
    if (strchr(mode, 'w') != NULL)
        faccess |= OSYS_FTEST_WRITE;
    if (strchr(mode, 'x') != NULL)
        faccess |= OSYS_FTEST_EXEC;
    if (faccess == 0 && freg == 0)
        if (strchr(mode, 'e') == NULL)
            return 0;

#if defined OSYS_UNIX || defined OSYS_DOS_OS2

    {
        struct stat sbuf;

        if (stat(path, &sbuf) != 0)
            return -1;
        if (freg && !(sbuf.st_mode & S_IFREG))
            return -1;
        if (faccess == 0)
            return 0;
        return access(path, faccess);
    }

#elif defined OSYS_WINDOWS

    {
        DWORD attr;

        attr = GetFileAttributesA(path);
        if (attr == 0xffffffffU)
            return -1;
        if (freg && (attr & FILE_ATTRIBUTE_DIRECTORY))
            return -1;
        if ((faccess & OSYS_FTEST_WRITE) && (attr & FILE_ATTRIBUTE_READONLY))
            return -1;
        return 0;
    }

#else  /* generic */

    {
        FILE *stream;

        if (faccess & OSYS_FTEST_WRITE)
            stream = fopen(path, "r+b");
        else
            stream = fopen(path, "rb");
        if (stream == NULL)
            return -1;
        fclose(stream);
        return 0;
    }

#endif
}


/*
 * Determines if two accessible file paths are equivalent,
 * i.e. they point to the same physical location.
 */
int
osys_ftest_eq(const char *path1, const char *path2)
{
#if defined OSYS_UNIX || defined OSYS_DOS_OS2

    struct stat sbuf1, sbuf2;

    if (stat(path1, &sbuf1) != 0 || stat(path2, &sbuf2) != 0)
    {
        /* Can't stat the paths. */
        return -1;
    }
    if (sbuf1.st_dev == sbuf2.st_dev && sbuf1.st_ino == sbuf2.st_ino)
    {
        /* The two paths have the same device and inode numbers. */
        /* The inode numbers are reliable only if they're not 0. */
        return (sbuf1.st_ino != 0) ? 1 : -1;
    }
    else
    {
        /* The two paths have different device or inode numbers. */
        return 0;
    }

#elif defined OSYS_WINDOWS

    HANDLE hFile1, hFile2;
    BY_HANDLE_FILE_INFORMATION fileInfo1, fileInfo2;
    int result;

    hFile1 = CreateFileA(path1, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    if (hFile1 == INVALID_HANDLE_VALUE)
        return -1;
    hFile2 = CreateFileA(path2, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    if (hFile2 == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile1);
        return -1;
    }
    if (!GetFileInformationByHandle(hFile1, &fileInfo1) ||
        !GetFileInformationByHandle(hFile2, &fileInfo2))
    {
        /* Can't retrieve the file info. */
        result = -1;
    }
    else
    if (fileInfo1.nFileIndexLow == fileInfo2.nFileIndexLow &&
        fileInfo1.nFileIndexHigh == fileInfo2.nFileIndexHigh &&
        fileInfo1.dwVolumeSerialNumber == fileInfo2.dwVolumeSerialNumber)
    {
        /* The two paths have the same ID on the same volume. */
        result = 1;
    }
    else
    {
        /* The two paths have different IDs or sit on different volumes. */
        result = 0;
    }
    CloseHandle(hFile1);
    CloseHandle(hFile2);
    return result;

#else  /* generic */

    return -1;  /* unknown */

#endif
}


/*
 * Copies the attributes (access mode, time stamp, etc.)
 * of the source file path onto the destination file path.
 */
int
osys_fattr_copy(const char *dest_path, const char *src_path)
{
#if defined OSYS_UNIX || defined OSYS_DOS_OS2

    struct stat sbuf;
    int /* mode_t */ mode;
    struct utimbuf utbuf;

    if (stat(src_path, &sbuf) != 0)
        return -1;

    mode = (int)sbuf.st_mode;
    utbuf.actime = sbuf.st_atime;
    utbuf.modtime = sbuf.st_mtime;

    if (utime(dest_path, &utbuf) == 0 && chmod(dest_path, mode) == 0)
        return 0;
    else
        return -1;

#elif defined OSYS_WINDOWS

    static int isWinNT = -1;
    HANDLE hFile;
    FILETIME ftLastWrite;
    BOOL result;

    if (isWinNT < 0)
        isWinNT = (GetVersion() < 0x80000000U) ? 1 : 0;

    hFile = CreateFileA(src_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;
    result = GetFileTime(hFile, NULL, NULL, &ftLastWrite);
    CloseHandle(hFile);
    if (!result)
        return -1;

    hFile = CreateFileA(dest_path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
        (isWinNT ? FILE_FLAG_BACKUP_SEMANTICS : 0), 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;
    result = SetFileTime(hFile, NULL, NULL, &ftLastWrite);
    CloseHandle(hFile);
    if (!result)
        return -1;

    /* TODO: Copy the access mode. */

    return 0;

#else  /* generic */

    /* Do nothing. */
    return 0;

#endif
}


/*
 * Creates a new directory.
 */
int
osys_dir_make(const char *dirname)
{
    size_t len;

    len = strlen(dirname);
    if (len == 0)  /* current directory */
        return 0;

#ifdef OSYS_PATH_DOS
    if (len == 2 && dirname[1] == ':' && OSYS_PATH_IS_DRIVE(dirname[0]))
        return 0;
#endif

#if defined OSYS_UNIX || defined OSYS_DOS_OS2

    {
        struct stat sbuf;

        if (stat(dirname, &sbuf) == 0)
            return (sbuf.st_mode & S_IFDIR) ? 0 : -1;

        /* There is no directory, so create one now. */
#if defined OSYS_DOS_OS2
        return mkdir(dirname);
#else
        return mkdir(dirname, 0777);
#endif
    }

#elif defined OSYS_WINDOWS

    {
        char *wildname;
        HANDLE hFind;
        WIN32_FIND_DATAA wfd;

        /* See if dirname exists: find files in (dirname + "\\*"). */
        if (len + 3 < len)  /* overflow */
            return -1;
        wildname = (char *)malloc(len + 3);
        if (wildname == NULL)  /* out of memory */
            return -1;
        strcpy(wildname, dirname);
        if (strchr(OSYS_PATH_STRLIST_SLASH, wildname[len - 1]) == NULL)
            wildname[len++] = OSYS_PATH_CHR_SLASH;
        wildname[len++] = OSYS_PATH_CHR_STAR;
        wildname[len] = '\0';
        hFind = FindFirstFileA(wildname, &wfd);
        free(wildname);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);
            return 0;
        }

        /* There is no directory, so create one now. */
        return CreateDirectoryA(dirname, NULL) ? 0 : -1;
    }

#else  /* generic */

    /* Do nothing. */
    return 0;

#endif
}


/*
 * Prints an error message to stderr and terminates the program
 * execution immediately, exiting with code 70 (EX_SOFTWARE).
 */
void
osys_terminate(void)
{
    static const char *msg =
        "The execution of this program has been terminated abnormally.\n";
    fputs(msg, stderr);
    exit(70);  /* EX_SOFTWARE */
}
