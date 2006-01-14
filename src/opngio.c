/*
 * opngio.c - libpng extension: I/O state query
 *
 * Copyright (C) 2001-2004 Cosmin Truta.
 * The program is distributed under the same licensing and warranty terms
 * as libpng.
 *
 * CAUTION:
 * Currently, these functions have a simplistic implementation that allows
 * only one reading and one writing png_ptr, and they are not thread-safe.
 * If added to libpng, the code will be completely rewritten. It will be
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

static png_uint_32
opng_priv_read_crt_chunk_len, opng_priv_write_crt_chunk_len;

static const char *opng_priv_errmsg =
   "Internal OPNGIO error: incorrect use of the opng_ functions";


/* Update io_state and call the user-supplied read/write functions. */
static void
opng_priv_read_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
   png_rw_ptr io_data_fn;
   int *io_state_ptr, io_state_op;
   png_uint_32 *io_crt_chunk_len_ptr, partial_chunk_len;

   if (png_ptr == opng_priv_read_ptr)
   {
      io_data_fn = opng_priv_read_fn;
      io_state_ptr = &opng_priv_read_io_state;
      io_state_op = OPNG_IO_READING;
      io_crt_chunk_len_ptr = &opng_priv_read_crt_chunk_len;
   }
   else if (png_ptr == opng_priv_write_ptr)
   {
      io_data_fn = opng_priv_write_fn;
      io_state_ptr = &opng_priv_write_io_state;
      io_state_op = OPNG_IO_WRITING;
      io_crt_chunk_len_ptr = &opng_priv_write_crt_chunk_len;
   }
   else
   {
      io_data_fn = NULL;  /* dummy, keep compilers happy */
      io_state_ptr = NULL;
      io_state_op = OPNG_IO_UNKNOWN;
      io_crt_chunk_len_ptr = NULL;
      png_error(png_ptr, opng_priv_errmsg);
   }

   while (length > 0)
   {
      switch (*io_state_ptr & OPNG_IO_MASK_LOC)
      {
         case OPNG_IO_SIG:
            /* This code assumes that the signature is written in
               a single i/o session. */
            io_data_fn(png_ptr, data, length);
            *io_state_ptr = io_state_op | OPNG_IO_LEN;
            return;
         case OPNG_IO_LEN:
            OPNG_ASSERT(data != NULL);
            OPNG_ASSERT(length >= 4);
            io_data_fn(png_ptr, data, 4);
            *io_crt_chunk_len_ptr = png_get_uint_32(data);
            *io_state_ptr = io_state_op | OPNG_IO_HDR;
            data   += 4;
            length -= 4;
            break;
         case OPNG_IO_HDR:
            OPNG_ASSERT(length >= 4);
            io_data_fn(png_ptr, data, 4);
            *io_state_ptr = io_state_op | OPNG_IO_DATA;
            data   += 4;
            length -= 4;
            break;
         case OPNG_IO_DATA:
            /* This code assumes that the chunk data may be written in
               multiple i/o sessions. */
            partial_chunk_len = (length < *io_crt_chunk_len_ptr) ?
               length : *io_crt_chunk_len_ptr;
            io_data_fn(png_ptr, data, partial_chunk_len);
            if ((*io_crt_chunk_len_ptr -= partial_chunk_len) == 0)
               *io_state_ptr = io_state_op | OPNG_IO_CRC;
            data   += partial_chunk_len;
            length -= partial_chunk_len;
            break;
         case OPNG_IO_CRC:
            OPNG_ASSERT(length >= 4);
            io_data_fn(png_ptr, data, 4);
            *io_state_ptr = io_state_op | OPNG_IO_LEN;
            data   += 4;
            length -= 4;
            break;
      }
   }
}


/* The only public opng_ function that may remain in libpng.
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
   return OPNG_IO_UNKNOWN;
}


/* The next two functions need to override the standard png_set_read/write_fn.
 * The override will not be necessary when integrating this into libpng.
 */
void PNGAPI
opng_set_read_fn(png_structp png_ptr, png_voidp io_ptr,
   png_rw_ptr read_data_fn)
{
   opng_priv_read_ptr = png_ptr;
   opng_priv_write_ptr = NULL;
   opng_priv_read_fn = read_data_fn;
   png_set_read_fn(png_ptr, io_ptr, opng_priv_read_write);
   opng_priv_read_io_state = OPNG_IO_READING | OPNG_IO_SIG;
}

void PNGAPI
opng_set_write_fn(png_structp png_ptr, png_voidp io_ptr,
   png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn)
{
   opng_priv_write_ptr = png_ptr;
   opng_priv_read_ptr = NULL;
   opng_priv_write_fn = write_data_fn;
   png_set_write_fn(png_ptr, io_ptr, opng_priv_read_write, output_flush_fn);
   opng_priv_write_io_state = OPNG_IO_WRITING | OPNG_IO_SIG;
}
