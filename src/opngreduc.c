/*
 * opngreduc.c - libpng extension: image reductions
 *
 * Copyright (C) 2003-2006 Cosmin Truta.
 * The code is distributed under the same licensing and warranty terms
 * as libpng.
 *
 * CAUTION:
 * This code is intended to be integrated into libpng.  It is written
 * as if it were a part of libpng, which means that it accesses the
 * internal libpng structures directly.
 * IT IS IMPORTANT TO LINK THIS CODE STATICALLY to libpng, in order to
 * avoid possible data corruption problems that may occur when linking
 * to a dynamic libpng build, due to a possible png_struct mismatch.
 *
 * When integrating this code into libpng, it is recommended to rename
 * this source file to "pngreduc.c", and to rename the var/func/macro
 * names as follows:
 *   opng_xxx -> png_xxx
 *   OPNG_XXX -> PNG_XXX
 *
 * For simplicity, this code does not function if the pixels are stored
 * in a non-implicit form, i.e. if any PNG_TRANSFORM is in effect.
 *
 * For more info, see opng.h.
 */

#include "opng.h"


#ifdef OPNG_IMAGE_REDUCTIONS_SUPPORTED


#ifndef PNG_INFO_IMAGE_SUPPORTED
__error__ "OPNG_IMAGE_REDUCTIONS_SUPPORTED" requires "PNG_INFO_IMAGE_SUPPORTED"
#endif

#if !defined(PNG_bKGD_SUPPORTED) || !defined(PNG_hIST_SUPPORTED) || \
    !defined(PNG_sBIT_SUPPORTED) || !defined(PNG_tRNS_SUPPORTED)
__error__ "OPNG_IMAGE_REDUCTIONS_SUPPORTED" requires support for bKGD,hIST,sBIT,tRNS
#endif


/*
 * Indicate whether the image information is valid.
 * (The image information is valid if the critical information
 * is present in the png structures.)
 * The function returns 1 if this info is valid, and 0 otherwise.
 * If there is some inconsistency in the internal structures
 * (due to incorrect libpng use, or to libpng bugs), the function
 * issues a png_error.
 */
int PNGAPI
opng_validate_image(png_structp png_ptr, png_infop info_ptr)
{
   int result = 1, error = 0;

   png_debug(1, "in opng_reduce_image_type\n");

   if (png_ptr == NULL || info_ptr == NULL)
      return 0;

   /* Validate IHDR. */
   /* We cannot use png_get_IHDR() because it issues an error. */
   if (info_ptr->width > 0 && info_ptr->height > 0)
   {
      if (info_ptr->width > PNG_MAX_UINT || info_ptr->height > PNG_MAX_UINT ||
          info_ptr->bit_depth == 0 || info_ptr->bit_depth > 16 ||
          info_ptr->interlace_type >= PNG_INTERLACE_LAST)
          /* need more checks... */
         error = 1;
   }
   else
      result = 0;

   /* Validate PLTE. */
   if (info_ptr->color_type & PNG_COLOR_MASK_PALETTE)
   {
      if (info_ptr->valid & PNG_INFO_PLTE)
      {
         if (info_ptr->palette == NULL || info_ptr->num_palette == 0)
            error = 1;
      }
      else
         result = 0;
   }

   /* Validate IDAT. */
   if (info_ptr->valid & PNG_INFO_IDAT)
   {
      if (info_ptr->row_pointers == NULL)
         error = 1;
   }
   else
      result = 0;

   if (error)
      png_error(png_ptr, "Inconsistent data in libpng");
   return result;
}


#define OPNG_CMP_COLOR(R1, G1, B1, R2, G2, B2) \
   (((int)(R1) != (int)(R2)) ?      \
      ((int)(R1) - (int)(R2)) :     \
      (((int)(G1) != (int)(G2)) ?   \
         ((int)(G1) - (int)(G2)) :  \
         ((int)(B1) - (int)(B2))))

#define OPNG_CMP_ALPHA_COLOR(A1, R1, G1, B1, A2, R2, G2, B2) \
   (((int)(A1) != (int)(A2)) ?          \
      ((int)(A1) - (int)(A2)) :         \
      (((int)(R1) != (R2)) ?            \
         ((int)(R1) - (int)(R2)) :      \
         (((int)(G1) != (int)(G2)) ?    \
            ((int)(G1) - (int)(G2)) :   \
            ((int)(B1) - (int)(B2)))))


/*
 * Build a color+alpha palette in which the entries are sorted by
 * (alpha, red, green, blue) in this particular order.
 * Use the insertion sort algorithm.
 * The function returns:
 *   1 if the insertion was successful;     *index = position of new entry;
 *   0 if the insertion was not successful; *index = position of crt entry;
 *  -1 if overflow;                *num_palette = *num_trans = *index = -1;
 */
