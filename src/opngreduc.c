/*
 * opngreduc.c - libpng extension: image reductions
 *
 * Copyright (C) 2003-2004 Cosmin Truta.
 * The code is distributed under the same licensing and warranty terms
 * as libpng.
 *
 * CAUTION:
 * This code is intended to become a part of libpng. Due to this reason,
 * it accesses the internal libpng structures directly. Although it
 * works well with dynamic libpng builds (.so / .dll), it is recommended
 * to be linked to static libpng builds.
 *
 * When this code will emerge into libpng, it is recommended to rename
 * this source file to "pngreduc.c", and to rename the var/func/macro
 * names as follows:
 *   opng_xxx -> png_xxx
 *   OPNG_XXX -> PNG_XXX
 *
 * CAUTION:
 * Currently, libpng does not offer a reliable way to know when
 * the alpha channel is before the other components (e.g. AG, ARGB).
 * For example, this code fails to function properly under
 * PNG_TRANSFORM_SWAP_ALPHA. As an interim solution, we rely on
 * the presence of PNG_TRANSFORM and PNG_FILLER_AFTER.
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


/*
 * Analyze the possibility of reducing the image type.
 * The parameter reductions indicates the intended reductions.
 * The function returns the possible reductions.
 */
png_uint_32 /* PRIVATE */
opng_analyze_image_reduction(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 reductions)
{
   png_bytepp row_ptr;
   png_bytep component_ptr;
   png_uint_32 height, width, i, j;
   unsigned int bit_depth, byte_depth, color_type, channels, sample_size,
      offset_color, offset_alpha;

   png_debug(1, "in opng_analyze_image_reduction\n");

   bit_depth = info_ptr->bit_depth;
   if (bit_depth < 8)
      return OPNG_REDUCE_NONE;  /* nothing needs to be done */

   color_type = info_ptr->color_type;
   if (color_type & PNG_COLOR_MASK_PALETTE)
      return OPNG_REDUCE_NONE;  /* let opng_reduce_palette() handle it */

   byte_depth  = bit_depth / 8;
   channels    = (png_ptr->usr_channels > 0) ?
      png_ptr->usr_channels : info_ptr->channels;
   sample_size = channels * byte_depth;

   /* The following reductions are not implemented yet, or do not apply. */
   reductions &= ~(OPNG_REDUCE_8_TO_421 |
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
      if (background->red != background->green &&
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
 * Reduce the image type to a lower bit depth and color type
 * without losing valuable information.
 * The parameter reductions indicates the intended reductions.
 * The function returns the successful reductions.
 * All reductions are performed in a single step.
 */
png_uint_32 /* PRIVATE */
opng_reduce_image_type(png_structp png_ptr, png_infop info_ptr,
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

   png_debug(1, "in opng_reduce_image_type\n");

   /* See which reductions may be performed. */
   reductions = opng_analyze_image_reduction(png_ptr, info_ptr, reductions);
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
   int num_palette, num_trans, last_color, is_gray, k;
   png_color_16 gray_trans;

   png_debug(1, "in opng_reduce_palette\n");

   /* Only the following reductions apply. */
   reductions &= (OPNG_REDUCE_PALETTE_TO_GRAY | OPNG_REDUCE_PALETTE_FAST);
   if (info_ptr->bit_depth < 8)
      reductions &= OPNG_REDUCE_PALETTE_FAST;

   if (reductions == OPNG_REDUCE_NONE ||
       info_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
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
   opng_analyze_sample_usage(png_ptr, info_ptr, is_used);
   for (k = num_trans - 1; k >= 0; --k)
   {
      if (!is_used[k])
         continue;
      if (trans[k] < 255)
         break;
   }
   num_trans = k + 1;
   for ( ; k >= 0; --k)
      is_used[k] = 1;

   is_gray = (reductions & OPNG_REDUCE_PALETTE_TO_GRAY) ? 1 : 0;
   last_color = -1;
   for (k = 0; k < 256; ++k)
      if (is_used[k])
      {
         last_color = k;
         if (palette[k].red != palette[k].green ||
             palette[k].red != palette[k].blue)
            is_gray = 0;
      }
   OPNG_ASSERT(last_color >= 0);

   if (last_color >= num_palette)
   {
      png_warning(png_ptr, "Invalid number of colors in palette");
      return OPNG_REDUCE_NONE;
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

   /* Test if tRNS can be reduced to grayscale. */
   if (is_gray && num_trans > 0)
   {
      k = num_trans - 1;
      gray_trans.gray = palette[k].red;
      for (--k ; k >= 0; --k)
         if (trans[k] < 255 && gray_trans.gray != palette[k].red)
         {
            is_gray = 0;
            break;
         }
   }

   if (is_gray)
   {
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

   if (reductions & OPNG_REDUCE_PALETTE_FAST)
   {
      if (num_palette > last_color + 1)
      {
         /* Reduce PLTE. */
         /* hIST is reduced automatically. */
         info_ptr->num_palette = (png_uint_16)(last_color + 1);
         result = OPNG_REDUCE_PALETTE_FAST;
      }

      if ((info_ptr->valid & PNG_INFO_tRNS) &&
          (int)info_ptr->num_trans > num_trans)
      {
         /* Reduce tRNS. */
         info_ptr->num_trans = (png_uint_16)num_trans;
         result = OPNG_REDUCE_PALETTE_FAST;
      }
   }

   return result;
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
   png_debug(1, "in opng_reduce_image_type\n");

   if (!opng_validate_image(png_ptr, info_ptr))
   {
      png_warning(png_ptr,
         "Image reduction requires the presence of the critical info.");
      return OPNG_REDUCE_NONE;
   }

   return
      opng_reduce_image_type(png_ptr, info_ptr, reductions) |
      opng_reduce_palette(png_ptr, info_ptr, reductions);
}


#endif  /* OPNG_IMAGE_REDUCTIONS_SUPPORTED */
