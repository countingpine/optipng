/*
 * ioutil.h
 * I/O utilities.
 *
 * Copyright (C) 2003-2017 Cosmin Truta and the Contributing Authors.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#ifndef OPNG_IOUTIL_H_
#define OPNG_IOUTIL_H_

#include <stdio.h>


/*
 * File offset and size types:
 *
 * opng_foffset_t: the file offset type (a signed integer type)
 * OPNG_FOFFSET_MIN: the minimum value (less than 0)
 * OPNG_FOFFSET_MAX: the maximum value (greater than 0)
 * OPNG_FOFFSET_SCNd: macro for scanf format specifier ("%d"-like)
 * OPNG_FOFFSET_SCNx: macro for scanf format specifier ("%x"-like)
 * OPNG_FOFFSET_PRId: macro for printf format specifier ("%d"-like)
 * OPNG_FOFFSET_PRIx: macro for printf format specifier ("%x"-like)
 * OPNG_FOFFSET_PRIX: macro for printf format specifier ("%X"-like)
 *
 * opng_fsize_t: the file size type (an unsigned integer type)
 * OPNG_FSIZE_MAX: the maximum value
 * OPNG_FSIZE_SCNu: macro for scanf format specifier ("%u"-like)
 * OPNG_FSIZE_SCNx: macro for scanf format specifier ("%x"-like)
 * OPNG_FSIZE_PRIu: macro for printf format specifier ("%u"-like)
 * OPNG_FSIZE_PRIx: macro for printf format specifier ("%x"-like)
 * OPNG_FSIZE_PRIX: macro for printf format specifier ("%X"-like)
 */
#include <limits.h>
#if (LONG_MAX > 0x7fffffffL) || (LONG_MAX > INT_MAX)

typedef long opng_foffset_t;
#define OPNG_FOFFSET_MIN LONG_MIN
#define OPNG_FOFFSET_MAX LONG_MAX
#define OPNG_FOFFSET_SCNd "ld"
#define OPNG_FOFFSET_SCNx "lx"
#define OPNG_FOFFSET_PRId "ld"
#define OPNG_FOFFSET_PRIx "lx"
#define OPNG_FOFFSET_PRIX "lX"

typedef unsigned long opng_fsize_t;
#define OPNG_FSIZE_MAX ULONG_MAX
#define OPNG_FSIZE_SCNu "lu"
#define OPNG_FSIZE_SCNx "lx"
#define OPNG_FSIZE_PRIu "lu"
#define OPNG_FSIZE_PRIx "lx"
#define OPNG_FSIZE_PRIX "lX"

#elif defined _I64_MAX && (defined _WIN32 || defined __WIN32__)

typedef __int64 opng_foffset_t;
#define OPNG_FOFFSET_MIN _I64_MIN
#define OPNG_FOFFSET_MAX _I64_MAX
#define OPNG_FOFFSET_SCNd "I64d"
#define OPNG_FOFFSET_SCNx "I64x"
#define OPNG_FOFFSET_PRId "I64d"
#define OPNG_FOFFSET_PRIx "I64x"
#define OPNG_FOFFSET_PRIX "I64X"

typedef unsigned __int64 opng_fsize_t;
#define OPNG_FSIZE_MAX _UI64_MAX
#define OPNG_FSIZE_SCNu "I64u"
#define OPNG_FSIZE_SCNx "I64x"
#define OPNG_FSIZE_PRIu "I64u"
#define OPNG_FSIZE_PRIx "I64x"
#define OPNG_FSIZE_PRIX "I64X"

#else

#include <inttypes.h>
#include <stdint.h>

typedef int_least64_t opng_foffset_t;
#define OPNG_FOFFSET_MIN INT_LEAST64_MIN
#define OPNG_FOFFSET_MAX INT_LEAST64_MAX
#define OPNG_FOFFSET_SCNd SCNdLEAST64
#define OPNG_FOFFSET_SCNx SCNxLEAST64
#define OPNG_FOFFSET_PRId PRIdLEAST64
#define OPNG_FOFFSET_PRIx PRIxLEAST64
#define OPNG_FOFFSET_PRIX PRIXLEAST64

typedef uint_least64_t opng_fsize_t;
#define OPNG_FSIZE_MAX UINT_LEAST64_MAX
#define OPNG_FSIZE_SCNu SCNuLEAST64
#define OPNG_FSIZE_SCNx SCNxLEAST64
#define OPNG_FSIZE_PRIu PRIuLEAST64
#define OPNG_FSIZE_PRIx PRIxLEAST64
#define OPNG_FSIZE_PRIX PRIXLEAST64

