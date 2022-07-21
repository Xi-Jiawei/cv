#include "demosaic.h"
#include "utils.h"

#define ABS(a) (a < 0 ? -(a) : a)
#define MIN(a,b) (a < b ? a : b)
#define MAX(a,b) (a > b ? a : b)

#define CLAMP255(v) (v) < 0 ? 0 : ((v) > 255 ? 255 : (v))
#define CLIP255(v) (v) < 0 ? 0 : ((v) > 255 ? 255 : (v))

static uint8_t *padding_matrix(uint8_t *src, int width, int height, int samples_per_pixel, int pad)
{
    uint8_t *dst;
    int padding_width, padding_height;
    int linesize, padding_linesize;
    int i, j, ret;
    uint8_t *p, *q;

    padding_width = width + pad * 2;
    padding_height = height + pad * 2;
    linesize = width * samples_per_pixel;
    padding_linesize = padding_width * samples_per_pixel;

    dst = (uint8_t*)calloc(padding_width * padding_height * samples_per_pixel, sizeof(uint8_t));
    for (i = 0, p = dst + (padding_width * pad + pad) * samples_per_pixel, q = src; i < height; ++i, p += padding_linesize, q += linesize)
        memcpy(p, q, linesize);

    fprintf(stderr, "Padding succeeded.\n");

    return dst;
}

static uint16_t *padding_matrix_16bit(uint16_t *src, int width, int height, int samples_per_pixel, int pad)
{
    uint16_t *dst;
    int padding_width, padding_height;
    int linesize, padding_linesize;
    int i, j, ret;
    uint16_t *p, *q;

    padding_width = width + pad * 2;
    padding_height = height + pad * 2;
    linesize = width * samples_per_pixel;
    padding_linesize = padding_width * samples_per_pixel;

    dst = (uint16_t*)calloc(padding_width * padding_height * samples_per_pixel, sizeof(uint16_t));
    for (i = 0, p = dst + (padding_width * pad + pad) * samples_per_pixel, q = src; i < height; ++i, p += padding_linesize, q += linesize)
        memcpy(p, q, linesize * sizeof(uint16_t));

    fprintf(stderr, "Padding succeeded.\n");

    return dst;
}

int rgb2bayer(uint8_t **dst, uint8_t *src, int width, int height, int bitcount/* = 24*/)
{
    int i, j, ret;
    uint8_t *p;

    // rgb转bayer
    *dst = (uint8_t*)malloc(width * height);
    p = *dst;
    for (j = 0; j < height; ++j, p += width) {
        for (i = 0; i < width; ++i) {
            p[i] = src[(width * j + i) * 3 + (!(j & 1) + (i & 1))]; // 第0行: gbgb...; 第1行: rgrg...
        }
    }

    fprintf(stderr, "Transform rgb to bayer succeeded.\n");

    return 0;
}

int create_cfa_rggb(uint8_t **dst, uint8_t *src, int width, int height)
{
    int i, j, ret;
    uint8_t *p, *q;

    *dst = (uint8_t*)malloc(width * height);
    p = *dst;
    q = src;

    // r
    for (j = 0; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[width * j + i] = q[(width * j + i) * 3];
        }
    }

    // g
    for (j = 0; j < height; j += 2) {
        for (i = 1; i < width; i += 2) {
            p[width * j + i] = q[(width * j + i) * 3 + 1];
        }
    }
    for (j = 1; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[width * j + i] = q[(width * j + i) * 3 + 1];
        }
    }

    // b
    for (j = 1; j < height; j += 2) {
        for (i = 1; i < width; i += 2) {
            p[width * j + i] = q[(width * j + i) * 3 + 2];
        }
    }
    
    fprintf(stderr, "Transform rgb to bayer succeeded.\n");

    return 0;
}

