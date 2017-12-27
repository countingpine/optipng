/*
 * tiffread.c
 * File input routines for minitiff.
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
 * Error messages.
 */
static const char *msg_err_alloc =
    "Error allocating memory for TIFF file";
static const char *msg_err_read =
    "Error reading TIFF file";
static const char *msg_err_seek =
    "Error repositioning TIFF file";
static const char *msg_err_notiff =
    "Not a TIFF file";
static const char *msg_err_bigtiff =
    "Unsupported BigTIFF file";
static const char *msg_err_invalid =
    "Invalid TIFF file";
static const char *msg_err_range =
    "Value out of the supported range in TIFF file";
static const char *msg_err_unsupported =
    "Unsupported data in TIFF file";
static const char *msg_warn_metadata =
    "Unrecognized metadata in TIFF file";
static const char *msg_warn_multiple =
    "Selected first image from multi-image TIFF file";


/*
 * Memory reader structure.
 */
struct minitiff_getter
{
    unsigned int (*get_ushort)(const unsigned char *buf_ptr);
    unsigned long (*get_ulong)(const unsigned char *buf_ptr);
};

/*
 * Memory reader.
 */
static unsigned int get_ushort_m(const unsigned char *buf_ptr)
{
    return ((unsigned int)buf_ptr[0] << 8) +
           ((unsigned int)buf_ptr[1]);
}

/*
 * Memory reader.
 */
static unsigned int get_ushort_i(const unsigned char *buf_ptr)
{
    return ((unsigned int)buf_ptr[0]) +
           ((unsigned int)buf_ptr[1] << 8);
}

/*
 * Memory reader.
 */
static unsigned long get_ulong_m(const unsigned char *buf_ptr)
{
    return ((unsigned long)buf_ptr[0] << 24) +
           ((unsigned long)buf_ptr[1] << 16) +
           ((unsigned long)buf_ptr[2] << 8) +
           ((unsigned long)buf_ptr[3]);
}

/*
 * Memory reader.
 */
static unsigned long get_ulong_i(const unsigned char *buf_ptr)
{
    return ((unsigned long)buf_ptr[0]) +
           ((unsigned long)buf_ptr[1] << 8) +
           ((unsigned long)buf_ptr[2] << 16) +
           ((unsigned long)buf_ptr[3] << 24);
}

/*
 * Memory reader.
 */
static unsigned long get_ulong_value(const struct minitiff_getter *getter_ptr,
                                     int tag_type,
                                     const unsigned char *buf_ptr)
{
    switch (tag_type)
    {
    case MINITIFF_TYPE_BYTE:
        return (unsigned long)buf_ptr[0];
    case MINITIFF_TYPE_SHORT:
        return (unsigned long)getter_ptr->get_ushort(buf_ptr);
    case MINITIFF_TYPE_LONG:
        return getter_ptr->get_ulong(buf_ptr);
    default:
        return (unsigned long)-1L;  /* error */
    }
}

/*
 * Memory allocator.
 */
static unsigned long *alloc_ulong_array(struct minitiff_info *info_ptr,
                                        size_t count)
{
    unsigned long *result;
    if (count > (size_t)(-1) / sizeof(unsigned long))
        minitiff_error(info_ptr, msg_err_range);
    result = (unsigned long *)malloc(count * sizeof(unsigned long));
    if (result == NULL)
        minitiff_error(info_ptr, msg_err_alloc);
    return result;
}

/*
 * Type-casting utilities.
 */
#if UINT_MAX >= 0xffffffffUL
#define cast_ulong_to_uint(info_ptr, value) ((unsigned int)(value))
#define cast_ulong_to_size(info_ptr, value) ((size_t)(value))
#else
static unsigned int cast_ulong_to_uint(struct minitiff_info *info_ptr,
                                       unsigned long value)
{
    unsigned int result = (unsigned int)value;
    if (result != value)
        minitiff_error(info_ptr, msg_err_range);
    return result;
}
#define cast_ulong_to_size(info_ptr, value) \
    ((size_t)cast_ulong_to_uint(info_ptr, value))
