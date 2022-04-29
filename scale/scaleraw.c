#include "scaleraw.h"
#include "demosaic.h"
#include "scale.h"
#include "bits.h"

#define DEBUG_MODE 0

float bayer_linear(float x)
{
    x = x < 0 ? -x : x;
    if (x >= -2 && x <= 2)
        return 1 / 2 - x / 4;
    else return 0;
}
int scale_bayer_direct_bilinear_rggb(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
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

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_gbrg(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
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

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_bggr(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
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

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ !(i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_grbg(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
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

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_rggb_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_gbrg_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_bggr_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ !(i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_grbg_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_rggb_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint32_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_gbrg_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint32_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_bggr_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint32_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ !(i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear_grbg_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint32_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -1; s < 3; ++s) {
                for (r = -1; r < 3; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_linear(src_x - x);
                    wy = bayer_linear(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bilinear(uint8_t **dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height, int sample_bits, int cfa_pattern)
{
    if (sample_bits == 8) {
        *dst = (uint8_t*)malloc(dst_width * dst_height);
        if (cfa_pattern == 0x01000201) { // gbrg
            scale_bayer_direct_bilinear_gbrg(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x01020001) { // grbg
            scale_bayer_direct_bilinear_grbg(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x02010100) { // rggb
            scale_bayer_direct_bilinear_rggb(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x00010102) { // bggr
            scale_bayer_direct_bilinear_bggr(*dst, dst_width, dst_height, src, src_width, src_height);
        }
    } else if (sample_bits == 16) {
        *dst = (uint16_t*)malloc(dst_width * dst_height * sizeof(uint16_t));
        if (cfa_pattern == 0x01000201) { // gbrg
            scale_bayer_direct_bilinear_gbrg_16bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x01020001) { // grbg
            scale_bayer_direct_bilinear_grbg_16bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x02010100) { // rggb
            scale_bayer_direct_bilinear_rggb_16bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x00010102) { // bggr
            scale_bayer_direct_bilinear_bggr_16bit(*dst, dst_width, dst_height, src, src_width, src_height);
        }
    } else if (sample_bits == 32) {
        *dst = (uint32_t*)malloc(dst_width * dst_height * sizeof(uint32_t));
        if (cfa_pattern == 0x01000201) { // gbrg
            scale_bayer_direct_bilinear_gbrg_32bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x01020001) { // grbg
            scale_bayer_direct_bilinear_grbg_32bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x02010100) { // rggb
            scale_bayer_direct_bilinear_rggb_32bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x00010102) { // bggr
            scale_bayer_direct_bilinear_bggr_32bit(*dst, dst_width, dst_height, src, src_width, src_height);
        }
    }
}
float bayer_cubic(float x)
{
    x = x < 0 ? -x : x;
    if (x >= 0 && x <= 2)
        return x * x * x / 32 - x * x / 8 + 1.0 / 3;
    else if (x > 2 && x <= 4)
        return (4 - x) * (4 - x) * (4 - x) / 96;
    else return 0;
}
int scale_bayer_direct_bicubic_rggb(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
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

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_gbrg(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
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

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_bggr(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
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

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ !(i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_grbg(uint8_t *dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
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

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_rggb_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_gbrg_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_bggr_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ !(i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_grbg_16bit(uint16_t *dst, int dst_width, int dst_height, uint16_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_rggb_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint32_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_gbrg_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint32_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && (i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_bggr_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint32_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ !(i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if (!(j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic_grbg_32bit(uint32_t *dst, int dst_width, int dst_height, uint32_t *src, int src_width, int src_height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint32_t *p;

    p = dst;
    for (j = 0; j < dst_height; ++j) {
        for (i = 0; i < dst_width; ++i) {
            x = i * src_width * 1.0 / dst_width;
            y = j * src_height * 1.0 / dst_height;
            x_int = (int)x;
            y_int = (int)y;
            x_adj = x - x_int;
            y_adj = y - y_int;

            vr = vg = vb = 0;
            wr = wg = wb = 0;
            for (s = -3; s < 5; ++s) {
                for (r = -3; r < 5; ++r) {
                    src_x = x_int + r;
                    src_y = y_int + s;
                    if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height) continue;
                    wx = bayer_cubic(src_x - x);
                    wy = bayer_cubic(src_y - y);

                    if (!(src_y & 1)) {
                        if (!(src_x & 1)) { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // r
                            wr += wx * wy;
                            vr += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "r[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    } else {
                        if (!(src_x & 1)) { // b
                            wb += wx * wy;
                            vb += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "b[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        } else { // g
                            wg += wx * wy;
                            vg += wx * wy * src[src_y * src_width + src_x];
                            //fprintf(stdout, "g[%d, %d]: %d, wx: %.2f, wy: %.2f\n", src_x, src_y, src[src_y * src_width + src_x], wx, wy);
                        }
                    }
                }
            }
            //fprintf(stdout, "vr: %.2f, vg: %.2f, vb: %.2f, wr: %.2f, wg: %.2f, wb: %.2f\n", vr, vg, vb, wr, wg, wb);
            if (!((j & 1) ^ (i & 1))) { // g
                p[dst_width * j + i] = vg / wg;
                //fprintf(stdout, "g[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else if ((j & 1) && !(i & 1)) { // b
                p[dst_width * j + i] = vb / wb;
                //fprintf(stdout, "b[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            } else {
                p[dst_width * j + i] = vr / wr;
                //fprintf(stdout, "r[%d, %d]: %d\n", i, j, p[dst_width * j + i]);
            }
        }
    }
}
int scale_bayer_direct_bicubic(uint8_t **dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height, int sample_bits, int cfa_pattern)
{
    if (sample_bits == 8) {
        *dst = (uint8_t*)malloc(dst_width * dst_height);
        if (cfa_pattern == 0x01000201) { // gbrg
            scale_bayer_direct_bicubic_gbrg(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x01020001) { // grbg
            scale_bayer_direct_bicubic_grbg(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x02010100) { // rggb
            scale_bayer_direct_bicubic_rggb(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x00010102) { // bggr
            scale_bayer_direct_bicubic_bggr(*dst, dst_width, dst_height, src, src_width, src_height);
        }
    } else if (sample_bits == 16) {
        *dst = (uint16_t*)malloc(dst_width * dst_height * sizeof(uint16_t));
        if (cfa_pattern == 0x01000201) { // gbrg
            scale_bayer_direct_bicubic_gbrg_16bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x01020001) { // grbg
            scale_bayer_direct_bicubic_grbg_16bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x02010100) { // rggb
            scale_bayer_direct_bicubic_rggb_16bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x00010102) { // bggr
            scale_bayer_direct_bicubic_bggr_16bit(*dst, dst_width, dst_height, src, src_width, src_height);
        }
    } else if (sample_bits == 32) {
        *dst = (uint32_t*)malloc(dst_width * dst_height * sizeof(uint32_t));
        if (cfa_pattern == 0x01000201) { // gbrg
            scale_bayer_direct_bicubic_gbrg_32bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x01020001) { // grbg
            scale_bayer_direct_bicubic_grbg_32bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x02010100) { // rggb
            scale_bayer_direct_bicubic_rggb_32bit(*dst, dst_width, dst_height, src, src_width, src_height);
        } else if (cfa_pattern == 0x00010102) { // bggr
            scale_bayer_direct_bicubic_bggr_32bit(*dst, dst_width, dst_height, src, src_width, src_height);
        }
    }
}
// scale bayer directly
int scale_bayer_dir(uint8_t **dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height, int sample_bits, int cfa_pattern)
{
    //scale_bayer_direct_bilinear(dst, dst_width, dst_height, src, src_width, src_height, sample_bits, cfa_pattern);
    scale_bayer_direct_bicubic(dst, dst_width, dst_height, src, src_width, src_height, sample_bits, cfa_pattern);
}

TIFFDirectory *find_main_ifd(TIFFDirectory *td)
{
    TIFFDirectory *res = NULL;
    if (td->new_subfile_type == 0) return td;
    for (int i = 0; i < td->num_subifd; ++i) {
        if ((res = find_main_ifd((TIFFDirectory*)(td->subifd[i].ctx_ptr))) != NULL)
            return res;
    }
    return res;
}

int convert_decrease_12bit_to_8bit(uint8_t **dst, int16_t *src, int size)
{
    int i, j, ret;
    uint8_t *p;
    int16_t *q;

    // 16bit转8bit
    *dst = (uint8_t*)malloc(size);
    p = *dst, q = src;
    for (i = 0; i < size; ++i, p += 1, q += 1)
        p[0] = q[0] >> 4;

    fprintf(stdout, "Transform rgb to gray succeeded.\n");

    return 0;
}

int scale_bayer_deb(uint8_t **dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height, int sample_bits, int cfa_pattern)
{
    uint8_t *rgb, *scaled_rgb;

    demosaicing(&rgb, src, src_width, src_height, sample_bits, cfa_pattern);
    //uint8_t *rgb_bit8;
    //TIFFContext *c;
    //convert_decrease_12bit_to_8bit(&rgb_bit8, rgb, src_width * src_height * 3);
    //init_tiff_ctx(&c, rgb_bit8, src_width, src_height, 24, 3, 2);
    //write_tiff(c, "rgb.tif");
    //bmp_write("rgb.bmp", rgb_bit8, src_width, src_height, 24);

    scale(&scaled_rgb, dst_width, dst_height, rgb, src_width, src_height, sample_bits, 1);
    //bmp_write("scaled_rgb.bmp", scaled_rgb, dst_width, dst_height, 24);

    create_cfa(dst, scaled_rgb, dst_width, dst_height, sample_bits, cfa_pattern);
}

int scale_bayer(uint8_t **dst, int dst_width, int dst_height, uint8_t *src, int src_width, int src_height, int sample_bits, int cfa_pattern)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;

    if (sample_bits == 8) {
        scale_bayer_deb(dst, dst_width, dst_height, src, src_width, src_height, 8, cfa_pattern);
    } else if (sample_bits == 10 || sample_bits == 12 || sample_bits == 14 || sample_bits == 16) {
        uint16_t *src_bit16, *dst_bit16;
        if (sample_bits == 10) convert_10bit_to_16bit(&src_bit16, src, src_width, src_height);
        else if (sample_bits == 12) convert_12bit_to_16bit(&src_bit16, src, src_width, src_height);
        else if (sample_bits == 14) convert_14bit_to_16bit(&src_bit16, src, src_width, src_height);
        else src_bit16 = (uint16_t*)src;

        //scale_bayer_dir(&dst_bit16, dst_width, dst_height, src_bit16, src_width, src_height, 16, cfa_pattern);
        scale_bayer_deb(&dst_bit16, dst_width, dst_height, src_bit16, src_width, src_height, 16, cfa_pattern);

        #if DEBUG_MODE
        int fd;
        if ((fd = open("src_bayer.bin", O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1) {
            fprintf(stderr, "Error: failed to create file \"src_bayer.bin\"\n");
            return -1;
        }
        if ((ret = write(fd, src_bit16, src_width * src_height * sizeof(uint16_t))) != src_width * src_height * sizeof(uint16_t)) {
            fprintf(stderr, "Error: failed to write bayer data\n");
            return -1;
        }
        close(fd);
        #endif
        
        if (sample_bits == 10) convert_16bit_to_10bit(dst, dst_bit16, dst_width, dst_height);
        else if (sample_bits == 12) convert_16bit_to_12bit(dst, dst_bit16, dst_width, dst_height);
        else if (sample_bits == 14) convert_16bit_to_14bit(dst, dst_bit16, dst_width, dst_height);
        else *dst = (uint8_t*)dst_bit16;
    } else if (sample_bits == 24 || sample_bits == 32) {
        uint32_t *src_bit32, *dst_bit32;
        if (sample_bits == 24) convert_24bit_to_32bit(&src_bit32, src, src_width, src_height);
        else src_bit32 = (uint32_t*)src;
        
        scale_bayer_deb(&dst_bit32, dst_width, dst_height, src_bit32, src_width, src_height, 32, cfa_pattern);

        if (sample_bits == 24) convert_32bit_to_24bit(dst, dst_bit32, src_width, src_height);
        else *dst = (uint8_t*)dst_bit32;
    }
}

int scaleraw(TIFFDirectory *td, int dst_width, int dst_height)
{
    int src_width, src_height;
    int i, ret;
    TIFFDirectory *s = find_main_ifd(td);

    src_width = s->image_width;
    src_height = s->image_length;
    s->image_width = dst_width;
    s->image_length = dst_height;
    if (s->default_crop_size[0] > 0 && s->default_crop_size[1] > 0) {
        float x_scale_ratio = dst_width * 1.0 / s->default_crop_size[0], y_scale_ratio = dst_height * 1.0 / s->default_crop_size[1];
        s->default_crop_origin[0] *= x_scale_ratio;
        s->default_crop_origin[1] *= y_scale_ratio;
        s->default_crop_size[0] = dst_width;
        s->default_crop_size[1] = dst_height;
        s->image_width = s->default_crop_size[0] + s->default_crop_origin[0] * 2;
        s->image_length = s->default_crop_size[1] + s->default_crop_origin[1] * 2;

        tiff_modify_default_crop_origin(s, s->default_crop_origin);
        tiff_modify_default_crop_size(s, s->default_crop_size);
    }
    if (s->active_area[2] > 0 && s->active_area[3] > 0) {
        if (s->default_crop_size[0] > 0 && s->default_crop_size[1] > 0) {
            s->active_area[0] = s->default_crop_origin[1]; // top
            s->active_area[1] = s->default_crop_origin[0]; // left
            s->active_area[2] = s->default_crop_origin[1] + s->default_crop_size[1]; // bottom
            s->active_area[3] = s->default_crop_origin[0] + s->default_crop_size[0]; // right
        } else {
            float x_scale_ratio = dst_width * 1.0 / (s->active_area[3] - s->active_area[1]), y_scale_ratio = dst_height * 1.0 / (s->active_area[2] - s->active_area[0]);
            s->active_area[0] = s->active_area[0] * y_scale_ratio; // top
            s->active_area[1] = s->active_area[1] * x_scale_ratio; // left
            s->active_area[2] = s->active_area[2] * y_scale_ratio; // bottom
            s->active_area[3] = s->active_area[3] * x_scale_ratio; // right
            s->image_width = s->active_area[3] + s->active_area[1];
            s->image_length = s->active_area[2] + s->active_area[0];
        }
        tiff_modify_default_crop_origin(s, s->default_crop_origin);
    }
    tiff_modify_image_width(s, s->image_width);
    tiff_modify_image_height(s, s->image_length);

    s->image_size = ((s->image_width * s->bitcount + 7) / 8) * s->image_length;

    s->rows_per_strip = (s->image_length + s->strips_per_image - 1) / s->strips_per_image;
    s->strips_per_image = (s->image_length + s->rows_per_strip - 1) / s->rows_per_strip;

    int byte_counts_per_strip = ((s->image_width * s->bitcount + 7) >> 3) * s->rows_per_strip;
    if (s->image_length % s->rows_per_strip) {
        for (i = 0; i < s->strips_per_image - 1; ++i)
            s->strip_byte_counts[i] = byte_counts_per_strip;
        s->strip_byte_counts[s->strips_per_image - 1] = ((s->image_width * s->bitcount + 7) >> 3) * (s->image_length % s->rows_per_strip);
    } else {
        for (i = 0; i < s->strips_per_image; ++i)
            s->strip_byte_counts[i] = byte_counts_per_strip;
    }
    for (i = 1; i < s->strips_per_image; ++i)
        s->strip_offsets[i] = s->strip_offsets[i - 1] + s->strip_byte_counts[i - 1];
    tiff_modify_rows_per_strip(s, s->rows_per_strip);
    tiff_modify_strip_offsets(s, s->strip_offsets, s->strips_per_image);
    tiff_modify_strip_byte_counts(s, s->strip_byte_counts, s->strips_per_image);
    
    if (s->samples_per_pixel == 1 && s->photo_interp == 32803) { // CFA
        uint8_t *dst;
        scale_bayer(&dst, s->image_width, s->image_length, s->image_data, src_width, src_height, s->bits_per_sample[0], s->cfa_pattern);
        s->image_data = dst;
    }

    return ret;
}
