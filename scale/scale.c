#include "scale.h"
#include "bits.h"
#include "utils.h"

float bilinear_calculator(float x)
{
    x = x < 0 ? -x : x;
    if (x >= 0 && x <= 1)
        return 1 - x;
    else return 0;
}
int scale_bilinear_8bit(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float w, wx, wy, v, vx, vy;
    uint8_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = (i + 0.5) * src_width / dst_width - 0.5;
            y = (j + 0.5) * src_height / dst_height - 0.5;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;
            for (k = 0; k < 3; ++k) {
                w = v = 0;
                for (s = 0; s < 2; ++s) {
                    for (r = 0; r < 2; ++r) {
                        src_x = x_int + r;
                        src_y = y_int + s;
                        if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                        wx = bilinear_calculator(src_x - x);
                        wy = bilinear_calculator(src_y - y);
                        w += wx * wy;
                        v += wx * wy * src[(src_y * src_width + src_x) * 3 + k];
                        //fprintf(stdout, "src[%d, %d][%d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, k, src[(src_y * src_width + src_x) * 3 + k], wx, wy);
                    }
                }
                p[(j * dst_width + i) * 3 + k] = v / w;
                //fprintf(stdout, "v: %.2f, w: %.2f, dst[%d, %d][%d]: %d\n", v, w, i, j, k, p[(j * dst_width + i) * 3 + k]);
            }
        }
    }
}
int scale_bilinear_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height, int sample_bits)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float w, wx, wy, v, vx, vy;
    uint16_t *p;
    int max_val = (1 << sample_bits) - 1;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = (i + 0.5) * src_width / dst_width - 0.5;
            y = (j + 0.5) * src_height / dst_height - 0.5;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;
            for (k = 0; k < 3; ++k) {
                w = v = 0;
                for (s = 0; s < 2; ++s) {
                    for (r = 0; r < 2; ++r) {
                        src_x = x_int + r;
                        src_y = y_int + s;
                        if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                        wx = bilinear_calculator(src_x - x);
                        wy = bilinear_calculator(src_y - y);
                        w += wx * wy;
                        v += wx * wy * src[(src_y * src_width + src_x) * 3 + k];
                        //fprintf(stdout, "src[%d, %d][%d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, k, src[(src_y * src_width + src_x) * 3 + k], wx, wy);
                    }
                }
                //p[(j * dst_width + i) * 3 + k] = CLIP(v / w, 0, max_val);
                p[(j * dst_width + i) * 3 + k] = CLIP(v, 0, max_val);
                //fprintf(stdout, "v: %.2f, w: %.2f, dst[%d, %d][%d]: %d\n", v, w, i, j, k, p[(j * dst_width + i) * 3 + k]);
            }
        }
    }
}
int scale_bilinear_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height, int sample_bits)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float w, wx, wy, v, vx, vy;
    uint32_t *p;
    int max_val = (1 << sample_bits) - 1;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = (i + 0.5) * src_width / dst_width - 0.5;
            y = (j + 0.5) * src_height / dst_height - 0.5;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;
            for (k = 0; k < 3; ++k) {
                w = v = 0;
                for (s = 0; s < 2; ++s) {
                    for (r = 0; r < 2; ++r) {
                        src_x = x_int + r;
                        src_y = y_int + s;
                        if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                        wx = bilinear_calculator(src_x - x);
                        wy = bilinear_calculator(src_y - y);
                        w += wx * wy;
                        v += wx * wy * src[(src_y * src_width + src_x) * 3 + k];
                        //fprintf(stdout, "src[%d, %d][%d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, k, src[(src_y * src_width + src_x) * 3 + k], wx, wy);
                    }
                }
                //p[(j * dst_width + i) * 3 + k] = CLIP(v / w, 0, max_val);
                p[(j * dst_width + i) * 3 + k] = CLIP(v, 0, max_val);
                //fprintf(stdout, "v: %.2f, w: %.2f, dst[%d, %d][%d]: %d\n", v, w, i, j, k, p[(j * dst_width + i) * 3 + k]);
            }
        }
    }
}
int scale_bilinear(uint8_t **dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height, int sample_bits, int data_convertion_needed)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    
    if (sample_bits == 8) {
        *dst = (uint8_t*)malloc(dst_width * dst_height * 3);
        scale_bilinear_8bit(*dst, dst_width, dst_height, src, src_width, src_height);
    } else if (sample_bits == 10 || sample_bits == 12 || sample_bits == 14 || sample_bits == 16) {
        uint16_t *src_bit16, *dst_bit16;
        if (data_convertion_needed) {
            if (sample_bits == 10) convert_10bit_to_16bit(&src_bit16, src, src_width, src_height);
            else if (sample_bits == 12) convert_12bit_to_16bit(&src_bit16, src, src_width, src_height);
            else if (sample_bits == 14) convert_14bit_to_16bit(&src_bit16, src, src_width, src_height);
            else src_bit16 = (uint16_t*)src;
        } else {
            src_bit16 = (uint16_t*)src;
        }

        dst_bit16 = (uint16_t*)malloc(dst_width * dst_height * 3 * sizeof(uint16_t));
        scale_bilinear_16bit(dst_bit16, dst_width, dst_height, src_bit16, src_width, src_height, sample_bits);

        if (data_convertion_needed) {
            if (sample_bits == 10) convert_16bit_to_10bit(dst, dst_bit16, dst_width, dst_height);
            else if (sample_bits == 12) convert_16bit_to_12bit(dst, dst_bit16, dst_width, dst_height);
            else if (sample_bits == 14) convert_16bit_to_14bit(dst, dst_bit16, dst_width, dst_height);
            else *dst = (uint8_t*)dst_bit16;
        } else {
            *dst = (uint8_t*)dst_bit16;
        }
    } else if (sample_bits == 24 || sample_bits == 32) {
        uint32_t *src_bit32, *dst_bit32;
        if (data_convertion_needed) {
            if (sample_bits == 24) convert_24bit_to_32bit(&src_bit32, src, src_width, src_height);
            else src_bit32 = (uint32_t*)src;
        } else {
            src_bit32 = (uint32_t*)src;
        }
        
        dst_bit32 = (uint32_t*)malloc(dst_width * dst_height * 3 * sizeof(uint32_t));
        scale_bilinear_32bit(dst_bit32, dst_width, dst_height, src_bit32, src_width, src_height, sample_bits);

        if (data_convertion_needed) {
            if (sample_bits == 24) convert_32bit_to_24bit(dst, dst_bit32, src_width, src_height);
            else *dst = (uint8_t*)dst_bit32;
        } else {
            *dst = (uint8_t*)dst_bit32;
        }
    }
}

