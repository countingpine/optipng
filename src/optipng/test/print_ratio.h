/*
 * print_ratio.h
 * Declarations for *print_ratio()
 *
 * Copyright (C) 2008-2011 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the attached LICENSE for more information.
 */

#ifndef PRINT_RATIO_H
#define PRINT_RATIO_H

#include <stdio.h>

int
fprint_ratio(FILE *stream,
             unsigned long num, unsigned long denom,
             int force_percent);

int
sprint_ratio(char *buf, size_t bufsize,
             unsigned long num, unsigned long denom,
             int force_percent);

#endif  /* PRINT_RATIO_H */
