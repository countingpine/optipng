/**
 * pnmwrite.c
 * Copyright (C) 2002-2005 Cosmin Truta.
 * This file is part of the pnmio library, distributed under the zlib license.
 * For conditions of distribution and use, see copyright notice in pnmio.h.
 **/


#include <stddef.h>
#include <stdio.h>
#include "pnmio.h"


/**
 * Error messages.
 **/
static const char *pnm_err_param = "invalid parameter(s)";
static const char *pnm_err_write = "write error";


/**
 * Writes an integer to a file.
 **/
#define pnm_putint(value, fp) \
    fprintf(fp, "%d", value)


/**
 * Writes a PNM header to a file.
 **/
void pnm_write_header(FILE *fp, pnm_struct *pnm_ptr)
{
    /* check the function parameters */
    if (pnm_ptr == NULL || fp == NULL)
        pnm_error(pnm_ptr, pnm_err_param);
    if (pnm_ptr->type_code < PNM_P1 || pnm_ptr->type_code > PNM_P6)
        pnm_error(pnm_ptr, pnm_err_param);
    if (pnm_ptr->width <= 0 || pnm_ptr->height <= 0 ||
            pnm_ptr->maxval <= 0 || pnm_ptr->maxval > 65535U)
        pnm_error(pnm_ptr, pnm_err_param);

    /* write the header */
    fprintf(fp, "P%c\n%d %d\n",
        pnm_ptr->type_code + '0', pnm_ptr->width, pnm_ptr->height);
    if (pnm_ptr->type_code % 3 != 1)  /* not PBM */
        fprintf(fp, "%d\n", pnm_ptr->maxval);
}


/**
 * Writes a PNM row to a file.
 **/
void pnm_write_row(FILE *fp, pnm_struct *pnm_ptr, unsigned int *row_ptr)
{
    unsigned int type_code = pnm_ptr->type_code;
    unsigned int sample_count = (type_code % 3 == 0) ? 3 : 1;
    unsigned int row_size = sample_count * pnm_ptr->width;
    unsigned int i;
    int is_raw = (type_code >= PNM_P4);
    int j;
    char ch;

    /* write the row */
    if (type_code % 3 == 1)  /* PBM */
    {
        if (is_raw)
        {
            for (i = 0; i < row_size; )
            {
                ch = 0;
                for (j = 7; j >= 0 && i < row_size; --j, ++i)
                    if ((row_ptr[i] & 1) == 0)
                        ch |= (char)(1 << j);
                if (putc(ch, fp) == EOF)
                    pnm_error(pnm_ptr, pnm_err_write);
            }
        }
        else
        {
            for (i = 0; i < row_size; ++i)
                if (putc((char)((row_ptr[i] & 1) ? '0' : '1'), fp) == EOF)
                    pnm_error(pnm_ptr, pnm_err_write);
            if (putc('\n', fp) == EOF)
                pnm_error(pnm_ptr, pnm_err_write);
        }
    }
    else  /* PGM, PPM */
    {
        if (is_raw)
        {
            if (pnm_ptr->maxval <= 255)  /* one byte per sample */
            {
               for (i = 0; i < row_size; ++i)
                   if (putc(((char)row_ptr[i]), fp) == EOF)
                       pnm_error(pnm_ptr, pnm_err_write);
            }
            else  /* two bytes per sample, big endian */
            {
               for (i = 0; i < row_size; ++i)
                   if (putc(((char)(row_ptr[i] >> 8)), fp) == EOF ||
                           putc(((char)row_ptr[i]), fp) == EOF)
                       pnm_error(pnm_ptr, pnm_err_write);
            }
        }
        else
        {
            if (fprintf(fp, "%u", row_ptr[0]) == EOF)
                pnm_error(pnm_ptr, pnm_err_write);
            for (i = 1; i < row_size; ++i)
                if (fprintf(fp, " %u", row_ptr[i]) == EOF)
                    pnm_error(pnm_ptr, pnm_err_write);
            if (fprintf(fp, "\n") == EOF)
                pnm_error(pnm_ptr, pnm_err_write);
        }
    }
}
