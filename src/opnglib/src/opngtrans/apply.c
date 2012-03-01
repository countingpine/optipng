/*
 * opngtrans/apply.c
 * Apply the image transformations.
 *
 * Copyright (C) 2011 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "opnglib/opngtrans.h"

#include "png.h"

#define OPNGLIB_INTERNAL
#include "trans.h"


/*
 * The BT.601 chromaticity coefficients.
 * In floating-point arithmetic:
 *    Y = 0.299 * R + 0.587 * G + 0.114 * B
 * In 16-bit fixed-point arithmetic:
 *    Y = 19595 * R + 38470 * G + 7471 * B
 */
enum
{
    Kr_bt601 = 19595U,
    Kg_bt601 = 38470U,
    Kb_bt601 =  7471U
};

/*
 * The BT.709 chromaticity coefficients.
 * In floating-point arithmetic:
 *    Y = 0.2126 * R + 0.7152 * G + 0.0722 * B
 * In 16-bit fixed-point arithmetic:
 *    Y = 13932.9536 * R + 46871.3472 * G + 4731.6992 * B
 */
enum
{
    Kr_bt709 = 13933U,
    Kg_bt709 = 46871U,
    Kb_bt709 =  4731U
};

/*
 * Convert an 8-bit RGB pixel to grayscale, as specified in ITU BT.601.
 */
static void
opng_reset_chroma8_bt601(png_bytep rgb8)
{
    /* Use 16-bit multiplications and 24-bit additions. */
    png_uint_32 r24 = rgb8[0] * (png_uint_32)Kr_bt601;
    png_uint_32 g24 = rgb8[1] * (png_uint_32)Kg_bt601;
    png_uint_32 b24 = rgb8[2] * (png_uint_32)Kb_bt601;
    png_uint_32 y24 = r24 + g24 + b24;
    png_byte y8 = (png_byte)((y24 + 32767U) / 65535U);
    rgb8[0] = y8;
    rgb8[1] = y8;
    rgb8[2] = y8;
}

/*
 * Convert an 8-bit RGB pixel to grayscale, as specified in ITU BT.709.
 */
static void
opng_reset_chroma8_bt709(png_bytep rgb8)
{
    /* Use 16-bit multiplications and 24-bit additions. */
    png_uint_32 r24 = rgb8[0] * (png_uint_32)Kr_bt709;
    png_uint_32 g24 = rgb8[1] * (png_uint_32)Kg_bt709;
    png_uint_32 b24 = rgb8[2] * (png_uint_32)Kb_bt709;
    png_uint_32 y24 = r24 + g24 + b24;
    png_byte y8 = (png_byte)((y24 + 32767U) / 65535U);
    rgb8[0] = y8;
    rgb8[1] = y8;
    rgb8[2] = y8;
}

/*
 * Convert a 16-bit RGB pixel to grayscale, as specified in ITU BT.601.
 */
static void
opng_reset_chroma16_bt601(png_bytep rgb16)
{
    /* Use 16-bit multiplications and 32-bit additions. */
    unsigned int r16 = png_get_uint_16(rgb16 + 0);
    unsigned int g16 = png_get_uint_16(rgb16 + 2);
    unsigned int b16 = png_get_uint_16(rgb16 + 4);
    png_uint_32 r32 = r16 * (png_uint_32)Kr_bt601;
    png_uint_32 g32 = g16 * (png_uint_32)Kg_bt601;
    png_uint_32 b32 = b16 * (png_uint_32)Kb_bt601;
    png_uint_32 y32 = r32 + g32 + b32;
    unsigned int y16 = (unsigned int)((y32 + 32767U) / 65535U);
    png_save_uint_16(rgb16 + 0, y16);
    png_save_uint_16(rgb16 + 2, y16);
    png_save_uint_16(rgb16 + 4, y16);
}

/*
 * Convert a 16-bit RGB pixel to grayscale, as specified in ITU BT.709.
 */
static void
opng_reset_chroma16_bt709(png_bytep rgb16)
{
    /* Use 16-bit multiplications and 32-bit additions. */
    unsigned int r16 = png_get_uint_16(rgb16 + 0);
    unsigned int g16 = png_get_uint_16(rgb16 + 2);
    unsigned int b16 = png_get_uint_16(rgb16 + 4);
    png_uint_32 r32 = r16 * (png_uint_32)Kr_bt709;
    png_uint_32 g32 = g16 * (png_uint_32)Kg_bt709;
    png_uint_32 b32 = b16 * (png_uint_32)Kb_bt709;
    png_uint_32 y32 = r32 + g32 + b32;
    unsigned int y16 = (unsigned int)((y32 + 32767U) / 65535U);
    png_save_uint_16(rgb16 + 0, y16);
    png_save_uint_16(rgb16 + 2, y16);
    png_save_uint_16(rgb16 + 4, y16);
}