int create_cfa_gbrg(uint8_t **dst, uint8_t *src, int width, int height)
{
    int i, j, ret;
    uint8_t *p, *q;

    *dst = (uint8_t*)malloc(width * height);
    p = *dst;
    q = src;

    // r
    for (j = 1; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[width * j + i] = q[(width * j + i) * 3];
        }
    }

    // g
    for (j = 0; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[width * j + i] = q[(width * j + i) * 3 + 1];
        }
    }
    for (j = 1; j < height; j += 2) {
        for (i = 1; i < width; i += 2) {
            p[width * j + i] = q[(width * j + i) * 3 + 1];
        }
    }

    // b
    for (j = 0; j < height; j += 2) {
        for (i = 1; i < width; i += 2) {
            p[width * j + i] = q[(width * j + i) * 3 + 2];
        }
    }
    
    fprintf(stderr, "Transform rgb to bayer succeeded.\n");

    return 0;
}

#define SRC(x, y) src_padding[(y) * width_padding + (x)] // 注意: 别写成src_padding[y * width_padding + x]
#define WEIGHT(x, y) weights[(y) * width_padding + x]
#define WEIGHTED_SRC(x, y) SRC(x, y) * WEIGHT(x, y)
#define DST_R(x, y) dst_padding[((y) * width_padding + (x)) * 3]
#define DST_G(x, y) dst_padding[((y) * width_padding + (x)) * 3 + 1]
#define DST_B(x, y) dst_padding[((y) * width_padding + (x)) * 3 + 2]

// bilinear demosaicing. pattern: 'gbrg'
int demosaicing_bilinear_gbrg(uint8_t *dst, uint8_t *src, int width, int height)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint8_t *src_padding, *dst_padding;
    int8_t *weights;
    uint8_t *p, *q;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, p = (uint8_t*)(weights + width_padding * padding + padding); j < height; ++j, p += width_padding)
        memset(p, 1, width);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // g b
            // r g

            // g0
            x = i + padding;
            y = j + padding;
            p[(width * j + i) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)));
            p[(width * j + i) * 3 + 1] = SRC(x, y);
            p[(width * j + i) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)));

            // b
            x = (i + 1) + padding;
            y = j + padding;
            p[(width * j + (i + 1)) * 3] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)));
            p[(width * j + (i + 1)) * 3 + 1] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1)));
            p[(width * j + (i + 1)) * 3 + 2] = SRC(x, y);

            // r
            x = i + padding;
            y = (j + 1) + padding;
            p[(width * (j + 1) + i) * 3] = SRC(x, y);
            p[(width * (j + 1) + i) * 3 + 1] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1)));
            p[(width * (j + 1) + i) * 3 + 2] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)));

            // g1
            x = (i + 1) + padding;
            y = (j + 1) + padding;
            p[(width * (j + 1) + (i + 1)) * 3] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)));
            p[(width * (j + 1) + (i + 1)) * 3 + 1] = SRC(x, y);
            p[(width * (j + 1) + (i + 1)) * 3 + 2] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)));
        }
    }

    if (width & 1) {
        for (j = 0, i = width - 1, p = dst + i * 3; j < height; j += 2) {
            // g0
            x = i + padding;
            y = j + padding;
            p[(width * j + i) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)));
            p[(width * j + i) * 3 + 1] = SRC(x, y);
            p[(width * j + i) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)));

            // r
            x = i + padding;
            y = (j + 1) + padding;
            p[(width * (j + 1) + i) * 3] = SRC(x, y);
            p[(width * (j + 1) + i) * 3 + 1] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1)));
            p[(width * (j + 1) + i) * 3 + 2] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)));
        }
    }

    if (height & 1) {
        for (i = 0, j = height - 1, p = dst + width * j * 3; i < width; i += 2) {
            // g0
            x = i + padding;
            y = j + padding;
            p[(width * j + i) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)));
            p[(width * j + i) * 3 + 1] = SRC(x, y);
            p[(width * j + i) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)));

            // b
            x = (i + 1) + padding;
            y = j + padding;
            p[(width * j + (i + 1)) * 3] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)));
            p[(width * j + (i + 1)) * 3 + 1] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1)));
            p[(width * j + (i + 1)) * 3 + 2] = SRC(x, y);
        }
    }

    if ((width & 1) && (height & 1)) {
        i = width - 1, j = height - 1;
        // g0
        x = i + padding;
        y = j + padding;
        p[(width * j + i) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)));
        p[(width * j + i) * 3 + 1] = SRC(x, y);
        p[(width * j + i) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)));
    }

    return 0;
}

