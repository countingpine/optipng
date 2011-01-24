/*
 * gifdump.c
 *
 * Copyright (C) 2003-2010 Cosmin Truta.
 * This software is distributed under the same licensing and warranty terms
 * as gifread.c.
 */


#include <stdio.h>
#include <stdlib.h>
#include "gifread.h"


int GIFDump(const char *filename)
{
    FILE *stream;
    struct GIFScreen screen;
    struct GIFImage image;
    struct GIFExtension ext;
    struct GIFGraphicCtlExt graphicExt;

    stream = fopen(filename, "rb");
    if (stream == NULL)
    {
        fprintf(stderr, "Error: Can't open %s\n", filename);
        return -1;
    }

    printf("File: %s\n", filename);

    GIFReadScreen(&screen, stream);
    printf("Screen: %u x %u\n", screen.Width, screen.Height);
    if (screen.GlobalColorFlag)
        printf("  Global colors: %u\n", screen.GlobalNumColors);
    if (screen.PixelAspectRatio != 0)
        printf("  Pixel aspect ratio = %u\n", screen.PixelAspectRatio);

    GIFInitImage(&image, &screen, NULL);
    GIFInitExtension(&ext, &screen, NULL, 0);

    for ( ; ; )
    {
        switch (GIFReadNextBlock(&image, &ext, stream))
        {
        case GIF_TERMINATOR:  /* ';' */
            fclose(stream);
            return 0;
        case GIF_IMAGE:       /* ',' */
            printf("Image: %u x %u @ (%u, %u)\n",
                   image.Width, image.Height, image.LeftPos, image.TopPos);
            if (image.LocalColorFlag)
                printf("  Local colors: %u\n", image.LocalNumColors);
            printf("  Interlaced: %s\n", image.InterlaceFlag ? "YES" : "NO");
            break;
        case GIF_EXTENSION:   /* '!' */
            if (ext.Label == GIF_GRAPHICCTL)
            {
                GIFGetGraphicCtl(&ext, &graphicExt);
                printf("Graphic Control Extension: 0x%02X\n", ext.Label);
                printf("  Disposal method: %u\n", graphicExt.DisposalMethod);
                printf("  User input flag: %u\n", graphicExt.InputFlag);
                printf("  Delay time     : %u\n", graphicExt.DelayTime);
                if (graphicExt.TransparentFlag)
                    printf("  Transparent    : %u\n", graphicExt.Transparent);
            }
            else
                printf("Extension: 0x%02X\n", ext.Label);
            break;
        }
    }
}


int main(int argc, char *argv[])
{
    int exitCode;
    int i;

    if (argc <= 1)
    {
        printf("Usage: gifdump <files.gif...>\n");
        return 0;
    }

    exitCode = EXIT_SUCCESS;
    for (i = 1; i < argc; ++i)
    {
        if (GIFDump(argv[i]) == -1)
            exitCode = EXIT_FAILURE;
        printf("\n");
    }

    return exitCode;
}
