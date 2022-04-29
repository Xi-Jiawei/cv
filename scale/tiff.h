#ifndef __TIFF_H
#define __TIFF_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"

#define TIFF_VALUE_SIZE 4

#define TIFFTAG_NEWSUBFILETYPE      0xfe
#define TIFFTAG_IMAGEWIDTH          0x100
#define TIFFTAG_IMAGELENGTH         0x101
#define TIFFTAG_ROWSPERSTRIP        0x116
#define TIFFTAG_STRIPOFFSETS        0x111
#define TIFFTAG_STRIPBYTECOUNTS     0x117
#define TIFFTAG_SAMPLESPERPIXEL     0x115
#define TIFFTAG_BITSPERSAMPLE       0x102
#define TIFFTAG_SUBIFDS             0x14a
#define TIFFTAG_DEFAULTCROPORIGIN   0xc61f
#define TIFFTAG_DEFAULTCROPSIZE     0xc620
#define TIFFTAG_ACTIVEAREA          0xc68d
#define TIFFTAG_PHOTOINTERP         0x106
#define TIFFTAG_CFAREPEATPATTERNDIM 0x828d
#define TIFFTAG_CFAPATTERN          0x828e

#define TIFF_MAX_NUM_SUBIFD 16

typedef enum {
    TIFF_DATA_TYPE_NONE            = 0, /*!< Undefined or not yet set */
    TIFF_DATA_TYPE_BYTE            = 1, /*!< Byte */
    TIFF_DATA_TYPE_ASCII           = 2, /*!< Characters */
    TIFF_DATA_TYPE_SHORT           = 3, /*!< Unsigned 16-bit Integer */
    TIFF_DATA_TYPE_LONG            = 4, /*!< Unsigned 32-bit Integer */
    TIFF_DATA_TYPE_RATIONAL        = 5, /*!< Tow LONGs: the first represents the numerator of a fraction; the second, the denominator. */
    TIFF_DATA_TYPE_SIGNED_BYTE     = 6, /*!< Signed Byte */
    TIFF_DATA_TYPE_UNDEFINED       = 7, /*!< Undefined */
    TIFF_DATA_TYPE_SIGNED_SHORT    = 8, /*!< Signed 16-bit Integer */
    TIFF_DATA_TYPE_SIGNED_LONG     = 9, /*!< Signed 32-bit Integer */
    TIFF_DATA_TYPE_SIGNED_RATIONAL = 10, /*!< Tow SLONGs: the first represents the numerator of a fraction; the second, the denominator. */
    TIFF_DATA_TYPE_FLOAT           = 11, /*!< Float */
    TIFF_DATA_TYPE_DOUBLE          = 12, /*!< Double */
} TIFFDataType;

static size_t typesize_table[] = { 1, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8, 4 };

typedef struct TIFFDirEntry {
    uint16_t tag;   /* data tag */
    uint16_t type;  /* data type */
    uint32_t count; /* number of items; length in spec */
    uint32_t value; /* either offset or the data itself if fits */
} __attribute__((packed)) TIFFDirEntry;

typedef struct SubIFD {
    void *ctx_ptr; // pointer of TIFFContext type
    uint32_t offset; // subifd offset
} SubIFD;

typedef struct TIFFContext {
    //const TIFFClass *class; ///< class for private options
    uint8_t *image_data;

    uint32_t new_subfile_type;
    uint32_t image_width; // width
    uint32_t image_length; // height
    uint16_t bitcount;
    uint32_t image_size;
    uint16_t *bits_per_sample; // # of values is samples_per_pixel. when these values are "Vendor Unique", samples_per_pixel > 32767.
    uint16_t compression;
    uint16_t photo_interp; // photometric_interpretation. 1: BlackIsZero(grayscale images); 2: RGB; 6: YCbCr; 32803: CFA
    char *image_description;
    char *make;
    char *model;
    int strips_per_image;
    uint32_t *strip_offsets; // # of values is strips_per_image. strips_per_image = (image_length + rows_per_strip - 1) / rows_per_strip.
    uint16_t orientation;
    uint16_t samples_per_pixel;
    uint32_t rows_per_strip;
    uint32_t *strip_byte_counts; // # of values is strips_per_image.
    uint32_t x_resolution[2];
    uint32_t y_resolution[2];
    uint16_t planar_configuration;
    uint16_t resolution_unit;
    char *software;
    char datetime[20];
    char *artist;
    uint32_t tile_width; // tile_across = (image_width + tile_width - 1) / tile_width
    uint32_t tile_length; // tile_down = (image_length + tile_length - 1) / tile_length
    int tiles_per_image;
    uint32_t *tile_offsets; // # of values is tiles_per_image. tiles_per_image = tile_across * tile_down.
    uint32_t *tile_byte_counts; // # of values is tiles_per_image.
    char *jpeg_tables;
    uint64_t ycbcr_coefficients[3];
    uint16_t ycbcr_sub_sampling[2];
    uint16_t ycbcr_positioning;
    uint64_t reference_black_white[6];
    uint16_t cfa_repeat_pattern_dim[2]; // cfa_repeat_rows = cfa_repeat_pattern_dim[0], cfa_repeat_cols = cfa_repeat_pattern_dim[1]
    uint8_t *cfa_pattern; // # of values is cfa_repeat_rows * cfa_repeat_cols.
    uint8_t *batterylevel;
    uint8_t *copyright;
    uint64_t exposure_time[2];
    uint64_t f_number[2];
    uint8_t *iptc_naa;
    uint8_t *inter_color_profile;
    uint16_t exposure_program;
    uint8_t *spectral_sensitivity;
    uint32_t gps_info;
    uint16_t iso_speed_ratings;
    uint8_t *oecf;
    uint16_t interlace;
    int16_t timezone_offset[2];
    uint16_t self_timer_mode;
    uint8_t ateTimeOriginal[20];
    uint64_t compressed_bits_per_pixel;
    uint64_t shutter_speed_value;
    uint64_t aperture_value;
    int64_t brightness_value[2];
    int64_t exposure_bias_value[2];
    uint64_t max_aperture_value;
    int64_t subject_distance[2];
    uint16_t metering_mode;
    uint16_t light_source;
    uint16_t flash;
    uint64_t focal_length[2];
    uint64_t flash_energy[2];
    uint8_t *spatial_frequency_response;
    uint8_t *noise;
    uint64_t focal_plane_x_resolution;
    uint64_t focal_plane_y_resolution;
    uint16_t focal_plane_resolution_unit;
    uint32_t image_number;
    uint8_t *security_classification;
    uint8_t *image_history;
    uint16_t subject_location[4];
    uint64_t exposure_index[2];
    uint8_t tiff_ep_standard_id[4];
    uint16_t sensing_method;

    // DNG-specific tags
    uint8_t dng_version[4];
    uint8_t *unique_camera_model;
    int num_linearization_table;
    uint16_t *linearization_table;
    int num_white_level;
    uint32_t *white_level;
    int num_color_matrix1;
    int32_t color_matrix1[9][2];
    uint64_t default_crop_origin[2];
    uint64_t default_crop_size[2];
    uint16_t calibration_illuminant1;
    int num_as_shot_neutral;
    uint64_t *as_shot_neutral;

    SubIFD *subifd;
    int num_subifd;
} TIFFContext;