// gradiant-based demosaicing. pattern: 'gbrg'
int demosaicing_gradiant_gbrg(uint8_t *dst, uint8_t *src, int width, int height)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint8_t *src_padding, *dst_padding;
    int8_t *weights;
    uint8_t *p, *q;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, p = (uint8_t*)(weights + width_padding * padding + padding); j < height; ++j, p += width_padding)
        memset(p, 1, width);

    // bayer转rgb
    int linebytes = width * 3;
    // calculate all g
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[(width * j + i) * 3 + 1] = src[width * j + i];
            p[(width * (j + 1) + (i + 1)) * 3 + 1] = src[width * (j + 1) + (i + 1)];
        }
    }
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int h_grad, v_grad;

            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }

            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
        }
    }

    // 扩展目的矩阵
    dst_padding = padding_matrix(dst, width, height, 3, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // g b
            // r g

            // g0
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));

            // b
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);

            // r at r
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));

            // r/g/b at g1
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));
        }
    }

    return 0;
}

// gradiant-based demosaicing. pattern: 'grbg'
int demosaicing_gradiant_grbg(uint8_t *dst, uint8_t *src, int width, int height)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint8_t *src_padding, *dst_padding;
    int8_t *weights;
    uint8_t *p, *q;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, p = (int8_t*)(weights + width_padding * padding + padding); j < height; ++j, p += width_padding)
        memset(p, 1, width);

    // bayer转rgb
    int linebytes = width * 3;
    // calculate all g
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[(width * j + i) * 3 + 1] = src[width * j + i];
            p[(width * (j + 1) + (i + 1)) * 3 + 1] = src[width * (j + 1) + (i + 1)];
        }
    }
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int h_grad, v_grad;

            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }

            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
        }
    }

    // 扩展目的矩阵
    dst_padding = padding_matrix(dst, width, height, 3, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // g b
            // r g

            // g0
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));

            // r
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));

            // b
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);

            // g1
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));
        }
    }

    return 0;
}

// gradiant-based demosaicing. pattern: 'rggb'
int demosaicing_gradiant_rggb(uint8_t *dst, uint8_t *src, int width, int height)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint8_t *src_padding, *dst_padding;
    int8_t *weights;
    uint8_t *p, *q;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, p = (int8_t*)(weights + width_padding * padding + padding); j < height; ++j, p += width_padding)
        memset(p, 1, width);

    // bayer转rgb
    int linebytes = width * 3;
    // calculate all g
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[(width * j + (i + 1)) * 3 + 1] = src[width * j + (i + 1)];
            p[(width * (j + 1) + i) * 3 + 1] = src[width * (j + 1) + i];
        }
    }
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int h_grad, v_grad;

            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }

            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
        }
    }

    // 扩展目的矩阵
    dst_padding = padding_matrix(dst, width, height, 3, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // r g
            // g b

            // r
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));

            // g0
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));

            // g1
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));

            // b
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);
        }
    }

    return 0;
}

// gradiant-based demosaicing. pattern: 'rggb'
int demosaicing_gradiant_bggr(uint8_t *dst, uint8_t *src, int width, int height)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint8_t *src_padding, *dst_padding;
    int8_t *weights;
    uint8_t *p, *q;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, p = (int8_t*)(weights + width_padding * padding + padding); j < height; ++j, p += width_padding)
        memset(p, 1, width);

    // bayer转rgb
    int linebytes = width * 3;
    // calculate all g
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[(width * j + (i + 1)) * 3 + 1] = src[width * j + (i + 1)];
            p[(width * (j + 1) + i) * 3 + 1] = src[width * (j + 1) + i];
        }
    }
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int h_grad, v_grad;

            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }

            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
        }
    }

    // 扩展目的矩阵
    dst_padding = padding_matrix(dst, width, height, 3, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // b g
            // g r

            // b
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);

            // g0
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));

            // g1
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));

            // r
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));
        }
    }

    return 0;
}

