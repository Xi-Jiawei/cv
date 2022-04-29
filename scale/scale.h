#ifndef __SCALE_H
#define __SCALE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int scale(uint8_t **dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height, int sample_bits, int calculator);

#endif /* __SCALE_H */