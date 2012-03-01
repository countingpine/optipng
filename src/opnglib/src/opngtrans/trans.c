/*
 * opngtrans/trans.c
 * Image transformations.
 *
 * Copyright (C) 2011 Cosmin Truta.
 *
 * This software is distributed under the zlib license.
 * Please see the accompanying LICENSE file.
 */

#include "opnglib/opngtrans.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define OPNGLIB_INTERNAL
#include "trans.h"
#include "parser.h"
#include "../opngcore/codec.h"


/*
 * Creates a transformer object.
 */
opng_transformer_t *
opng_create_transformer(void)
{
    opng_transformer_t *result;

    result = (opng_transformer_t *)calloc(1, sizeof(struct opng_transformer));
    if (result == NULL)
        return NULL;

    opng_sigs_init(&result->strip_sigs);
    opng_sigs_init(&result->protect_sigs);
    return result;
}

/*
 * This is a helper used by opng_transform_set_object.
 */
static void
opng_transform_set_precision(int *precision_ptr, const char *value_str,
                             const char **err_message_ptr)
{
    int precision;

    precision = opng_string_to_smallint(value_str);
    if (precision <= 0)
    {
        *err_message_ptr = "Incorrect precision";
        return;
    }
    if (*precision_ptr == 0)
    {
        *precision_ptr = precision;
        return;
    }
    if (*precision_ptr != precision)
    {
        *err_message_ptr = "Inconsistent precision setting";
        return;
    }
}

/*
 * Specifies an object to be set by the given transformer.
 */
int
opng_transform_set_object(opng_transformer_t *transformer,
                          const char *object_name_eq_value,
                          size_t *err_objname_offset_ptr,
                          size_t *err_objname_length_ptr,
                          const char **err_message_ptr)
{
    int result;
    opng_id_t id;
    const char *value;
    size_t value_offset;
    struct opng_parse_err_info err_info;

    result = opng_parse_object_value(&id,
                                     &value_offset,
                                     object_name_eq_value,
                                     OPNG_CAN_SET_IDS,
                                     &err_info);
    *err_objname_offset_ptr = err_info.objname_offset;
    *err_objname_length_ptr = err_info.objname_length;
    *err_message_ptr = NULL;

    if (result < 0)
    {
        switch (err_info.id)
        {
        case OPNG_ID_CHUNK_META:
            *err_message_ptr = "Setting metadata is not implemented";
            break;
        default:
            *err_message_ptr = opng_id_to_strerr(err_info.id);
            if (*err_message_ptr == NULL && *err_objname_length_ptr != 0)
                *err_message_ptr = "Can't set this object";
        }
        return -1;
    }

    /* Get the precision value. */
    value = object_name_eq_value + value_offset;
    switch (id)
    {
    case OPNG_ID_IMAGE_GRAY_PRECISION:
        *err_message_ptr =
            "Set image.rgb.precision to control grayscale samples";
        return -1;
    case OPNG_ID_IMAGE_PRECISION:
        opng_transform_set_precision(&transformer->precision,
                                     value, err_message_ptr);
        break;
    case OPNG_ID_IMAGE_RED_PRECISION:
        opng_transform_set_precision(&transformer->red_precision,
                                     value, err_message_ptr);
        break;
    case OPNG_ID_IMAGE_GREEN_PRECISION:
        opng_transform_set_precision(&transformer->green_precision,
                                     value, err_message_ptr);
        break;
    case OPNG_ID_IMAGE_BLUE_PRECISION:
        opng_transform_set_precision(&transformer->blue_precision,
                                     value, err_message_ptr);
        break;
    case OPNG_ID_IMAGE_RGB_PRECISION:
        opng_transform_set_precision(&transformer->red_precision,
                                     value, err_message_ptr);
        opng_transform_set_precision(&transformer->green_precision,
                                     value, err_message_ptr);
        opng_transform_set_precision(&transformer->blue_precision,
                                     value, err_message_ptr);
        break;
    case OPNG_ID_IMAGE_ALPHA_PRECISION:
        opng_transform_set_precision(&transformer->alpha_precision,
                                     value, err_message_ptr);
        break;
    default:
        return -1;
    }
    return (*err_message_ptr == NULL) ? 0 : -1;
}

/*
 * Specifies a list of objects to be reset by the given transformer.
 */