// gradiant-based demosaicing. pattern: 'gbrg'
int demosaicing_gradiant_gbrg_16bit(uint16_t *dst, uint16_t *src, int width, int height, int sample_bits)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint16_t *src_padding, *dst_padding;
    int8_t *weights, *w;
    uint16_t *p, *q;
    int val, max_val = (1 << sample_bits) - 1;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix_16bit(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, w = (int8_t*)(weights + width_padding * padding + padding); j < height; ++j, w += width_padding)
        memset(w, 1, width);

    // bayer转rgb
    int linebytes = width * 3;
    // calculate all g
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[(width * j + i) * 3 + 1] = src[width * j + i];
            p[(width * (j + 1) + (i + 1)) * 3 + 1] = src[width * (j + 1) + (i + 1)];
        }
    }
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int h_grad, v_grad;

            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }

            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
        }
    }

    // 扩展目的矩阵
    dst_padding = padding_matrix_16bit(dst, width, height, 3, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // g b
            // r g

            /// g0
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ g0
            val = (SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ g0
            val = (SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);

            /// b
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ b
            val = (SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ b
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);

            /// r
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ r
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            // b @ r
            val = (SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);

            /// g1
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ g1
            val = (SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ g1
            val = (SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);
        }
    }

    return 0;
}

// gradiant-based demosaicing. pattern: 'grbg'
int demosaicing_gradiant_grbg_16bit(uint16_t *dst, uint16_t *src, int width, int height, int sample_bits)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint16_t *src_padding, *dst_padding;
    int8_t *weights, *w;
    uint16_t *p, *q;
    int val, max_val = (1 << sample_bits) - 1;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix_16bit(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, w = (int8_t*)(weights + width_padding * padding + padding); j < height; ++j, w += width_padding)
        memset(w, 1, width);

    // bayer转rgb
    int linebytes = width * 3;
    // calculate all g
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[(width * j + i) * 3 + 1] = src[width * j + i];
            p[(width * (j + 1) + (i + 1)) * 3 + 1] = src[width * (j + 1) + (i + 1)];
        }
    }
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int h_grad, v_grad;

            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }

            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
        }
    }

    // 扩展目的矩阵
    dst_padding = padding_matrix_16bit(dst, width, height, 3, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // g r
            // b g

            /// g0
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ g0
            val = (SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ g0
            val = (SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);

            /// r
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ r
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            val = (SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y);
            // b @ r
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);

            /// b
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ b
            val = (SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ b
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);

            /// g1
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ g1
            val = (SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ g1
            val = (SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);
        }
    }

    return 0;
}

// gradiant-based demosaicing. pattern: 'rggb'
int demosaicing_gradiant_rggb_16bit(uint16_t *dst, uint16_t *src, int width, int height, int sample_bits)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint16_t *src_padding, *dst_padding;
    int8_t *weights;
    uint16_t *p, *q;
    int val, max_val = (1 << sample_bits) - 1;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix_16bit(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, p = (int8_t*)(weights + width_padding * padding + padding); j < height; ++j, p += width_padding)
        memset(p, 1, width);

    // bayer转rgb
    int linebytes = width * 3;
    // calculate all g
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[(width * j + (i + 1)) * 3 + 1] = src[width * j + (i + 1)];
            p[(width * (j + 1) + i) * 3 + 1] = src[width * (j + 1) + i];
        }
    }
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int h_grad, v_grad;

            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }

            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
        }
    }

    // 扩展目的矩阵
    dst_padding = padding_matrix_16bit(dst, width, height, 3, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // r g
            // g b

            /// r
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ r
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            val = (SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y);
            // b @ r
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);

            /// g0
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ g0
            val = (SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ g0
            val = (SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);

            /// g1
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ g1
            val = (SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ g1
            val = (SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);

            /// b
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ b
            val = (SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ b
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);
        }
    }

    return 0;
}

