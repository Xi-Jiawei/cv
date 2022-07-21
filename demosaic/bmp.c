#include "bmp.h"

int bmp_read(uint8_t **data, int *width, int *height, int *bitcount, char *filename)
{
	BITMAPFILEHEADER file_header;
	BITMAPINFOHEADER info_header;
	RGBQUAD color_table[256];
	int fd;
    struct stat st;
    int size, offset = 0;
    int i, j, bits, ret;
    uint8_t *buffer, *p;
    int linebytes, linesize;

	if ((fd = open(filename, O_RDONLY, 0666)) == -1) {
		fprintf(stderr, "Error: failed to open file \"%s\"\n", filename);
		return -1;
	}
	// bitmap file header固定长度是14字节
	if ((size = read(fd, &file_header, sizeof(BITMAPFILEHEADER))) != sizeof(BITMAPFILEHEADER)) {
		fprintf(stderr, "Error: failed to read bitmap file header of file \"%s\"\n", filename);
		return -1;
	}
	offset += size;
	if ((stat(filename, &st) < 0) || file_header.filesize > st.st_size) {
		fprintf(stderr, "Error: filesize in bitmap file header beyond actual size of file \"%s\"\n", filename);
		return -1;
	}
	// bitmap info header默认长度是40字节，实际情况可能大于40字节
	if ((size = read(fd, &info_header, sizeof(BITMAPINFOHEADER))) != sizeof(BITMAPINFOHEADER)) {
		fprintf(stderr, "Error: failed to read bitmap info header of file \"%s\"\n", filename);
		return -1;
	}
	offset += info_header.infosize;
	lseek(fd, offset, SEEK_SET);
	if (offset != file_header.offset) {
		if (file_header.offset - offset != sizeof(RGBQUAD) * 256) {
			fprintf(stderr, "Error: format wrong in bitmap header of file \"%s\"\n", filename);
			return -1;
		}
		if ((size = read(fd, color_table, sizeof(RGBQUAD) * 256)) != sizeof(RGBQUAD) * 256) {
			fprintf(stderr, "Error: failed to read color table of file \"%s\"\n", filename);
			return -1;
		}
		offset += size;
		if (info_header.bitcount >= 16) fprintf(stderr, "Warning: color table redundancy\n");
	}
    linebytes = (info_header.width * info_header.bitcount + 7) >> 3;
    linesize = ((linebytes + 3) >> 2) << 2; // 位图的行数据是4字节对齐的
	if (info_header.size < linesize * info_header.height || offset + info_header.size > file_header.filesize) {
		fprintf(stderr, "Error: format wrong in file \"%s\"\n", filename);
		return -1;
	}
	buffer = (uint8_t*)malloc(info_header.size);
	if ((size = read(fd, buffer, info_header.size)) != info_header.size) {
		fprintf(stderr, "Error: failed to read bitmap image data of file \"%s\"\n", filename);
		return -1;
	}
	close(fd);

    *width = info_header.width;
    *height = info_header.height;
    *bitcount = 24; // info_header.bitcount

    // bmp data to rgb
    linebytes = ((*width) * (*bitcount) + 7) >> 3;
	*data = (uint8_t*)malloc(linebytes * (*height));
    p = *data;
	for (j = 0; j < *height; ++j, p += linebytes) {
        for (i = 0; i < *width; ++i) {
            bits = j * linesize * 8 + i * info_header.bitcount;
            if (bits & 7) {

            } else {
                p[i * 3] = buffer[(bits >> 3) + 2];     // r
                p[i * 3 + 1] = buffer[(bits >> 3) + 1]; // g
                p[i * 3 + 2] = buffer[(bits >> 3)];     // b
            }
        }
    }

    fprintf(stderr, "Extract data succeeded from the bitmap.\n");

end:
	free(buffer);

    return 0;
}

int bmp_write(char *filename, uint8_t *rgb, int width, int height, int bitcount)
{
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    RGBQUAD color_table[256];
    int fd;
    int linebytes, linesize, size;
    int i, j, ret;
    uint8_t *data, *p, *q;

    if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1) {
        fprintf(stderr, "Error: failed to create file \"%s\"\n", filename);
        return -1;
    }

    linebytes = (width * bitcount + 7) >> 3;
    linesize = ((linebytes + 3) >> 2) << 2; // 位图的行数据是4字节对齐的
    size = linesize * height;

    file_header.type         = 0x4d42; // "MB"，小端存储为"BM"
    if (bitcount < 16) {
        file_header.filesize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256 + linesize * height;
        file_header.offset   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;
    } else {
        file_header.filesize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + linesize * height;
        file_header.offset   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    }
    file_header.reserved1    = 0;
    file_header.reserved2    = 0;
    if ((ret = write(fd, &file_header, sizeof(BITMAPFILEHEADER))) != sizeof(BITMAPFILEHEADER)) {
        fprintf(stderr, "Error: failed to write bitmap file header into file \"%s\"\n", filename);
        return -1;
    }

    info_header.infosize          = sizeof(BITMAPINFOHEADER); // Bitmap Info Header大小
    info_header.width             = width;
    info_header.height            = height;
    info_header.planes            = 1; // 固定为1
    info_header.bitcount          = bitcount < 24 ? bitcount : 24; // 32位以24位存储，舍弃alpha信息
    info_header.compression       = 0; // 是否压缩
    info_header.size              = size; // 图像大小，size = linesize * height
    info_header.xpixels_per_meter = 0; // x轴分辨率
    info_header.ypixels_per_meter = 0; // y轴分辨率
    info_header.color_used        = 0; // 实际用到的颜色数
    info_header.color_important   = 0; // 重要的颜色数，如果该值为零，则认为所有的颜色都是重要的
    if ((ret = write(fd, &info_header, sizeof(BITMAPINFOHEADER))) != sizeof(BITMAPINFOHEADER)) {
        fprintf(stderr, "Error: failed to write bitmap info header into file \"%s\"\n", filename);
        return -1;
    }

    if (bitcount < 16) {
        memset(color_table, 0, sizeof(RGBQUAD) * 256);
        for(int i = 0; i < 256; ++i)
            color_table[i].red = color_table[i].green = color_table[i].blue = i;
        if ((ret = write(fd, color_table, sizeof(RGBQUAD) * 256)) != sizeof(RGBQUAD) * 256) {
            fprintf(stderr, "Error: failed to write color table into file \"%s\"\n", filename);
            return -1;
        }
    }

    // rgb to bmp data
    data = (uint8_t*)malloc(linesize * height);
    if (bitcount == 24 || bitcount == 32) {
        for (j = 0, p = data; j < height; ++j, p += linesize) {
            for (i = 0; i < width; ++i) {
                p[i * 3] = rgb[linebytes * j + i * bitcount / 8 + 2];     // b
                p[i * 3 + 1] = rgb[linebytes * j + i * bitcount / 8 + 1]; // g
                p[i * 3 + 2] = rgb[linebytes * j + i * bitcount / 8];     // r
            }
        }
    } else {
        for (j = 0, p = data, q = rgb; j < height; ++j, p += linesize, q += linebytes)
            memcpy(p, q, linebytes);
    }

    if ((ret = write(fd, data, size)) != size) {
        fprintf(stderr, "Error: failed to write bitmap image data into file \"%s\"\n", filename);
        return -1;
    }
    close(fd);

    fprintf(stderr, "Write bitmap succeeded.\n");

end:
    free(data);

    return 0;
}
