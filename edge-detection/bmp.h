#ifndef __BMP_H
#define __BMP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/* BMP格式存储规则
1. 在windows中，颜色顺序是BGR；
2. BMP的内存行顺序和图像显示的行顺序是上下颠倒的。即：BMP内存第0行，是真实图像下面的最后一行。
   例，假如图像为m*n大小，像素三颜色按照RGB的顺序， 我们看到的图像为：
   r00 g00 b00， r01 g01 b01
   r10 g10 b10， r11 g11 b11
   …
   r(n-1)0 g(n-1)0 b(n-1)0， r(n-1)1 g(n-1)1 b(n-1)1
   内存表示如下：
   b(n-1)0 g(n-1)0 r(n-1)0， b(n-1)1 g(n-1)1 r(n-1)1
   …
   b10 g10 g10， b11 g11 r11
   b00 g00 g00， b01 g01 r01*/

typedef struct __BITMAPFILEHEADER {
	uint16_t type;              // 位图类型，固定为"BM"
	uint32_t filesize;            // 文件大小
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct __BITMAPINFOHEADER {
	uint32_t infosize;            // Bitmap Info Header大小
	int32_t width;
	int32_t height;
	uint16_t planes;            // 固定为1
	uint16_t bitcount;
	uint32_t compression;       // 是否压缩
	uint32_t size;              // 图像大小，size = linesize * height
	int32_t xpixels_per_meter;  // x轴分辨率
	int32_t ypixels_per_meter;  // y轴分辨率
	uint32_t color_used;        // 实际用到的颜色数
	uint32_t color_important;   // 重要的颜色数，如果该值为零，则认为所有的颜色都是重要的
} __attribute__((packed)) BITMAPINFOHEADER;

typedef struct _RGBQUAD {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t reserved;
} RGBQUAD;

typedef struct __BITMAP {
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	RGBQUAD *colorTable;
	uint8_t *data;
} BITMAP;

int bmp_read(uint8_t **data, int *width, int *height, int *bitcount, char *filename);
int bmp_write(char *filename, uint8_t *rgb, int width, int height, int bitcount);

#endif /* __BMP_H */