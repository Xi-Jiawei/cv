#include "bits.h"

int convert_10bit_to_16bit(uint16_t **dst, uint8_t *src, int width, int height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;
    uint8_t *q;

    /* 10bits bitstream
        |  8bits | 2|   6  |  4 |  4 |  6   | 2|    8   |
        |  a-h8  | a| b-h6 |b-l4|c-h4| c-l6 | d|  d-l8  |
        |<--10bits->|<--10bits->|<--10bits->|<--10bits->|
     */

    *dst = (uint16_t*)malloc(width * height * sizeof(uint16_t));
    p = *dst;
    q = src;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 4) {
            p[width * j + i] = (q[0] << 2) | (q[1] >> 6);
            p[width * j + i + 1] = ((q[1] & 0x3f) << 4) | (q[2] >> 4);
            p[width * j + i + 2] = ((q[2] & 0x0f) << 6) | (q[3] >> 2);
            p[width * j + i + 3] = ((q[3] & 0x03) << 8) | q[4];
            q += 5;
        }
    }
}
int convert_12bit_to_16bit(uint16_t **dst, uint8_t *src, int width, int height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;
    uint8_t *q;

    /* 12bits bitstream
        |  8bits |  4 |  4 |    8   |
        |  a-h8  |a-l4|b-h4|  b-l8  |
        |<---12bits-->|<---12bits-->|
     */

    *dst = (uint16_t*)malloc(width * height * sizeof(uint16_t));
    p = *dst;
    q = src;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 2) {
            p[width * j + i] = (q[0] << 4) | (q[1] >> 4);
            p[width * j + i + 1] = ((q[1] & 0x0f) << 8) | q[2];
            q += 3;
        }
    }
}
int convert_14bit_to_16bit(uint16_t **dst, uint8_t *src, int width, int height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint16_t *p;
    uint8_t *q;

    /* 10bits bitstream
        |  8bits  |   6  | 2|    8   |  4 |  4 |    8   | 2|   6  |    8    |
        |   a-h8  | a-h6 | b|b-(4-12)|b-l4|c-h4|c-(4-12)| c| d-l4 |   d-h8  |
        |<----14bits---->|<----14bits---->|<----14bits---->|<----14bits---->|
     */

    *dst = (uint16_t*)malloc(width * height * sizeof(uint16_t));
    p = *dst;
    q = src;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 4) {
            p[width * j + i] = (q[0] << 6) | (q[1] >> 2);
            p[width * j + i + 1] = ((q[1] & 0x03) << 12) | (q[2] << 4) | (q[3] >> 4);
            p[width * j + i + 2] = ((q[3] & 0x0f) << 10) | (q[4] << 2) | (q[5] >> 6);
            p[width * j + i + 3] = ((q[5] & 0x3f) << 8) | q[6];
            q += 7;
        }
    }
}
int convert_16bit_to_10bit(uint8_t **dst, uint16_t *src, int width, int height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint8_t *p;
    uint16_t *q;

    /* 10bits bitstream
        |  8bits | 2|   6  |  4 |  4 |  6   | 2|    8   |
        |  a-h8  | a| b-h6 |b-l4|c-h4| c-l6 | d|  d-l8  |
        |<--10bits->|<--10bits->|<--10bits->|<--10bits->|
     */

    *dst = (uint8_t*)malloc(((width * 10 + 7) >> 3) * height);
    p = *dst;
    q = src;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 4) {
            p[0] = q[width * j + i] >> 2;
            p[1] = ((q[width * j + i] & 0x03) << 6) | (q[width * j + i + 1] >> 4);
            p[2] = ((q[width * j + i + 1] & 0x0f) << 4) | (q[width * j + i + 2] >> 6);
            p[3] = ((q[width * j + i + 2] & 0x3f) << 2) | (q[width * j + i + 3] >> 8);
            p[4] = q[width * j + i + 3] & 0xff;
            p += 5;
        }
    }
}
int convert_16bit_to_12bit(uint8_t **dst, uint16_t *src, int width, int height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint8_t *p;
    uint16_t *q;

    /* 12bits bitstream
        |  8bits |  4 |  4 |    8   |
        |  a-h8  |a-l4|b-h4|  b-l8  |
        |<---12bits-->|<---12bits-->|
     */

    *dst = (uint8_t*)malloc(((width * 12 + 7) >> 3) * height);
    p = *dst;
    q = src;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 2) {
            p[0] = q[width * j + i] >> 4;
            p[1] = ((q[width * j + i] & 0x0f) << 4) | (q[width * j + i + 1] >> 8);
            p[2] = q[width * j + i + 1] & 0xff;
            p += 3;
        }
    }
}
int convert_16bit_to_14bit(uint8_t **dst, uint16_t *src, int width, int height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint8_t *p;
    uint16_t *q;

    /* 10bits bitstream
        |  8bits  |   6  | 2|    8   |  4 |  4 |    8   | 2|   6  |    8    |
        |   a-h8  | a-h6 | b|b-(4-12)|b-l4|c-h4|c-(4-12)| c| d-l4 |   d-h8  |
        |<----14bits---->|<----14bits---->|<----14bits---->|<----14bits---->|
     */

    *dst = (uint8_t*)malloc(((width * 12 + 7) >> 3) * height);
    p = *dst;
    q = src;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; i += 4) {
            p[0] = q[width * j + i] >> 6;
            p[1] = ((q[width * j + i] & 0x3f) << 2) | (q[width * j + i + 1] >> 12);
            p[2] = (q[width * j + i + 1] >> 4) & 0xff;
            p[3] = ((q[width * j + i + 1] & 0x0f) << 4) | (q[width * j + i + 2] >> 10);
            p[4] = (q[width * j + i + 2] >> 2) & 0xff;
            p[5] = ((q[width * j + i + 2] & 0x03) << 6) | (q[width * j + i + 3] >> 8);
            p[6] = q[width * j + i + 3] & 0xff;
            p += 7;
        }
    }
}
int convert_24bit_to_32bit(uint32_t **dst, uint8_t *src, int width, int height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint32_t *p;
    uint8_t *q;

    /* 24bits bitstream
        |  8bits |    8   |    8   |    8   |    8   |    8   |
        |  a-h8  |a-(8-16)|  a-l8  |  a-h8  |a-(8-16)|  a-l8  |
        |<---------24bits--------->|<---------24bits--------->|
     */

    *dst = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    p = *dst;
    q = src;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            p[width * j + i] = (q[0] << 16) | (q[1] << 8) | q[2];
            q += 3;
        }
    }
}
int convert_32bit_to_24bit(uint8_t **dst, uint32_t *src, int width, int height)
{
    int i, j, k, r, s, ret;
    float x, y, x_adj, y_adj;
    int x_int, y_int, src_x, src_y;
    float vr, vg, vb, wx, wy, wr, wg, wb;
    uint8_t *p;
    uint32_t *q;

    /* 24bits bitstream
        |  8bits |    8   |    8   |    8   |    8   |    8   |
        |  a-h8  |a-(8-16)|  a-l8  |  a-h8  |a-(8-16)|  a-l8  |
        |<---------24bits--------->|<---------24bits--------->|
     */

    *dst = (uint8_t*)malloc(width * height * sizeof(uint32_t));
    p = *dst;
    q = src;
    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            p[0] = q[width * j + i] >> 16;
            p[1] = (q[width * j + i] >> 8) & 0xff;
            p[2] = q[width * j + i] & 0xff;
            p += 3;
        }
    }
}
