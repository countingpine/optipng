/*
 * gifread.c
 *
 * Copyright (C) 2003 Cosmin Truta.
 * This code was derived from "giftopnm.c" by David Koblas, and
 * it is distributed under the same copyright and warranty terms.
 *
 * The original copyright notice is provided below.
 */

/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, 1991, 1993, David Koblas.  (koblas@netcom.com)    | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */


#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gifread.h"


#define FALSE   0
#define TRUE    1

#define MAX_LZW_BITS    12


/** These macros are masquerading as inline functions. **/

#define GIF_FREAD(buf, len, file) \
    { if (fread(buf, len, 1, file) <= 0) GIFError(ErrRead); }

#define GIF_FGETC(ch, file) \
    { if ((ch = getc(file)) == EOF) GIFError(ErrRead); }

#define GIF_GETW(buf) \
    ((buf)[0] + ((buf)[1] << 8))

static const char *ErrRead = "Error reading file or unexpected end of file";


static void GIFReadNextImage(struct GIFImage *image, FILE *fp);
static void GIFReadNextExtension(struct GIFExtension *ext, FILE *fp);
static void ReadImageData(struct GIFImage *image, FILE *fp);
static void SkipDataBlocks(FILE *fp);
static int ReadDataBlock(unsigned char *buf, FILE *fp);
static int LZWGetCode(int code_size, int flag, FILE *fp);
static int LZWReadByte(int flag, int input_code_size, FILE *fp);


/**
 * Reads the GIF screen and the global color table.
 **/
void GIFReadScreen(struct GIFScreen *screen, FILE *fp)
{
    unsigned char buf[7];

    GIF_TRACE(("Reading Header\n"));
    GIF_FREAD(buf, 6, fp);
    if (strncmp((char *)buf, "GIF", 3) != 0)
        GIFError("Not a GIF file");
    if ((strncmp((char *)buf + 3, "87a", 3) != 0) &&
        (strncmp((char *)buf + 3, "89a", 3) != 0))
        GIFWarning("Invalid GIF version number, not \"87a\" or \"89a\"");

    GIF_TRACE(("Reading Logical Screen Descriptor\n"));
    GIF_FREAD(buf, 7, fp);
    screen->Width            = GIF_GETW(buf + 0);
    screen->Height           = GIF_GETW(buf + 2);
    screen->GlobalColorFlag  = (buf[4] & 0x80) ? 1 : 0;
    screen->ColorResolution  = ((buf[4] & 0x70) >> 3) + 1;
    screen->SortFlag         = (buf[4] & 0x08) ? 1 : 0;
    screen->GlobalNumColors  = 2 << (buf[4] & 0x07);
    screen->Background       = buf[5];
    screen->PixelAspectRatio = buf[6];

    if (screen->GlobalColorFlag)
    {
        GIF_TRACE(("Reading Global Color Table\n"));
        GIF_FREAD(screen->GlobalColorTable, 3 * screen->GlobalNumColors, fp);
    }

    GIF_TRACE(("Validating Logical Screen Descriptor\n"));
    if (screen->Width == 0 || screen->Height == 0)
        GIFError("Invalid image dimensions");
    if (screen->Background > 0)
    {
        if ((screen->GlobalColorFlag &&
             (screen->Background >= screen->GlobalNumColors)) ||
            !screen->GlobalColorFlag)
        {
#if 0   /* too noisy */
            GIFWarning("Invalid background color index");
#endif
            screen->Background = 0;
        }
    }
}


/**
 * Initializes the GIF image structure.
 **/
void GIFInitImage(struct GIFImage *image,
    struct GIFScreen *screen, unsigned char **rows)
{
    image->Screen = screen;
    image->Rows   = rows;
}


/**
 * Initializes the GIF extension structure.
 */
void GIFInitExtension(struct GIFExtension *ext,
    struct GIFScreen *screen, unsigned char *buf, unsigned int size)
{
    ext->Screen     = screen;
    ext->BufferSize = size;
    ext->Buffer     = buf;
}


