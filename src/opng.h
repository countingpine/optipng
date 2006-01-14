/*
 * opng.h - auxiliary functions used by OptiPNG and proposed for libpng.
 *
 * Copyright (C) 2001-2004 Cosmin Truta.
 * The program is distributed under the same licensing and warranty terms
 * as libpng.
 *
 * These functions are not currently offered by libpng.
 * The author encourages their addition to libpng.
 *
 * The prefix of the newly-added macros is OPNG_;
 * the prefix of the newly_added functions is opng_.
 * If something goes into libpng, its prefix will change to PNG_/png_.
 * Until then, we step over the public access by #defining PNG_INTERNAL.
 */


/* The following functions/macros are part of libpng-private:
 *   png_get_int_16  / png_save_int_16
 *   png_get_uint_16 / png_save_uint_16
 *   png_get_int_32  / png_save_int_32
 *   png_get_uint_32 / png_save_uint_32
 * However, they should be public, for the following reasons:
 * - Any application may benefit from them when accessing various
 *   PNG chunks.
 * - They are safe: when called, they do not affect the validity of
 *   the libpng internal structures.
 * - They are independent and do not introduce extra coupling in
 *   libpng.
 */

#define PNG_INTERNAL

#include "png.h"

#define OPNG_IMAGE_REDUCTIONS_SUPPORTED  /* implemented here */


#if !defined(PNG_bKGD_SUPPORTED) || !defined(PNG_hIST_SUPPORTED) || \
    !defined(PNG_sBIT_SUPPORTED) || !defined(PNG_tRNS_SUPPORTED)
#error This program requires libpng with support for bKGD, hIST, sBIT and tRNS
#endif

#ifndef PNG_INFO_IMAGE_SUPPORTED
#error This program requires libpng with PNG_INFO_IMAGE_SUPPORTED
#endif

#ifndef PNG_FREE_ME_SUPPORTED
#error This program requires libpng with PNG_FREE_ME_SUPPORTED
#endif

#ifndef OPNG_IMAGE_REDUCTIONS_SUPPORTED
#error This program requires libpng-cos with OPNG_IMAGE_REDUCTIONS_SUPPORTED
#endif


/*
 * OPNG_ASSERT
 * Vanilla version, based on assert().
 * A full implementation based on png_error() may require png_ptr.
 */
#ifndef OPNG_ASSERT
#define DEBUG
#if (!defined(PNG_DEBUG) || !PNG_DEBUG) && !defined(DEBUG)
#define NDEBUG
#endif
#include <assert.h>
#define OPNG_ASSERT(cond) assert(cond)
#define OPNG_ASSERT_MSG(cond, msg) assert(cond)
#endif


/*
 * The following code introduces io_state and the related constants.
 * io_state is a field in which the current i/o state is stored,
 * and may be accessed when performing an event-based i/o handling.
 */
#define OPNG_IO_UNKNOWN   0x0000
#define OPNG_IO_READING   0x0001
#define OPNG_IO_WRITING   0x0002
#define OPNG_IO_LEN       0x0010
#define OPNG_IO_HDR       0x0020
#define OPNG_IO_DATA      0x0040
#define OPNG_IO_CRC       0x0080
#define OPNG_IO_SIG       0x0100
#define OPNG_IO_MASK_OP   0x000f  /* reading/writing */
#define OPNG_IO_MASK_LOC  0x0ff0  /* sig/len/data/crc */

/*
 * This function may be called from the custom implementation of the read/write
 * functions hooked via png_set_read_fn() or png_set_write_fn().
 */
extern PNG_EXPORT(png_uint_32,opng_get_io_state)
   PNGARG((png_structp png_ptr));
/* There is no opng_set_io_state */

/*
 * These opng_ functions replace the corresponding png_ functions
 * supplied by libpng.
 * CAUTION: These functions must be called before performing any I/O operation.
 */
extern PNG_EXPORT(void,opng_set_read_fn) PNGARG((png_structp png_ptr,
   png_voidp io_ptr, png_rw_ptr read_data_fn));
extern PNG_EXPORT(void,opng_set_write_fn) PNGARG((png_structp png_ptr,
   png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn));


/*
 * Indicate whether the image information is valid.
 * (The image information is valid if the critical information
 * is present in the png structures.)
 */
extern PNG_EXPORT(int,opng_validate_image)
   PNGARG((png_structp png_ptr, png_infop info_ptr));


/*
 * Reduce the image (bit depth + color type + palette) without
 * losing any information. The image data must be present
 * (e.g. after calling png_set_rows(), or after loading IDAT).
 */
extern PNG_EXPORT(png_uint_32,opng_reduce_image)
   PNGARG((png_structp png_ptr, png_infop info_ptr, png_uint_32 reductions));

/*
 * PNG reduction flags.
 */
#define OPNG_REDUCE_16_TO_8          0x0001
#define OPNG_REDUCE_8_TO_421         0x0002
#define OPNG_REDUCE_RGB_TO_GRAY      0x0010  /* also RGBA to GA */
#define OPNG_REDUCE_RGB_TO_PALETTE   0x0020  /* also RGBA to palette/tRNS */
#define OPNG_REDUCE_PALETTE_TO_GRAY  0x0040  /* also palette/tRNS to GA */
#define OPNG_REDUCE_STRIP_ALPHA      0x0080  /* also create tRNS if needed */
#define OPNG_REDUCE_PALETTE_FAST     0x0100  /* remove trailing bogus entries
                                                and update tRNS */
#define OPNG_REDUCE_PALETTE_FULL     0x0200  /* remove all bogus entries
                                                and optimize tRNS */
#define OPNG_REDUCE_NONE             0x0000

#define OPNG_REDUCE_BIT_DEPTH  \
   (OPNG_REDUCE_16_TO_8 | OPNG_REDUCE_8_TO_421)

#define OPNG_REDUCE_COLOR_TYPE  \
   (OPNG_REDUCE_RGB_TO_GRAY | OPNG_REDUCE_RGB_TO_PALETTE |  \
    OPNG_REDUCE_PALETTE_TO_GRAY | OPNG_REDUCE_STRIP_ALPHA)

#define OPNG_REDUCE_PALETTE  \
   (OPNG_REDUCE_PALETTE_FAST | OPNG_REDUCE_PALETTE_FULL)

#define OPNG_REDUCE_ALL \
   (OPNG_REDUCE_BIT_DEPTH | OPNG_REDUCE_COLOR_TYPE | OPNG_REDUCE_PALETTE)

