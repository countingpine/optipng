/*
 * minitiff.h
 * Minimal I/O interface to the Tagged Image File Format (TIFF).
 * Version 0.2 (draft).
 *
 * Copyright (C) 2006-2017 Cosmin Truta.
 *
 * minitiff is open-source software, distributed under the zlib license.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.  If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef MINITIFF_H_
#define MINITIFF_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * The minitiff data structure.
 */
struct minitiff_info
{
    void (*error_handler)(const char *msg);
    void (*warning_handler)(const char *msg);
    int byte_order;
    size_t width;
    size_t height;
    unsigned int bits_per_sample;
    unsigned int compression;
    unsigned int photometric;
    size_t strip_offsets_count;
    unsigned long *strip_offsets;
    unsigned int orientation;
    unsigned int samples_per_pixel;
    size_t rows_per_strip;
};


/*
 * Constructor, validator and destructor.
 */
void minitiff_init_info(struct minitiff_info *info_ptr);
void minitiff_validate_info(const struct minitiff_info *info_ptr);
void minitiff_destroy_info(struct minitiff_info *info_ptr);

/*
 * Input functions.
 */
void minitiff_read_info(struct minitiff_info *info_ptr, FILE *stream);
void minitiff_read_row(struct minitiff_info *info_ptr,
                       unsigned char *row_ptr, size_t row_index,
                       FILE *stream);

/*
 * Output functions.
 */
void minitiff_write_info(const struct minitiff_info *info_ptr, FILE *stream);
void minitiff_write_row(const struct minitiff_info *info_ptr,
                        const unsigned char *row_ptr, size_t row_index,
                        FILE *stream);

/*
 * Error-reporting functions.
 */
void minitiff_error(const struct minitiff_info *info_ptr, const char *msg);
void minitiff_warning(const struct minitiff_info *info_ptr, const char *msg);


/*
 * TIFF data type constants.
 */
enum
{
    MINITIFF_TYPE_NONE      = 0x0000,
    MINITIFF_TYPE_BYTE      = 0x0001,  /* 8-bit unsigned integer */
    MINITIFF_TYPE_ASCII     = 0x0002,  /* null-terminated string of bytes */
    MINITIFF_TYPE_SHORT     = 0x0003,  /* 16-bit unsigned integer */
    MINITIFF_TYPE_LONG      = 0x0004,  /* 32-bit unsigned integer */
    MINITIFF_TYPE_RATIONAL  = 0x0005,  /* 64-bit unsigned rational */
    MINITIFF_TYPE_SBYTE     = 0x0006,  /* 8-bit signed integer */
    MINITIFF_TYPE_UNDEFINED = 0x0007,  /* blob of bytes */
    MINITIFF_TYPE_SSHORT    = 0x0008,  /* 16-bit signed integer */
    MINITIFF_TYPE_SLONG     = 0x0009,  /* 32-bit signed integer */
    MINITIFF_TYPE_SRATIONAL = 0x000a,  /* 64-bit signed rational */
    MINITIFF_TYPE_FLOAT     = 0x000b,  /* 32-bit IEEE floating-point */
    MINITIFF_TYPE_DOUBLE    = 0x000c,  /* 64-bit IEEE floating-point */
    MINITIFF_TYPE_IFD       = 0x000d,  /* 32-bit file offset */
    MINITIFF_TYPE_LONG64    = 0x0010,  /* BigTIFF 64-bit unsigned integer */
    MINITIFF_TYPE_SLONG64   = 0x0011,  /* BigTIFF 64-bit signed integer */
    MINITIFF_TYPE_IFD64     = 0x0012   /* BigTIFF 64-bit file offset */
};

/*
 * TIFF tag constants.
 */