static void
opng_set_precision8(png_bytep alpha_ptr, int new_precision)
{
    /* assert(new_precision > 0 && new_precision < 8); */
    unsigned int old_value = alpha_ptr[0];
    unsigned int chop_value = old_value >> (8 - new_precision);
    unsigned int chop_max = (1U << new_precision) - 1;
    unsigned int new_value = (chop_value * 255 + chop_max / 2) / chop_max;
    alpha_ptr[0] = (png_byte)new_value;
}

static void
opng_set_precision16(png_bytep alpha_ptr, int new_precision)
{
    /* assert(new_precision > 0 && new_precision < 16); */
    unsigned int old_value = png_get_uint_16(alpha_ptr);
    unsigned int chop_value = old_value >> (16 - new_precision);
    unsigned int chop_max = (1U << new_precision) - 1;
    unsigned int new_value =
        (unsigned int)
            (((png_uint_32)chop_value * 65535U + chop_max / 2) / chop_max);
    png_save_uint_16(alpha_ptr, new_value);
}

/*
 * Sets the precision of the alpha channel to a given value;
 * if this value is 0, resets (i.e. sets to MAX) the alpha channel.
 * Returns 1 if this operation is applied, or 0 otherwise.
 */
int
opng_transform_set_alpha_precision(png_structp libpng_ptr, png_infop info_ptr,
                                   int alpha_precision)
{
    png_uint_32 width, height;
    int bit_depth, color_type;
    png_bytep trans_alpha;
    int num_trans;
    png_color_16p trans_color_ptr;
    png_bytepp row_ptr;
    png_bytep alpha_ptr;
    unsigned int num_channels, channel_size, pixel_size;
    unsigned int alpha_offset;
    png_uint_32 i, j;
    int k;
    int result;

    typedef void (*set_precision_fn_t)(png_bytep, int);
    set_precision_fn_t set_precision_fn;

    if (alpha_precision < 0 || alpha_precision >= 16)
        return 0;
    png_get_IHDR(libpng_ptr, info_ptr,
                 &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    if (bit_depth <= 8 && alpha_precision >= 8)
        return 0;

    result = 0;
    set_precision_fn = (bit_depth <= 8) ?
                       opng_set_precision8 : opng_set_precision16;

    /* Invalidate tRNS. */
    if (png_get_tRNS(libpng_ptr, info_ptr,
                     &trans_alpha, &num_trans, &trans_color_ptr))
    {
        if (alpha_precision > 0)
        {
            if (num_trans > 0)
            {
                for (k = 0; k < num_trans; ++k)
                    opng_set_precision8(trans_alpha + k, alpha_precision);
                result = 1;
            }
            /* else trans_color needs no transformation */
        }
        else  /* alpha_precision == 0 */
        {
            png_set_invalid(libpng_ptr, info_ptr, PNG_INFO_tRNS);
            result = 1;
        }
        /* Continue regardless whether PLTE is present.
         * PLTE may exist even if the color type is not palette.
         */
    }

    /* Set the alpha plane to opaque in IDAT. */
    if (color_type & PNG_COLOR_MASK_ALPHA)
    {
        row_ptr = png_get_rows(libpng_ptr, info_ptr);
        channel_size = (bit_depth > 8) ? 2 : 1;
        num_channels = png_get_channels(libpng_ptr, info_ptr);
        pixel_size = num_channels * channel_size;
        alpha_offset = pixel_size - channel_size;
        if (alpha_precision > 0)
        {
            for (i = 0; i < height; ++i, ++row_ptr)
            {
                alpha_ptr = *row_ptr + alpha_offset;
                for (j = 0; j < width; ++j, alpha_ptr += pixel_size)
                    (*set_precision_fn)(alpha_ptr, alpha_precision);
            }
        }
        else  /* alpha_precision == 0 */
        {
            for (i = 0; i < height; ++i, ++row_ptr)
            {
                alpha_ptr = *row_ptr + alpha_offset;
                for (j = 0; j < width; ++j, alpha_ptr += pixel_size)
                {
                    alpha_ptr[0] = 255;
                    alpha_ptr[channel_size - 1] = 255;
                }
            }
        }
        result = 1;
    }

    return result;
}

/*
 * Resets (i.e. sets to zero) the chroma channel.
 * Returns 1 if this operation is applied, or 0 otherwise.
 */
int
opng_transform_reset_chroma(png_structp libpng_ptr, png_infop info_ptr,
                            int chroma_spec)
{
    png_uint_32 width, height;
    int bit_depth, color_type;
    png_colorp palette;
    int num_palette;
    png_colorp palette_entry_ptr;
    png_bytepp row_ptr;
    png_bytep pixel_ptr;
    unsigned int channel_size, pixel_size;
    png_byte rgb_buf[6];
    png_uint_32 i, j;
    int k;

    typedef void (*reset_chroma_fn_t)(png_bytep);
    reset_chroma_fn_t reset_chroma8_fn, reset_chroma16_fn, reset_chroma_fn;

    png_get_IHDR(libpng_ptr, info_ptr,
                 &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    /* Set up the reset_chroma function */
    if (chroma_spec == 601)  /* BT.601 */
    {
        reset_chroma8_fn = opng_reset_chroma8_bt601;
        reset_chroma16_fn = opng_reset_chroma16_bt601;
    }
    else if (chroma_spec == 709)  /* BT.709 */
    {
        reset_chroma8_fn = opng_reset_chroma8_bt709;
        reset_chroma16_fn = opng_reset_chroma16_bt709;
    }
#if 0  /* not implemented */
    else if (chroma_spec == 240)  /* SMPTE 240M */
    {
        reset_chroma8_fn = opng_reset_chroma8_smpte240;
        reset_chroma16_fn = opng_reset_chroma16_smpte240;
    }
#endif
    else
    {
        /* Unknown chroma coefficients; do nothing. */
        return 0;
    }
    if (bit_depth <= 8)
    {
        channel_size = 1;
        reset_chroma_fn = reset_chroma8_fn;
    }
    else
    {
        channel_size = 2;
        reset_chroma_fn = reset_chroma16_fn;
    }

    if (color_type & PNG_COLOR_MASK_PALETTE)
    {
        /* Reset chroma in PLTE. */
        if (!png_get_PLTE(libpng_ptr, info_ptr, &palette, &num_palette))
            return 0;
        palette_entry_ptr = palette;
        for (k = 0; k < num_palette; ++k, ++palette_entry_ptr)
        {
            /* This is not very efficient, but the iterations are few. */
            rgb_buf[0] = palette_entry_ptr->red;
            rgb_buf[1] = palette_entry_ptr->green;
            rgb_buf[2] = palette_entry_ptr->blue;
            (*reset_chroma8_fn)(rgb_buf);
            palette_entry_ptr->red =
                palette_entry_ptr->green =
                    palette_entry_ptr->blue = rgb_buf[0];
        }
    }
    else if (color_type & PNG_COLOR_MASK_COLOR)
    {
        /* Reset chroma in IDAT. */
        pixel_size = png_get_channels(libpng_ptr, info_ptr) * channel_size;
        row_ptr = png_get_rows(libpng_ptr, info_ptr);
        for (i = 0; i < height; ++i, ++row_ptr)
        {
            pixel_ptr = *row_ptr;
            for (j = 0; j < width; ++j, pixel_ptr += pixel_size)
                (*reset_chroma_fn)(pixel_ptr);
        }
    }
    else
    {
        /* The pixels are grayscale; do nothing. */
        return 0;
    }

    /* Invalidate the unsafe-to-copy ancillary chunks that can't stay. */
    if (!(color_type & PNG_COLOR_MASK_PALETTE))
    {
        /* PLTE, if it exists in this situation, has the same role as sPLT. */
        png_set_invalid(libpng_ptr, info_ptr, PNG_INFO_PLTE);
    }
#ifdef PNG_sPLT_SUPPORTED
    png_set_invalid(libpng_ptr, info_ptr, PNG_INFO_sPLT);
#endif
#ifdef PNG_hIST_SUPPORTED
    png_set_invalid(libpng_ptr, info_ptr, PNG_INFO_hIST);
#endif
#ifdef PNG_sBIT_SUPPORTED
    png_set_invalid(libpng_ptr, info_ptr, PNG_INFO_hIST);
#endif

    return 1;
}

/*
 * Applies all set/reset transformations to the given libpng image structures.
 */
opng_id_t
opng_transform_libpng_image(const opng_transformer_t *transformer,
                            png_structp libpng_ptr, png_infop info_ptr)
{
    opng_id_t result;
    int alpha_precision;

    opng_transform_query_set_precision(transformer,
                                       NULL, NULL, NULL, NULL,
                                       &alpha_precision);
    result = 0;

    /* Modify or reset the alpha samples. */
    if (transformer->reset_ids & OPNG_ID_IMAGE_ALPHA)
    {
        if (opng_transform_set_alpha_precision(libpng_ptr, info_ptr, 0))
            result |= OPNG_ID_IMAGE_ALPHA;
    }
    else if (alpha_precision > 0)
    {
        if (opng_transform_set_alpha_precision(libpng_ptr, info_ptr,
                                               alpha_precision))
            result |= OPNG_ID_IMAGE_ALPHA;
    }

    /* Reset chroma. */
    if (transformer->reset_ids & OPNG_ID_IMAGE_CHROMA_BT601)
    {
        if (opng_transform_reset_chroma(libpng_ptr, info_ptr, 601))
            result |= OPNG_ID_IMAGE_CHROMA_BT601;
    }
    else if (transformer->reset_ids & OPNG_ID_IMAGE_CHROMA_BT709)
    {
        if (opng_transform_reset_chroma(libpng_ptr, info_ptr, 709))
            result |= OPNG_ID_IMAGE_CHROMA_BT709;
    }

    return result;
}