/**
 * Reads the next GIF block (image or extension) structure.
 **/
int GIFReadNextBlock(struct GIFImage *image,
    struct GIFExtension *ext, FILE *fp)
{
    int ch;
    int foundBogus;

    foundBogus = 0;
    for ( ;; )
    {
        GIF_FGETC(ch, fp);
        switch (ch)
        {
            case GIF_IMAGE:       /* ',' */
                GIFReadNextImage(image, fp);
                return ch;
            case GIF_EXTENSION:   /* '!' */
                GIFReadNextExtension(ext, fp);
                return ch;
            case GIF_TERMINATOR:  /* ';' */
                return ch;
            default:
                if (!foundBogus)
                    GIFWarning("Bogus data in GIF");
                foundBogus = 1;
        }
    }
}


/**
 * Reads the next GIF image and local color table.
 **/
static void GIFReadNextImage(struct GIFImage *image, FILE *fp)
{
    struct GIFScreen *screen;
    unsigned char buf[9];

    GIF_TRACE(("Reading Local Image Descriptor\n"));
    GIF_FREAD(buf, 9, fp);
    if (image == NULL)
    {
        SkipDataBlocks(fp);
        return;
    }

    image->LeftPos        = GIF_GETW(buf + 0);
    image->TopPos         = GIF_GETW(buf + 2);
    image->Width          = GIF_GETW(buf + 4);
    image->Height         = GIF_GETW(buf + 6);
    image->LocalColorFlag = (buf[8] & 0x80) ? 1 : 0;
    image->InterlaceFlag  = (buf[8] & 0x40) ? 1 : 0;
    image->SortFlag       = (buf[8] & 0x20) ? 1 : 0;
    image->LocalNumColors = image->LocalColorFlag ? (2 << (buf[8] & 0x07)) : 0;

    if (image->LocalColorFlag)
    {
        GIF_TRACE(("Reading Local Color Table\n"));
        GIF_FREAD(image->LocalColorTable, 3 * image->LocalNumColors, fp);
    }

    GIF_TRACE(("Validating Logical Screen Descriptor\n"));
    screen = image->Screen;

    if (image->Width == 0 || image->Height == 0 ||
            image->LeftPos + image->Width > screen->Width ||
            image->TopPos + image->Height > screen->Height)
        GIFError("Invalid image dimensions");

    ReadImageData(image, fp);
}


/**
 * Reads the next GIF extension.
 **/
static void GIFReadNextExtension(struct GIFExtension *ext, FILE *fp)
{
    unsigned char *ptr;
    unsigned int len;
    int count, label;

    GIF_FGETC(label, fp);
    GIF_TRACE(("Reading Extension (0x%X)\n", label));
    if (ext != NULL)
        ext->Label = (unsigned char)label;
    if (ext == NULL || ext->Buffer == NULL)
    {
        SkipDataBlocks(fp);
        return;
    }

    ptr = ext->Buffer;
    len = ext->BufferSize;
    for ( ;; )
    {
        if (len < UCHAR_MAX)
        {
            len += 1024;
            ext->BufferSize += 1024;
            ext->Buffer = realloc(ext->Buffer, ext->BufferSize);
        }
        count = ReadDataBlock(ptr, fp);
        if (count == 0)
            break;
        ptr += count;
        len -= count;
    }
}

#if 0
    switch (label)
    {
        case GIF_PLAINTEXT:    /* 0x01 */
        case GIF_GRAPHICCTL:   /* 0xF9 */
        case GIF_COMMENT:      /* 0xFE */
        case GIF_APPLICATION:  /* 0xFF */
    }
#endif

