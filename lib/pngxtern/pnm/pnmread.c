/**
 * pnmread.c
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
static const char *pnm_err_param  = "Invalid parameter(s)";
static const char *pnm_err_read   = "Error reading PNM data";
static const char *pnm_err_notpnm = "Not a PNM file";
static const char *pnm_err_badpnm = "Invalid PNM file";


/**
 * Verifies if the character is a space: ' ', '\t', '\n' or '\r'.
 **/
#define pnm_isspace(ch) \
    ((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r')


/**
 * Verifies if the character is a digit: '0'-'9'.
 **/
#define pnm_isdigit(ch) \
    ((ch) >= '0' && (ch) <= '9')


/**
 * Reads a character from a file.
 * Translates the line endings to '\n'.
 * If a comment is found, skips the rest of the line and returns '\n'.
 **/
static int pnm_getchar(FILE *fp)
{
    int ch = getc(fp);

    /* skip the comments */
    if (ch == '#')
    {
        do
            ch = getc(fp);
        while (ch != EOF && ch != '\n' && ch != '\r');
    }

    /* translate the line endings */
    if (ch == '\r')
    {
        ch = getc(fp);
        if (ch != EOF && ch != '\n')
        {
            ungetc(ch, fp);
            ch = '\n';
        }
    }

    return ch;
}


/**
 * Reads an integer from a file.
 **/
static int pnm_getint(FILE *fp)
{
    int result = 0;
    int ch;

    /* skip the leading whitespaces */
    do
    {
        ch = pnm_getchar(fp);
        if (ch == EOF)
            return -1;
    } while (pnm_isspace(ch));
    if (!pnm_isdigit(ch))
        return -1;

    /* read the value */
    do
    {
        result = result * 10 + (ch - '0');
        ch = pnm_getchar(fp);
    } while (pnm_isdigit(ch));

    /* check for errors */
    if (ch == EOF)
        return -1;
    else if (!pnm_isspace(ch))
        return -1;

    /* return the result */
    return result;
}


/**
 * Reads a PNM header from a file.
 **/
void pnm_read_header(FILE *fp, pnm_struct *pnm_ptr)
{
    int ch;

    /* check the function parameters */
    if (pnm_ptr == NULL || fp == NULL)
        pnm_error(pnm_ptr, pnm_err_param);

    /* check the file type */
    ch = getc(fp);
    if (ch != 'P')
        pnm_error(pnm_ptr, pnm_err_notpnm);
    ch = getc(fp);
    if (ch < '1' || ch > '6')
        pnm_error(pnm_ptr, pnm_err_notpnm);
    pnm_ptr->type_code = ch - '0';
    ch = pnm_getchar(fp);
    if (!pnm_isspace(ch))
        pnm_error(pnm_ptr, pnm_err_notpnm);

    /* read the header info */
    if ((ch = pnm_getint(fp)) <= 0)
        pnm_error(pnm_ptr, pnm_err_badpnm);
    pnm_ptr->width = ch;
    if ((ch = pnm_getint(fp)) <= 0)
        pnm_error(pnm_ptr, pnm_err_badpnm);
    pnm_ptr->height = ch;
    if (pnm_ptr->type_code % 3 == 1)  /* PBM */
        pnm_ptr->maxval = 1;
    else
    {
        if ((ch = pnm_getint(fp)) <= 0)
            pnm_error(pnm_ptr, pnm_err_badpnm);
        pnm_ptr->maxval = ch;
    }
    if (pnm_ptr->maxval > 65535U)
        pnm_error(pnm_ptr, pnm_err_badpnm);
}


/**
 * Reads a PNM row from a file.
 **/
void pnm_read_row(FILE *fp, pnm_struct *pnm_ptr, unsigned int *row_ptr)
{
    unsigned int type_code = pnm_ptr->type_code;
    unsigned int sample_count = (type_code % 3 == 0) ? 3 : 1;
    unsigned int row_size = sample_count * pnm_ptr->width;
    unsigned int i;
    int is_raw = (type_code >= PNM_P4);
    int ch, j;
    unsigned int uch;

    /* read the row */
    if (type_code % 3 == 1)  /* PBM */
    {
        if (is_raw)
        {
            for (i = 0; i < row_size; )
            {
                ch = getc(fp);
                if (ch == EOF)
                    pnm_error(pnm_ptr, pnm_err_read);
                for (j = 7; j >= 0 && i < row_size; --j, ++i)
                    row_ptr[i] = 1 - ((ch >> j) & 1);
            }
        }
        else
        {
            for (i = 0; i < row_size; ++i)
            {
                do
                {
                    ch = pnm_getchar(fp);
                    if (ch == EOF)
                        pnm_error(pnm_ptr, pnm_err_read);
                } while (pnm_isspace(ch));
                if (ch != '0' && ch != '1')
                    pnm_error(pnm_ptr, pnm_err_badpnm);
                row_ptr[i] = (ch == '0') ? 1 : 0;
            }
        }
    }
    else  /* PGM, PPM */
    {
        if (is_raw)
        {
            if (pnm_ptr->maxval <= 255)  /* one byte per sample */
            {
                for (i = 0; i < row_size; ++i)
                {
                    ch = getc(fp);
                    if (ch == EOF)
                        pnm_error(pnm_ptr, pnm_err_read);
                    row_ptr[i] = ch;
                }
            }
            else  /* two bytes per sample, big endian */
            {
                for (i = 0; i < row_size; ++i)
                {
                    uch = getc(fp);
                    ch = getc(fp);
                    if (ch == EOF)
                        pnm_error(pnm_ptr, pnm_err_read);
                    row_ptr[i] = (uch << 8) + ch;
                }
            }
        }
        else
        {
            for (i = 0; i < row_size; ++i)
            {
                if ((ch = pnm_getint(fp)) < 0)
                    pnm_error(pnm_ptr, pnm_err_badpnm);
                row_ptr[i] = ch;
            }
        }
    }
}