int
opng_transform_reset_objects(opng_transformer_t *transformer,
                             const char *object_names,
                             size_t *err_objname_offset_ptr,
                             size_t *err_objname_length_ptr,
                             const char **err_message_ptr)
{
    int result;
    opng_id_t reset_chroma_ids;
    struct opng_parse_err_info err_info;

    result = opng_parse_objects(&transformer->reset_ids,
                                NULL,
                                object_names,
                                OPNG_CAN_RESET_IDS,
                                &err_info);
    *err_objname_offset_ptr = err_info.objname_offset;
    *err_objname_length_ptr = err_info.objname_length;
    *err_message_ptr = NULL;

    if (result < 0)
    {
        /*
         * The option -reset was used incorrectly.
         * Issue an error message as helpful as possible.
         */
        switch (err_info.id)
        {
        case OPNG_ID_ALL:
            *err_message_ptr = "Can't reset this object; can only strip it";
            break;
        case OPNG_ID_CHUNK_IMAGE:
        case OPNG_ID_CHUNK_META:
            *err_message_ptr = "Can't reset individual chunks";
            break;
        default:
            *err_message_ptr = opng_id_to_strerr(err_info.id);
            if (*err_message_ptr == NULL && *err_objname_length_ptr != 0)
                *err_message_ptr = "Can't reset this object";
        }
        return -1;
    }

    /* Check the mutually-exclusive colorspaces. */
    reset_chroma_ids =
        transformer->reset_ids &
            (OPNG_ID_IMAGE_CHROMA_BT601 | OPNG_ID_IMAGE_CHROMA_BT709);
    /* Use Wegner's formula. */
    if ((reset_chroma_ids & (reset_chroma_ids - 1)) != 0)
    {
        *err_message_ptr =
            "image.chroma.bt601 and image.chroma.bt709 are mutually exclusive";
        return -1;
    }
    /* TODO: Add more checks. */
    return 0;
}

/*
 * Specifies a list of objects to be stripped by the given transformer.
 */
int
opng_transform_strip_objects(opng_transformer_t *transformer,
                             const char *object_names,
                             size_t *err_objname_offset_ptr,
                             size_t *err_objname_length_ptr,
                             const char **err_message_ptr)
{
    int result;
    const char *chunk_name;
    struct opng_parse_err_info err_info;

    result = opng_parse_objects(&transformer->strip_ids,
                                &transformer->strip_sigs,
                                object_names,
                                OPNG_CAN_STRIP_IDS,
                                &err_info);
    *err_objname_offset_ptr = err_info.objname_offset;
    *err_objname_length_ptr = err_info.objname_length;
    *err_message_ptr = NULL;

    if (result < 0)
    {
        /*
         * The option -strip was used incorrectly.
         * Issue an error message as helpful as possible.
         */
        if (err_info.id == OPNG_ID_CHUNK_IMAGE)
        {
            /* Tried to (but shouldn't) strip image data? */
            chunk_name = object_names + *err_objname_offset_ptr;
            if (strncmp(chunk_name, "tRNS", 4) == 0)
                *err_message_ptr = "Can't strip tRNS; can only reset image.alpha";
            else
                *err_message_ptr = "Can't strip critical chunks";
        }
        else if ((err_info.id & OPNG_CAN_RESET_IDS) != 0)
        {
            /* The option -strip was used instead of -reset? */
            *err_message_ptr = "Can't strip this object; can only reset it";
        }
        else
        {
            *err_message_ptr = opng_id_to_strerr(err_info.id);
            if (*err_message_ptr == NULL && *err_objname_length_ptr != 0)
                *err_message_ptr = "Can't strip this object";
        }
        return -1;
    }
    return 0;
}

/*
 * Specifies a list of objects to be protected (i.e. not stripped) by
 * the given transformer.
 */
int
opng_transform_protect_objects(opng_transformer_t *transformer,
                               const char *object_names,
                               size_t *err_objname_offset_ptr,
                               size_t *err_objname_length_ptr,
                               const char **err_message_ptr)
{
    int result;
    struct opng_parse_err_info err_info;

    result = opng_parse_objects(&transformer->protect_ids,
                                &transformer->protect_sigs,
                                object_names,
                                OPNG_CAN_PROTECT_IDS,
                                &err_info);
    *err_objname_offset_ptr = err_info.objname_offset;
    *err_objname_length_ptr = err_info.objname_length;
    *err_message_ptr = NULL;

    if (result < 0)
    {
        *err_message_ptr = opng_id_to_strerr(err_info.id);
        if (*err_message_ptr == NULL && *err_objname_length_ptr != 0)
            *err_message_ptr = "Can't protect this object";
        return -1;
    }
    return 0;
}