#if 0
static void
DoExtension(fd, label)
FILE   *fd;
int    label;
{
       static char     buf[256];

       switch (label) {
       case 0x01:              /* Plain Text Extension */
#ifdef GIF_PLAIN_TEXT_EXT_SUPPORTED
               if (GetDataBlock(fd, (unsigned char*) buf) == 0)
                       ;

               lpos   = GIF_GETW(buf + 0);
               tpos   = GIF_GETW(buf + 2);
               width  = GIF_GETW(buf + 4);
               height = GIF_GETW(buf + 6);
               cellw  = buf[8];
               cellh  = buf[9];
               foreground = buf[10];
               background = buf[11];

               while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
                       PPM_ASSIGN(image[ypos][xpos],
                                       cmap[CM_RED][v],
                                       cmap[CM_GREEN][v],
                                       cmap[CM_BLUE][v]);
                       ++index;
               }

               return;
#else
               static int foundPlainTextExt = 0;

               if (! foundPlainTextExt) {
                       GIFWarning("Ignoring Plain Text Extension(s) in GIF");
                       foundPlainTextExt = 1;
               }
               break;
#endif
       case 0xff:              /* Application Extension */
               break;
       case 0xfe:              /* Comment Extension */
               while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
                       pm_message("GIF comment: %s", buf );
               }
               return;
       case 0xf9:              /* Graphic Control Extension */
               if (GetDataBlock(fd, (unsigned char*) buf) < 4) {
                       GIFError("Invalid Graphic Control Extension in GIF");
               }
               GIF89.disposal    = (buf[0] >> 2) & 0x7;
               GIF89.inputFlag   = (buf[0] >> 1) & 0x1;
               GIF89.delayTime   = GIF_GETW(buf + 1);
               if ((buf[0] & 0x1) != 0)
                       GIF89.transparent = buf[3];

               while (GetDataBlock(fd, (unsigned char*) buf) != 0)
                       ;
               return;
       default:                /* Unknown Extension */
               break;
       }

       while (GetDataBlock(fd, (unsigned char*) buf) != 0)
               ;
}
#endif


static int ZeroDataBlock = FALSE;

static int ReadDataBlock(unsigned char *buf, FILE *fp)
{
    int count;

    GIF_FGETC(count, fp);
    if (count > 0)
    {
        ZeroDataBlock = FALSE;
        GIF_FREAD(buf, (unsigned int)count, fp);
    }
    else
        ZeroDataBlock = TRUE;

    return count;
}

static void SkipDataBlocks(FILE *fp)
{
    int count;
    unsigned char buf[UCHAR_MAX];

    for ( ;; )
    {
        GIF_FGETC(count, fp)
        if (count > 0)
        {
            GIF_FREAD(buf, (unsigned int)count, fp);
        }
        else
            return;
    }
}

static int LZWGetCode(int code_size, int flag, FILE *fp)
{
       static unsigned char    buf[280];
       static int              curbit, lastbit, done, last_byte;
       int                     count, i, j, ret;

       if (flag) {
               curbit = 0;
               lastbit = 0;
               done = FALSE;
               return 0;
       }

       if ( (curbit+code_size) >= lastbit) {
               if (done) {
                       if (curbit >= lastbit)
                               GIFError("GIF/LZW error: ran off the end of my bits");
                       return -1;
               }
               buf[0] = buf[last_byte-2];
               buf[1] = buf[last_byte-1];

               if ((count = ReadDataBlock(&buf[2], fp)) == 0)
                       done = TRUE;

               last_byte = 2 + count;
               curbit = (curbit - lastbit) + 16;
               lastbit = (2+count)*8 ;
       }

       ret = 0;
       for (i = curbit, j = 0; j < code_size; ++i, ++j)
               ret |= ((buf[ i / 8 ] & (1 << (i % 8))) != 0) << j;

       curbit += code_size;

       return ret;
}

