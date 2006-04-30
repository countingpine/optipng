/*
 * opng.h - auxiliary functions used by OptiPNG and proposed for libpng.
 *
 * Copyright (C) 2001-2006 Cosmin Truta.
 * This software is distributed under the same licensing and warranty terms
 * as libpng.
 *
 * The prefix of the newly-added macros is OPNG_;
 * the prefix of the newly-added functions is opng_.
 * If something goes into libpng, its prefix will change to PNG_/png_.
 * Until then, we step over the public access by #defining PNG_INTERNAL.
 */


#define PNG_INTERNAL

#include "png.h"

#if !(PNG_LIBPNG_BUILD_TYPE & PNG_LIBPNG_BUILD_PRIVATE)
#error This program requires the BUNDLED libpng version 1.2.x-optipng
#endif


#define OPNG_IO_STATE_SUPPORTED          /* implemented here */

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


/* To be added to png.h */
#ifdef OPNG_IO_STATE_SUPPORTED
extern PNG_EXPORT(png_uint_32,opng_get_io_state)
   PNGARG((png_structp png_ptr));

extern PNG_EXPORT(png_bytep,opng_get_io_chunk_name)
   PNGARG((png_structp png_ptr));

/* The flags returned by png_get_io_state() are the following: */
#define OPNG_IO_NONE        0x0000  /* no I/O at this moment */
#define OPNG_IO_READING     0x0001  /* currently reading */
#define OPNG_IO_WRITING     0x0002  /* currently writing */
#define OPNG_IO_SIGNATURE   0x0010  /* currently at the file signature */
#define OPNG_IO_CHUNK_HDR   0x0020  /* currently at the chunk header */
#define OPNG_IO_CHUNK_DATA  0x0040  /* currently at the chunk data */
#define OPNG_IO_CHUNK_CRC   0x0080  /* currently at the chunk crc */
#define OPNG_IO_MASK_OP     0x000f  /* current operation: reading/writing */
#define OPNG_IO_MASK_LOC    0x00f0  /* current location: sig/hdr/data/crc */
#endif /* ?OPNG_IO_STATE_SUPPORTED */


/*
 * To be deleted when adding the above code to png.h.
 *
 * These opng_ functions replace the corresponding png_ functions
 * supplied by libpng.
 */
extern PNG_EXPORT(void,opng_set_read_fn) PNGARG((png_structp png_ptr,
   png_voidp io_ptr, png_rw_ptr read_data_fn));
extern PNG_EXPORT(void,opng_set_write_fn) PNGARG((png_structp png_ptr,
   png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn));


/* To be added to png.h */
#ifdef OPNG_IMAGE_REDUCTIONS_SUPPORTED
/*
 * Indicate whether the image information is valid.
 * (The image information is valid if the critical information
 * is present in the png structures.)
 */
extern PNG_EXPORT(int,opng_validate_image)
   PNGARG((png_structp png_ptr, png_infop info_ptr));


/*
 * Reduce the image (bit depth + color type + palette) without
 * losing any information.  The image data must be present
 * (e.g. after calling png_set_rows(), or after loading IDAT).
 */
extern PNG_EXPORT(png_uint_32,opng_reduce_image)
   PNGARG((png_structp png_ptr, png_infop info_ptr, png_uint_32 reductions));

/*
 * PNG reduction flags.
 */
#define OPNG_REDUCE_16_TO_8          0x0001
#define OPNG_REDUCE_8_TO_4_2_1       0x0002
#define OPNG_REDUCE_RGB_TO_GRAY      0x0010  /* also RGBA to GA */
#define OPNG_REDUCE_RGB_TO_PALETTE   0x0020  /* also RGBA to palette/tRNS */
#define OPNG_REDUCE_PALETTE_TO_GRAY  0x0040  /* also palette/tRNS to GA */
#define OPNG_REDUCE_STRIP_ALPHA      0x0080  /* also create tRNS if needed */
#define OPNG_REDUCE_PALETTE_FAST     0x0100  /* remove trailing sterile entries
                                                and update tRNS */
#define OPNG_REDUCE_PALETTE_FULL     0x0200  /* remove all sterile entries
                                                and optimize tRNS */
#define OPNG_REDUCE_NONE             0x0000

#define OPNG_REDUCE_BIT_DEPTH  \
   (OPNG_REDUCE_16_TO_8 | OPNG_REDUCE_8_TO_4_2_1)

#define OPNG_REDUCE_COLOR_TYPE  \
   (OPNG_REDUCE_RGB_TO_GRAY | OPNG_REDUCE_RGB_TO_PALETTE |  \
    OPNG_REDUCE_PALETTE_TO_GRAY | OPNG_REDUCE_STRIP_ALPHA)

#define OPNG_REDUCE_PALETTE  \
   (OPNG_REDUCE_PALETTE_FAST | OPNG_REDUCE_PALETTE_FULL)

#define OPNG_REDUCE_ALL \
   (OPNG_REDUCE_BIT_DEPTH | OPNG_REDUCE_COLOR_TYPE | OPNG_REDUCE_PALETTE)

#endif /* ?OPNG_IMAGE_REDUCTIONS_SUPPORTED */
