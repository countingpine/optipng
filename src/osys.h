/*
 * osys.h - system extensions
 *
 * Copyright (C) 2003-2006 Cosmin Truta.
 * This program is open-source software.  See LICENSE for more details.
 */


#ifndef OSYS_H
#define OSYS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>


/**
 * Allocates memory safely.
 * On success, the function returns the address of the allocated block.
 * On error, it prints a message to stderr and aborts.
 * If the requested block size is 0, it does nothing and returns NULL.
 **/
void *osys_malloc(size_t size);


/**
 * Deallocates memory safely.
 * The function does nothing if the given pointer is NULL.
 **/
void osys_free(void *ptr);


/**
 * Creates a backup file name.
 * On success, returns buffer.
 * On error, returns NULL.
 **/
char *osys_fname_mkbak(char *buffer, size_t bufsize, const char *fname);


/**
 * Creates a file name by changing the directory of a given file name.
 * The new directory name can be the empty string, indicating that
 * the new file name has no directory (or is in the default directory).
 * The directory name may or may not contain the trailing directory
 * separator (usually '/').
 * On success, returns buffer.
 * On error, returns NULL.
 **/
char *osys_fname_chdir(char *buffer, size_t bufsize,
    const char *oldname, const char *newdir);


/**
 * Creates a file name by changing the extension of a given file name.
 * The new extension can be the empty string, indicating that the new
 * file name has no extension.  Otherwise, it must begin with the
 * extension separator (usually '.').
 * On success, returns buffer.
 * On error, returns NULL.
 **/
char *osys_fname_chext(char *buffer, size_t bufsize,
    const char *oldname, const char *newext);


/**
 * Compares one file name to another.
 * It returns a value (less than, equal to, or greater than 0)
 * based on the result of comparing name1 to name2.
 * The comparison may or may not be case sensitive, depending on
 * the operating system.
 **/
int osys_fname_cmp(const char *name1, const char *name2);


/**
 * Reads a block of data from the specified file offset.
 * The file-position indicator is saved and restored after reading.
 * The file buffer is flushed before and after reading.
 * On success, the function returns the number of bytes read.
 * On error, it returns 0.
 **/
size_t osys_fread_at(FILE *stream, long offset, int whence,
    void *block, size_t blocksize);


/**
 * Writes a block of data at the specified file offset.
 * The file-position indicator is saved and restored after writing.
 * The file buffer is flushed before and after writing.
 * On success, the function returns the number of bytes written.
 * On error, it returns 0.
 **/
size_t osys_fwrite_at(FILE *stream, long offset, int whence,
    const void *block, size_t blocksize);


/**
 * Copies the file mode and the time stamp of the file
 * named by destname into the file named by srcname.
 * On success, returns 0.
 * On error, sets the global variable errno and returns -1.
 **/
int osys_fattr_copy(const char *destname, const char *srcname);


/**
 * Creates a new directory with the given name.
 * If the directory is successfully created, or if it already exists,
 * the function returns 0.
 * Otherwise, it sets the global variable errno and returns -1.
 **/
int osys_dir_make(const char *dirname);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* OSYS_H */
