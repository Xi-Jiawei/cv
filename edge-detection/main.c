#include "utils.h"
#include "bmp.h"
#include "tiff.h"
#include "edgedetect.h"

int main(int argc, char **argv) {
	uint8_t *src;
    int fd;
    int i, ret;
    char outfile[255], *fname, *fext;
    float scale_ratio = -1;

	if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n"
                "Example program to show how to detect the edge of an image.\n", argv[0]);
        exit(1);
    }
    fname = filename(argv[1]);
    fext = fileext(argv[1]);
    if (match_ext(fext, "tiff") || match_ext(fext, "tif") || match_ext(fext, "dng") || match_ext(fext, "DNG")) {
        int w1, h1, bpp1, s1, phi1;
        int width, height, bitcount, samples_per_pixel, photo_interp;
        extract_tiff_data(&src, &width, &height, &bitcount, &samples_per_pixel, &photo_interp, argv[1]);
        if (samples_per_pixel == 3 && photo_interp == 2 && bitcount == samples_per_pixel * 8) {
            //sprintf(outfile, "%s_test.tif", filename(argv[1]));
            //TIFFContext *c;
            //init_tiff_ctx(&c, src, width, height, 24, 3, 2);
            //write_tiff(c, outfile);

            uint8_t *gray, *gray_blur, *gray_blur_gradiant_nms, *gray_blur_gradiant_nms_edge;
            int16_t *gray_blur_gradiant;
            int16_t *dx, *dy;

            TIFFContext *c;
            init_tiff_ctx(&c, src, width, height, 8, 1, 1);

            rgb2gray(&gray, src, width, height);
            sprintf(outfile, "%s_gray.tif", fname);
            c->image_data = gray;
            write_tiff(c, outfile);

            gaussian_blur(&gray_blur, gray, width, height, 8, 1.8);
            sprintf(outfile, "%s_gray_blur.tif", fname);
            c->image_data = gray_blur;
            write_tiff(c, outfile);

            //gradient_laplacian(&gray_blur_gradiant, gray_blur, width, height, 1);
            //sprintf(outfile, "%s_gray_blur_gradiant.tif", fname);
            //uint8_t *gray_blur_gradiant_8bits;
            //reduce_16bit_to_8bit(&gray_blur_gradiant_8bits, gray_blur_gradiant, width * height);
            //c->image_data = gray_blur_gradiant_8bits;
            //write_tiff(c, outfile);
            //detect_sharp_edge(&gray_blur_gradiant_nms_edge, gray_blur_gradiant_8bits, width, height);
            //sprintf(outfile, "%s_gray_blur_gradiant_nms_edge.tif", fname);
            //c->image_data = gray_blur_gradiant_nms_edge;
            //write_tiff(c, outfile);
            //return;

            gradient_sobel(&gray_blur_gradiant, &dx, &dy, gray_blur, width, height);
            uint8_t *grad_8bits, *gradx_8bits, *grady_8bits;
            reduce_16bit_to_8bit(&grad_8bits, gray_blur_gradiant, width * height);
            c->image_data = grad_8bits;
            sprintf(outfile, "%s_gray_blur_gradiant.tif", fname);
            write_tiff(c, outfile);
            //reduce_16bit_to_8bit(&gradx_8bits, dx, width * height);
            //reduce_16bit_to_8bit(&grady_8bits, dy, width * height);
            //c->image_data = gradx_8bits;
            //sprintf(outfile, "%s_gray_blur_gradiantx.tif", fname);
            //write_tiff(c, outfile);
            //c->image_data = grady_8bits;
            //sprintf(outfile, "%s_gray_blur_gradianty.tif", fname);
            //write_tiff(c, outfile);

            gradient_nms(&gray_blur_gradiant_nms, gray_blur_gradiant, dx, dy, width, height);
            sprintf(outfile, "%s_gray_blur_gradiant_nms.tif", fname);
            c->image_data = gray_blur_gradiant_nms;
            write_tiff(c, outfile);

            detect_edge(&gray_blur_gradiant_nms_edge, gray_blur_gradiant_nms, width, height);
            sprintf(outfile, "%s_gray_blur_gradiant_nms_edge.tif", fname);
            c->image_data = gray_blur_gradiant_nms_edge;
            write_tiff(c, outfile);

            //detect_sharp_edge(&gray_blur_gradiant_nms_edge, gray_blur_gradiant_nms, width, height);
            //sprintf(outfile, "%s_gray_blur_gradiant_nms_edge.tif", fname);
            //c->image_data = gray_blur_gradiant_nms_edge;
            //write_tiff(c, outfile);
        }
    } else if (match_ext(fext, "bmp")) {
        int width, height, bitcount;
        bmp_read(&src, &width, &height, &bitcount, argv[1]);
        if (bitcount == 24) {
            //sprintf(outfile, "%s_test.bmp", filename(argv[1]));
            //bmp_write(outfile, src, width, height, 24);

            uint8_t *gray, *gray_blur, *gray_blur_gradiant_nms, *gray_blur_gradiant_nms_edge;
            int16_t *gray_blur_gradiant;
            int16_t *dx, *dy;

            rgb2gray(&gray, src, width, height);
            sprintf(outfile, "%s_gray.bmp", fname);
            bmp_write(outfile, gray, width, height, 8);

            if ((fd = open("gray.bin", O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1) {
                fprintf(stderr, "Error: failed to create file \"bayer.bin\"\n");
                return -1;
            }
            if ((ret = write(fd, gray, width * height)) != width * height) {
                fprintf(stderr, "Error: failed to write bayer data\n");
                return -1;
            }
            close(fd);

            gaussian_blur(&gray_blur, gray, width, height, 8, 1.8);
            sprintf(outfile, "%s_gray_blur.bmp", fname);
            bmp_write(outfile, gray_blur, width, height, 8);

            gradient_sobel(&gray_blur_gradiant, &dx, &dy, gray_blur, width, height);
            sprintf(outfile, "%s_gray_blur_gradiant.bmp", fname);
            uint8_t *gray_blur_gradiant_8bits;
            reduce_16bit_to_8bit(&gray_blur_gradiant_8bits, gray_blur_gradiant, width * height);
            bmp_write(outfile, gray_blur_gradiant_8bits, width, height, 8);

            gradient_nms(&gray_blur_gradiant_nms, gray_blur_gradiant, dx, dy, width, height);
            sprintf(outfile, "%s_gray_blur_gradiant_nms.bmp", fname);
            bmp_write(outfile, gray_blur_gradiant_nms, width, height, 8);

            detect_edge(&gray_blur_gradiant_nms_edge, gray_blur_gradiant_nms, width, height);
            sprintf(outfile, "%s_gray_blur_gradiant_nms_edge.bmp", fname);
            bmp_write(outfile, gray_blur_gradiant_nms_edge, width, height, 8);

            //detect_sharp_edge(&gray_blur_gradiant_nms_edge, gray_blur_gradiant_nms, width, height);
            //sprintf(outfile, "%s_gray_blur_gradiant_nms_edge.bmp", fname);
            //bmp_write(outfile, gray_blur_gradiant_nms_edge, width, height, 8);
        }
    }

	printf("succeed to process picture.\n");
	
    return 0;
}
