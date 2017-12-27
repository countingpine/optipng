/*
 * tiffutil.c
 * General-purpose routines for minitiff.
 *
 * Copyright (C) 2006-2017 Cosmin Truta.
 *
 * minitiff is open-source software, distributed under the zlib license.
 * For conditions of distribution and use, see copyright notice in minitiff.h.
 */

#include "minitiff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Constructor.
 */
void minitiff_init_info(struct minitiff_info *info_ptr)
{
    memset(info_ptr, 0, sizeof(*info_ptr));
    info_ptr->photometric = (unsigned int)(-1);
}

/*
 * Validator.
 */
void minitiff_validate_info(const struct minitiff_info *info_ptr)
{
    if (info_ptr->width == 0 || info_ptr->height == 0)
        minitiff_error(info_ptr, "Invalid image dimensions in TIFF file");
    if (info_ptr->bits_per_sample == 0 || info_ptr->samples_per_pixel == 0)
        minitiff_error(info_ptr, "Invalid pixel info in TIFF file");
    if (info_ptr->strip_offsets == NULL || info_ptr->rows_per_strip == 0)
        minitiff_error(info_ptr, "Invalid strip info in TIFF file");
    if (info_ptr->compression != MINITIFF_COMPRESSION_NONE)
        minitiff_error(info_ptr,
                       "Unsupported compression method in TIFF file");
    if (info_ptr->photometric >= MINITIFF_PHOTOMETRIC_PALETTE)
        minitiff_error(info_ptr,
                       "Unsupported photometric interpretation in TIFF file");
}

/*
 * Destructor.
 */
void minitiff_destroy_info(struct minitiff_info *info_ptr)
{
    if (info_ptr->strip_offsets != NULL)
        free(info_ptr->strip_offsets);
}

/*
 * Error handling utility.
 */
static void default_error_handler(const char *msg)
{
    fprintf(stderr, "minitiff: error: %s\n", msg);
    exit(EXIT_FAILURE);
}

/*
 * Error handler.
 */
void minitiff_error(const struct minitiff_info *info_ptr, const char *msg)
{
    if (info_ptr->error_handler != NULL)
        info_ptr->error_handler(msg);
    else
        default_error_handler(msg);
    abort();
}

/*
 * Warning handling utility.
 */
static void default_warning_handler(const char *msg)
{
    fprintf(stderr, "minitiff: warning: %s\n", msg);
}

/*
 * Warning handler.
 */
void minitiff_warning(const struct minitiff_info *info_ptr, const char *msg)
{
    if (info_ptr->warning_handler != NULL)
        info_ptr->warning_handler(msg);
    else
        default_warning_handler(msg);
}

/*
 * Global constants: TIFF file signature.
 */
const char minitiff_sig_m[4] = { 0x4d, 0x4d, 0x00, 0x2a };  /* "MM\0*" */
const char minitiff_sig_i[4] = { 0x49, 0x49, 0x2a, 0x00 };  /* "II*\0" */

/*
 * Global constants: BigTIFF file signature.
 */
const char minitiff_sig_bigm[4] = { 0x4d, 0x4d, 0x00, 0x2b };  /* "MM\0*" */
const char minitiff_sig_bigi[4] = { 0x49, 0x49, 0x2b, 0x00 };  /* "II*\0" */
