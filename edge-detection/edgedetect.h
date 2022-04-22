#ifndef __EDGEDETECT_H
#define __EDGEDETECT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define INT_MAX     ((int)(~0U>>1))
#define INT_MIN     (-INT_MAX - 1)

int padding(uint8_t **dst, uint8_t *src, int width, int height, int samples_per_pixel, int padding_size);
int rgb2gray(uint8_t **dst, uint8_t *src, int width, int height);
double *gaussian_matrix(int winsize, double sigma);
int gaussian_blur(uint8_t **dst, uint8_t *src, int width, int height, int bitcount, double sigma);
int gradient_simple(int16_t **dst, uint8_t *src, int width, int height);
int gradient_sobel(int16_t **dst, int16_t **dx, int16_t **dy, uint8_t *src, int width, int height);
int gradient_laplacian(int16_t **dst, uint8_t *src, int width, int height, int winsize, int has_diagonals);
int convert_scale_abs(uint8_t **dst, int16_t *src, int size);
int gradient_nms(uint8_t **dst, int16_t *src, int16_t *dx, int16_t *dy, int width, int height);
int detect_edge(uint8_t **dst, uint8_t *src, int width, int height);

#endif /* __EDGEDETECT_H */