typedef struct TIFFParseTableEntry {
    uint16_t tag;
    int (*parse)(TIFFContext *ctx, IOContext *s, TIFFDirEntry *entry);
} TIFFParseTableEntry;

int tiff_read_ifd(TIFFContext *c, IOContext *s, uint32_t offset);
int read_tiff(TIFFContext *c, char *filename);

int extract_tiff_data(uint8_t **data, int *width, int *height, int *bitcount, int *samples_per_pixel, int *photo_interp, char *filename);

// write_tiff_default
#define ROWS_PER_STRIP 60
#define SAMPLES_PER_PIXEL 3
#define BITS_PER_SAMPLE 8

int write_tiff_default(char *filename, uint8_t *rgb, int width, int height);

int init_tiff_ctx(TIFFContext **s, uint8_t *data, int width, int height, int bitcount, int samples_per_pixel, int photo_interp/* = 2*/);
int write_tiff_tag(IOContext *s, uint16_t tag, uint16_t type, uint32_t count, uint8_t *values, uint32_t *offset);
int write_tiff(TIFFContext *c, char *filename);
int init_dng_ctx(TIFFContext **s, uint8_t *bayer, int width, int height);
int write_dng(TIFFContext *c, char *filename);

//
// tiff_rewrite
//

typedef struct TIFFEntry {
    uint16_t tag;      /* data tag */
    uint16_t type;     /* data type */
    uint32_t count;    /* number of items; length in spec */
    int is_offset;     /* 1 if value is offset */
    uint32_t value;    /* either offset or the data itself if fits */
    uint8_t *data;     /* the data if value is offset, or null if value is the data itself */
} TIFFEntry;

typedef struct TIFFDirectory {
    //const TIFFClass *class; ///< class for private options
    TIFFEntry *entries;
    int num_entries;
    int entries_allocated;

    int offset;

    uint8_t *image_data;

    uint32_t new_subfile_type;
    uint32_t image_width; // width
    uint32_t image_length; // height
    uint16_t bitcount;
    uint32_t image_size;
    uint32_t strips_per_image;
    uint32_t *strip_offsets; // # of values is strips_per_image. strips_per_image = (image_length + rows_per_strip - 1) / rows_per_strip.
    uint16_t samples_per_pixel;
    uint16_t *bits_per_sample; // # of values is samples_per_pixel. when these values are "Vendor Unique", samples_per_pixel > 32767.
    uint32_t rows_per_strip;
    uint32_t *strip_byte_counts; // # of values is strips_per_image.
    uint64_t default_crop_origin[2];
    uint64_t default_crop_size[2];
    uint32_t active_area[4];
    uint16_t photo_interp;
    uint16_t cfa_repeat_pattern_dim[2];
    uint32_t cfa_pattern;

    SubIFD *subifd;
    int num_subifd;
} TIFFDirectory;

void tiff_modify_image_width(TIFFDirectory *td, uint32_t image_width);
void tiff_modify_image_height(TIFFDirectory *td, uint32_t image_height);
void tiff_modify_default_crop_origin(TIFFDirectory *td, uint64_t default_crop_origin[2]);
void tiff_modify_default_crop_size(TIFFDirectory *td, uint64_t default_crop_size[2]);
void tiff_modify_active_area(TIFFDirectory *td, uint32_t active_area[4]);
void tiff_modify_rows_per_strip(TIFFDirectory *td, uint32_t rows_per_strip);
void tiff_modify_strip_offsets(TIFFDirectory *td, uint32_t *strip_offsets, int strips_per_image);
void tiff_modify_strip_byte_counts(TIFFDirectory *td, uint32_t *strip_byte_counts, int strips_per_image);

int tiff_rewrite(TIFFDirectory *td, char *filename);
int tiff_parse(TIFFDirectory *td, char *filename);

#endif /* __TIFF_H */