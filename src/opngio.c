/*
 * opngio.c - libpng extension: I/O state query
 *
 * Copyright (C) 2001-2006 Cosmin Truta.
 * This software is distributed under the same licensing and warranty terms
 * as libpng.
 *
 * CAUTION:
 * Currently, these functions have a simplistic implementation that allows
 * only one reading and one writing png_ptr, and they are not thread-safe.
 * If added to libpng, the code will be completely rewritten.  It will be
 * thread-safe and much simpler, due to the addition of io_state to png_struct.
 * In this case, the opng_priv_* variables and functions will be removed,
 * and the other opng_* public functions will be renamed to png_* and
 * merged into libpng.
 *
 * For more info, see opng.h.
 */

#include "opng.h"


/* Here comes the kludge... */

static png_structp
opng_priv_read_ptr, opng_priv_write_ptr;

static png_rw_ptr
opng_priv_read_fn, opng_priv_write_fn;

static int
opng_priv_read_io_state, opng_priv_write_io_state;

static png_byte
opng_priv_read_crt_chunk_hdr[8], opng_priv_write_crt_chunk_hdr[8];

static unsigned int
opng_priv_read_crt_chunk_hdr_len, opng_priv_write_crt_chunk_hdr_len;

static png_uint_32
opng_priv_read_crt_len, opng_priv_write_crt_len;

static const char *opng_priv_errmsg =
   "Internal OPNGIO error: incorrect use of the opng_ functions";


/* Update io_state and call the user-supplied read/write functions. */
void /* PRIVATE */
opng_priv_read_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
   png_rw_ptr io_data_fn;
   int *io_state_ptr, io_state_op;
   png_bytep io_crt_chunk_hdr;
   unsigned int *io_crt_chunk_hdr_len_ptr;
   png_uint_32 *io_crt_len_ptr;

   if (png_ptr == opng_priv_read_ptr)
   {
      io_data_fn = opng_priv_read_fn;
      io_state_ptr = &opng_priv_read_io_state;
      io_state_op = OPNG_IO_READING;
      io_crt_chunk_hdr = opng_priv_read_crt_chunk_hdr;
      io_crt_chunk_hdr_len_ptr = &opng_priv_read_crt_chunk_hdr_len;
      io_crt_len_ptr = &opng_priv_read_crt_len;
   }
   else if (png_ptr == opng_priv_write_ptr)
   {
      io_data_fn = opng_priv_write_fn;
      io_state_ptr = &opng_priv_write_io_state;
      io_state_op = OPNG_IO_WRITING;
      io_crt_chunk_hdr = opng_priv_write_crt_chunk_hdr;
      io_crt_chunk_hdr_len_ptr = &opng_priv_write_crt_chunk_hdr_len;
      io_crt_len_ptr = &opng_priv_write_crt_len;
   }
   else
   {
      png_error(png_ptr, opng_priv_errmsg);
      return;
   }

   switch (*io_state_ptr & OPNG_IO_MASK_LOC)
   {
   case OPNG_IO_SIGNATURE:
      /* The signature should be serialized in a single I/O session.
       * (This limitation is imposed for simplicity purposes.)
       */
      OPNG_ASSERT(length <= 8);
      io_data_fn(png_ptr, data, length);
      *io_state_ptr = io_state_op | OPNG_IO_CHUNK_HDR;
      *io_crt_chunk_hdr_len_ptr = 0;
      return;
   case OPNG_IO_CHUNK_HDR:
      /* The chunk header may be serialized in multiple I/O sessions.
       * (For performance reasons, libpng should do it in a single session.)
       */
      OPNG_ASSERT(length + *io_crt_chunk_hdr_len_ptr <= 8);
      if (io_state_op == OPNG_IO_READING)
      {
         if (*io_crt_chunk_hdr_len_ptr == 0)
            io_data_fn(png_ptr, io_crt_chunk_hdr, 8);
         memcpy(data, io_crt_chunk_hdr + *io_crt_chunk_hdr_len_ptr, length);
         *io_crt_chunk_hdr_len_ptr += length;
         if (*io_crt_chunk_hdr_len_ptr < 8)
            return;
         *io_crt_len_ptr = png_get_uint_32(io_crt_chunk_hdr);
         memcpy(png_ptr->chunk_name, io_crt_chunk_hdr + 4, 4);
      }
      else  /* io_state_op == OPNG_IO_WRITING */
      {
         memcpy(io_crt_chunk_hdr + *io_crt_chunk_hdr_len_ptr, data, length);
         *io_crt_chunk_hdr_len_ptr += length;
         if (*io_crt_chunk_hdr_len_ptr < 8)
            return;
         *io_crt_len_ptr = png_get_uint_32(io_crt_chunk_hdr);
         memcpy(png_ptr->chunk_name, io_crt_chunk_hdr + 4, 4);
         io_data_fn(png_ptr, io_crt_chunk_hdr, 8);
      }
      *io_crt_chunk_hdr_len_ptr = 0;
      *io_state_ptr = io_state_op | OPNG_IO_CHUNK_DATA;
      return;
   case OPNG_IO_CHUNK_DATA:
      /* Chunk data may be serialized in multiple I/O sessions. */
      if (length == 0)
         return;
      if (*io_crt_len_ptr > 0)
      {
         OPNG_ASSERT(length <= *io_crt_len_ptr);
         io_data_fn(png_ptr, data, length);
         if ((*io_crt_len_ptr -= length) == 0)
            *io_state_ptr = io_state_op | OPNG_IO_CHUNK_CRC;
         return;
      }
      /* ... else go to the next case. */
   case OPNG_IO_CHUNK_CRC:
      /* The CRC must be serialized in a single I/O session.
       * (libpng already complies to this.)
       */
      OPNG_ASSERT(length == 4);
      io_data_fn(png_ptr, data, 4);
      *io_state_ptr = io_state_op | OPNG_IO_CHUNK_HDR;
      return;
   }
}