/*
 * Returns 1 if the given chunk ought to be stripped, or 0 otherwise.
 */
int
opng_transform_query_strip_chunk(const opng_transformer_t *transformer,
                                 png_byte *chunk_sig)
{
    opng_id_t reset_ids, strip_ids, protect_ids;

    reset_ids = transformer->reset_ids;
    strip_ids = transformer->strip_ids;
    protect_ids = transformer->protect_ids;

    /* APNG falls outside of the PNG rules. Handle it first. */
    if ((reset_ids & OPNG_ID_ANIMATION) != 0)
    {
        if (opng_is_apng_chunk(chunk_sig))
            return 1;
    }

    /* Rule out the conditions when the chunk is not stripped. */
    if ((strip_ids & (OPNG_ID_ALL | OPNG_ID_CHUNK_META)) == 0)
    {
        /* Nothing is stripped. */
        return 0;
    }
    if ((protect_ids & OPNG_ID_ALL) != 0)
    {
        /* Everything is protected. */
        return 0;
    }
    if (opng_is_image_chunk(chunk_sig))
    {
        /* Image chunks (i.e. critical chunks and tRNS) are not stripped. */
        return 0;
    }
    if ((strip_ids & OPNG_ID_ALL) == 0 &&
        !opng_sigs_find(&transformer->strip_sigs, chunk_sig))
    {
        /* The chunk signature is not in the strip set. */
        return 0;
    }
    if (opng_sigs_find(&transformer->protect_sigs, chunk_sig))
    {
        /* The chunk signature is in the protect set. */
        return 0;
    }

    /* The chunk is stripped and not protected. */
    return 1;
}

/*
 * Retrieves the precision values to be set for each of the RGBA channels.
 */
void
opng_transform_query_set_precision(const opng_transformer_t *transformer,
                                   int *gray_precision_ptr,
                                   int *red_precision_ptr,
                                   int *green_precision_ptr,
                                   int *blue_precision_ptr,
                                   int *alpha_precision_ptr)
{
    int precision, default_precision;

    default_precision = transformer->precision;
    if (gray_precision_ptr != NULL)
    {
        /* image.gray.precision = max(image.red.precision,
         *                            image.green.precision,
         *                            image.blue.precision)
         */
        precision = transformer->red_precision;
        if (precision < transformer->green_precision)
            precision = transformer->green_precision;
        if (precision < transformer->blue_precision)
            precision = transformer->blue_precision;
        if (precision > 0)
            *gray_precision_ptr = precision;
        else
            *gray_precision_ptr = default_precision;
    }
    if (red_precision_ptr != NULL)
    {
        if (transformer->red_precision > 0)
            *red_precision_ptr = transformer->red_precision;
        else
            *red_precision_ptr = default_precision;
    }
    if (green_precision_ptr != NULL)
    {
        if (transformer->green_precision > 0)
            *green_precision_ptr = transformer->green_precision;
        else
            *green_precision_ptr = default_precision;
    }
    if (blue_precision_ptr != NULL)
    {
        if (transformer->blue_precision > 0)
            *blue_precision_ptr = transformer->blue_precision;
        else
            *blue_precision_ptr = default_precision;
    }
    if (alpha_precision_ptr != NULL)
    {
        if (transformer->alpha_precision > 0)
            *alpha_precision_ptr = transformer->alpha_precision;
        else
            *alpha_precision_ptr = default_precision;
    }
}

/*
 * Seals a transformer object.
 */
const opng_transformer_t *
opng_seal_transformer(opng_transformer_t *transformer)
{
    opng_sigs_sort_uniq(&transformer->strip_sigs);
    opng_sigs_sort_uniq(&transformer->protect_sigs);
    return transformer;
}

/*
 * Destroys a transformer object.
 */
void
opng_destroy_transformer(opng_transformer_t *transformer)
{
    if (transformer == NULL)
        return;

    opng_sigs_clear(&transformer->strip_sigs);
    opng_sigs_clear(&transformer->protect_sigs);
    free(transformer);
}
