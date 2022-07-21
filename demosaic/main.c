#include "utils.h"
#include "bmp.h"
#include "tiff.h"
#include "demosaic.h"

#define DEBUG_MODE 0

int demosaic_test(char *file) {
    uint8_t *src, *dst;
    int fd;
    int i, ret;
    char outfile[255], *fname, *fext;

    fname = get_file_name(file);
    fext = get_file_ext(file);
    sprintf(outfile, "%s_bayer_debayer.%s", fname, fext);
    if (!strcmp(fext, "tiff") || !strcmp(fext, "tif")) {
        int w1, h1, bpp1, s1, phi1;
        int width, height, bitcount, samples_per_pixel, photo_interp;
        extract_tiff_data(&src, &width, &height, &bitcount, &samples_per_pixel, &photo_interp, file);
        if (samples_per_pixel == 3 && photo_interp == 2 && bitcount == samples_per_pixel * 8) {
            //sprintf(outfile, "%s_test.tif", fname);
            //TIFFContext *c;
            //init_tiff_ctx(&c, src, width, height, 24, 3, 2);
            //write_tiff(c, outfile);

            uint8_t *cfa;
            create_cfa_gbrg(&cfa, src, width, height);

            int cfa_pattern = 0x01000201;
            demosaicing(&dst, cfa, width, height, cfa_pattern, 8);

            TIFFContext *c;
            init_tiff_ctx(&c, dst, width, height, bitcount, samples_per_pixel, photo_interp);
            write_tiff(c, outfile);
        }
    } else if (!strcmp(fext, "bmp")) {
        int width, height, bitcount;
        bmp_read(&src, &width, &height, &bitcount, file);
        if (bitcount == 24) {
            //sprintf(outfile, "%s_test.bmp", filename(file));
            //bmp_write(outfile, src, width, height, 24);

            uint8_t *cfa;
            create_cfa_gbrg(&cfa, src, width, height);

            int cfa_pattern = 0x01000201;
            demosaicing(&dst, cfa, width, height, cfa_pattern, 8);

            bmp_write(outfile, dst, width, height, 24);
        }
    } else if (!strcmp(fext, "dng") || !strcmp(fext, "DNG")) {
        sprintf(outfile, "%s_debayer.tiff", fname);
        TIFFContext *c;
        parse_tiff(&c, file);

        #if DEBUG_MODE
        int fd = 0;
        if (c->new_subfile_type == 0) {
            if ((fd = open("raw.bin", O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1) {
                fprintf(stderr, "Error: failed to create file \"bayer.bin\"\n");
                return -1;
            }
            if ((ret = write(fd, c->image_data, c->image_width * c->image_length * c->bits_per_sample[0] / 8)) != c->image_size) {
                fprintf(stderr, "Error: failed to write bayer data\n");
                return -1;
            }
            close(fd);
        }
        #endif

        if (c->samples_per_pixel == 3 && c->photo_interp == 32803) {
            int cfa_pattern = *((int*)c->cfa_pattern);
            demosaicing(&dst, c->image_data, c->image_width, c->image_length, cfa_pattern, c->bits_per_sample[0]);

            TIFFContext *ctx;
            init_tiff_ctx(&ctx, dst, c->image_width, c->image_length, c->bitcount, c->samples_per_pixel, 2);
            write_tiff(ctx, outfile);
        }
    }
}

int main(int argc, char **argv) {
    int i, ret;

	if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n"
                "Example program to show how to detect the edge of an image.\n", argv[0]);
        exit(1);
    }
    demosaic_test(argv[1]);

	printf("succeed to process picture.\n");
	
    return 0;
}
