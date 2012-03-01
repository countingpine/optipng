/*
 * opngtrans/trans.h
 * Image transformations.
 *
 * Copyright (C) 2011 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#ifndef OPNGTRANS_TRANS_H
#define OPNGTRANS_TRANS_H

#ifndef OPNGLIB_INTERNAL
#error This header, internal to opnglib, is not meant to be visible outside.
#endif

#include "opnglib/opngtrans.h"

#include <stdlib.h>

#include "png.h"

#include "chunksig.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Object IDs.
 *
 * OPNG_ID_FOO_BAR represents "foo.bar", unless otherwise noted.
 * OPNG_ID_CHUNK_IMAGE represents a chunk that stores image data.
 * OPNG_ID_CHUNK_META represents a chunk that stores metadata.
 *
 * The underlying bitset representation is highly efficient, although
 * it restricts the number of recognized IDs. This is an acceptable
 * restriction for the time being, because only a few IDs are currently
 * in use.
 */
typedef enum
{
    OPNG_ID_ALL                   = 0x0001,  /* "all" */
    OPNG_ID_CHUNK_IMAGE           = 0x0002,  /* critical chunk or tRNS */
    OPNG_ID_CHUNK_META            = 0x0004,  /* ancillary chunk except tRNS */
    OPNG_ID_IMAGE                 = 0x0008,  /* "image" or inaccessible obj */
    OPNG_ID_IMAGE_GRAY            = 0x0010,  /* "image.gray" */
    OPNG_ID_IMAGE_RED             = 0x0020,  /* "image.red" */
    OPNG_ID_IMAGE_GREEN           = 0x0040,  /* "image.green" */
    OPNG_ID_IMAGE_BLUE            = 0x0080,  /* "image.blue" */
    OPNG_ID_IMAGE_ALPHA           = 0x0100,  /* "image.alpha" */
    OPNG_ID_IMAGE_PRECISION       = 0x0200,  /* "image.precision" */
    OPNG_ID_IMAGE_GRAY_PRECISION  = 0x0400,  /* "image.gray.precision" */
    OPNG_ID_IMAGE_RED_PRECISION   = 0x0800,  /* "image.red.precision" */
    OPNG_ID_IMAGE_GREEN_PRECISION = 0x1000,  /* "image.green.precision" */
    OPNG_ID_IMAGE_BLUE_PRECISION  = 0x2000,  /* "image.blue.precision" */
    OPNG_ID_IMAGE_RGB_PRECISION   = 0x4000,  /* "image.rgb.precision" */
    OPNG_ID_IMAGE_ALPHA_PRECISION = 0x8000,  /* "image.alpha.precision" */
    OPNG_ID_IMAGE_CHROMA_BT601   = 0x10000,  /* "image.chroma.bt601" */
    OPNG_ID_IMAGE_CHROMA_BT709   = 0x20000,  /* "image.chroma.bt709" */

    OPNG_ID_ANIMATION           = 0x100000,  /* "animation" */

    OPNG_ID_ERR               = 0x10000000,  /* generic error */
    OPNG_ID_ERR_SYNTAX,       /* ERR + 1 */  /* syntax error */
    OPNG_ID_ERR_UNKNOWN       /* ERR + 2 */  /* unrecognized object */
} opng_id_t;

/*
 * Object ID properties: "can reset", "can set", "can strip".
 */
enum
{
    OPNG_CAN_RESET_IDS =
        OPNG_ID_IMAGE_ALPHA |
        OPNG_ID_IMAGE_CHROMA_BT601 |
        OPNG_ID_IMAGE_CHROMA_BT709 |
        OPNG_ID_ANIMATION,
    OPNG_CAN_SET_IDS =
        OPNG_ID_IMAGE_PRECISION |
        OPNG_ID_IMAGE_GRAY_PRECISION |
        OPNG_ID_IMAGE_RED_PRECISION |
        OPNG_ID_IMAGE_GREEN_PRECISION |
        OPNG_ID_IMAGE_BLUE_PRECISION |
        OPNG_ID_IMAGE_RGB_PRECISION |
        OPNG_ID_IMAGE_ALPHA_PRECISION,
    OPNG_CAN_STRIP_IDS =
        OPNG_ID_ALL |
        OPNG_ID_CHUNK_META,
    OPNG_CAN_PROTECT_IDS =
        OPNG_CAN_STRIP_IDS
};

/*
 * The transformer structure.
 */
struct opng_transformer
{
    struct opng_sigs strip_sigs;
    struct opng_sigs protect_sigs;
    opng_id_t strip_ids;
    opng_id_t protect_ids;
    opng_id_t reset_ids;
    int precision;
    int red_precision;
    int green_precision;
    int blue_precision;
    int alpha_precision;
};

/*
 * Returns 1 if the given chunk ought to be stripped, or 0 otherwise.
 */
int
opng_transform_query_strip_chunk(const opng_transformer_t *transformer,
                                 png_byte *chunk_sig);

/*
 * Retrieves the precision values to be set for each of the RGBA channels.
 */
void
opng_transform_query_set_precision(const opng_transformer_t *transformer,
                                   int *gray_precision_ptr,
                                   int *red_precision_ptr,
                                   int *green_precision_ptr,
                                   int *blue_precision_ptr,
                                   int *alpha_precision_ptr);

/*
 * Applies all set/reset transformations to the given libpng image structures.
 * Returns the ids of the objects that have been altered.
 */
opng_id_t
opng_transform_libpng_image(const opng_transformer_t *transformer,
                            png_structp libpng_ptr, png_infop info_ptr);


#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif  /* OPNGTRANS_TRANS_H */
