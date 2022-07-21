#ifndef __DEMOSAIC_H
#define __DEMOSAIC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int rgb2bayer(uint8_t **dst, uint8_t *src, int width, int height, int bitcount/* = 24*/);
int create_cfa_rggb(uint8_t **dst, uint8_t *src, int width, int height);
int create_cfa_gbrg(uint8_t **dst, uint8_t *src, int width, int height);
int demosaicing_bilinear_gbrg(uint8_t *dst, uint8_t *src, int width, int height);
int demosaicing_gradiant_gbrg(uint8_t *dst, uint8_t *src, int width, int height);
int demosaicing_wang_gbrg(uint8_t *dst, uint8_t *src, int width, int height);
int demosaicing_wang_gbrg2(uint8_t *dst, uint8_t *src, int width, int height);
int demosaicing(uint8_t **dst, uint8_t *src, int width, int height, int cfa_pattern, int sample_bits);

#endif /* __DEMOSAIC_H */