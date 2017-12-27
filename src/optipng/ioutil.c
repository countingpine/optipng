/*
 * ioutil.c
 * I/O utilities.
 *
 * Copyright (C) 2003-2017 Cosmin Truta and the Contributing Authors.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "ioutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Auto-configuration.
 */
#if defined UNIX || defined __UNIX__ || defined __unix || defined __unix__ || \
    defined _BSD_SOURCE || defined _GNU_SOURCE || defined _SVID_SOURCE || \
    defined _POSIX_SOURCE || defined _POSIX_C_SOURCE || defined _XOPEN_SOURCE
#  define OPNG_OS_UNIX
#endif

#if defined __APPLE__ && defined __MACH__
#  define OPNG_OS_DARWIN
#  ifndef OPNG_OS_UNIX
     /* The macros __unix and __unix__ are not predefined on Darwin. */
#    define OPNG_OS_UNIX
#  endif
#endif

#if defined WIN32 || defined _WIN32 || defined _WIN32_WCE || \
    defined __WIN32__ || defined __NT__
#  define OPNG_OS_WIN32
#endif

#if defined WIN64 || defined _WIN64 || defined __WIN64__
#  define OPNG_OS_WIN64
#endif

#if defined OPNG_OS_WIN32 || defined OPNG_OS_WIN64
#  define OPNG_OS_WINDOWS
#endif

#if defined DOS || defined _DOS || defined __DOS__ || \
    defined MSDOS || defined _MSDOS || defined __MSDOS__
#  define OPNG_OS_DOS
#endif

#if defined OS2 || defined OS_2 || defined __OS2__
#  define OPNG_OS_OS2
#endif

#if defined OPNG_OS_DOS || defined OPNG_OS_OS2
#  define OPNG_OS_DOSISH
#endif

#if defined __CYGWIN__ || defined __DJGPP__ || defined __EMX__
#  define OPNG_OS_UNIXISH
#  ifndef OPNG_OS_UNIX
     /* Strictly speaking, this is not correct, but "it works". */
#    define OPNG_OS_UNIX
#  endif
#endif

#if defined OPNG_OS_UNIX || \
    (!defined OPNG_OS_WINDOWS && !defined OPNG_OS_DOSISH)
#  include <unistd.h>
#  if defined _POSIX_VERSION || defined _XOPEN_VERSION
#    ifndef OPNG_OS_UNIX
#      define OPNG_OS_UNIX
#    endif
#  endif
#endif

#if defined OPNG_OS_UNIX || defined OPNG_OS_WINDOWS || defined OPNG_OS_DOSISH
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  if defined _MSC_VER || defined __WATCOMC__
#    include <sys/utime.h>
#  else
#    include <utime.h>
#  endif
#endif

#if defined OPNG_OS_WINDOWS || \
    defined OPNG_OS_DOSISH || defined OPNG_OS_UNIXISH
#  include <io.h>
#endif

#if defined OPNG_OS_WINDOWS
#  include <windows.h>
#endif

#if defined OPNG_OS_DOSISH
#  include <process.h>
#endif


/*
 * More auto-configuration.
 */
#if (defined OPNG_OS_WINDOWS || defined OPNG_OS_DOSISH) && \
    !defined OPNG_OS_UNIXISH
#  define OPNG_PATH_DIRSEP '\\'
#  define OPNG_PATH_DIRSEP_STR "\\"
#  define OPNG_PATH_DIRSEP_ALL_STR "/\\"
#else
#  define OPNG_PATH_DIRSEP '/'
#  define OPNG_PATH_DIRSEP_STR "/"
#  if defined OPNG_OS_UNIXISH
#    define OPNG_PATH_DIRSEP_ALL_STR "/\\"
#  elif defined OPNG_OS_DARWIN
#    define OPNG_PATH_DIRSEP_ALL_STR "/:"
#  else  /* OPNG_OS_UNIX and others */
#    define OPNG_PATH_DIRSEP_ALL_STR "/"
#  endif
#endif
#define OPNG_PATH_EXTSEP '.'
#define OPNG_PATH_EXTSEP_STR "."