/* The following two functions should be added to pngget.c,
 * implemented as simple get functions that retrieve
 * png_ptr members (png_ptr->io_state and png_ptr->chunk_name).
 */
png_uint_32 PNGAPI
opng_get_io_state(png_structp png_ptr)
{
   /* CAUTION: ugly code. */
   /* This mess will disappear when io_state is added to png_ptr. */
   if (png_ptr == opng_priv_read_ptr)
      return opng_priv_read_io_state;
   else if (png_ptr == opng_priv_write_ptr)
      return opng_priv_write_io_state;
   png_error(png_ptr, opng_priv_errmsg);
   /* Never get here... */
   return OPNG_IO_NONE;
}

png_bytep PNGAPI
opng_get_io_chunk_name(png_structp png_ptr)
{
   return png_ptr->chunk_name;
}


/* The next two functions must override the standard png_set_read/write_fn.
 * The override will not be necessary when integrating this into libpng,
 * so these functions should be removed at that time.
 */
void PNGAPI
opng_set_read_fn(png_structp png_ptr, png_voidp io_ptr,
   png_rw_ptr read_data_fn)
{
   opng_priv_read_ptr = png_ptr;
   opng_priv_write_ptr = NULL;
   opng_priv_read_fn = read_data_fn;
   png_set_read_fn(png_ptr, io_ptr, opng_priv_read_write);
   opng_priv_read_io_state = OPNG_IO_READING | OPNG_IO_SIGNATURE;
}

void PNGAPI
opng_set_write_fn(png_structp png_ptr, png_voidp io_ptr,
   png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn)
{
   opng_priv_write_ptr = png_ptr;
   opng_priv_read_ptr = NULL;
   opng_priv_write_fn = write_data_fn;
   png_set_write_fn(png_ptr, io_ptr, opng_priv_read_write, output_flush_fn);
   opng_priv_write_io_state = OPNG_IO_WRITING | OPNG_IO_SIGNATURE;
}