float bicubic_calculator(float x)
{
    x = x < 0 ? -x : x;
    if (x >= 0 && x <= 1)
        return 0.5 * x * x * x - x * x + 2.0 / 3;
    else if (x > 1 && x <= 2)
        return (2 - x) * (2 - x) * (2 - x) / 6;
    else return 0;
}
float bicubic_calculator2(float x)
{
    x = x < 0 ? -x : x;
    if (x >= 0 && x <= 1)
        return 1.5 * x * x * x - 2.5 * x * x + 1;
    else if (x > 1 && x <= 2)
        return -0.5 * x * x * x + 2.5 * x * x - 4 * x + 2;
    else return 0;
}
int scale_bicubic_8bit(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float w, wx, wy, v, vx, vy;
    uint8_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;
            for (k = 0; k < 3; ++k) {
                w = v = 0;
                for (s = -1; s < 3; ++s) {
                    for (r = -1; r < 3; ++r) {
                        src_x = x_int + r;
                        src_y = y_int + s;
                        if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                        //wx = bicubic_calculator(src_x - x);
                        //wy = bicubic_calculator(src_y - y);
                        wx = bicubic_calculator2(src_x - x);
                        wy = bicubic_calculator2(src_y - y);
                        w += wx * wy;
                        v += wx * wy * src[(src_y * src_width + src_x) * 3 + k];
                        //fprintf(stdout, "src[%d, %d][%d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, k, src[(src_y * src_width + src_x) * 3 + k], wx, wy);
                    }
                }
                //p[(j * dst_width + i) * 3 + k] = CLIP255(v / w);
                p[(j * dst_width + i) * 3 + k] = CLIP255(v);
                //fprintf(stdout, "v: %.2f, w: %.2f, dst[%d, %d][%d]: %d\n", v, w, i, j, k, p[(j * dst_width + i) * 3 + k]);
            }
        }
    }
}
int scale_bicubic_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height, int sample_bits)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float w, wx, wy, v, vx, vy;
    uint16_t *p;
    int max_val = (1 << sample_bits) - 1;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;
            for (k = 0; k < 3; ++k) {
                w = v = 0;
                for (s = -1; s < 3; ++s) {
                    for (r = -1; r < 3; ++r) {
                        src_x = x_int + r;
                        src_y = y_int + s;
                        if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                        //wx = bicubic_calculator(src_x - x);
                        //wy = bicubic_calculator(src_y - y);
                        wx = bicubic_calculator2(src_x - x);
                        wy = bicubic_calculator2(src_y - y);
                        w += wx * wy;
                        v += wx * wy * src[(src_y * src_width + src_x) * 3 + k];
                        //fprintf(stdout, "src[%d, %d][%d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, k, src[(src_y * src_width + src_x) * 3 + k], wx, wy);
                    }
                }
                //p[(j * dst_width + i) * 3 + k] = CLIP(v / w, 0, max_val);
                p[(j * dst_width + i) * 3 + k] = CLIP(v, 0, max_val);
                //fprintf(stdout, "v: %.2f, w: %.2f, dst[%d, %d][%d]: %d\n", v, w, i, j, k, p[(j * dst_width + i) * 3 + k]);
            }
        }
    }
}
int scale_bicubic_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height, int sample_bits)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float w, wx, wy, v, vx, vy;
    uint32_t *p;
    int max_val = (1 << sample_bits) - 1;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;
            for (k = 0; k < 3; ++k) {
                w = v = 0;
                for (s = -1; s < 3; ++s) {
                    for (r = -1; r < 3; ++r) {
                        src_x = x_int + r;
                        src_y = y_int + s;
                        if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                        //wx = bicubic_calculator(src_x - x);
                        //wy = bicubic_calculator(src_y - y);
                        wx = bicubic_calculator2(src_x - x);
                        wy = bicubic_calculator2(src_y - y);
                        w += wx * wy;
                        v += wx * wy * src[(src_y * src_width + src_x) * 3 + k];
                        //fprintf(stdout, "src[%d, %d][%d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, k, src[(src_y * src_width + src_x) * 3 + k], wx, wy);
                    }
                }
                //p[(j * dst_width + i) * 3 + k] = CLIP(v / w, 0, max_val);
                p[(j * dst_width + i) * 3 + k] = CLIP(v, 0, max_val);
                //fprintf(stdout, "v: %.2f, w: %.2f, dst[%d, %d][%d]: %d\n", v, w, i, j, k, p[(j * dst_width + i) * 3 + k]);
            }
        }
    }
}
int scale_bicubic(uint8_t **dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height, int sample_bits, int data_convertion_needed)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;

    if (sample_bits == 8) {
        *dst = (uint8_t*)malloc(dst_width * dst_height * 3);
        scale_bicubic_8bit(*dst, dst_width, dst_height, src, src_width, src_height);
    } else if (sample_bits == 10 || sample_bits == 12 || sample_bits == 14 || sample_bits == 16) {
        uint16_t *src_bit16, *dst_bit16;
        if (data_convertion_needed) {
            if (sample_bits == 10) convert_10bit_to_16bit(&src_bit16, src, src_width, src_height);
            else if (sample_bits == 12) convert_12bit_to_16bit(&src_bit16, src, src_width, src_height);
            else if (sample_bits == 14) convert_14bit_to_16bit(&src_bit16, src, src_width, src_height);
            else src_bit16 = (uint16_t*)src;
        } else {
            src_bit16 = (uint16_t*)src;
        }

        dst_bit16 = (uint16_t*)malloc(dst_width * dst_height * 3 * sizeof(uint16_t));
        scale_bicubic_16bit(dst_bit16, dst_width, dst_height, src_bit16, src_width, src_height, sample_bits);

        if (data_convertion_needed) {
            if (sample_bits == 10) convert_16bit_to_10bit(dst, dst_bit16, dst_width, dst_height);
            else if (sample_bits == 12) convert_16bit_to_12bit(dst, dst_bit16, dst_width, dst_height);
            else if (sample_bits == 14) convert_16bit_to_14bit(dst, dst_bit16, dst_width, dst_height);
            else *dst = (uint8_t*)dst_bit16;
        } else {
            *dst = (uint8_t*)dst_bit16;
        }
    } else if (sample_bits == 24 || sample_bits == 32) {
        uint32_t *src_bit32, *dst_bit32;
        if (data_convertion_needed) {
            if (sample_bits == 24) convert_24bit_to_32bit(&src_bit32, src, src_width, src_height);
            else src_bit32 = (uint32_t*)src;
        } else {
            src_bit32 = (uint32_t*)src;
        }
        
        dst_bit32 = (uint32_t*)malloc(dst_width * dst_height * 3 * sizeof(uint32_t));
        scale_bicubic_32bit(dst_bit32, dst_width, dst_height, src_bit32, src_width, src_height, sample_bits);

        if (data_convertion_needed) {
            if (sample_bits == 24) convert_32bit_to_24bit(dst, dst_bit32, src_width, src_height);
            else *dst = (uint8_t*)dst_bit32;
        } else {
            *dst = (uint8_t*)dst_bit32;
        }
    }
}

int scale(uint8_t **dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height, int sample_bits, int data_convertion_needed, int calculator)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;

    if (calculator == 0) {
        scale_bilinear(dst, dst_width, dst_height, src, src_width, src_height, sample_bits, data_convertion_needed);
    } else if (calculator == 1) {
        scale_bicubic(dst, dst_width, dst_height, src, src_width, src_height, sample_bits, data_convertion_needed);
    }

    fprintf(stderr, "Scale rgb succeeded.\n");
}