// gradiant-based demosaicing. pattern: 'rggb'
int demosaicing_gradiant_bggr_16bit(uint16_t *dst, uint16_t *src, int width, int height, int sample_bits)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint16_t *src_padding, *dst_padding;
    int8_t *weights;
    uint16_t *p, *q;
    int val, max_val = (1 << sample_bits) - 1;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix_16bit(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, p = (int8_t*)(weights + width_padding * padding + padding); j < height; ++j, p += width_padding)
        memset(p, 1, width);

    // bayer转rgb
    int linebytes = width * 3;
    // calculate all g
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[(width * j + (i + 1)) * 3 + 1] = src[width * j + (i + 1)];
            p[(width * (j + 1) + i) * 3 + 1] = src[width * (j + 1) + i];
        }
    }
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int h_grad, v_grad;

            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }

            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                p[(width * dst_y + dst_x) * 3 + 1] = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
        }
    }

    // 扩展目的矩阵
    dst_padding = padding_matrix_16bit(dst, width, height, 3, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // b g
            // g r

            /// b
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ b
            val = (SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ b
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);

            /// g0
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ g0
            val = (SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ g0
            val = (SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);

            /// g1
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ g1
            val = (SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3] = CLIP(val, 0, max_val);
            // b @ g1
            val = (SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);

            /// r
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r @ r
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            // b @ r
            val = (SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLIP(val, 0, max_val);
        }
    }

    return 0;
}

int demosaicing_wang_gbrg(uint8_t *dst, uint8_t *src, int width, int height)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint8_t *src_padding, *dst_padding;
    int8_t *weights;
    uint8_t *p, *q;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix(src, width, height, 1, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int topleft, topright, bottomleft, bottomright;
            int tl_br, tr_bl;
            int h_grad, v_grad;
            int val;

            // g b
            // r g

            // g0
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r/g/b at g0
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1)) / 2 + (SRC(x, y) * 2 - SRC(x, y - 2) - SRC(x, y + 2)) / 4);
            p[(width * dst_y + dst_x) * 3 + 1] = SRC(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y)) / 2 + (SRC(x, y) * 2 - SRC(x - 2, y) - SRC(x + 2, y)) / 4);

            // b
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // interpolate r at b
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1)) / 4 + (SRC(x, y) * 4 - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8);
            // interpolate g at b
            h_grad = ABS(SRC(x - 1, y) - SRC(x + 1, y)) + ABS(SRC(x, y) * 2 - SRC(x - 2, y) - SRC(x + 2, y));
            v_grad = ABS(SRC(x, y - 1) - SRC(x, y + 1)) + ABS(SRC(x, y) * 2 - SRC(x, y - 2) - SRC(x, y + 2));
            if (h_grad < v_grad) {
            	val = (SRC(x - 1, y) + SRC(x + 1, y)) / 2 + (SRC(x, y) * 2 - SRC(x - 2, y) - SRC(x + 2, y)) / 4;
            } else if (h_grad < v_grad) {
            	val = (SRC(x, y - 1) + SRC(x, y + 1)) / 2 + (SRC(x, y) * 2 - SRC(x, y - 2) - SRC(x, y + 2)) / 4;
            } else {
            	val = (SRC(x, y - 1) + SRC(x, y + 1) + SRC(x - 1, y) + SRC(x + 1, y)) / 4 + (SRC(x, y) * 4 - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8;
            }
            p[(width * dst_y + dst_x) * 3 + 1] = CLAMP255(val);
            // b at b
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);

            // r
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r at r
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            // interpolate g at r
            h_grad = ABS(SRC(x - 1, y) - SRC(x + 1, y)) + ABS(SRC(x, y) * 2 - SRC(x - 2, y) - SRC(x + 2, y));
            v_grad = ABS(SRC(x, y - 1) - SRC(x, y + 1)) + ABS(SRC(x, y) * 2 - SRC(x, y - 2) - SRC(x, y + 2));
            if (h_grad < v_grad) {
            	val = (SRC(x - 1, y) + SRC(x + 1, y)) / 2 + (SRC(x, y) * 2 - SRC(x - 2, y) - SRC(x + 2, y)) / 4;
            } else if (h_grad < v_grad) {
            	val = (SRC(x, y - 1) + SRC(x, y + 1)) / 2 + (SRC(x, y) * 2 - SRC(x, y - 2) - SRC(x, y + 2)) / 4;
            } else {
            	val = (SRC(x, y - 1) + SRC(x, y + 1) + SRC(x - 1, y) + SRC(x + 1, y)) / 4 + (SRC(x, y) * 4 - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8;
            }
            p[(width * dst_y + dst_x) * 3 + 1] = CLAMP255(val);
            // interpolate b at r
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1)) / 4 + (SRC(x, y) * 4 - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8);

            // r/g/b at g1
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y)) / 2 + (SRC(x, y) * 2 - SRC(x - 2, y) - SRC(x + 2, y)) / 4);
            p[(width * dst_y + dst_x) * 3 + 1] = SRC(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1)) / 2 + (SRC(x, y) * 2 - SRC(x, y - 2) - SRC(x, y + 2)) / 4);
        }
    }

    return 0;
}

