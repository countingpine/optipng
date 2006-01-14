/**
 * pnmio.h
 * Simple I/O interface to the Portable Any Map (PNM) image file format.
 * Version 0.2.1, Release 2005-Dec-01.
 *
 * Copyright (C) 2002-2005 Cosmin Truta.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.  If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 **/


#ifndef PNMIO_H
#define PNMIO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * PNM type codes.
 **/
enum
{
    PNM_P1 = 1, PNM_P2 = 2, PNM_P3 = 3,
    PNM_P4 = 4, PNM_P5 = 5, PNM_P6 = 6
};


/**
 * The PNM data structure.
 **/
typedef struct pnm_struct
{
    unsigned int type_code;
    unsigned int width;
    unsigned int height;
    unsigned int maxval;
} pnm_struct;


/**
 * Error handling.
 **/
extern void (*pnm_error)(pnm_struct *pnm_ptr, const char *msg);
extern void (*pnm_warning)(pnm_struct *pnm_ptr, const char *msg);


/**
 * Read functions.
 **/
void pnm_read_header(FILE *fp, pnm_struct *pnm_ptr);
void pnm_read_row(FILE *fp, pnm_struct *pnm_ptr, unsigned int *row_ptr);
void pnm_read_image(FILE *fp, pnm_struct *pnm_ptr, ...);  /* not implemented */


/**
 * Write functions.
 **/
void pnm_write_header(FILE *fp, pnm_struct *pnm_ptr);
void pnm_write_row(FILE *fp, pnm_struct *pnm_ptr, unsigned int *row_ptr);
void pnm_write_image(FILE *fp, pnm_struct *pnm_ptr, ...); /* not implemented */


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* PNMIO_H */
