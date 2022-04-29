#include "utils.h"
#include "bmp.h"
#include "tiff.h"
#include "scale.h"
#include "scaleraw.h"

#define DEBUG_MODE 1

int scale_test(char *file, int dst_width, int dst_height) {
    uint8_t *src, *dst;
    int fd;
    int i, ret;
    char outfile[255], *fname, *fext;

    fname = get_file_name(file);
    fext = get_file_ext(file);
    sprintf(outfile, "%s_%dx%d.%s", fname, dst_width, dst_height, fext);
    if (!strcmp(fext, "tiff") || !strcmp(fext, "tif")) {
        //TIFFDirectory *td = tiff_alloc_context();
        //tiff_parse(td, file);
        //tiff_rewrite(td, outfile);

        int w1, h1, bpp1, s1, phi1;
        int width, height, bitcount, samples_per_pixel, photo_interp;
        extract_tiff_data(&src, &width, &height, &bitcount, &samples_per_pixel, &photo_interp, file);
        if (samples_per_pixel == 3 && photo_interp == 2 && bitcount == samples_per_pixel * 8) {
            //sprintf(outfile, "%s_test.tif", fname);
            //TIFFContext *c;
            //init_tiff_ctx(&c, src, width, height, 24, 3, 2);
            //write_tiff(c, outfile);

            scale(&dst, dst_width, dst_height, src, width, height, 8, 1);

            TIFFContext *c;
            init_tiff_ctx(&c, dst, dst_width, dst_height, bitcount, samples_per_pixel, photo_interp);
            write_tiff(c, outfile);
        }
    } else if (!strcmp(fext, "bmp")) {
        int width, height, bitcount;
        bmp_read(&src, &width, &height, &bitcount, file);
        if (bitcount == 24) {
            //sprintf(outfile, "%s_test.bmp", filename(file));
            //bmp_write(outfile, src, width, height, 24);

            scale(&dst, dst_width, dst_height, src, width, height, 8, 1);

            bmp_write(outfile, dst, dst_width, dst_height, 24);
        }
    } else if (!strcmp(fext, "dng") || !strcmp(fext, "DNG")) {
        //TIFFDirectory *td = tiff_alloc_context();
        //tiff_parse(td, file);
        //tiff_rewrite(td, outfile);

        TIFFDirectory *td = tiff_alloc_context();
        tiff_parse(td, file);
        scaleraw(td, dst_width, dst_height);
        tiff_rewrite(td, outfile);
    }
}

int main(int argc, char **argv) {
    int i, ret;
    int dst_width, dst_height;

	if (argc < 3) {
        fprintf(stderr, "Usage: %s file w:h\n"
                "Example program to show how to detect the edge of an image.\n", argv[0]);
        exit(1);
    }
    sscanf(argv[2], "%d:%d", &dst_width, &dst_height);
    scale_test(argv[1], dst_width, dst_height);

	printf("Scale picture succeeded.\n");
	
    return 0;
}