int demosaicing_wang_gbrg2(uint8_t *dst, uint8_t *src, int width, int height)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint8_t *src_padding, *dst_padding;
    int8_t *weights;
    uint8_t *p, *q;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix(src, width, height, 1, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int topleft, topright, bottomleft, bottomright;
            int tl_br, tr_bl;
            int h_grad, v_grad;
            int val;

            // g b
            // r g

            // g0
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // r/g/b at g0
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1)) / 2 + (SRC(x, y) * 2 - SRC(x, y - 2) - SRC(x, y + 2)) / 4);
            p[(width * dst_y + dst_x) * 3 + 1] = SRC(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y)) / 2 + (SRC(x, y) * 2 - SRC(x - 2, y) - SRC(x + 2, y)) / 4);

            // b
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            // interpolate r at b
            topleft = SRC(x - 1, y - 1), topright = SRC(x + 1, y - 1), bottomleft = SRC(x - 1, y + 1), bottomright = SRC(x + 1, y + 1);
            tl_br = ABS(topleft - bottomright), tr_bl = ABS(topright - bottomleft);
            if (tl_br < tr_bl) {
            	val = (topleft + bottomright) / 2;
            } else if (tl_br > tr_bl) {
            	val = (topright + bottomleft) / 2;
            } else {
            	val = (topleft + bottomright + topright + bottomleft) / 4;
            }
            p[(width * dst_y + dst_x) * 3] = CLAMP255(val + (SRC(x, y) * 4 - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8);
            // interpolate g at b
            h_grad = ABS(SRC(x - 1, y) - SRC(x + 1, y));
            v_grad = ABS(SRC(x, y - 1) - SRC(x, y + 1));
            if (h_grad < v_grad) {
            	val = (SRC(x - 1, y) + SRC(x + 1, y)) / 2;
            } else if (h_grad < v_grad) {
            	val = (SRC(x, y - 1) + SRC(x, y + 1)) / 2;
            } else {
            	val = (SRC(x, y - 1) + SRC(x, y + 1) + SRC(x - 1, y) + SRC(x + 1, y)) / 4;
            }
            p[(width * dst_y + dst_x) * 3 + 1] = CLAMP255(val + (SRC(x, y) * 4 - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8);
            // b at b
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);

            // r
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r at r
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            // interpolate g at r
            h_grad = ABS(SRC(x - 1, y) - SRC(x + 1, y));
            v_grad = ABS(SRC(x, y - 1) - SRC(x, y + 1));
            if (h_grad < v_grad) {
            	val = (SRC(x - 1, y) + SRC(x + 1, y)) / 2;
            } else if (h_grad < v_grad) {
            	val = (SRC(x, y - 1) + SRC(x, y + 1)) / 2;
            } else {
            	val = (SRC(x, y - 1) + SRC(x, y + 1) + SRC(x - 1, y) + SRC(x + 1, y)) / 4;
            }
            p[(width * dst_y + dst_x) * 3 + 1] = CLAMP255(val + (SRC(x, y) * 4 - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8);
            // interpolate b at r
            topleft = SRC(x - 1, y - 1), topright = SRC(x + 1, y - 1), bottomleft = SRC(x - 1, y + 1), bottomright = SRC(x + 1, y + 1);
            tl_br = ABS(topleft - bottomright), tr_bl = ABS(topright - bottomleft);
            if (tl_br < tr_bl) {
            	val = (topleft + bottomright) / 2;
            } else if (tl_br > tr_bl) {
            	val = (topright + bottomleft) / 2;
            } else {
            	val = (topleft + bottomright + topright + bottomleft) / 4;
            }
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255(val + (SRC(x, y) * 4 - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8);

            // g1
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            // r/g/b at g1
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y)) / 2 + (SRC(x, y) * 2 - SRC(x - 2, y) - SRC(x + 2, y)) / 4);
            p[(width * dst_y + dst_x) * 3 + 1] = SRC(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1)) / 2 + (SRC(x, y) * 2 - SRC(x, y - 2) - SRC(x, y + 2)) / 4);
        }
    }

    return 0;
}