static int /* PRIVATE */
opng_insert_palette_entry(png_colorp palette, int *num_palette,
   png_bytep trans, int *num_trans, int max_tuples,
   unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha,
   int *index)
{
   int low, high, mid, cmp, i;

   OPNG_ASSERT(*num_palette >= 0 && *num_palette <= max_tuples);
   OPNG_ASSERT(*num_trans >= 0 && *num_trans <= *num_palette);

   /* (Binary) Search for the tuple. */
   if (alpha < 255)
   {
      low  = 0;
      high = *num_trans - 1;
      while (low <= high)
      {
         mid = (low + high) / 2;
         cmp = OPNG_CMP_ALPHA_COLOR(alpha, red, green, blue,
            trans[mid],
            palette[mid].red, palette[mid].green, palette[mid].blue);
         if (cmp < 0)
            high = mid - 1;
         else if (cmp > 0)
            low = mid + 1;
         else /* cmp == 0 */
         {
            *index = mid;
            return 0;
         }
      }
   }
   else  /* increase speed: search only within fully opaque tuples. */
   {
      low  = *num_trans;
      high = *num_palette - 1;
      while (low <= high)
      {
         mid = (low + high) / 2;
         cmp = OPNG_CMP_COLOR(red, green, blue,
            palette[mid].red, palette[mid].green, palette[mid].blue);
         if (cmp < 0)
            high = mid - 1;
         else if (cmp > 0)
            low = mid + 1;
         else /* cmp == 0 */
         {
            *index = mid;
            return 0;
         }
      }
   }

   /* Check for overflow. */
   if (*num_palette + 1 == max_tuples)
   {
       *num_palette = *num_trans = *index = -1;
       return -1;
   }

   /* Insert new tuple at [low]. */
   OPNG_ASSERT(low >= 0 && low <= *num_palette);
   for (i = *num_palette; i > low; --i)
      palette[i] = palette[i - 1];
   palette[low].red   = (png_byte)red;
   palette[low].green = (png_byte)green;
   palette[low].blue  = (png_byte)blue;
   ++(*num_palette);
   if (alpha < 255)
   {
      OPNG_ASSERT(low <= *num_trans);
      for (i = *num_trans; i > low; --i)
         trans[i] = trans[i - 1];
      trans[low] = (png_byte)alpha;
      ++(*num_trans);
   }
   return 1;
}


/*
 * Retrieve the alpha samples from the given image row.
 */
static void /* PRIVATE */
opng_get_alpha_row(png_structp png_ptr, png_infop info_ptr,
   png_bytep row, png_bytep alpha_row)
{
   png_bytep sample_ptr;
   png_uint_32 width, i;
   unsigned int channels;
   png_color_16 *trans_values;

   OPNG_ASSERT(info_ptr->bit_depth == 8);
   OPNG_ASSERT(!(info_ptr->color_type & PNG_COLOR_MASK_PALETTE));

   width = info_ptr->width;
   if (!(info_ptr->color_type & PNG_COLOR_MASK_ALPHA))
   {
      if (!(info_ptr->valid & PNG_INFO_tRNS))
      {
         memset(alpha_row, 255, (size_t)width);
         return;
      }
      trans_values = &info_ptr->trans_values;
      if (info_ptr->color_type == PNG_COLOR_TYPE_RGB)
      {
         png_byte trans_red   = (png_byte)trans_values->red;
         png_byte trans_green = (png_byte)trans_values->green;
         png_byte trans_blue  = (png_byte)trans_values->blue;
         for (i = 0; i < width; ++i)
            alpha_row[i] = (png_byte)
               ((row[3*i] == trans_red && row[3*i+1] == trans_green &&
                 row[3*i+2] == trans_blue) ? 0 : 255);
      }
      else
      {
         png_byte trans_gray = (png_byte)trans_values->gray;
         OPNG_ASSERT(info_ptr->color_type == PNG_COLOR_TYPE_GRAY);
         for (i = 0; i < width; ++i)
            alpha_row[i] = (png_byte)(row[i] == trans_gray ? 0 : 255);
      }
      return;
   }

   /* There is a real alpha channel. */
   channels = (png_ptr->usr_channels > 0) ?
      png_ptr->usr_channels : info_ptr->channels;
   sample_ptr = row;
   if (!(png_ptr->transformations & PNG_FILLER) ||
        (png_ptr->flags & PNG_FLAG_FILLER_AFTER))
      sample_ptr += channels - 1;  /* alpha sample is the last in RGBA tuple */
   for (i = 0; i < width; ++i, sample_ptr += channels, ++alpha_row)
      *alpha_row = *sample_ptr;
}


/*
 * Analyze the redundancy of bits inside the image.
 * The parameter reductions indicates the intended reductions.
 * The function returns the possible reductions.
 */