#endif


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Returns the current value of the file position indicator.
 * On error, the function returns (opng_foffset_t)(-1).
 */
opng_foffset_t
opng_ftello(FILE *stream);

/*
 * Sets the file position indicator at the specified file offset.
 * On success, the function returns 0. On error, it returns -1.
 */
int
opng_fseeko(FILE *stream, opng_foffset_t offset, int whence);

/*
 * Reads a block of data from the specified file offset.
 * The file-position indicator is saved and restored after reading.
 * The file buffer is flushed before and after reading.
 * On success, the function returns the number of bytes read.
 * On error, it returns 0.
 */
size_t
opng_freado(FILE *stream, opng_foffset_t offset, int whence,
            void *block, size_t blocksize);

/*
 * Writes a block of data at the specified file offset.
 * The file-position indicator is saved and restored after writing.
 * The file buffer is flushed before and after writing.
 * On success, the function returns the number of bytes written.
 * On error, it returns 0.
 */
size_t
opng_fwriteo(FILE *stream, opng_foffset_t offset, int whence,
             const void *block, size_t blocksize);

/*
 * Gets the size of the specified file stream.
 * This function may change the file position indicator.
 * On success, the function returns 0. On error, it returns -1.
 */
int
opng_fgetsize(FILE *stream, opng_fsize_t *size);

/*
 * Makes a new path name by replacing the directory component of
 * a specified path name.
 * The new directory name can be the empty string, indicating that
 * the resulting path has no directory. Otherwise, the directory
 * name should follow the conventions specific to the host operating
 * system, and may optionally be ended with the directory separator
 * (e.g. "/" on Unix).
 * On success, the function returns buffer.
 * On error, it returns NULL.
 */
char *
opng_path_replace_dir(char *buffer, size_t bufsize,
                      const char *old_path, const char *new_dirname);

/*
 * Makes a new path name by replacing the extension component of
 * a specified path name.
 * The new extension name can be the empty string, indicating that
 * the resulting path has no extension. Otherwise, the extension
 * name must begin with the extension separator (e.g. "." on Unix).
 * On success, the function returns buffer.
 * On error, it returns NULL.
 */
char *
opng_path_replace_ext(char *buffer, size_t bufsize,
                      const char *old_path, const char *new_extname);

/*
 * Makes a backup path name.
 * On success, the function returns buffer.
 * On error, it returns NULL.
 */
char *
opng_path_make_backup(char *buffer, size_t bufsize, const char *path);

/*
 * Changes the name of a file system object.
 * On success, the function returns 0.
 * On error, it returns -1.
 */
int
opng_os_rename(const char *src_path, const char *dest_path, int clobber);

/*
 * Copies the attributes (access mode, time stamp, etc.) of a file system
 * object.
 * On success, the function returns 0.
 * On error, it returns -1.
 */
int
opng_os_copy_attr(const char *src_path, const char *dest_path);

/*
 * Creates a new directory.
 * If the directory is successfully created, or if it already exists,
 * the function returns 0.
 * Otherwise, it returns -1.
 */
int
opng_os_create_dir(const char *dirname);

/*
 * Determines if the accessibility of the specified file system object
 * satisfies the specified access mode. The access mode consists of one
 * or more characters that indicate the checks to be performed, as follows:
 *   'e': the path exists; it needs not be a regular file.
 *   'f': the path exists and is a regular file.
 *   'r': the path exists and read permission is granted.
 *   'w': the path exists and write permission is granted.
 *   'x': the path exists and execute permission is granted.
 * For example, to determine if a file can be opened for reading using
 * fopen(), use "fr" in the access mode.
 * If all checks succeed, the function returns 0.
 * Otherwise, it returns -1.
 */
int
opng_os_test(const char *path, const char *mode);

/*
 * Determines if two accessible paths are equivalent, i.e. they
 * refer to the same file system object.
 * If the two paths are equivalent, the function returns 1.
 * If the two paths are not equivalent, the function returns 0.
 * If at least one path is not accessible or does not exist, or
 * if the check cannot be performed, the function returns -1.
 */
int
opng_os_test_eq(const char *path1, const char *path2);

/*
 * Removes a directory entry.
 * On success, the function returns 0.
 * On error, it returns -1.
 */
int
opng_os_unlink(const char *path);


#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif  /* OPNG_IOUTIL_H_ */