// demosaicing, gradiant-based && wang. pattern: 'gbrg'
int demosaicing_gradiant_wang_gbrg(uint8_t *dst, uint8_t *src, int width, int height)
{
    int i, j, x, y, dst_y, dst_x, pos, ret;
    int padding, width_padding, height_padding;
    uint8_t *src_padding, *dst_padding;
    int8_t *weights;
    uint8_t *p, *q;

    padding = 2;
    width_padding = width + padding * 2;
    height_padding = height + padding * 2;

    // 扩展源矩阵
    src_padding = padding_matrix(src, width, height, 1, padding);

    // 权值矩阵
    weights = (int8_t*)malloc(width_padding * height_padding);
    memset(weights, 0, width_padding * height_padding);
    for (j = 0, p = (uint8_t*)(weights + width_padding * padding + padding); j < height; ++j, p += width_padding)
        memset(p, 1, width);

    // bayer转rgb
    int linebytes = width * 3;
    // calculate all g
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            p[(width * j + i) * 3 + 1] = src[width * j + i];
            p[(width * (j + 1) + (i + 1)) * 3 + 1] = src[width * (j + 1) + (i + 1)];
        }
    }
    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int h_grad, v_grad;
            int val;

            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                val = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                val = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                val = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
            p[(width * dst_y + dst_x) * 3 + 1] = CLAMP255(val + (SRC(x, y) * (WEIGHT(x, y - 2) + WEIGHT(x, y + 2) + WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8);

            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            h_grad = ABS((SRC(x - 2, y) + SRC(x + 2, y)) / (WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y));
            v_grad = ABS((SRC(x, y - 2) + SRC(x, y + 2)) / (WEIGHT(x, y - 2) + WEIGHT(x, y + 2)) - SRC(x, y));
            if (h_grad < v_grad) {
                val = (SRC(x - 1, y) + SRC(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y));
            } else if (h_grad > v_grad) {
                val = (SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            } else {
                val = (SRC(x - 1, y) + SRC(x + 1, y) + SRC(x, y - 1) + SRC(x, y + 1)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y) + WEIGHT(x, y - 1) + WEIGHT(x, y + 1));
            }
            p[(width * dst_y + dst_x) * 3 + 1] = CLAMP255(val + (SRC(x, y) * (WEIGHT(x, y - 2) + WEIGHT(x, y + 2) + WEIGHT(x - 2, y) + WEIGHT(x + 2, y)) - SRC(x, y - 2) - SRC(x, y + 2) - SRC(x - 2, y) - SRC(x + 2, y)) / 8);
        }
    }

    // 扩展目的矩阵
    dst_padding = padding_matrix(dst, width, height, 3, padding);

    for (j = 0, p = dst; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            // g b
            // r g

            // g0
            dst_x = i;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));

            // b
            dst_x = i + 1;
            dst_y = j;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = SRC(x, y);

            // r at r
            dst_x = i;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = SRC(x, y);
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x - 1, y - 1) + SRC(x + 1, y - 1) + SRC(x - 1, y + 1) + SRC(x + 1, y + 1) - DST_G(x - 1, y - 1) - DST_G(x + 1, y - 1) - DST_G(x - 1, y + 1) - DST_G(x + 1, y + 1)) / (WEIGHT(x - 1, y - 1) + WEIGHT(x + 1, y - 1) + WEIGHT(x - 1, y + 1) + WEIGHT(x + 1, y + 1)) + DST_G(x, y));

            // r/g/b at g1
            dst_x = i + 1;
            dst_y = j + 1;
            x = dst_x + padding;
            y = dst_y + padding;
            p[(width * dst_y + dst_x) * 3] = CLAMP255((SRC(x - 1, y) + SRC(x + 1, y) - DST_G(x - 1, y) - DST_G(x + 1, y)) / (WEIGHT(x - 1, y) + WEIGHT(x + 1, y)) + DST_G(x, y));
            p[(width * dst_y + dst_x) * 3 + 2] = CLAMP255((SRC(x, y - 1) + SRC(x, y + 1) - DST_G(x, y - 1) - DST_G(x, y + 1)) / (WEIGHT(x, y - 1) + WEIGHT(x, y + 1)) + DST_G(x, y));
        }
    }

    return 0;
}