png_uint_32 /* PRIVATE */
opng_analyze_bits(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 reductions)
{
   png_bytepp row_ptr;
   png_bytep component_ptr;
   png_uint_32 height, width, i, j;
   unsigned int bit_depth, byte_depth, color_type, channels, sample_size,
      offset_color, offset_alpha;

   png_debug(1, "in opng_analyze_bits\n");

   bit_depth = info_ptr->bit_depth;
   if (bit_depth < 8)
      return OPNG_REDUCE_NONE;  /* nothing is done in this case */

   color_type = info_ptr->color_type;
   if (color_type & PNG_COLOR_MASK_PALETTE)
      return OPNG_REDUCE_NONE;  /* let opng_reduce_palette() handle it */

   byte_depth  = bit_depth / 8;
   channels    = (png_ptr->usr_channels > 0) ?
      png_ptr->usr_channels : info_ptr->channels;
   sample_size = channels * byte_depth;

   /* The following reductions are not implemented yet, or do not apply. */
   reductions &= ~(OPNG_REDUCE_8_TO_4_2_1 |
      OPNG_REDUCE_RGB_TO_PALETTE | OPNG_REDUCE_PALETTE_TO_GRAY |
      OPNG_REDUCE_PALETTE);

   if (bit_depth <= 8)
      reductions &= ~OPNG_REDUCE_16_TO_8;
   if (!(color_type & PNG_COLOR_MASK_COLOR))
      reductions &= ~OPNG_REDUCE_RGB_TO_GRAY;
   if (!(color_type & PNG_COLOR_MASK_ALPHA))
      reductions &= ~OPNG_REDUCE_STRIP_ALPHA;

   offset_color = offset_alpha = 0;
   if ((png_ptr->transformations & PNG_FILLER) &&
       !(png_ptr->flags & PNG_FLAG_FILLER_AFTER))
      offset_color = byte_depth;
   else
      offset_alpha = (channels - 1) * byte_depth;

   /* Test if the ancillary chunk info allows these reductions. */
   if ((reductions & OPNG_REDUCE_RGB_TO_GRAY) &&
       (info_ptr->valid & PNG_INFO_bKGD))
   {
      png_color_16p background = &info_ptr->background;
      if (background->red != background->green ||
          background->red != background->blue)
         reductions &= ~OPNG_REDUCE_RGB_TO_GRAY;
   }

   /* Test for each reduction, row by row. */
   row_ptr = info_ptr->row_pointers;
   height  = info_ptr->height;
   width   = info_ptr->width;
   for (i = 0; i < height; ++i, ++row_ptr)
   {
      if (reductions == OPNG_REDUCE_NONE)
         return OPNG_REDUCE_NONE;  /* no need to go any further */

      /* Test if it is possible to reduce the bit depth to 8. */
      if (reductions & OPNG_REDUCE_16_TO_8)
      {
         component_ptr = *row_ptr;
         for (j = 0; j < channels * width; ++j, component_ptr += 2)
         {
            if (component_ptr[0] != component_ptr[1])
            {
               reductions &= ~OPNG_REDUCE_16_TO_8;
               break;
            }
         }
      }

      if (bit_depth == 8)
      {
         /* Test if it is possible to reduce rgb -> gray. */
         if (reductions & OPNG_REDUCE_RGB_TO_GRAY)
         {
            component_ptr = *row_ptr + offset_color;
            for (j = 0; j < width; ++j, component_ptr += sample_size)
            {
               if (component_ptr[0] != component_ptr[1] ||
                   component_ptr[0] != component_ptr[2])
               {
                  reductions &= ~OPNG_REDUCE_RGB_TO_GRAY;
                  break;
               }
            }
         }

         /* Test if it is possible to strip the alpha channel. */
         if (reductions & OPNG_REDUCE_STRIP_ALPHA)
         {
            component_ptr = *row_ptr + offset_alpha;
            for (j = 0; j < width; ++j, component_ptr += sample_size)
            {
               if (component_ptr[0] != 255)
               {
                  reductions &= ~OPNG_REDUCE_STRIP_ALPHA;
                  break;
               }
            }
         }
      }
      else /* bit_depth == 16 */
      {
         /* Test if it is possible to reduce rgb -> gray. */
         if (reductions & OPNG_REDUCE_RGB_TO_GRAY)
         {
            component_ptr = *row_ptr + offset_color;
            for (j = 0; j < width; ++j, component_ptr += sample_size)
            {
               if (component_ptr[0] != component_ptr[2] ||
                   component_ptr[0] != component_ptr[4] ||
                   component_ptr[1] != component_ptr[3] ||
                   component_ptr[1] != component_ptr[5])
               {
                  reductions &= ~OPNG_REDUCE_RGB_TO_GRAY;
                  break;
               }
            }
         }

         /* Test if it is possible to strip the alpha channel. */
         if (reductions & OPNG_REDUCE_STRIP_ALPHA)
         {
            component_ptr = *row_ptr + offset_alpha;
            for (j = 0; j < width; ++j, component_ptr += sample_size)
            {
               if (component_ptr[0] != 255 || component_ptr[1] != 255)
               {
                  reductions &= ~OPNG_REDUCE_STRIP_ALPHA;
                  break;
               }
            }
         }
      } /* end if (bit_depth == 8) */
   } /* end for (i = 0; i < height; ++i, ++row_ptr) */

   return reductions;
}


/*
 * Reduce the image type to a lower bit depth and color type,
 * by removing redundant bits.
 * Possible reductions: 16bpp to 8bpp; RGB to gray; strip alpha.
 * The parameter reductions indicates the intended reductions.
 * The function returns the successful reductions.
 * All reductions are performed in a single step.
 */