#endif

/*
 * File reader.
 */
static size_t read_ulong_values(const struct minitiff_getter *getter_ptr,
                                int tag_type,
                                unsigned long values[], size_t count,
                                FILE *stream)
{
    unsigned char buf[4];
    size_t value_size;
    size_t i;

    switch (tag_type)
    {
    case MINITIFF_TYPE_BYTE:
        value_size = 1;
        break;
    case MINITIFF_TYPE_SHORT:
        value_size = 2;
        break;
    case MINITIFF_TYPE_LONG:
        value_size = 4;
        break;
    default:
        return 0;  /* read nothing */
    }

    for (i = 0; i < count; ++i)
    {
        if (fread(buf, value_size, 1, stream) != 1)
            break;
        values[i] = get_ulong_value(getter_ptr, tag_type, buf);
    }
    return i;
}

/*
 * File seeker.
 */
static void seek_to_offset(struct minitiff_info *info_ptr,
                           long offset, FILE *stream)
{
    if (offset < 0)
        minitiff_error(info_ptr, msg_err_range);
    if (ftell(stream) == offset)
        return;
    if (fseek(stream, offset, SEEK_SET) != 0)
        minitiff_error(info_ptr, msg_err_seek);
}

/*
 * TIFF structure reader.
 */
void minitiff_read_info(struct minitiff_info *info_ptr, FILE *stream)
{
    struct minitiff_getter getter;
    unsigned char buf[12];
    unsigned char *vbuf = buf + 8;
    unsigned long ulvals[4], ulval;
    long dir_offset;
    unsigned int dir_size, i;
    unsigned int tag_id, tag_type;
    size_t count;
    size_t bits_per_sample_count;
    unsigned int bits_per_sample_tag_type, strip_offsets_tag_type;
    long bits_per_sample_offset, strip_offsets_offset;
    int unknown_metadata_found;

    /* Read the TIFF header. */
    if (fread(buf, 8, 1, stream) != 1)
        goto err_read;
    if (memcmp(buf, minitiff_sig_m, 4) == 0)
    {
        info_ptr->byte_order = 'M';
        getter.get_ushort = get_ushort_m;
        getter.get_ulong = get_ulong_m;
    }
    else if (memcmp(buf, minitiff_sig_i, 4) == 0)
    {
        info_ptr->byte_order = 'I';
        getter.get_ushort = get_ushort_i;
        getter.get_ulong = get_ulong_i;
    }
    else if (memcmp(buf, minitiff_sig_bigm, 4) == 0 ||
             memcmp(buf, minitiff_sig_bigi, 4) == 0)
    {
        minitiff_error(info_ptr, msg_err_bigtiff);
        return;
    }
    else
    {
        minitiff_error(info_ptr, msg_err_notiff);
        return;
    }
    bits_per_sample_count = 0;
    bits_per_sample_tag_type = strip_offsets_tag_type = 0;
    bits_per_sample_offset = strip_offsets_offset = 0;
    dir_offset = (long)getter.get_ulong(buf + 4);
    if (dir_offset >= 0 && dir_offset < 8)
        goto err_invalid;
    seek_to_offset(info_ptr, dir_offset, stream);

    /* Read the TIFF directory. */
    if (fread(buf, 2, 1, stream) != 1)
        goto err_read;
    dir_size = getter.get_ushort(buf);
    unknown_metadata_found = 0;
    for (i = 0; i < dir_size; ++i)
    {
        if (fread(buf, 12, 1, stream) != 1)
            goto err_read;
        tag_id = getter.get_ushort(buf);
        tag_type = getter.get_ushort(buf + 2);
        count = cast_ulong_to_size(info_ptr, getter.get_ulong(buf + 4));
        if (count == 0)
            goto err_unsupported;
        switch (tag_id)
        {
        case MINITIFF_TAG_SUBFILE_TYPE:
            if (count != 1)
                goto err_unsupported;
            ulval = get_ulong_value(&getter, tag_type, vbuf);
            if (ulval != 0 && ulval != 1)
                goto err_unsupported;
            break;
        case MINITIFF_TAG_WIDTH:
            if (count != 1)
                goto err_unsupported;
            ulval = get_ulong_value(&getter, tag_type, vbuf);
            info_ptr->width = cast_ulong_to_size(info_ptr, ulval);
            break;
        case MINITIFF_TAG_HEIGHT:
            if (count != 1)
                goto err_unsupported;
            ulval = get_ulong_value(&getter, tag_type, vbuf);
            info_ptr->height = cast_ulong_to_size(info_ptr, ulval);
            break;
        case MINITIFF_TAG_BITS_PER_SAMPLE:
            if (count == 1)
            {
                ulval = get_ulong_value(&getter, tag_type, vbuf);
                info_ptr->bits_per_sample =
                    cast_ulong_to_uint(info_ptr, ulval);
            }
            else
            {
                bits_per_sample_count = count;
                bits_per_sample_tag_type = tag_type;
                bits_per_sample_offset = (long)getter.get_ulong(vbuf);
            }
            break;
        case MINITIFF_TAG_COMPRESSION:
            if (count != 1)
                goto err_unsupported;
            ulval = get_ulong_value(&getter, tag_type, vbuf);
            info_ptr->compression = cast_ulong_to_uint(info_ptr, ulval);
            break;
        case MINITIFF_TAG_PHOTOMETRIC:
            if (count != 1)
                goto err_unsupported;
            ulval = get_ulong_value(&getter, tag_type, vbuf);
            info_ptr->photometric = cast_ulong_to_uint(info_ptr, ulval);
            break;
        case MINITIFF_TAG_STRIP_OFFSETS:
            info_ptr->strip_offsets_count = count;
            if (count == 1)
            {
                if (info_ptr->strip_offsets != NULL)
                    goto err_invalid;
                info_ptr->strip_offsets = alloc_ulong_array(info_ptr, 1);
                info_ptr->strip_offsets[0] =
                    get_ulong_value(&getter, tag_type, vbuf);
            }
            else
            {
                strip_offsets_tag_type = tag_type;
                strip_offsets_offset = (long)getter.get_ulong(vbuf);
            }
            break;
        case MINITIFF_TAG_ORIENTATION:
            if (count != 1)
                goto err_unsupported;
            ulval = get_ulong_value(&getter, tag_type, vbuf);
            info_ptr->orientation = cast_ulong_to_uint(info_ptr, ulval);
            break;
        case MINITIFF_TAG_SAMPLES_PER_PIXEL:
            if (count != 1)
                goto err_unsupported;
            ulval = get_ulong_value(&getter, tag_type, vbuf);
            info_ptr->samples_per_pixel = cast_ulong_to_uint(info_ptr, ulval);
            break;
        case MINITIFF_TAG_ROWS_PER_STRIP:
            if (count != 1)
                goto err_unsupported;
            ulval = get_ulong_value(&getter, tag_type, vbuf);
            info_ptr->rows_per_strip = cast_ulong_to_size(info_ptr, ulval);
            break;
        case MINITIFF_TAG_STRIP_BYTE_COUNTS:
            /* ignored for uncompressed images */
            break;
        case MINITIFF_TAG_PLANAR_CONFIGURATION:
        case MINITIFF_TAG_PREDICTOR:
            if (count != 1 || get_ulong_value(&getter, tag_type, vbuf) != 1)
                goto err_unsupported;
            break;
        case MINITIFF_TAG_XMP:
        case MINITIFF_TAG_IPTC:
        case MINITIFF_TAG_EXIF_IFD:
        case MINITIFF_TAG_ICC_PROFILE:
        case MINITIFF_TAG_GPS_IFD:
        case MINITIFF_TAG_INTEROPERABILITY_IFD:
        case MINITIFF_TAG_PRINT_IM:
            if (!unknown_metadata_found)
            {
                unknown_metadata_found = 1;
                minitiff_warning(info_ptr, msg_warn_metadata);
            }
            break;
        }
    }

    /* Is this the last TIFF directory? */
    if (fread(buf, 4, 1, stream) != 1)
        goto err_read;
    if (getter.get_ulong(buf) != 0)
        minitiff_warning(info_ptr, msg_warn_multiple);

    /* Finish up the incomplete readings. */
    if (bits_per_sample_offset != 0)
    {
        count = bits_per_sample_count;
        if (count != info_ptr->samples_per_pixel)
            goto err_invalid;
        if (count > 4)
            goto err_unsupported;
        seek_to_offset(info_ptr, bits_per_sample_offset, stream);
        if (read_ulong_values(&getter, bits_per_sample_tag_type,
                              ulvals, count, stream) != count)
            goto err_read;
        while (--count > 0)
            if (ulvals[0] != ulvals[count])
                goto err_unsupported;
        info_ptr->bits_per_sample = cast_ulong_to_uint(info_ptr, ulvals[0]);
    }
    if (strip_offsets_offset != 0)
    {
        count = info_ptr->strip_offsets_count;
        if (count == 0 || count > info_ptr->height)
            goto err_invalid;
        if (info_ptr->strip_offsets != NULL)
            goto err_invalid;
        info_ptr->strip_offsets = alloc_ulong_array(info_ptr, count);
        seek_to_offset(info_ptr, strip_offsets_offset, stream);
        if (read_ulong_values(&getter, strip_offsets_tag_type,
                              info_ptr->strip_offsets, count,
                              stream) != count)
            goto err_read;
    }

    /* Return successfully. */
    return;

    /* Quick and dirty goto labels. */
err_read:
    minitiff_error(info_ptr, msg_err_read);
err_invalid:
    minitiff_error(info_ptr, msg_err_invalid);
err_unsupported:
    minitiff_error(info_ptr, msg_err_unsupported);
}

