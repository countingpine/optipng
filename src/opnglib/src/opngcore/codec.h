/*
 * opngcore/codec.h
 * PNG encoding and decoding.
 *
 * Copyright (C) 2001-2011 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#ifndef OPNGCORE_CODEC_H
#define OPNGCORE_CODEC_H

#ifndef OPNGLIB_INTERNAL
#error This header, internal to opnglib, is not meant to be visible outside.
#endif

#include "opnglib/opngtrans.h"

#include <limits.h>
#include <stdio.h>

#include "opngreduc.h"
#include "png.h"

#include "image.h"
#include "../opngtrans/trans.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 * The encoding parameters structure.
 */
struct opng_encoding_params
{
    int filter;
    int interlace;
    int zcompr_level;
    int zmem_level;
    int zstrategy;
    int zwindow_bits;
};

/*
 * The encoding statistics structure.
 */
struct opng_encoding_stats
{
    png_uint_32 flags;
    png_uint_32 idat_size;
    png_uint_32 plte_trns_size;
#if LONG_MAX > 0x7fffffffL
    png_uint_32 reserved;  /* align on 64-bit machines */
#endif
    unsigned long file_size;
    long datastream_offset;
};

/*
 * Encoding flags.
 */
enum
{
    OPNG_IS_PNG_FILE           = 0x0001,
    OPNG_HAS_PNG_DATASTREAM    = 0x0002,
    OPNG_HAS_PNG_SIGNATURE     = 0x0004,
    OPNG_HAS_DIGITAL_SIGNATURE = 0x0008,
    OPNG_HAS_MULTIPLE_IMAGES   = 0x0010,
    OPNG_HAS_SNIPPED_IMAGES    = 0x0020,
    OPNG_HAS_STRIPPED_METADATA = 0x0040,
    OPNG_HAS_JUNK              = 0x0080,
    OPNG_HAS_ERRORS            = 0x0100,
    OPNG_NEEDS_NEW_FILE        = 0x1000,
    OPNG_NEEDS_NEW_IDAT        = 0x2000
};

/*
 * The codec context structure.
 * Everything that libpng and its callbacks use is found in here.
 * Although this structure is exposed so that it can be placed on
 * the stack, the caller must pretend not to know what's inside.
 */
struct opng_codec_context
{
    struct opng_image *image;
    struct opng_encoding_stats *stats;
    FILE *stream;
    const char *fname;
    png_structp libpng_ptr;
    png_infop info_ptr;
    const opng_transformer_t *transformer;
    long crt_idat_offset;
    png_uint_32 crt_idat_size;
    png_uint_32 crt_idat_crc;
    png_uint_32 expected_idat_size;
    int crt_chunk_is_allowed;
    int crt_chunk_is_idat;
};

/*
 * Initializes a codec context object.
 */
void
opng_init_codec_context(struct opng_codec_context *context,
                        struct opng_image *image,
                        struct opng_encoding_stats *stats,
                        png_uint_32 expected_idat_size,
                        const opng_transformer_t *transformer);

/*
 * Decodes an image from an image file stream.
 * The image may be either in PNG format or in an external file format.
 * The function returns 0 on success or -1 on error.
 */
int
opng_decode_image(struct opng_codec_context *context,
                  FILE *stream,
                  const char *fname,
                  const char **format_name_ptr,
                  const char **format_xdesc_ptr);

/*
 * Attempts to reduce the imported image.
 * The function returns a mask of successful reductions (0 for no reductions),
 * or -1 on error.
 * No error is normally expected to occur; if it does, it indicates a defect.
 */
int
opng_decode_reduce_image(struct opng_codec_context *context,
                         int reductions);

/*
 * Attempts to set and/or reset image data objects within the imported image.
 * The function returns a mask of the objects that have been altered.
 */
opng_id_t
opng_decode_set_reset_data(struct opng_codec_context *context);

/*
 * Stops the decoder.
 * Frees the stored PNG image data and clears the internal image object,
 * if required.
 */
void
opng_decode_finish(struct opng_codec_context *context,
                   int free_data);

/*
 * Encodes an image to a PNG file stream.
 * If the output file stream is NULL, PNG encoding is still done,
 * and statistics are still collected, but no actual data is written.
 * The function returns 0 on success or -1 on error.
 */
int
opng_encode_image(struct opng_codec_context *context,
                  const struct opng_encoding_params *params,
                  FILE *stream,
                  const char *fname);

/*
 * Stops the decoder.
 * Frees the stored PNG image data and clears the internal image object,
 * if required.
 */
void
opng_encode_finish(struct opng_codec_context *context);

/*
 * Copies a PNG file stream to another PNG file stream.
 * The function returns 0 on success or -1 on error.
 */
int
opng_copy_png(struct opng_codec_context *context,
              FILE *in_stream, const char *in_fname,
              FILE *out_stream, const char *out_fname);


/*
 * The chunk signatures recognized and handled by this codec.
 */
extern const png_byte opng_sig_PLTE[4];
extern const png_byte opng_sig_tRNS[4];
extern const png_byte opng_sig_IDAT[4];
extern const png_byte opng_sig_IEND[4];
extern const png_byte opng_sig_bKGD[4];
extern const png_byte opng_sig_hIST[4];
extern const png_byte opng_sig_sBIT[4];
extern const png_byte opng_sig_dSIG[4];
extern const png_byte opng_sig_acTL[4];
extern const png_byte opng_sig_fcTL[4];
extern const png_byte opng_sig_fdAT[4];

/*
 * Tests whether the given chunk is an image chunk.
 * An image chunk is a chunk that stores image data: a critical chunk or tRNS.
 *
 * It can be argued that APNG fdAT is also an image chunk, but we don't
 * consider that here. We handle APNG separately.
 */
int
opng_is_image_chunk(const png_byte *chunk_type);

/*
 * Tests whether the given chunk is a metadata chunk.
 * Ancillary chunks other than tRNS are all considered to store metadata,
 * but see the comment about APNG above.
 */
int
opng_is_metadata_chunk(const png_byte *chunk_type);

/*
 * Tests whether the given chunk is an APNG chunk.
 */
int
opng_is_apng_chunk(const png_byte *chunk_type);


#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif  /* OPNGCORE_CODEC_H */