enum
{
    MINITIFF_TAG_SUBFILE_TYPE           = 0x00fe,
    MINITIFF_TAG_OLD_SUBFILE_TYPE       = 0x00ff,
    MINITIFF_TAG_WIDTH                  = 0x0100,
    MINITIFF_TAG_HEIGHT                 = 0x0101,
    MINITIFF_TAG_BITS_PER_SAMPLE        = 0x0102,
    MINITIFF_TAG_COMPRESSION            = 0x0103,
    MINITIFF_TAG_PHOTOMETRIC            = 0x0106,
    MINITIFF_TAG_THRESHOLDING           = 0x0107,
    MINITIFF_TAG_CELL_WIDTH             = 0x0108,
    MINITIFF_TAG_CELL_LENGTH            = 0x0109,
    MINITIFF_TAG_FILL_ORDER             = 0x010a,
    MINITIFF_TAG_DOCUMENT_NAME          = 0x010d,
    MINITIFF_TAG_IMAGE_DESCRIPTION      = 0x010e,
    MINITIFF_TAG_MAKE                   = 0x010f,
    MINITIFF_TAG_MODEL                  = 0x0110,
    MINITIFF_TAG_STRIP_OFFSETS          = 0x0111,
    MINITIFF_TAG_ORIENTATION            = 0x0112,
    MINITIFF_TAG_SAMPLES_PER_PIXEL      = 0x0115,
    MINITIFF_TAG_ROWS_PER_STRIP         = 0x0116,
    MINITIFF_TAG_STRIP_BYTE_COUNTS      = 0x0117,
    MINITIFF_TAG_MIN_SAMPLE_VALUE       = 0x0118,
    MINITIFF_TAG_MAX_SAMPLE_VALUE       = 0x0119,
    MINITIFF_TAG_X_RESOLUTION           = 0x011a,
    MINITIFF_TAG_Y_RESOLUTION           = 0x011b,
    MINITIFF_TAG_PLANAR_CONFIGURATION   = 0x011c,
    MINITIFF_TAG_PAGE_NAME              = 0x011d,
    MINITIFF_TAG_X_POSITION             = 0x011e,
    MINITIFF_TAG_Y_POSITION             = 0x011f,
    MINITIFF_TAG_RESOLUTION_UNIT        = 0x0128,
    MINITIFF_TAG_PAGE_NUMBER            = 0x0129,
    MINITIFF_TAG_TRANSFER_FUNCTION      = 0x012d,
    MINITIFF_TAG_SOFTWARE               = 0x0131,
    MINITIFF_TAG_DATE_TIME              = 0x0132,
    MINITIFF_TAG_ARTIST                 = 0x013b,
    MINITIFF_TAG_HOST_COMPUTER          = 0x013c,
    MINITIFF_TAG_PREDICTOR              = 0x013d,
    MINITIFF_TAG_WHITE_POINT            = 0x013e,
    MINITIFF_TAG_PRIMARY_CHROMATICITIES = 0x013f,
    MINITIFF_TAG_COLOR_MAP              = 0x0140,
    MINITIFF_TAG_HALFTONE_HINTS         = 0x0141,
    MINITIFF_TAG_TILE_WIDTH             = 0x0142,
    MINITIFF_TAG_TILE_LENGTH            = 0x0143,
    MINITIFF_TAG_TILE_OFFSETS           = 0x0144,
    MINITIFF_TAG_BYTE_COUNTS            = 0x0145,
    MINITIFF_TAG_XMP                    = 0x02bc,
    MINITIFF_TAG_COPYRIGHT              = 0x8298,
    MINITIFF_TAG_IPTC                   = 0x83bb,
    MINITIFF_TAG_EXIF_IFD               = 0x8769,
    MINITIFF_TAG_ICC_PROFILE            = 0x8773,
    MINITIFF_TAG_GPS_IFD                = 0x8825,
    MINITIFF_TAG_INTEROPERABILITY_IFD   = 0xa005,
    MINITIFF_TAG_PRINT_IM               = 0xc4a5
};

/*
 * TIFF compression constants.
 */
