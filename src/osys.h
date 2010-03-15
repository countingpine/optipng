/*
 * osys.h
 * System extensions.
 *
 * Copyright (C) 2003-2010 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the attached LICENSE for more information.
 */


#ifndef OSYS_H
#define OSYS_H

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Creates a new file path by changing the directory component of
 * a given file path.
 * The new directory name can be the empty string, indicating that
 * the resulting path has no directory.  Otherwise, the directory
 * name should follow the conventions specific to the host operating
 * system, and may optionally be ended with the directory separator
 * (e.g. "/" on Unix).
 * On success, the function returns buffer.
 * On error, it returns NULL.
 */
char *
osys_path_chdir(char *buffer, size_t bufsize,
                const char *old_path, const char *new_dirname);


/*
 * Creates a new file path by changing the extension component of
 * a given file path.
 * The new extension name can be the empty string, indicating that
 * the resulting path has no extension.  Otherwise, the extension
 * name must begin with the extension separator (e.g. "." on Unix).
 * On success, the function returns buffer.
 * On error, it returns NULL.
 */
char *
osys_path_chext(char *buffer, size_t bufsize,
                const char *old_path, const char *new_extname);


/*
 * Compares one file path to another.
 * It returns a value (less than, equal to, or greater than 0)
 * based on string comparison between path1 to path2.
 * The details concerning string comparison (locale-awareness,
 * case-sensitivity, etc.) depend on the host operating system.
 */
int
osys_path_cmp(const char *path1, const char *path2);


/*
 * Creates a backup file path.
 * On success, the function returns buffer.
 * On error, it returns NULL.
 */
char *
osys_path_mkbak(char *buffer, size_t bufsize, const char *path);


/*
 * Opens a file and positions it at the specified file offset.
 * On success, the function returns the pointer to the file stream.
 * On error, it returns NULL.
 */
FILE *
osys_fopen_at(const char *path, const char *mode,
              long offset, int whence);


/*
 * Reads a block of data from the specified file offset.
 * The file-position indicator is saved and restored after reading.
 * The file buffer is flushed before and after reading.
 * On success, the function returns the number of bytes read.
 * On error, it returns 0.
 */
size_t
osys_fread_at(FILE *stream, long offset, int whence,
              void *block, size_t blocksize);


/*
 * Writes a block of data at the specified file offset.
 * The file-position indicator is saved and restored after writing.
 * The file buffer is flushed before and after writing.
 * On success, the function returns the number of bytes written.
 * On error, it returns 0.
 */
size_t
osys_fwrite_at(FILE *stream, long offset, int whence,
               const void *block, size_t blocksize);


/*
 * Determines if the accessibility of the specified file path satisfies
 * the specified access mode.  The access mode consists of one or more
 * characters that indicate the checks to be performed, as follows:
 *  'e': the file exists; it needs not be a regular file.
 *  'f': the file exists and is a regular file.
 *  'r': the file exists and read permission is granted.
 *  'w': the file exists and write permission is granted.
 *  'x': the file exists and execute permission is granted.
 * For example, to determine if a file can be opened for reading using
 * fopen(), use "fr" in the access mode.
 * If all checks succeed, the function returns 0.
 * Otherwise, it returns -1.
 */
int
osys_ftest(const char *path, const char *mode);


/*
 * Determines if two accessible file paths are equivalent,
 * i.e. they point to the same physical location.
 * If the two paths are equivalent, the function returns 1.
 * If the two paths are not equivalent, the function returns 0.
 * If at least one path is not accessible or does not exist, or
 * if the check cannot be performed, the function returns -1.
 */
int
osys_ftest_eq(const char *path1, const char *path2);


/*
 * Copies the attributes (access mode, time stamp, etc.)
 * of the source file path onto the destination file path.
 * On success, the function returns 0.
 * On error, it returns -1.
 */
int
osys_fattr_copy(const char *dest_path, const char *src_path);


/*
 * Creates a new directory.
 * If the directory is successfully created, or if it already exists,
 * the function returns 0.
 * Otherwise, it returns -1.
 */
int
osys_dir_make(const char *dirname);


/*
 * Prints an error message to stderr and terminates the program
 * execution immediately, exiting with code 70 (EX_SOFTWARE).
 * This function does not raise SIGABRT, and it does not generate
 * other files (like core dumps, where applicable).
 */
void
osys_terminate(void);


#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif  /* OSYS_H */