#if defined OPNG_OS_WINDOWS || \
    defined OPNG_OS_DOSISH || defined OPNG_OS_UNIXISH
#  define OPNG_PATH_DOS
#endif

#ifdef R_OK
#  define OPNG_TEST_READ R_OK
#else
#  define OPNG_TEST_READ 4
#endif
#ifdef W_OK
#  define OPNG_TEST_WRITE W_OK
#else
#  define OPNG_TEST_WRITE 2
#endif
#ifdef X_OK
#  define OPNG_TEST_EXEC X_OK
#else
#  define OPNG_TEST_EXEC 1
#endif
#ifdef F_OK
#  define OPNG_TEST_FILE F_OK
#else
#  define OPNG_TEST_FILE 0
#endif


/*
 * Utility macros.
 */
#ifdef OPNG_PATH_DOS
#  define OPNG_PATH_IS_DRIVE_LETTER(ch) \
          (((ch) >= 'A' && (ch) <= 'Z') || ((ch) >= 'a' && (ch) <= 'z'))
#endif

#ifdef OPNG_OS_WINDOWS
#  if defined OPNG_OS_WIN64 || (defined _MSC_VER && _MSC_VER >= 1500)
#    define OPNG_HAVE_STDIO__I64
#    define OPNG_OS_WINDOWS_IS_WIN9X() 0
#  else
#    if (defined _MSC_VER && _MSC_VER >= 1400) || \
        (defined __MSVCRT_VERSION__ && __MSVCRT_VERSION__ >= 0x800)
#      define OPNG_HAVE_STDIO__I64
#    endif
#    define OPNG_OS_WINDOWS_IS_WIN9X() (GetVersion() >= 0x80000000U)
#  endif
#endif


/*
 * Returns the current value of the file position indicator.
 */
opng_foffset_t
opng_ftello(FILE *stream)
{
#if defined OPNG_HAVE_STDIO__I64

    return (opng_foffset_t)_ftelli64(stream);

#elif defined OPNG_OS_UNIX && (OPNG_FOFFSET_MAX > LONG_MAX)

    /* We don't know if off_t is sufficiently wide, we only know that
     * long isn't. We are trying just a little harder, in the absence
     * of an fopen64/ftell64 solution.
     */
    return (opng_foffset_t)ftello(stream);

#else  /* generic */

    return (opng_foffset_t)ftell(stream);

#endif
}

/*
 * Sets the file position indicator at the specified file offset.
 */
int
opng_fseeko(FILE *stream, opng_foffset_t offset, int whence)
{
#if defined OPNG_HAVE_STDIO__I64

    return _fseeki64(stream, (__int64)offset, whence);

#elif defined OPNG_OS_UNIX

#if OPNG_FOFFSET_MAX > LONG_MAX
    /* We don't know if off_t is sufficiently wide, we only know that
     * long isn't. We are trying just a little harder, in the absence
     * of an fopen64/fseek64 solution.
     */
    return fseeko(stream, (off_t)offset, whence);
#else
    return fseek(stream, (long)offset, whence);
#endif

#else  /* generic */

    return (fseek(stream, (long)offset, whence) == 0) ? 0 : -1;

#endif
}

/*
 * Reads a block of data from the specified file offset.
 */