png_uint_32 /* PRIVATE */
opng_reduce_bits(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 reductions)
{
   png_bytepp row_ptr;
   png_bytep src_ptr, dest_ptr;
   png_uint_32 height, width, i, j;
   unsigned int
      src_bit_depth, dest_bit_depth, src_byte_depth, dest_byte_depth,
      src_color_type, dest_color_type, src_channels, dest_channels,
      src_sample_size, dest_sample_size, src_offset_alpha;
   unsigned int tran_tbl[8], k;

   png_debug(1, "in opng_reduce_bits\n");

   /* See which reductions may be performed. */
   reductions = opng_analyze_bits(png_ptr, info_ptr, reductions);
   /* Strip the filler even if it is not an alpha channel. */
   if (png_ptr->transformations & PNG_FILLER)
      reductions |= OPNG_REDUCE_STRIP_ALPHA;
   if (reductions == OPNG_REDUCE_NONE)
      return OPNG_REDUCE_NONE;  /* nothing can be reduced */

   /* Compute the new image parameters bit_depth, color_type, etc. */
   src_bit_depth = info_ptr->bit_depth;
   OPNG_ASSERT(src_bit_depth >= 8);
   if (reductions & OPNG_REDUCE_16_TO_8)
   {
      OPNG_ASSERT(src_bit_depth == 16);
      dest_bit_depth = 8;
   }
   else
      dest_bit_depth = src_bit_depth;

   src_byte_depth = src_bit_depth / 8;
   dest_byte_depth = dest_bit_depth / 8;

   src_color_type = dest_color_type = info_ptr->color_type;
   if (reductions & OPNG_REDUCE_RGB_TO_GRAY)
   {
      OPNG_ASSERT(src_color_type & PNG_COLOR_MASK_COLOR);
      dest_color_type &= ~PNG_COLOR_MASK_COLOR;
   }
   if (reductions & OPNG_REDUCE_STRIP_ALPHA)
   {
      OPNG_ASSERT(src_color_type & PNG_COLOR_MASK_ALPHA);
      dest_color_type &= ~PNG_COLOR_MASK_ALPHA;
   }

   src_channels  = (png_ptr->usr_channels > 0) ?
      png_ptr->usr_channels : info_ptr->channels;
   dest_channels =
      ((dest_color_type & PNG_COLOR_MASK_COLOR) ? 3 : 1) +
      ((dest_color_type & PNG_COLOR_MASK_ALPHA) ? 1 : 0);

   src_sample_size  = src_channels * src_byte_depth;
   dest_sample_size = dest_channels * dest_byte_depth;
   OPNG_ASSERT(src_sample_size > dest_sample_size);

   if (!(png_ptr->transformations & PNG_FILLER) ||
       (png_ptr->flags & PNG_FLAG_FILLER_AFTER))
      src_offset_alpha = (src_channels - 1) * src_byte_depth;
   else
      src_offset_alpha = 0;

   /* Pre-compute the intra-sample translation table. */
   for (k = 0; k < 4 * dest_byte_depth; ++k)
      tran_tbl[k] = k * src_bit_depth / dest_bit_depth;
   /* If rgb -> gray and the alpha channel remains in the right,
      shift the alpha component two positions to the left. */
   if ((reductions & OPNG_REDUCE_RGB_TO_GRAY) &&
       (dest_color_type & PNG_COLOR_MASK_ALPHA) &&
       (src_offset_alpha != 0))
   {
      tran_tbl[dest_byte_depth] = tran_tbl[3 * dest_byte_depth];
      if (dest_byte_depth == 2)
         tran_tbl[dest_byte_depth + 1] = tran_tbl[3 * dest_byte_depth + 1];
   }
   /* If alpha is in the left, and it is being stripped,
      shift the components that come after it. */
   if ((src_channels == 2 || src_channels == 4) /* alpha or filler */ &&
       !(dest_color_type & PNG_COLOR_MASK_ALPHA) &&
       (src_offset_alpha == 0))
   {
      for (k = 0; k < dest_sample_size; )
      {
         if (dest_byte_depth == 1)
         {
            tran_tbl[k] = tran_tbl[k + 1];
            ++k;
         }
         else
         {
            tran_tbl[k] = tran_tbl[k + 2];
            tran_tbl[k + 1] = tran_tbl[k + 3];
            k += 2;
         }
      }
   }

   /* Translate the samples to the new image type. */
   row_ptr = info_ptr->row_pointers;
   height  = info_ptr->height;
   width   = info_ptr->width;
   for (i = 0; i < height; ++i, ++row_ptr)
   {
      src_ptr = dest_ptr = *row_ptr;
      for (j = 0; j < width; ++j)
      {
         for (k = 0; k < dest_sample_size; ++k)
            dest_ptr[k] = src_ptr[tran_tbl[k]];
         src_ptr += src_sample_size;
         dest_ptr += dest_sample_size;
      }
   }

   /* Update the ancillary chunk info. */
   if (reductions & OPNG_REDUCE_RGB_TO_GRAY)
   {
      if (info_ptr->valid & PNG_INFO_bKGD)
         info_ptr->background.gray = info_ptr->background.red;
      if (info_ptr->valid & PNG_INFO_sBIT)
      {
         png_color_8p sig_bit_ptr = &info_ptr->sig_bit;
         png_byte max_sig_bit = sig_bit_ptr->red;
         if (max_sig_bit < sig_bit_ptr->green)
            max_sig_bit = sig_bit_ptr->green;
         if (max_sig_bit < sig_bit_ptr->blue)
            max_sig_bit = sig_bit_ptr->blue;
         png_ptr->sig_bit.gray = info_ptr->sig_bit.gray = max_sig_bit;
      }
      if (info_ptr->valid & PNG_INFO_tRNS)
      {
         png_color_16p trans_values = &info_ptr->trans_values;
         if (trans_values->red == trans_values->green ||
             trans_values->red == trans_values->blue)
            trans_values->gray = trans_values->red;
         else
         {
            /* A non-gray tRNS entry is useless in a grayscale image. */
            png_free_data(png_ptr, info_ptr, PNG_FREE_TRNS, -1);
            info_ptr->valid &= ~PNG_INFO_tRNS;
         }
      }
   }

   /* Update the image info. */
   png_ptr->bit_depth   = info_ptr->bit_depth   = (png_byte)dest_bit_depth;
   png_ptr->color_type  = info_ptr->color_type  = (png_byte)dest_color_type;
   png_ptr->channels    = info_ptr->channels    = (png_byte)dest_channels;
   png_ptr->pixel_depth = info_ptr->pixel_depth =
      (png_byte)(dest_bit_depth * dest_channels);
   if (reductions & OPNG_REDUCE_STRIP_ALPHA)
   {
      png_ptr->transformations &= ~PNG_FILLER;
      if (png_ptr->usr_channels > 0)
         --png_ptr->usr_channels;
   }

   return reductions;
}


