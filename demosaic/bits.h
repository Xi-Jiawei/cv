#ifndef __BITS_H
#define __BITS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int convert_10bit_to_16bit(uint16_t **dst, uint8_t *src, int width, int height);
int convert_12bit_to_16bit(uint16_t **dst, uint8_t *src, int width, int height);
int convert_14bit_to_16bit(uint16_t **dst, uint8_t *src, int width, int height);
int convert_16bit_to_10bit(uint8_t **dst, uint16_t *src, int width, int height);
int convert_16bit_to_12bit(uint8_t **dst, uint16_t *src, int width, int height);
int convert_16bit_to_14bit(uint8_t **dst, uint16_t *src, int width, int height);
int convert_24bit_to_32bit(uint32_t **dst, uint8_t *src, int width, int height);
int convert_32bit_to_24bit(uint8_t **dst, uint32_t *src, int width, int height);

#endif /* __BITS_H */