size_t
opng_freado(FILE *stream, opng_foffset_t offset, int whence,
            void *block, size_t blocksize)
{
    fpos_t pos;
    size_t result;

    if (fgetpos(stream, &pos) != 0)
        return 0;
    if (opng_fseeko(stream, offset, whence) == 0)
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
opng_fwriteo(FILE *stream, opng_foffset_t offset, int whence,
             const void *block, size_t blocksize)
{
    fpos_t pos;
    size_t result;

    if (fgetpos(stream, &pos) != 0 || fflush(stream) != 0)
        return 0;
    if (opng_fseeko(stream, offset, whence) == 0)
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
 * Gets the size of the specified file stream.
 */
int
opng_fgetsize(FILE *stream, opng_fsize_t *size)
{
#if defined OPNG_OS_WINDOWS

    HANDLE hFile;
    DWORD dwSizeLow, dwSizeHigh;

    hFile = (HANDLE)_get_osfhandle(_fileno(stream));
    dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
    if (GetLastError() != NO_ERROR)
        return -1;
    *size = (opng_fsize_t)dwSizeLow + ((opng_fsize_t)dwSizeHigh << 32);
    return 0;

#elif defined OPNG_OS_UNIX

    struct stat sbuf;

    if (fstat(fileno(stream), &sbuf) != 0)
        return -1;
    if (sbuf.st_size < 0)
        return -1;
    *size = (opng_fsize_t)sbuf.st_size;
    return 0;

#else  /* generic */

    opng_foffset_t offset;

    if (opng_fseeko(stream, 0, SEEK_END) != 0)
        return -1;
    offset = opng_ftello(stream);
    if (offset < 0)
        return -1;
    *size = (opng_fsize_t)offset;
    return 0;

#endif
}

/*
 * Makes a new path name by replacing the directory component of
 * a specified path name.
 */
char *
opng_path_replace_dir(char *buffer, size_t bufsize,
                      const char *old_path, const char *new_dirname)
{
    const char *path, *ptr;
    size_t dirlen;

    /* Extract file name from old_path. */
    path = old_path;
#ifdef OPNG_PATH_DOS
    /* Skip the drive name, if present. */
    if (OPNG_PATH_IS_DRIVE_LETTER(path[0]) && path[1] == ':')
        path += 2;
#endif
    for ( ; ; )
    {
        ptr = strpbrk(path, OPNG_PATH_DIRSEP_ALL_STR);
        if (ptr == NULL)
            break;
        path = ptr + 1;
    }

    /* Make sure the buffer is large enough. */
    dirlen = strlen(new_dirname);
    if (dirlen + strlen(path) + 2 >= bufsize)  /* overflow */
        return NULL;

    /* Copy the new directory name. Also append a slash if necessary. */
    if (dirlen > 0)
    {
        strcpy(buffer, new_dirname);
#ifdef OPNG_PATH_DOS
        if (dirlen == 2 && buffer[1] == ':' &&
            OPNG_PATH_IS_DRIVE_LETTER(buffer[0]))
        {
            /* Special case: do not append slash to "C:". */
        }
        else
#endif
        {
            if (strchr(OPNG_PATH_DIRSEP_ALL_STR, buffer[dirlen - 1]) == NULL)
                buffer[dirlen++] = OPNG_PATH_DIRSEP;
        }
    }

    /* Append the file name. */
    strcpy(buffer + dirlen, path);
    return buffer;
}

/*
 * Makes a new path name by changing the extension component of
 * a specified path name.
 */
char *
opng_path_replace_ext(char *buffer, size_t bufsize,
                      const char *old_path, const char *new_extname)
{
    size_t i, pos;

    if (new_extname[0] != OPNG_PATH_EXTSEP)  /* invalid argument */
        return NULL;
    for (i = 0, pos = (size_t)(-1); old_path[i] != '\0'; ++i)
    {
        if (i >= bufsize)  /* overflow */
            return NULL;
        if ((buffer[i] = old_path[i]) == OPNG_PATH_EXTSEP)
            pos = i;
    }
    if (i > pos)
    {
        /* An extension already exists in old_path. Go back. */
        i = pos;
    }
    for ( ; ; ++i, ++new_extname)
    {
        if (i >= bufsize)  /* overflow */
            return NULL;
        if ((buffer[i] = *new_extname) == '\0')
            return buffer;
    }
}

/*
 * Makes a backup path name.
 */
char *
opng_path_make_backup(char *buffer, size_t bufsize, const char *path)
{
    static const char bak_extname[] = OPNG_PATH_EXTSEP_STR "bak";

    if (strlen(path) + sizeof(bak_extname) > bufsize)
        return NULL;

#if defined OPNG_OS_DOS

    return opng_path_replace_ext(buffer, bufsize, path, bak_extname);

#else  /* OPNG_OS_UNIX and others */

    strcpy(buffer, path);
    strcat(buffer, bak_extname);
    return buffer;

#endif
}

/*
 * Changes the name of a file system object.
 */
int
opng_os_rename(const char *src_path, const char *dest_path, int clobber)
{
#if defined OPNG_OS_WINDOWS

    DWORD dwFlags;

#if !defined OPNG_OS_WIN64
    if (OPNG_OS_WINDOWS_IS_WIN9X())
    {
        /* MoveFileEx is not available under Win9X; use MoveFile. */
        if (MoveFileA(src_path, dest_path))
            return 0;
        if (!clobber)
            return -1;
        DeleteFileA(dest_path);
        return MoveFileA(src_path, dest_path) ? 0 : -1;
    }
#endif

    dwFlags = clobber ? MOVEFILE_REPLACE_EXISTING : 0;
    return MoveFileExA(src_path, dest_path, dwFlags) ? 0 : -1;

#elif defined OPNG_OS_UNIX

    if (!clobber)
    {
        if (access(dest_path, OPNG_TEST_FILE) >= 0)
            return -1;
    }
    return rename(src_path, dest_path);

#else  /* generic */

    if (opng_test(dest_path, "e") == 0)
    {
        if (!clobber)
            return -1;
        opng_unlink(dest_path);
    }
    return rename(src_path, dest_path);

#endif
}

/*
 * Copies the attributes (access mode, time stamp, etc.) of a file system
 * object.
 */
int
opng_os_copy_attr(const char *src_path, const char *dest_path)
{
#if defined OPNG_OS_WINDOWS

    HANDLE hFile;
    FILETIME ftLastWrite;
    BOOL success;

    hFile = CreateFileA(src_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;
    success = GetFileTime(hFile, NULL, NULL, &ftLastWrite);
    CloseHandle(hFile);
    if (!success)
        return -1;

    hFile = CreateFileA(dest_path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
        (OPNG_OS_WINDOWS_IS_WIN9X() ? 0 : FILE_FLAG_BACKUP_SEMANTICS), 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return -1;
    success = SetFileTime(hFile, NULL, NULL, &ftLastWrite);
    CloseHandle(hFile);
    if (!success)
        return -1;

    /* TODO: Copy the access mode. */

    return 0;

#elif defined OPNG_OS_UNIX || defined OPNG_OS_DOSISH

    struct stat sbuf;
    int result;

    if (stat(src_path, &sbuf) != 0)
        return -1;

    result = 0;

    if (chown(dest_path, sbuf.st_uid, sbuf.st_gid) != 0)
    {
        /* This is not required to succeed. Fall through. */
    }

    if (chmod(dest_path, sbuf.st_mode) != 0)
        result = -1;

#if defined AT_FDCWD && defined UTIME_NOW && defined UTIME_OMIT
    {
        struct timespec times[2];

#if defined OPNG_OS_DARWIN
        times[0] = sbuf.st_atimespec;
        times[1] = sbuf.st_mtimespec;
#else
        times[0] = sbuf.st_atim;
        times[1] = sbuf.st_mtim;
#endif
        if (utimensat(AT_FDCWD, dest_path, times, 0) != 0)
            result = -1;
    }
#else  /* legacy utime */
    {
        struct utimbuf utbuf;

        utbuf.actime = sbuf.st_atime;
        utbuf.modtime = sbuf.st_mtime;
        if (utime(dest_path, &utbuf) != 0)
            result = -1;
    }
#endif

    return result;

#else  /* generic */

    (void)src_path;  /* unused */
    (void)dest_path;  /* unused */

    /* Always fail. */
    return -1;

#endif
}

/*
 * Creates a new directory.
 */
int
opng_os_create_dir(const char *dirname)
{
    /* Exit early if there is no directory name. */
    if (dirname[0] == '\0')
        return 0;
#ifdef OPNG_PATH_DOS
    if (OPNG_PATH_IS_DRIVE_LETTER(dirname[0]) &&
        dirname[1] == ':' && dirname[2] == '\0')
        return 0;
#endif

#if defined OPNG_OS_WINDOWS

    {
        size_t dirlen;
        char *wildname;
        HANDLE hFind;
        WIN32_FIND_DATAA wfd;

        /* Fail early if dirname is too long. */
        dirlen = strlen(dirname);
        if (dirlen * 2 <= dirlen)
            return -1;

        /* Find files in (dirname + "\\*") and exit early if dirname exists. */
        wildname = (char *)malloc(dirlen + 3);
        if (wildname == NULL)
            return -1;
        strcpy(wildname, dirname);
        if (strchr(OPNG_PATH_DIRSEP_ALL_STR, wildname[dirlen - 1]) == NULL)
            wildname[dirlen++] = OPNG_PATH_DIRSEP;
        wildname[dirlen++] = '*';
        wildname[dirlen] = '\0';
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

#elif defined OPNG_OS_UNIX || defined OPNG_OS_DOSISH

    {
        struct stat sbuf;

        if (stat(dirname, &sbuf) == 0)
            return (sbuf.st_mode & S_IFDIR) ? 0 : -1;

        /* There is no directory, so create one now. */
#if defined OPNG_OS_UNIX
        return mkdir(dirname, 0777);
#else
        return mkdir(dirname);
#endif
    }

#else  /* generic */

    (void)dirname;  /* unused */

    /* Always fail. */
    return -1;

#endif
}

/*
 * Determines if the accessibility of the specified file system object
 * satisfies the specified access mode.
 */
int
opng_os_test(const char *path, const char *mode)
{
    int faccess, freg;

    faccess = freg = 0;
    if (strchr(mode, 'f') != NULL)
        freg = 1;
    if (strchr(mode, 'r') != NULL)
        faccess |= OPNG_TEST_READ;
    if (strchr(mode, 'w') != NULL)
        faccess |= OPNG_TEST_WRITE;
    if (strchr(mode, 'x') != NULL)
        faccess |= OPNG_TEST_EXEC;
    if (faccess == 0 && !freg)
    {
        if (strchr(mode, 'e') == NULL)
            return 0;
    }

#if defined OPNG_OS_WINDOWS

    {
        DWORD attr;

        attr = GetFileAttributesA(path);
        if (attr == 0xffffffffU)
            return -1;
        if (freg && (attr & FILE_ATTRIBUTE_DIRECTORY))
            return -1;
        if ((faccess & OPNG_TEST_WRITE) && (attr & FILE_ATTRIBUTE_READONLY))
            return -1;
        return 0;
    }

#elif defined OPNG_OS_UNIX || defined OPNG_OS_DOSISH

    {
        struct stat sbuf;

        if (stat(path, &sbuf) != 0)
            return -1;
        if (freg && ((sbuf.st_mode & S_IFREG) != S_IFREG))
            return -1;
        if (faccess == 0)
            return 0;
        return access(path, faccess);
    }

#else  /* generic */

    {
        FILE *stream;

        if (faccess & OPNG_TEST_WRITE)
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
 * Determines if two accessible paths are equivalent, i.e. they
 * refer to the same file system object.
 */
int
opng_os_test_eq(const char *path1, const char *path2)
{
#if defined OPNG_OS_WINDOWS

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
    else if (fileInfo1.nFileIndexLow == fileInfo2.nFileIndexLow &&
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

#elif defined OPNG_OS_UNIX || defined OPNG_OS_DOSISH

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

#else  /* generic */

    (void)path1;  /* unused */
    (void)path2;  /* unused */

    /* Always unknown. */
    return -1;

#endif
}

/*
 * Removes a directory entry.
 */
int
opng_os_unlink(const char *path)
{
#if defined OPNG_OS_WINDOWS

    return DeleteFileA(path) ? 0 : -1;

#elif defined OPNG_OS_UNIX || defined OPNG_OS_DOSISH

    return unlink(path);

#else  /* generic */

    return remove(path);

#endif
}