/*
 * Reduce the bit depth of a palette image to the lowest possible value.
 * The parameter reductions should contain OPNG_REDUCE_8_TO_4_2_1.
 * The function returns OPNG_REDUCE_8_TO_4_2_1 if successful.
 */
png_uint_32 /* PRIVATE */
opng_reduce_palette_bits(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 reductions)
{
   png_bytepp row_ptr;
   png_bytep src_sample_ptr, dest_sample_ptr;
   png_uint_32 width, height, i, j;
   unsigned int src_bit_depth, dest_bit_depth;
   unsigned int src_mask_init, src_mask, src_shift, dest_shift;
   unsigned int sample, dest_buf;

   png_debug(1, "in opng_reduce_palette_bits\n");

   /* Check if the reduction applies. */
   if (!(reductions & OPNG_REDUCE_8_TO_4_2_1) ||
       (info_ptr->color_type != PNG_COLOR_TYPE_PALETTE) ||
       (info_ptr->num_palette > 16))
      return OPNG_REDUCE_NONE;

   row_ptr = info_ptr->row_pointers;
   height  = info_ptr->height;
   width   = info_ptr->width;
   if (png_ptr->usr_bit_depth > 0)
      src_bit_depth = png_ptr->usr_bit_depth;
   else
      src_bit_depth = info_ptr->bit_depth;

   /* Find the smallest bit depth. */
   OPNG_ASSERT(info_ptr->num_palette > 0);
   if (info_ptr->num_palette <= 2)
      dest_bit_depth = 1;
   else if (info_ptr->num_palette <= 4)
      dest_bit_depth = 2;
   else if (info_ptr->num_palette <= 16)
      dest_bit_depth = 4;
   else
      dest_bit_depth = 8;
   if (dest_bit_depth >= src_bit_depth)
      return OPNG_REDUCE_NONE;

   /* Iterate through all sample values. */
   if (src_bit_depth == 8)
   {
      for (i = 0; i < height; ++i, ++row_ptr)
      {
         src_sample_ptr = dest_sample_ptr = *row_ptr;
         dest_shift = 8;
         dest_buf   = 0;
         for (j = 0; j < width; ++j)
         {
            dest_shift -= dest_bit_depth;
            if (dest_shift > 0)
               dest_buf |= *src_sample_ptr << dest_shift;
            else
            {
               *dest_sample_ptr++ = (png_byte)(dest_buf | *src_sample_ptr);
               dest_shift = 8;
               dest_buf   = 0;
            }
            ++src_sample_ptr;
         }
         if (dest_shift != 0)
            *dest_sample_ptr = (png_byte)dest_buf;
      }
   }
   else  /* src_bit_depth < 8 */
   {
      src_mask_init = (1 << (8 + src_bit_depth)) - (1 << 8);
      for (i = 0; i < height; ++i, ++row_ptr)
      {
         src_sample_ptr = dest_sample_ptr = *row_ptr;
         src_shift = dest_shift = 8;
         src_mask  = src_mask_init;
         dest_buf  = 0;
         for (j = 0; j < width; ++j)
         {
            src_shift -= src_bit_depth;
            src_mask >>= src_bit_depth;
            sample = (*src_sample_ptr & src_mask) >> src_shift;
            dest_shift -= dest_bit_depth;
            if (dest_shift > 0)
               dest_buf |= sample << dest_shift;
            else
            {
               *dest_sample_ptr++ = (png_byte)(dest_buf | sample);
               dest_shift = 8;
               dest_buf   = 0;
            }
            if (src_shift == 0)
            {
               src_shift = 8;
               src_mask  = src_mask_init;
               ++src_sample_ptr;
            }
         }
         if (dest_shift != 0)
            *dest_sample_ptr = (png_byte)dest_buf;
      }
   }

   /* Update the image info. */
   png_ptr->bit_depth   = info_ptr->bit_depth   =
   png_ptr->pixel_depth = info_ptr->pixel_depth = (png_byte)dest_bit_depth;

   return OPNG_REDUCE_8_TO_4_2_1;
}