// demosaicing/demosaic/debayer, pattern: 'gbrg'
int demosaicing(uint8_t **dst, uint8_t *src, int width, int height, int cfa_pattern, int sample_bits)
{
    // bayer转rgb
    int linebytes = width * 3;
    if (sample_bits == 8) {
        *dst = (uint8_t*)malloc(linebytes * height);
        // calculate all g
        if (cfa_pattern == 0x01000201) { // gbrg
            //demosaicing_bilinear_gbrg(*dst, src, width, height);
            demosaicing_gradiant_gbrg(*dst, src, width, height);
            //demosaicing_wang_gbrg(*dst, src, width, height);
            //demosaicing_wang_gbrg2(*dst, src, width, height);
            //demosaicing_gradiant_wang_gbrg(*dst, src, width, height);
        } else if (cfa_pattern == 0x01020001) { // grbg
            demosaicing_gradiant_grbg(*dst, src, width, height);
        } else if (cfa_pattern == 0x02010100) { // rggb
            demosaicing_gradiant_rggb(*dst, src, width, height);
        } else if (cfa_pattern == 0x00010102) { // bggr
            demosaicing_gradiant_bggr(*dst, src, width, height);
        }
    } else if (sample_bits == 10 || sample_bits == 12 || sample_bits == 14 || sample_bits == 16) {
        uint16_t *src_bit16, *dst_bit16;
        if (sample_bits == 10) convert_10bit_to_16bit(&src_bit16, src, width, height);
        else if (sample_bits == 12) convert_12bit_to_16bit(&src_bit16, src, width, height);
        else if (sample_bits == 14) convert_14bit_to_16bit(&src_bit16, src, width, height);
        else src_bit16 = (uint16_t*)src;

        // calculate all g
        dst_bit16 = (uint16_t*)malloc(linebytes * height * sizeof(uint16_t));
        if (cfa_pattern == 0x01000201) { // gbrg
            demosaicing_gradiant_gbrg_16bit(dst_bit16, src_bit16, width, height, sample_bits);
        } else if (cfa_pattern == 0x01020001) { // grbg
            demosaicing_gradiant_grbg_16bit(dst_bit16, src_bit16, width, height, sample_bits);
        } else if (cfa_pattern == 0x02010100) { // rggb
            demosaicing_gradiant_rggb_16bit(dst_bit16, src_bit16, width, height, sample_bits);
        } else if (cfa_pattern == 0x00010102) { // bggr
            demosaicing_gradiant_bggr_16bit(dst_bit16, src_bit16, width, height, sample_bits);
        }

        if (sample_bits == 10) convert_16bit_to_10bit(dst, dst_bit16, width * 3, height);
        else if (sample_bits == 12) convert_16bit_to_12bit(dst, dst_bit16, width * 3, height);
        else if (sample_bits == 14) convert_16bit_to_14bit(dst, dst_bit16, width * 3, height);
        else *dst = (uint16_t*)dst_bit16;
    }
    
    fprintf(stderr, "Transform bayer to rgb succeeded.\n");

    return 0;
}
