/**
 * Copies a BMP piece by piece, just because.
 */

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

//resize amt
int n;

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    n = atoi(argv[1]);
    char *infile = argv[2];
    char *outfile = argv[3];

    if (n > 100 || n < 0)
    {
        fprintf(stderr, "Resize value must be less than or equal to 100\n");
        return 1;
    }

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // declare new headers
    BITMAPINFOHEADER binew = bi;
    BITMAPFILEHEADER bfnew = bf;

    // define new header info
    binew.biWidth = bi.biWidth * n;
    binew.biHeight = bi.biHeight * n;

    // cursor offset for rewrite
    int offset = bi.biWidth * sizeof(RGBTRIPLE);

    // determine infile padding
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // determine new padding for scanlines
    int padding_new = (4 - (binew.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    binew.biSizeImage = ((sizeof(RGBTRIPLE) * binew.biWidth) + padding_new) * abs(binew.biHeight);
    bfnew.bfSize = binew.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&bfnew, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&binew, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        for (int r = 0; r < n - 1; r++)
        {
            // iterate over pixels in scanline
            for (int j = 0; j < bi.biWidth; j++)
            {

                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                for (int m = 0; m < n; m++)
                {
                    // write RGB triple to outfile
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }

            // write outfile padding
            if (padding_new != 0)
            {
                for (int k = 0; k < padding_new; k++)
                {
                    fputc(0x00, outptr);
                }
            }

            // send cursor back
            fseek(inptr, -offset, SEEK_CUR);
        }
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            for (int m = 0; m < n; m++)
            {
                // write RGB triple to outfile
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
            }
        }
        // write outfile padding
        if (padding_new != 0)
        {
            for (int k = 0; k < padding_new; k++)
            {
                fputc(0x00, outptr);
            }
        }

        // skip infile padding
        fseek(inptr, padding, SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