/*
 * Reduce the image type from RGB to palette, if possible.
 * The parameter reductions indicates the intended reductions.
 * The function returns the successful reductions.
 */
png_uint_32 /* PRIVATE */
opng_reduce_rgb_to_palette(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 reductions)
{
   png_uint_32 result;
   png_bytepp row_ptr;
   png_bytep sample_ptr, alpha_row;
   png_uint_32 height, width, i, j;
   png_color palette[256];
   png_byte trans[256];
   int num_palette, num_trans, index;
   unsigned int channels;
   unsigned int
      red, green, blue, alpha, prev_red, prev_green, prev_blue, prev_alpha;

   png_debug(1, "in opng_reduce_rgb_to_palette\n");

   /* Exit if the reductions do not apply. */
   if (!(reductions & OPNG_REDUCE_RGB_TO_PALETTE) ||
       (info_ptr->bit_depth != 8) ||
       (info_ptr->color_type & PNG_COLOR_MASK_PALETTE) ||
       !(info_ptr->color_type & PNG_COLOR_MASK_COLOR))
      return OPNG_REDUCE_NONE;

   row_ptr   = info_ptr->row_pointers;
   height    = info_ptr->height;
   width     = info_ptr->width;
   channels  = info_ptr->channels;
   alpha_row = (png_bytep)png_malloc(png_ptr, width);

   /* Analyze the possibility of this reduction. */
   num_palette = num_trans = 0;
   prev_red = prev_green = prev_blue = prev_alpha = (unsigned int)(-1);
   for (i = 0; i < height; ++i, ++row_ptr)
   {
      sample_ptr = *row_ptr;
      opng_get_alpha_row(png_ptr, info_ptr, *row_ptr, alpha_row);
      for (j = 0; j < width; ++j, sample_ptr += channels)
      {
         red   = sample_ptr[0];
         green = sample_ptr[1];
         blue  = sample_ptr[2];
         alpha = alpha_row[j];
         /* Check the cache first. */
         if (red != prev_red || green != prev_green || blue != prev_blue ||
             alpha != prev_alpha)
         {
            prev_red   = red;
            prev_green = green;
            prev_blue  = blue;
            prev_alpha = alpha;
            if (opng_insert_palette_entry(palette, &num_palette,
                trans, &num_trans, 256,
                red, green, blue, alpha, &index) < 0)  /* overflow */
            {
               OPNG_ASSERT(num_palette < 0);
               i = height;  /* forced exit from outer loop */
               break;
            }
         }
      }
   }

   /* Check if the uncompressed paletted image (pixels + PLTE + tRNS)
    * isn't bigger than the uncompressed RGB(A) image.
    * Chunk overhead is ignored.
    *
    * compare (pixels * channels) vs. (pixels + 3 * num_palette + num_trans)
    *   <=>
    * compare (pixels * (channels - 1)) vs. (3 * num_palette + num_trans)
    */
   if (num_palette >= 0)
   {
      OPNG_ASSERT(num_palette > 0 && num_palette <= 256);
      OPNG_ASSERT(num_trans >= 0 && num_trans <= num_palette);
      if (width <= 384 && height <= 384 && /* protect against arith overflow */
          (int)(width*height * (channels - 1)) <= 3 * num_palette + num_trans)
         num_palette = -1;
   }

   if (num_palette < 0)  /* can't reduce */
   {
      png_free(png_ptr, alpha_row);
      return OPNG_REDUCE_NONE;
   }

   /* Reduce. */
   row_ptr = info_ptr->row_pointers;
   index = -1;
   prev_red = prev_green = prev_blue = prev_alpha = (unsigned int)(-1);
   for (i = 0; i < height; ++i, ++row_ptr)
   {
      sample_ptr = *row_ptr;
      opng_get_alpha_row(png_ptr, info_ptr, *row_ptr, alpha_row);
      for (j = 0; j < width; ++j, sample_ptr += channels)
      {
         red   = sample_ptr[0];
         green = sample_ptr[1];
         blue  = sample_ptr[2];
         alpha = alpha_row[j];
         /* Check the cache first. */
         if (red != prev_red || green != prev_green || blue != prev_blue ||
             alpha != prev_alpha)
         {
            prev_red   = red;
            prev_green = green;
            prev_blue  = blue;
            prev_alpha = alpha;
            if (opng_insert_palette_entry(palette, &num_palette,
                trans, &num_trans, 256,
                red, green, blue, alpha, &index) != 0)
               index = -1;  /* this should never happen */
         }
         OPNG_ASSERT(index >= 0);
         (*row_ptr)[j] = (png_byte)index;
      }
   }

   /* Update the image info. */
   png_ptr->color_type  = info_ptr->color_type  = PNG_COLOR_TYPE_PALETTE;
   png_ptr->channels    = info_ptr->channels    = 1;
   png_ptr->pixel_depth = info_ptr->pixel_depth = 8;
   png_set_PLTE(png_ptr, info_ptr, palette, num_palette);
   if (num_trans > 0)
      png_set_tRNS(png_ptr, info_ptr, trans, num_trans, NULL);

   png_free(png_ptr, alpha_row);

   result = OPNG_REDUCE_RGB_TO_PALETTE;
   if (reductions & OPNG_REDUCE_8_TO_4_2_1)
      result |= opng_reduce_palette_bits(png_ptr, info_ptr, reductions);
   return result;
}


