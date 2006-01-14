/**
 * pnmerror.c
 * Copyright (C) 2002-2005 Cosmin Truta.
 * This file is part of the pnmio library, distributed under the zlib license.
 * For conditions of distribution and use, see copyright notice in pnmio.h.
 **/


#include <stdio.h>
#include <stdlib.h>
#include "pnmio.h"


/**
 * Error handling.
 **/

void pnm_default_error(pnm_struct *pnm_ptr, const char *msg)
{
    if (&pnm_ptr)
        fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

void pnm_default_warning(pnm_struct *pnm_ptr, const char *msg)
{
    if (&pnm_ptr)
        fprintf(stderr, "%s\n", msg);
}


void (*pnm_error)(pnm_struct *pnm_ptr, const char *msg)
    = pnm_default_error;

void (*pnm_warning)(pnm_struct *pnm_ptr, const char *msg)
    = pnm_default_warning;