enum
{
    MINITIFF_COMPRESSION_NONE          = 0x0001,  /* No compression */
    MINITIFF_COMPRESSION_CCITT_RLE     = 0x0002,  /* CCITT Huffman RLE */
    MINITIFF_COMPRESSION_CCITT_T4      = 0x0003,  /* CCITT T.4 (Group 3 fax) */
    MINITIFF_COMPRESSION_CCITT_FAX3    = 0x0003,  /* CCITT T.4 (Group 3 fax) */
    MINITIFF_COMPRESSION_CCITT_T6      = 0x0004,  /* CCITT T.6 (Group 4 fax) */
    MINITIFF_COMPRESSION_CCITT_FAX4    = 0x0004,  /* CCITT T.6 (Group 4 fax) */
    MINITIFF_COMPRESSION_LZW           = 0x0005,  /* Lempel, Zip & Welch */
    MINITIFF_COMPRESSION_OLD_JPEG      = 0x0006,  /* Old JPEG DCT */
    MINITIFF_COMPRESSION_JPEG          = 0x0007,  /* JPEG DCT */
    MINITIFF_COMPRESSION_ADOBE_DEFLATE = 0x0008,  /* Adobe Deflate */
    MINITIFF_COMPRESSION_ITU_T85       = 0x0009,  /* ITU-T T.85 (JBIG B/W) */
    MINITIFF_COMPRESSION_ITU_T43       = 0x000a,  /* ITU-T T.43 (JBIG color) */
    MINITIFF_COMPRESSION_NEXT_RLE      = 0x7ffe,  /* NeXT 2-bit RLE */
    MINITIFF_COMPRESSION_CCITT_RLEW    = 0x8003,  /* CCITT RLE, word align */
    MINITIFF_COMPRESSION_PACKBITS      = 0x8005,  /* Apple PackBits RLE */
    MINITIFF_COMPRESSION_THUNDERSCAN   = 0x8029,  /* ThunderScan 4-bit RLE */
    MINITIFF_COMPRESSION_IT8_CT_MP     = 0x807f,  /* IT8-CT and -MP, padding */
    MINITIFF_COMPRESSION_IT8_LW        = 0x8080,  /* IT8-LW, RLE */
    MINITIFF_COMPRESSION_IT8_HC        = 0x8081,  /* IT8-HC (not -MP), RLE */
    MINITIFF_COMPRESSION_IT8_BL        = 0x8082,  /* IT8-BL, RLE */
    MINITIFF_COMPRESSION_PIXARFILM     = 0x808c,  /* PixarFilm 10-bit LZW */
    MINITIFF_COMPRESSION_PIXARLOG      = 0x808d,  /* PixarLog 11-bit Deflate */
    MINITIFF_COMPRESSION_DEFLATE       = 0x80b2,  /* Deflate */
    MINITIFF_COMPRESSION_KODAK_DCS     = 0x80b3,  /* Kodak DCS */
    MINITIFF_COMPRESSION_JBIG          = 0x8765,  /* JBIG */
    MINITIFF_COMPRESSION_SGI_LOGLUV    = 0x8774,  /* SGI LogLuv 32-bit RLE */
    MINITIFF_COMPRESSION_SGI_LOGLUV24  = 0x8775,  /* SGI LogLuv 24-bit */
    MINITIFF_COMPRESSION_JPEG2000      = 0x8798,  /* JPEG-2000 */
    MINITIFF_COMPRESSION_LZMA2         = 0x886d   /* LZMA2 */
};

/*
 * TIFF photometric constants.
 */
enum
{
    MINITIFF_PHOTOMETRIC_MINWHITE  = 0x0000,  /* Min. value is white */
    MINITIFF_PHOTOMETRIC_MINBLACK  = 0x0001,  /* Max. value is black */
    MINITIFF_PHOTOMETRIC_RGB       = 0x0002,  /* RGB colors */
    MINITIFF_PHOTOMETRIC_PALETTE   = 0x0003,  /* Indexed color map */
    MINITIFF_PHOTOMETRIC_MASK      = 0x0004,  /* Mask */
    MINITIFF_PHOTOMETRIC_SEPARATED = 0x0005,  /* Separated color planes */
    MINITIFF_PHOTOMETRIC_YCBCR     = 0x0006,  /* ITU-R BT.601 YCbCr */
    MINITIFF_PHOTOMETRIC_CIELAB    = 0x0008,  /* CIE L*a*b* */
    MINITIFF_PHOTOMETRIC_ICCLAB    = 0x0009,  /* ICC L*a*b* (Technote 4) */
    MINITIFF_PHOTOMETRIC_ITULAB    = 0x000a,  /* ITU L*a*b* */
    MINITIFF_PHOTOMETRIC_CFA       = 0x8023,  /* Color Filter Array */
    MINITIFF_PHOTOMETRIC_LOGL      = 0x804c,  /* CIE Log2(L) */
    MINITIFF_PHOTOMETRIC_LOGLUV    = 0x804d   /* CIE Log2(L)u'v' */
};

/*
 * TIFF image orientation constants.
 */
enum
{
    MINITIFF_ORIENTATION_TOP_LEFT     = 0x0001,
    MINITIFF_ORIENTATION_TOP_RIGHT    = 0x0002,
    MINITIFF_ORIENTATION_BOTTOM_RIGHT = 0x0003,
    MINITIFF_ORIENTATION_BOTTOM_LEFT  = 0x0004,
    MINITIFF_ORIENTATION_LEFT_TOP     = 0x0005,
    MINITIFF_ORIENTATION_RIGHT_TOP    = 0x0006,
    MINITIFF_ORIENTATION_RIGHT_BOTTOM = 0x0007,
    MINITIFF_ORIENTATION_LEFT_BOTTOM  = 0x0008
};


/*
 * TIFF file signature constants.
 */
extern const char minitiff_sig_m[4];
extern const char minitiff_sig_i[4];
extern const char minitiff_sig_bigm[4];
extern const char minitiff_sig_bigi[4];


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* MINITIFF_H_ */