/*
 * Analyze the usage of samples.
 * The output value usage_map[n] indicates whether the sample n
 * is used. The usage_map[] array must have 256 entries.
 * The function requires a valid bit depth between 1 and 8.
 */
void /* PRIVATE */
opng_analyze_sample_usage(png_structp png_ptr, png_infop info_ptr,
   png_bytep usage_map)
{
   png_bytepp row_ptr;
   png_bytep sample_ptr;
   png_uint_32 width, height, i, j;
   unsigned int bit_depth, init_shift, init_mask, shift, mask;

   png_debug(1, "in opng_analyze_sample_usage\n");

   row_ptr = info_ptr->row_pointers;
   height  = info_ptr->height;
   width   = info_ptr->width;
   if (png_ptr->usr_bit_depth > 0)
      bit_depth = png_ptr->usr_bit_depth;
   else
      bit_depth = info_ptr->bit_depth;

   /* Initialize the output entries with 0. */
   memset(usage_map, 0, 256);

   /* Iterate through all sample values. */
   if (bit_depth == 8)
   {
      for (i = 0; i < height; ++i, ++row_ptr)
         for (j = 0, sample_ptr = *row_ptr; j < width; ++j, ++sample_ptr)
            usage_map[*sample_ptr] = 1;
   }
   else
   {
      OPNG_ASSERT(bit_depth < 8);
      init_shift = 8 - bit_depth;
      init_mask  = (1 << 8) - (1 << init_shift);
      for (i = 0; i < height; ++i, ++row_ptr)
         for (j = 0, sample_ptr = *row_ptr; j < width; ++sample_ptr)
         {
            mask  = init_mask;
            shift = init_shift;
            do
            {
               usage_map[(*sample_ptr & mask) >> shift] = 1;
               mask >>= bit_depth;
               shift -= bit_depth;
               ++j;
            } while (mask > 0 && j < width);
         }
   }

   /* bKGD also counts as a used sample. */
   if (info_ptr->valid & PNG_INFO_bKGD)
      usage_map[info_ptr->background.index] = 1;
}


/*
 * Reduce the palette (only the fast method is implemented).
 * The parameter reductions indicates the intended reductions.
 * The function returns the successful reductions.
 */