static int LZWReadByte(int flag, int input_code_size, FILE *fp)
{
       static int      fresh = FALSE;
       int             code, incode;
       static int      code_size, set_code_size;
       static int      max_code, max_code_size;
       static int      firstcode, oldcode;
       static int      clear_code, end_code;
       static int      table[2][(1<< MAX_LZW_BITS)];
       static int      stack[(1<<(MAX_LZW_BITS))*2], *sp;
       register int    i;

       if (flag) {
               set_code_size = input_code_size;
               code_size = set_code_size+1;
               clear_code = 1 << set_code_size ;
               end_code = clear_code + 1;
               max_code_size = 2*clear_code;
               max_code = clear_code+2;

               LZWGetCode(0, TRUE, fp);

               fresh = TRUE;

               for (i = 0; i < clear_code; ++i) {
                       table[0][i] = 0;
                       table[1][i] = i;
               }
               for (; i < (1<<MAX_LZW_BITS); ++i)
                       table[0][i] = table[1][0] = 0;

               sp = stack;

               return 0;
       } else if (fresh) {
               fresh = FALSE;
               do {
                       firstcode = oldcode =
                               LZWGetCode(code_size, FALSE, fp);
               } while (firstcode == clear_code);
               return firstcode;
       }

       if (sp > stack)
               return *--sp;

       while ((code = LZWGetCode(code_size, FALSE, fp)) >= 0) {
               if (code == clear_code) {
                       for (i = 0; i < clear_code; ++i) {
                               table[0][i] = 0;
                               table[1][i] = i;
                       }
                       for (; i < (1<<MAX_LZW_BITS); ++i)
                               table[0][i] = table[1][i] = 0;
                       code_size = set_code_size+1;
                       max_code_size = 2*clear_code;
                       max_code = clear_code+2;
                       sp = stack;
                       firstcode = oldcode =
                                       LZWGetCode(code_size, FALSE, fp);
                       return firstcode;
               } else if (code == end_code) {
                       int             count;
                       unsigned char   buf[260];

                       if (ZeroDataBlock)
                               return -2;

                       while ((count = ReadDataBlock(buf, fp)) > 0)
                               ;

#if 0  /* too noisy */
                       if (count != 0)
                               GIFWarning("missing EOD in data stream (common occurence)");
#endif
                       return -2;
               }

               incode = code;

               if (code >= max_code) {
                       *sp++ = firstcode;
                       code = oldcode;
               }

               while (code >= clear_code) {
                       *sp++ = table[1][code];
                       if (code == table[0][code])
                               GIFError("GIF/LZW error: circular table entry");
                       code = table[0][code];
               }

               *sp++ = firstcode = table[1][code];

               if ((code = max_code) <(1<<MAX_LZW_BITS)) {
                       table[0][code] = oldcode;
                       table[1][code] = firstcode;
                       ++max_code;
                       if ((max_code >= max_code_size) &&
                               (max_code_size < (1<<MAX_LZW_BITS))) {
                               max_code_size *= 2;
                               ++code_size;
                       }
               }

               oldcode = incode;

               if (sp > stack)
                       return *--sp;
       }
       return code;
}


