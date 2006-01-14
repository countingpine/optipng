/*
 * osys.h - system extensions
 *
 * Copyright (C) 2003-2004 Cosmin Truta.
 * This program is open-source software.  See LICENSE for more details.
 */


#include <stddef.h>


/**
 * Creates a backup file name.
 * On success, returns buffer.
 * On error, returns NULL.
 **/
char *osys_bak_nam(char *buffer, size_t bufsize, const char *fname);


/**
 * Copies the file mode and the time stamp of the file
 * named by destname into the file named by srcname.
 * On success, returns 0.
 * On error, sets the global variable errno and returns -1.
 **/
int osys_attr_cpy(const char *destname, const char *srcname);