png_uint_32 /* PRIVATE */
opng_reduce_palette(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 reductions)
{
   png_uint_32 result;
   png_colorp palette;
   png_bytep trans;
   png_bytepp rows;
   png_uint_32 width, height, i, j;
   png_byte is_used[256];
   int num_palette, num_trans, last_color_index, last_trans_index, is_gray, k;
   png_color_16 gray_trans;
   png_byte crt_trans_value, last_trans_value;

   png_debug(1, "in opng_reduce_palette\n");

   /* Exit if the reductions do not apply. */
   if (!(reductions & (OPNG_REDUCE_PALETTE_TO_GRAY |
                       OPNG_REDUCE_PALETTE_FAST |
                       OPNG_REDUCE_8_TO_4_2_1))
       || info_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
      return OPNG_REDUCE_NONE;

   height      = info_ptr->height;
   width       = info_ptr->width;
   palette     = info_ptr->palette;
   num_palette = info_ptr->num_palette;
   rows        = info_ptr->row_pointers;
   if (info_ptr->valid & PNG_INFO_tRNS)
   {
      trans     = info_ptr->trans;
      num_trans = info_ptr->num_trans;
      OPNG_ASSERT(trans != NULL && num_trans > 0);
   }
   else
   {
      trans     = NULL;
      num_trans = 0;
   }

   /* Analyze the possible reductions. */
   /* Also check the integrity of PLTE and tRNS. */
   opng_analyze_sample_usage(png_ptr, info_ptr, is_used);
   /* Palette-to-gray does not work (yet) if the bit depth is below 8. */
   is_gray = (reductions & OPNG_REDUCE_PALETTE_TO_GRAY) &&
             (info_ptr->bit_depth == 8);
   last_color_index = last_trans_index = -1;
   for (k = 0; k < 256; ++k)
   {
      if (!is_used[k])
         continue;
      last_color_index = k;
      if (k < num_trans && trans[k] < 255)
         last_trans_index = k;
      if (is_gray)
         if (palette[k].red != palette[k].green ||
             palette[k].red != palette[k].blue)
            is_gray = 0;
   }
   OPNG_ASSERT(last_color_index >= 0);
   if (last_color_index >= num_palette)
   {
      png_warning(png_ptr, "Too few colors in palette");
      /* Fix the palette by adding blank entries at the end. */
      num_palette = last_color_index + 1;
      info_ptr->num_palette = (png_uint_16)num_palette;
   }
   if (num_trans > num_palette)
   {
      png_warning(png_ptr, "Too many alpha values in tRNS");
      info_ptr->num_trans = info_ptr->num_palette;
   }
   num_trans = last_trans_index + 1;
   OPNG_ASSERT(num_trans <= num_palette);

   /* Test if tRNS can be reduced to grayscale. */
   if (is_gray && num_trans > 0)
   {
      gray_trans.gray = palette[last_trans_index].red;
      last_trans_value = trans[last_trans_index];
      for (k = 0; k <= last_color_index; ++k)
      {
         if (!is_used[k])
            continue;
         if (k <= last_trans_index)
         {
            crt_trans_value = trans[k];
            /* Cannot reduce if different colors have transparency. */
            if (crt_trans_value < 255 && palette[k].red != gray_trans.gray)
            {
               is_gray = 0;
               break;
            }
         }
         else
            crt_trans_value = 255;
         /* Cannot reduce if same color has multiple transparency levels. */
         if (palette[k].red == gray_trans.gray &&
             crt_trans_value != last_trans_value)
         {
            is_gray = 0;
            break;
         }
      }
   }

   /* Initialize result value. */
   result = OPNG_REDUCE_NONE;

   /* Remove tRNS if possible. */
   if ((info_ptr->valid & PNG_INFO_tRNS) && num_trans == 0)
   {
      png_free_data(png_ptr, info_ptr, PNG_FREE_TRNS, -1);
      info_ptr->valid &= ~PNG_INFO_tRNS;
      result = OPNG_REDUCE_PALETTE_FAST;
   }

   if (reductions & OPNG_REDUCE_PALETTE_FAST)
   {
      if (num_palette != last_color_index + 1)
      {
         /* Reduce PLTE. */
         /* hIST is reduced automatically. */
         info_ptr->num_palette = (png_uint_16)(last_color_index + 1);
         result = OPNG_REDUCE_PALETTE_FAST;
      }

      if ((info_ptr->valid & PNG_INFO_tRNS) &&
          (int)info_ptr->num_trans != num_trans)
      {
         /* Reduce tRNS. */
         info_ptr->num_trans = (png_uint_16)num_trans;
         result = OPNG_REDUCE_PALETTE_FAST;
      }
   }

   if (reductions & OPNG_REDUCE_8_TO_4_2_1)
      result |= opng_reduce_palette_bits(png_ptr, info_ptr, reductions);
   if (info_ptr->bit_depth < 8 || !is_gray)
      return result;

   /* Reduce palette -> grayscale. */
   for (i = 0; i < height; ++i)
      for (j = 0; j < width; ++j)
         rows[i][j] = palette[rows[i][j]].red;

   /* Update the ancillary chunk info. */
   if (info_ptr->valid & PNG_INFO_bKGD)
      info_ptr->background.gray = palette[info_ptr->background.index].red;
   if (info_ptr->valid & PNG_INFO_hIST)
   {
      png_free_data(png_ptr, info_ptr, PNG_FREE_HIST, -1);
      info_ptr->valid &= ~PNG_INFO_hIST;
   }
   if (info_ptr->valid & PNG_INFO_sBIT)
   {
      png_color_8p sig_bit_ptr = &info_ptr->sig_bit;
      png_byte max_sig_bit = sig_bit_ptr->red;
      if (max_sig_bit < sig_bit_ptr->green)
         max_sig_bit = sig_bit_ptr->green;
      if (max_sig_bit < sig_bit_ptr->blue)
         max_sig_bit = sig_bit_ptr->blue;
      png_ptr->sig_bit.gray = info_ptr->sig_bit.gray = max_sig_bit;
   }
   if (info_ptr->valid & PNG_INFO_tRNS)
      png_set_tRNS(png_ptr, info_ptr, NULL, 0, &gray_trans);

   /* Update the image info. */
   png_ptr->color_type = info_ptr->color_type = PNG_COLOR_TYPE_GRAY;
   png_free_data(png_ptr, info_ptr, PNG_FREE_PLTE, -1);
   info_ptr->valid &= ~PNG_INFO_PLTE;
   return OPNG_REDUCE_PALETTE_TO_GRAY;  /* ignore the former result */
}


/*
 * Reduce the image (bit depth + color type + palette) without
 * losing any information. The palette (if applicable) and the
 * image data must be present (e.g. by calling png_set_rows(),
 * or by loading IDAT).
 * The parameter reductions indicates the intended reductions.
 * The function returns the successful reductions.
 */
png_uint_32 PNGAPI
opng_reduce_image(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 reductions)
{
   png_uint_32 result;

   png_debug(1, "in opng_reduce_image_type\n");

   if (!opng_validate_image(png_ptr, info_ptr))
   {
      png_warning(png_ptr,
         "Image reduction requires the presence of the critical info");
      return OPNG_REDUCE_NONE;
   }

#if 0  /* PNG_INTERLACE must be recognized! */
   if (png_ptr->transformations)
   {
      png_warning(png_ptr,
         "Image reduction cannot be applied "
         "under the presence of transformations");
      return OPNG_REDUCE_NONE;
   }
#endif

   /* The reductions below must be applied in the given order.
    * Can't use opng_reduce_bits(...) | opng_reduce_palette(...) | ...
    * LTR parsing is guaranteed for operator | but LTR evaluation is not.
    */
   result  = opng_reduce_bits(png_ptr, info_ptr, reductions);
   result |= opng_reduce_palette(png_ptr, info_ptr, reductions);
   result |= opng_reduce_rgb_to_palette(png_ptr, info_ptr, reductions);
   return result;
}


#endif  /* OPNG_IMAGE_REDUCTIONS_SUPPORTED */