/*
 * TIFF row reader.
 */
void minitiff_read_row(struct minitiff_info *info_ptr,
                       unsigned char *row_ptr, size_t row_index, FILE *stream)
{
    size_t row_size, strip_index;
    unsigned int bytes_per_sample, sample_max;
    long offset;
    size_t i;

    /* Do not do validation here. */
    /* Call minitiff_validate_info() before calling this function. */

    bytes_per_sample = (info_ptr->bits_per_sample + 7) / 8;
    row_size =
        info_ptr->width * info_ptr->samples_per_pixel * bytes_per_sample;

    /* Position the file pointer to the beginning of the row,
     * if that has not been done already.
     */
    strip_index = row_index / info_ptr->rows_per_strip;
    if (strip_index >= info_ptr->strip_offsets_count)
        goto err_invalid;
    if ((long)info_ptr->strip_offsets[strip_index] < 0)
        goto err_range;
    offset = (long)(info_ptr->strip_offsets[strip_index] +
                    row_size * (row_index % info_ptr->rows_per_strip));
    seek_to_offset(info_ptr, offset, stream);

    /* Read the row, and do all the necessary adjustments. */
    if (fread(row_ptr, row_size, 1, stream) != 1)
        goto err_read;
    if (info_ptr->photometric == 0)
    {
        /* White is zero. */
        if (bytes_per_sample > 1)
            goto err_unsupported;
        sample_max = (1 << info_ptr->bits_per_sample) - 1;
        for (i = 0; i < row_size; ++i)
            row_ptr[i] = (unsigned char)(sample_max - row_ptr[i]);
    }

    /* Return successfully. */
    return;

    /* Quick and dirty goto labels. */
err_read:
    minitiff_error(info_ptr, msg_err_read);
err_invalid:
    minitiff_error(info_ptr, msg_err_invalid);
err_range:
    minitiff_error(info_ptr, msg_err_range);
err_unsupported:
    minitiff_error(info_ptr, msg_err_unsupported);
}