static void ReadImageData(struct GIFImage *image, FILE *fp)
{
    int minCodeSize, interlaced, val, pass;
    unsigned int width, height, numColors, xpos, ypos;
    unsigned char **rows;

    GIF_TRACE(("Reading Image Data\n"));

    /* Initialize the compression routines. */
    GIF_FGETC(minCodeSize, fp);
    if (minCodeSize >= MAX_LZW_BITS)  /* this should be in fact <= 8 */
        GIFError("GIF/LZW error: invalid LZW code size");

    if (LZWReadByte(TRUE, minCodeSize, fp) < 0)
        GIFError("Error reading GIF image");

    /* Ignore the picture if it is "uninteresting". */
    rows = image->Rows;
    if (rows == NULL)
    {
#if 1
        /* This is faster, but possible LZW errors may go undetected. */
        SkipDataBlocks(fp);
#else
        /* This is safer, but slower. */
        while (LZWReadByte(FALSE, minCodeSize, fp) >= 0)
            ;
#endif
        return;
    }

    width       = image->Width;
    height      = image->Height;
    interlaced  = image->InterlaceFlag;
    GIFGetColorTable(image, &numColors);
    xpos = ypos = 0;
    pass = 0;
    while ((val = LZWReadByte(FALSE, minCodeSize, fp)) >= 0)
    {
        if ((unsigned int)val >= numColors)
        {
           GIFWarning("Pixel value out of range");
           val = numColors - 1;
        }
        rows[ypos][xpos] = (unsigned char)val;
        if (++xpos == width)
        {
            xpos = 0;
            if (interlaced)
            {
                switch (pass)
                {
                    case 0:
                    case 1:
                        ypos += 8;
                        break;
                    case 2:
                        ypos += 4;
                        break;
                    case 3:
                        ypos += 2;
                        break;
                }
                if (ypos >= height)
                {
                    switch (++pass)
                    {
                        case 1:
                            ypos = 4;
                            break;
                        case 2:
                            ypos = 2;
                            break;
                        case 3:
                            ypos = 1;
                            break;
                        default:
                            goto fini;
                    }
                }
            }
            else
                ++ypos;
        }
        if (ypos >= height)
            break;
    }
fini:
    /* Ignore the trailing garbage. */
    while (LZWReadByte(FALSE, minCodeSize, fp) >= 0)
        ;
}


/**
 * Constructs a GIF graphic control extension structure
 * from a raw extension structure.
 **/
void GIFGetGraphicCtl(struct GIFExtension *ext,
    struct GIFGraphicCtlExt *graphicExt)
{
    unsigned char *buf = ext->Buffer;

    if (buf == NULL)
        return;
    if (ext->Label != GIF_GRAPHICCTL)
    {
        GIFWarning("Not a graphic control extension");
        return;
    }
    if (ext->BufferSize < 4)
    {
        GIFWarning("Broken graphic control extension");
        return;
    }

    graphicExt->DisposalMethod  = (buf[0] >> 2) & 0x07;
    graphicExt->InputFlag       = (buf[0] >> 1) & 0x01;
    graphicExt->TransparentFlag = buf[0] & 0x01;
    graphicExt->DelayTime       = GIF_GETW(buf + 1);
    graphicExt->Transparent     = buf[3];
}


/* The GIF spec says that if neither global nor local
 * color maps are present, the decoder should use a system
 * default map, which should have black and white as the
 * first two colors. So we use black, white, red, green, blue,
 * yellow, purple and cyan.
 * Missing color tables are not a common case, and are not
 * handled by most GIF readers.
 */
static /*const*/ unsigned char DefaultColorTable[] =
{
     0,   0,   0,  /* black  */
   255, 255, 255,  /* white  */
   255,   0,   0,  /* red    */
     0, 255, 255,  /* cyan   */
     0, 255,   0,  /* green  */
   255,   0, 255,  /* purple */
     0,   0, 255,  /* blue   */
   255, 255,   0,  /* yellow */
};

/**
 * Returns the (local or global) color table.
 **/
unsigned char *GIFGetColorTable(struct GIFImage *image,
    unsigned int *numColors)
{
    struct GIFScreen *screen;

    if (image->LocalColorFlag)
    {
        *numColors = image->LocalNumColors;
        return image->LocalColorTable;
    }

    screen = image->Screen;
    if (screen->GlobalColorFlag)
    {
        *numColors = screen->GlobalNumColors;
        return screen->GlobalColorTable;
    }

    *numColors = sizeof(DefaultColorTable) / 3;
    return DefaultColorTable;
}


/**
 * Error handling.
 **/

static void GIFDefaultError(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static void GIFDefaultWarning(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

void (*GIFError)(const char *msg)
    = GIFDefaultError;

void (*GIFWarning)(const char *msg)
    = GIFDefaultWarning;
