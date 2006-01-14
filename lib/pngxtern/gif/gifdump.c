#include <stdio.h>
#include "gifread.h"


unsigned int errCount = 0;

void GIFDump(const char *filename)
{
    FILE *fp;
    struct GIFScreen screen;
    struct GIFImage image;
    struct GIFExtension ext;
    struct GIFGraphicCtlExt graphicExt;
    unsigned char *buf;
    unsigned int bufsize;

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
       fprintf(stderr, "Can't open %s\n", filename);
       ++errCount;
       return;
    }

    printf("File: %s\n", filename);

    GIFReadScreen(&screen, fp);
    printf("Screen: %u x %u\n", screen.Width, screen.Height);
    if (screen.GlobalColorFlag)
        printf("Global colors: %u\n", screen.GlobalNumColors);
    if (screen.PixelAspectRatio != 0)
        printf("Pixel aspect ratio = %u\n", screen.PixelAspectRatio);

    GIFInitImage(&image, &screen, NULL);
    bufsize = 1024;
    buf = (unsigned char *)malloc(bufsize);
    GIFInitExtension(&ext, &screen, buf, bufsize);

    for ( ;; )
    {
      switch (GIFReadNextBlock(&image, &ext, fp))
      {
         case GIF_TERMINATOR:  /* ';' */
         {
            printf("\n");
            fclose(fp);
            return;
         }
         case GIF_IMAGE:       /* ',' */
         {
            printf("Image: %u x %u @ (%u, %u)\n",
                image.Width, image.Height, image.LeftPos, image.TopPos);
            if (image.LocalColorFlag)
                printf("Local colors: %u\n", image.LocalNumColors);
            printf(image.InterlaceFlag ? "Interlaced\n" : "Non-interlaced\n");
            break;
         }
         case GIF_EXTENSION:   /* '!' */
         {
            if (ext.Label == GIF_GRAPHICCTL)
            {
                GIFGetGraphicCtl(&ext, &graphicExt);
                printf("Graphic Control Extension:\n");
                printf("  Disposal method: %u\n", graphicExt.DisposalMethod);
                printf("  User input flag: %u\n", graphicExt.InputFlag);
                printf("  Delay time     : %u\n", graphicExt.DelayTime);
                if (graphicExt.TransparentFlag)
                    printf("  Transparent    : %u\n", graphicExt.Transparent);
            }
            else
               printf("Extension: 0x%x\n", ext.Label);
         }
      }
    }
}


int main(int argc, char *argv[])
{
    int i;

    if (argc <= 1)
    {
        fprintf(stderr, "Usage: gifdump <files.gif...>\n");
        return 1;
    }

    for (i = 1; i < argc; ++i)
        GIFDump(argv[i]);

    if (errCount > 0)
        fprintf(stderr, "%u error(s)\n", errCount);

    return 0;
}
