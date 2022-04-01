#include "tiff.h"

int tiff_read_new_subfile_type(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->new_subfile_type = entry->value;
    fprintf(stderr, "new_subfile_type: %d\n", c->new_subfile_type);
    return 0;
}

int tiff_read_image_width(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->image_width = entry->value;
    fprintf(stderr, "image_width: %d\n", c->image_width);
    return 0;
}

int tiff_read_image_length(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->image_length = entry->value;
    fprintf(stdout, "image_length: %d\n", c->image_length);
    return 0;
}

int tiff_read_photo_interp(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->photo_interp = entry->value;
    fprintf(stdout, "photo_interp: %d\n", c->photo_interp);
    return 0;
}

int tiff_read_samples_per_pixel(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->samples_per_pixel = entry->value;
    fprintf(stdout, "samples_per_pixel: %d\n", c->samples_per_pixel);
    return 0;
}

int tiff_read_bits_per_sample(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    if (c->samples_per_pixel) assert(c->samples_per_pixel == entry->count);
    else c->samples_per_pixel = entry->count;
    assert(entry->type == TIFF_DATA_TYPE_SHORT && entry->count != 0);
    size = entry->count * typesize_table[entry->type];
    c->bits_per_sample = (uint16_t*)malloc(size);
    if (size <= VALUE_SIZE) {
        memcpy(c->bits_per_sample, &entry->value, size);
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        io_read(s, (void*)c->bits_per_sample, size);
    }
    c->bitcount = 0;
    for (int i = 0; i < entry->count; ++i)
        c->bitcount += c->bits_per_sample[i];
    fprintf(stdout, "bits_per_sample.tag: %d, bits_per_sample.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "bits_per_sample[%d]: %d\n", i, c->bits_per_sample[i]);
    return 0;
}

int tiff_read_compression(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->compression = entry->value;
    fprintf(stdout, "compression.tag: %d, compression.type: %d, compression: %d\n", entry->tag, entry->type, c->compression);
    return 0;
}

int tiff_read_model(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    assert(entry->type == TIFF_DATA_TYPE_ASCII && entry->count != 0);
    size = entry->count;
    c->model = (char*)malloc(entry->count);
    if (size <= VALUE_SIZE) {
        memcpy(c->model, &entry->value, size);
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        io_read(s, (void*)c->model, size);
    }
    fprintf(stdout, "model: %s\n", c->model);
    return 0;
}

int tiff_read_orientation(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->orientation = entry->value;
    fprintf(stdout, "orientation.tag: %d, orientation.type: %d, orientation: %d\n", entry->tag, entry->type, c->orientation);
    return 0;
}

int tiff_read_planar_configuration(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->planar_configuration = entry->value;
    fprintf(stdout, "planar_configuration.tag: %d, planar_configuration.type: %d, planar_configuration: %d\n", entry->tag, entry->type, c->planar_configuration);
    return 0;
}

int tiff_read_x_resolution(TIFFContext *c, IOContext *s, DEntry *entry) {
    io_seek(s, entry->value, SEEK_SET);
    io_read(s, (void*)&(c->x_resolution[0]), sizeof(uint32_t));
    io_read(s, (void*)&(c->x_resolution[1]), sizeof(uint32_t));
    fprintf(stdout, "x_resolution.tag: %d, x_resolution.type: %d, x_resolution: %d/%d\n", entry->tag, entry->type, c->x_resolution[0], c->x_resolution[1]);
    return 0;
}

int tiff_read_y_resolution(TIFFContext *c, IOContext *s, DEntry *entry) {
    io_seek(s, entry->value, SEEK_SET);
    io_read(s, (void*)&(c->y_resolution[0]), sizeof(uint32_t));
    io_read(s, (void*)&(c->y_resolution[1]), sizeof(uint32_t));
    fprintf(stdout, "y_resolution.tag: %d, y_resolution.type: %d, y_resolution: %d/%d\n", entry->tag, entry->type, c->y_resolution[0], c->y_resolution[1]);
    return 0;
}

int tiff_read_resolution_unit(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->resolution_unit = entry->value;
    fprintf(stdout, "resolution_unit.tag: %d, resolution_unit.type: %d, resolution_unit: %d\n", entry->tag, entry->type, c->resolution_unit);
    return 0;
}

int tiff_read_strip_offsets(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    if (c->strips_per_image) assert(c->strips_per_image == entry->count);
    else c->strips_per_image = entry->count;
    assert(entry->count != 0);
    size = entry->count * typesize_table[entry->type];
    c->strip_offsets = (uint32_t*)malloc(size);
    if (size <= VALUE_SIZE) {
        memcpy(c->strip_offsets, &entry->value, size);
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        io_read(s, (void*)c->strip_offsets, size);
    }
    fprintf(stdout, "strip_offsets.tag: %d, strip_offsets.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "strip_offsets[%d]: %d\n", i, c->strip_offsets[i]);
    return 0;
}

int tiff_read_rows_per_strip(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->rows_per_strip = entry->value;
    int strips_per_image = (c->image_length + c->rows_per_strip - 1) / c->rows_per_strip;
    if (c->strips_per_image) assert(strips_per_image == c->strips_per_image);
    else c->strips_per_image = strips_per_image;
    fprintf(stdout, "rows_per_strip.tag: %d, rows_per_strip.type: %d, rows_per_strip: %d\n", entry->tag, entry->type, c->rows_per_strip);
    return 0;
}

int tiff_read_strip_byte_counts(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    assert(entry->count != 0);
    size = entry->count * typesize_table[entry->type];
    c->strip_byte_counts = (uint32_t*)malloc(size);
    if (size <= VALUE_SIZE) {
        memcpy(c->strip_byte_counts, &entry->value, size);
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        io_read(s, (void*)c->strip_byte_counts, size);
    }
    c->image_size = 0;
    for (int i = 0; i < entry->count; ++i)
        c->image_size += c->strip_byte_counts[i];
    fprintf(stdout, "strip_byte_counts.tag: %d, strip_byte_counts.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "strip_byte_counts[%d]: %d\n", i, c->strip_byte_counts[i]);
    return 0;
}

int tiff_read_tile_width(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->tile_width = entry->value;
    fprintf(stdout, "tile_width.tag: %d, tile_width.type: %d, tile_width: %d\n", entry->tag, entry->type, c->tile_width);
    return 0;
}

int tiff_read_tile_length(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->tile_length = entry->value;
    fprintf(stdout, "tile_length.tag: %d, tile_length.type: %d, tile_length: %d\n", entry->tag, entry->type, c->tile_length);
    return 0;
}

int tiff_read_tile_offsets(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    if (c->image_width && c->image_length && c->tile_width && c->tile_length) {
        int tile_across, tile_down;
        int tiles_per_image;
        tile_across = (c->image_width + c->tile_width - 1) / c->tile_width;
        tile_down = (c->image_length + c->tile_length - 1) / c->tile_length;
        tiles_per_image = tile_across * tile_down;
        assert(tiles_per_image == entry->count); // entry->count should equal tiles_per_image
    }
    c->tiles_per_image = entry->count;
    assert(entry->type == TIFF_DATA_TYPE_LONG && entry->count != 0);
    size = entry->count * typesize_table[entry->type];
    c->tile_offsets = (uint32_t*)malloc(entry->count * sizeof(uint32_t));
    if (size <= VALUE_SIZE) {
        c->tile_offsets[0] = entry->value;
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        for (int i = 0; i < entry->count; ++i) {
            ret = io_read(s, (void*)&(c->tile_offsets[i]), sizeof(uint32_t));
            if (ret < 0) return ret;
        }
    }
    fprintf(stdout, "tile_offsets.tag: %d, tile_offsets.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "tile_offsets[%d]: %d\n", i, c->tile_offsets[i]);
    return 0;
}

int tiff_read_cfa_repeat_pattern_dim(TIFFContext *c, IOContext *s, DEntry *entry) {
    short *arr = (short*)&entry->value;
    c->cfa_repeat_pattern_dim[0] = arr[0];
    c->cfa_repeat_pattern_dim[1] = arr[1];
    fprintf(stdout, "cfa_repeat_pattern_dim.tag: %d, cfa_repeat_pattern_dim.type: %d\n", entry->tag, entry->type);
    fprintf(stdout, "cfa_repeat_pattern_dim[0]: %d\n", c->cfa_repeat_pattern_dim[0]);
    fprintf(stdout, "cfa_repeat_pattern_dim[1]: %d\n", c->cfa_repeat_pattern_dim[1]);
    return 0;
}

int tiff_read_cfa_pattern(TIFFContext *c, IOContext *s, DEntry *entry) {
    int ret;
    if (c->cfa_repeat_pattern_dim[0]) assert(c->cfa_repeat_pattern_dim[0] * c->cfa_repeat_pattern_dim[1] == entry->count);
    assert(entry->type == TIFF_DATA_TYPE_BYTE && entry->count != 0);
    c->cfa_pattern = (uint8_t*)malloc(entry->count);
    if (entry->count <= VALUE_SIZE) {
        memcpy(c->cfa_pattern, &entry->value, entry->count);
    } else if (entry->count > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        io_read(s, (void*)c->cfa_pattern, entry->count);
    }
    fprintf(stdout, "cfa_pattern.tag: %d, cfa_pattern.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "cfa_pattern[%d]: %d\n", i, c->cfa_pattern[i]);
    return 0;
}

int tiff_read_subifds(TIFFContext *c, IOContext *s, DEntry *entry) {
    int ret;
    assert(entry->count != 0);
    c->subifds = (SubIfd *)malloc(entry->count * sizeof(SubIfd));
    c->subifds_num = 0;
    if (entry->count == 1) {
        c->subifds[0].offset = entry->value;
        c->subifds[0].ptr = malloc(sizeof(TIFFContext));
        c->subifds_num = 1;
    } else if (entry->count > 1) {
        io_seek(s, entry->value, SEEK_SET);
        for (int i = 0; i < entry->count; ++i) {
            ret = io_read(s, (void*)&(c->subifds[i].offset), sizeof(uint32_t));
            if (ret < 0) return ret;
            c->subifds[i].ptr = malloc(sizeof(TIFFContext));
            c->subifds_num++;
        }
    }
    fprintf(stdout, "subifds.tag: %d, subifds.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "subifds[%d]: %d\n", i, c->subifds[i].offset);
    return 0;
}

int tiff_read_cfa_dng_version(TIFFContext *c, IOContext *s, DEntry *entry) {
    uint8_t *arr = (uint8_t*)&entry->value;
    c->dng_version[0] = arr[0];
    c->dng_version[1] = arr[1];
    c->dng_version[2] = arr[2];
    c->dng_version[3] = arr[3];
    fprintf(stdout, "dng_version.tag: %d, dng_version.type: %d\n", entry->tag, entry->type);
    fprintf(stdout, "dng_version: %d.%d.%d.%d\n", c->dng_version[0], c->dng_version[1], c->dng_version[2], c->dng_version[3]);
    return 0;
}

int tiff_read_unique_camera_model(TIFFContext *c, IOContext *s, DEntry *entry) {
    assert(entry->count != 0);
    c->unique_camera_model = (uint8_t*)malloc(entry->count);
    if (entry->count <= VALUE_SIZE) {
        memcpy(c->unique_camera_model, &entry->value, entry->count);
    } else if (entry->count > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        io_read(s, (void*)c->unique_camera_model, entry->count);
    }
    fprintf(stdout, "unique_camera_model: %s\n", c->unique_camera_model);
    return 0;
}

int tiff_read_linearization_table(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    assert(entry->type == TIFF_DATA_TYPE_SHORT && entry->count != 0);
    c->linearization_table_num = entry->count;
    size = entry->count * typesize_table[entry->type];
    c->linearization_table = (uint16_t*)malloc(size);
    if (size <= VALUE_SIZE) {
        memcpy(c->linearization_table, &entry->value, size);
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        io_read(s, (void*)c->linearization_table, size);
    }
    fprintf(stdout, "linearization_table.tag: %d, linearization_table.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "linearization_table[%d]: %d\n", i, c->linearization_table[i]);
    return 0;
}

int tiff_read_default_crop_origin(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    assert(entry->count == 2);
    size = entry->count * typesize_table[entry->type];
    if (size <= VALUE_SIZE) {
        uint8_t *p = (uint8_t*)&entry->value;
        for (int i = 0; i < entry->count; ++i)
            memcpy((void*)&c->default_crop_origin[i], p + i * typesize_table[entry->type], typesize_table[entry->type]);
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        for (int i = 0; i < entry->count; ++i)
            io_read(s, (void*)&c->default_crop_origin[i], typesize_table[entry->type]);
    }
    fprintf(stdout, "default_crop_origin.tag: %d, default_crop_origin.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "default_crop_origin[%d]: %ld\n", i, c->default_crop_origin[i]);
    return 0;
}

int tiff_read_default_crop_size(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    assert(entry->count == 2);
    size = entry->count * typesize_table[entry->type];
    if (size <= VALUE_SIZE) {
        uint8_t *p = (uint8_t*)&entry->value;
        for (int i = 0; i < entry->count; ++i)
            memcpy((void*)&c->default_crop_size[i], p + i * typesize_table[entry->type], typesize_table[entry->type]);
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        for (int i = 0; i < entry->count; ++i)
            io_read(s, (void*)&c->default_crop_size[i], typesize_table[entry->type]);
    }
    fprintf(stdout, "default_crop_size.tag: %d, default_crop_size.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "default_crop_size[%d]: %ld\n", i, c->default_crop_size[i]);
    return 0;
}

int tiff_read_color_matrix1(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    assert(entry->type == TIFF_DATA_TYPE_SIGNED_RATIONAL && entry->count != 0 && entry->count <= 9);
    c->color_matrix1_num = entry->count;
    size = entry->count * typesize_table[entry->type];
    io_seek(s, entry->value, SEEK_SET);
    io_read(s, (void*)c->color_matrix1, size);
    fprintf(stdout, "color_matrix1.tag: %d, color_matrix1.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "color_matrix1[%d]: %d/%d\n", i, c->color_matrix1[i][0], c->color_matrix1[i][1]);
    return 0;
}

int tiff_read_white_level(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    if (c->samples_per_pixel) assert(entry->count == c->samples_per_pixel);
    assert(entry->count != 0);
    c->white_level_num = entry->count;
    size = entry->count * typesize_table[entry->type];
    c->white_level = (uint32_t*)malloc(entry->count * sizeof(uint32_t*));
    if (size <= VALUE_SIZE) {
        uint8_t *p = (uint8_t*)&entry->value;
        for (int i = 0; i < entry->count; ++i)
            memcpy((void*)&c->white_level[i], p + i * typesize_table[entry->type], typesize_table[entry->type]);
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        for (int i = 0; i < entry->count; ++i)
            io_read(s, (void*)&c->white_level[i], typesize_table[entry->type]);
    }
    fprintf(stdout, "white_level.tag: %d, white_level.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "white_level[%d]: %d\n", i, c->white_level[i]);
    return 0;
}

int tiff_read_calibration_illuminant1(TIFFContext *c, IOContext *s, DEntry *entry) {
    c->calibration_illuminant1 = entry->value;
    fprintf(stdout, "calibration_illuminant1.tag: %d, calibration_illuminant1.type: %d, calibration_illuminant1: %d\n", entry->tag, entry->type, c->calibration_illuminant1);
    return 0;
}

int tiff_read_as_shot_neutral(TIFFContext *c, IOContext *s, DEntry *entry) {
    int size, ret;
    assert(entry->count != 0);
    c->as_shot_neutral_num = entry->count;
    size = entry->count * typesize_table[entry->type];
    c->as_shot_neutral = (uint64_t*)malloc(entry->count * sizeof(uint64_t*));
    if (size <= VALUE_SIZE) {
        uint8_t *p = (uint8_t*)&entry->value;
        for (int i = 0; i < entry->count; ++i)
            memcpy((void*)&c->as_shot_neutral[i], p + i * typesize_table[entry->type], typesize_table[entry->type]);
    } else if (size > VALUE_SIZE) {
        io_seek(s, entry->value, SEEK_SET);
        for (int i = 0; i < entry->count; ++i)
            io_read(s, (void*)&c->as_shot_neutral[i], typesize_table[entry->type]);
    }
    fprintf(stdout, "as_shot_neutral.tag: %d, as_shot_neutral.type: %d\n", entry->tag, entry->type);
    for (int i = 0; i < entry->count; ++i)
        fprintf(stdout, "as_shot_neutral[%d]: %ld\n", i, c->as_shot_neutral[i]);
    return 0;
}

static const TIFFParseTableEntry tiff_default_parse_table[] = {
    { 0xfe,  tiff_read_new_subfile_type },
    { 0x100, tiff_read_image_width },
    { 0x101, tiff_read_image_length },
    { 0x106, tiff_read_photo_interp },
    { 0x115, tiff_read_samples_per_pixel },
    { 0x102, tiff_read_bits_per_sample },
    { 0x103, tiff_read_compression },
    { 0x110, tiff_read_model },
    { 0x112, tiff_read_orientation },
    { 0x11c, tiff_read_planar_configuration },
    { 0x11a, tiff_read_x_resolution },
    { 0x11b, tiff_read_y_resolution },
    { 0x128, tiff_read_resolution_unit },
    { 0x111, tiff_read_strip_offsets },
    { 0x116, tiff_read_rows_per_strip },
    { 0x117, tiff_read_strip_byte_counts },
    { 0x142, tiff_read_tile_width },
    { 0x143, tiff_read_tile_length },
    { 0x144, tiff_read_tile_offsets },
    { 0x14a, tiff_read_subifds },
    { 0x828d, tiff_read_cfa_repeat_pattern_dim },
    { 0x828e, tiff_read_cfa_pattern },
    { 0xc612, tiff_read_cfa_dng_version },
    { 0xc614, tiff_read_unique_camera_model },
    { 0xc618, tiff_read_linearization_table },
    { 0xc61d, tiff_read_white_level },
    { 0xc61f, tiff_read_default_crop_origin },
    { 0xc620, tiff_read_default_crop_size },
    { 0xc621, tiff_read_color_matrix1 },
    { 0xc65a, tiff_read_calibration_illuminant1 },
    { 0xc628, tiff_read_as_shot_neutral },
    { 0, NULL }
};

int tiff_read_ifd(TIFFContext *c, IOContext *s, uint32_t offset) {
    uint16_t num;
    int64_t pos;
    DEntry entry;
    int size, i, idx;
    int ret = 0;

    io_seek(s, offset, SEEK_SET);
    io_read(s, &num, 2);
    for (idx = 0; idx < num; ++idx) {
        if ((size = io_read(s, &entry, sizeof(DEntry))) != sizeof(DEntry)) {
            fprintf(stderr, "Error: failed to read entry at portion %ld\n", io_tell(s));
            return -1;
        }
        fprintf(stdout, "entries[%d].tag: %04x\n", idx, entry.tag);

        int (*parse)(TIFFContext*, IOContext*, DEntry*) = NULL;
        for (i = 0; tiff_default_parse_table[i].tag; i++)
            if (tiff_default_parse_table[i].tag == entry.tag) {
                parse = tiff_default_parse_table[i].parse;
                break;
            }

        if (parse) {
            pos = io_tell(s);
            ret = parse(c, s, &entry);
            if (ret < 0)
                return ret;
            int64_t left = pos - io_tell(s);
            if (left > 0) /* skip garbage at atom end */
                io_skip(s, left);
            else if (left < 0)
                io_seek(s, left, SEEK_CUR); // or io_seek(s, pos, SEEK_SET);
        }
    }
    if (c->strips_per_image) {
        pos = io_tell(s);
        assert(c->image_size == ((c->image_width * c->image_length * c->bitcount + 7) >> 3));
        c->image_data = (uint8_t*)malloc(c->image_size);
        uint8_t *p = c->image_data;
        for (i = 0; i < c->strips_per_image; ++i) {
            io_seek(s, c->strip_offsets[i], SEEK_SET);
            io_read(s, p, c->strip_byte_counts[i]);
            p += c->strip_byte_counts[i];
        }
        io_seek(s, pos, SEEK_SET);
    }
    if (c->subifds) {
        pos = io_tell(s);
        for (i = 0; i < c->subifds_num; ++i) {
            ret = tiff_read_ifd((TIFFContext *)(c->subifds[i].ptr), s, c->subifds[i].offset);
            if (ret < 0) {
                fprintf(stderr, "Error: failed to read ifd at portion %d\n", c->subifds[i].offset);
                return ret;
            }
        }
        io_seek(s, pos, SEEK_SET);
    }
    return ret;
}

int read_tiff(TIFFContext *c, char *filename)
{
    struct stat st;
    int size;
    int i, ret;
    uint8_t *p;
    uint16_t byteorder, version;
    uint32_t offset;
    IOContext *s;

    if ((ret = io_open(&s, filename, O_RDONLY)) == -1) {
        fprintf(stderr, "Error: failed to open file \"%s\"\n", filename);
        return -1;
    }

    // tiff file header固定长度是14字节
    io_read(s, &byteorder, 2);
    io_read(s, &version, 2);
    io_read(s, &offset, 4);
    while (offset && !io_feof(s)) {
        if ((ret = tiff_read_ifd(c, s, offset)) < 0) {
            fprintf(stderr, "Error: failed to read ifd at portion %d\n", offset);
            return ret;
        }
        io_read(s, &offset, 4); // next ifd offset
    }
}

int extract_tiff_data(uint8_t **data, int *width, int *height, int *bitcount, int *samples_per_pixel, int *photo_interp, char *filename)
{
    TIFFContext *c, *ctx;

    c = (TIFFContext *)malloc(sizeof(TIFFContext));
    memset(c, 0, sizeof(TIFFContext));
    read_tiff(c, filename);

    /*std::stack<TIFFContext*> st;
    st.push(c);
    while(st.size()) {
        ctx = st.top();
        st.pop();
        if (ctx->new_subfile_type == 0)
            break;
        for (int i = 0; i < c->subifds_num; ++i)
            st.push((TIFFContext*)(c->subifds[i].ptr));
    }*/
    ctx = c;

    *data              = ctx->image_data;
    *width             = ctx->image_width;
    *height            = ctx->image_length;
    *bitcount          = ctx->bitcount;
    *samples_per_pixel = ctx->samples_per_pixel;
    *photo_interp      = ctx->photo_interp;

    fprintf(stderr, "Extract succeeded.\n");
}

int write_tiff_default(char *filename, uint8_t *rgb, int width, int height)
{
    struct stat st;
    int size;
    int i, ret;
    uint8_t *p;
    uint16_t byteorder = 0x4949, version = 42;
    uint32_t offset = 8, pos = 10;
    uint16_t number = 17;
    IOContext *s;
    DEntry entry;

    if ((ret = io_open(&s, filename, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
        fprintf(stderr, "Error: failed to open file \"%s\"\n", filename);
        return -1;
    }

    // tiff file header固定长度是14字节
    io_write(s, &byteorder, 2);
    io_write(s, &version, 2);
    io_write(s, &offset, 4);

    // entries number
    io_write(s, &number, 2);

    offset = 8 + 2 + number * sizeof(DEntry);

    // newsubfile
    entry.tag   = 0xfe;
    entry.type  = TIFF_DATA_TYPE_LONG;
    entry.count = 1;
    entry.value = 0; // main image: 0; thumbnail image: not 0
    io_write(s, &entry, sizeof(DEntry));

    // width
    entry.tag   = 0x100;
    entry.type  = TIFF_DATA_TYPE_LONG;
    entry.count = 1;
    entry.value = width;
    io_write(s, &entry, sizeof(DEntry));

    // height
    entry.tag   = 0x101;
    entry.type  = TIFF_DATA_TYPE_LONG;
    entry.count = 1;
    entry.value = height;
    io_write(s, &entry, sizeof(DEntry));
    
    // photo_interp
    entry.tag   = 0x106;
    entry.type  = TIFF_DATA_TYPE_LONG;
    entry.count = 1;
    entry.value = 2; // rgb
    io_write(s, &entry, sizeof(DEntry));
    
    // samples_per_pixel
    entry.tag   = 0x115;
    entry.type  = TIFF_DATA_TYPE_SHORT;
    entry.count = 1;
    entry.value = SAMPLES_PER_PIXEL;
    io_write(s, &entry, sizeof(DEntry));
    
    // bits_per_sample
    int bitcount = BITS_PER_SAMPLE * SAMPLES_PER_PIXEL;
    int bytes_per_pixel = (bitcount + 7) >> 3;
    entry.tag   = 0x102;
    entry.type  = TIFF_DATA_TYPE_SHORT;
    entry.count = SAMPLES_PER_PIXEL;
    entry.value = offset;
    io_write(s, &entry, sizeof(DEntry));
    pos = io_tell(s);
    io_seek(s, offset, SEEK_SET);
    uint16_t *bits_per_sample = (uint16_t*)malloc(SAMPLES_PER_PIXEL * sizeof(uint16_t));
    for (i = 0; i < SAMPLES_PER_PIXEL; ++i) bits_per_sample[i] = BITS_PER_SAMPLE;
    io_write(s, bits_per_sample, SAMPLES_PER_PIXEL * sizeof(uint16_t));
    offset += 3 * sizeof(uint16_t);
    io_seek(s, pos, SEEK_SET);

    // compression
    entry.tag   = 0x103;
    entry.type  = TIFF_DATA_TYPE_SHORT;
    entry.count = 1;
    entry.value = 1;
    io_write(s, &entry, sizeof(DEntry));
    
    // orientation
    entry.tag   = 0x112;
    entry.type  = TIFF_DATA_TYPE_SHORT;
    entry.count = 1;
    entry.value = 1;
    io_write(s, &entry, sizeof(DEntry));
    
    // planar_configuration
    entry.tag   = 0x11c;
    entry.type  = TIFF_DATA_TYPE_SHORT;
    entry.count = 1;
    entry.value = 1;
    io_write(s, &entry, sizeof(DEntry));
    
    // x_resolution
    entry.tag   = 0x11a;
    entry.type  = TIFF_DATA_TYPE_RATIONAL;
    entry.count = 1;
    entry.value = offset;
    io_write(s, &entry, sizeof(DEntry));
    pos = io_tell(s);
    io_seek(s, offset, SEEK_SET);
    uint32_t x_resolution[2] = { 72, 1 };
    io_write(s, x_resolution, 2 * sizeof(uint32_t));
    offset += 2 * sizeof(uint32_t);
    io_seek(s, pos, SEEK_SET);
    
    // y_resolution
    entry.tag   = 0x11b;
    entry.type  = TIFF_DATA_TYPE_RATIONAL;
    entry.count = 1;
    entry.value = offset;
    io_write(s, &entry, sizeof(DEntry));
    pos = io_tell(s);
    io_seek(s, offset, SEEK_SET);
    uint32_t y_resolution[2] = { 72, 1 };
    io_write(s, y_resolution, 2 * sizeof(uint32_t));
    offset += 2 * sizeof(uint32_t);
    io_seek(s, pos, SEEK_SET);
    
    // resolution_unit
    entry.tag   = 0x128;
    entry.type  = TIFF_DATA_TYPE_SHORT;
    entry.count = 1;
    entry.value = 2;
    io_write(s, &entry, sizeof(DEntry));
    
    // rows_per_strip
    entry.tag   = 0x116;
    entry.type  = TIFF_DATA_TYPE_LONG;
    entry.count = 1;
    entry.value = ROWS_PER_STRIP;
    io_write(s, &entry, sizeof(DEntry));
    
    // strip_byte_counts
    int num_strips = (height + ROWS_PER_STRIP - 1) / ROWS_PER_STRIP;
    int byte_counts_per_strip = (width * bitcount * ROWS_PER_STRIP + 7) >> 3;
    entry.tag   = 0x117;
    entry.type  = TIFF_DATA_TYPE_LONG;
    entry.count = num_strips;
    entry.value = offset;
    io_write(s, &entry, sizeof(DEntry));
    pos = io_tell(s);
    io_seek(s, offset, SEEK_SET);
    uint32_t *strip_byte_counts = (uint32_t*)malloc(num_strips * sizeof(uint32_t));
    long long strip_bytes = 0;
    for (i = 0; i < num_strips - 1; ++i) {
        strip_byte_counts[i] = byte_counts_per_strip;
        strip_bytes += byte_counts_per_strip;
    }
    strip_byte_counts[i] = ((width * bitcount * height + 7) >> 3) - strip_bytes;
    io_write(s, strip_byte_counts, num_strips * sizeof(uint32_t));
    offset += num_strips * sizeof(uint32_t);
    io_seek(s, pos, SEEK_SET);

    // strip_offsets
    entry.tag   = 0x111;
    entry.type  = TIFF_DATA_TYPE_LONG;
    entry.count = num_strips;
    entry.value = offset;
    io_write(s, &entry, sizeof(DEntry));
    pos = io_tell(s);
    io_seek(s, offset, SEEK_SET);
    uint32_t strip_offsets[4];
    strip_offsets[0] = 4096;
    for (i = 1; i < num_strips; ++i)
        strip_offsets[i] = strip_offsets[i - 1] + strip_byte_counts[i - 1];
    io_write(s, strip_offsets, num_strips * sizeof(uint32_t));
    offset += num_strips * sizeof(uint32_t);
    io_seek(s, pos, SEEK_SET);

    /*// cfa_repeat_pattern_dim
    entry.tag   = 0x828d;
    entry.type  = TIFF_DATA_TYPE_SHORT;
    entry.count = 2;
    entry.value = 0x00020002;
    io_write(s, &entry, sizeof(DEntry));
    
    // cfa_pattern
    entry.tag   = 0x828e;
    entry.type  = TIFF_DATA_TYPE_BYTE;
    entry.count = 4;
    entry.value = 0x01000201; // gbrg
    io_write(s, &entry, sizeof(DEntry));*/

    // dng_version
    entry.tag   = 0xc612;
    entry.type  = TIFF_DATA_TYPE_BYTE;
    entry.count = 4;
    entry.value = 0x101;
    io_write(s, &entry, sizeof(DEntry));
    
    // unique_camera_model
    entry.tag   = 0xc614;
    entry.type  = TIFF_DATA_TYPE_BYTE;
    entry.count = 8;
    entry.value = offset;
    io_write(s, &entry, sizeof(DEntry));
    pos = io_tell(s);
    io_seek(s, offset, SEEK_SET);
    const char*unique_camera_model = "VEEVEEV";
    io_write(s, (void*)unique_camera_model, entry.count);
    offset += entry.count;
    io_seek(s, pos, SEEK_SET);
    
    p = rgb;
    for (i = 0; i < num_strips; p += strip_byte_counts[i], ++i) {
        io_seek(s, strip_offsets[i], SEEK_SET);
        io_write(s, p, strip_byte_counts[i]);
    }
}

int init_tiff_ctx(TIFFContext **s, uint8_t *data, int width, int height, int bitcount, int samples_per_pixel, int photo_interp/* = 2*/)
{
    TIFFContext *c;
    int i, ret;

    c = (TIFFContext *)malloc(sizeof(TIFFContext));
    memset(c, 0, sizeof(TIFFContext));
    c->image_data = data;
    c->new_subfile_type = 0;
    c->image_width = width; // width
    c->image_length = height; // height
    c->samples_per_pixel = samples_per_pixel;
    c->bits_per_sample = (uint16_t*)malloc(c->samples_per_pixel * sizeof(uint16_t)); // # of values is samples_per_pixel. when these values are "Vendor Unique", samples_per_pixel > 32767.
    for (i = 0; i < c->samples_per_pixel; ++i) c->bits_per_sample[i] = BITS_PER_SAMPLE;
    c->compression = 1;
    c->photo_interp = photo_interp; // photometric_interpretation. 1: BlackIsZero(grayscale images); 2: RGB; 6: YCbCr; 32803: CFA

    #if 0
    c->rows_per_strip = ROWS_PER_STRIP;
    c->strips_per_image = (c->image_length + c->rows_per_strip - 1) / c->rows_per_strip;
    c->strip_byte_counts = (uint32_t*)malloc(c->strips_per_image * sizeof(uint32_t)); // # of values is strips_per_image.
    c->strip_offsets = (uint32_t*)malloc(c->strips_per_image * sizeof(uint32_t)); // # of values is strips_per_image. strips_per_image = (image_length + rows_per_strip - 1) / rows_per_strip.
    int byte_counts_per_strip = (c->image_width * bitcount * c->rows_per_strip + 7) >> 3;
    c->strip_offsets[0] = 4096;
    for (i = 0; i < c->strips_per_image - 1; ++i) {
        c->strip_byte_counts[i] = byte_counts_per_strip;
        c->strip_offsets[i + 1] = c->strip_offsets[i] + byte_counts_per_strip;
    }
    c->strip_byte_counts[i] = ((c->image_width * bitcount * c->image_length + 7) >> 3) - byte_counts_per_strip * (c->strips_per_image - 1);
    #else
    c->rows_per_strip = c->image_length;
    c->strips_per_image = 1;
    c->strip_byte_counts = (uint32_t*)malloc(c->strips_per_image * sizeof(uint32_t)); // # of values is strips_per_image.
    c->strip_offsets = (uint32_t*)malloc(c->strips_per_image * sizeof(uint32_t)); // # of values is strips_per_image. strips_per_image = (image_length + rows_per_strip - 1) / rows_per_strip.
    c->strip_byte_counts[0] = (c->image_width * bitcount * c->rows_per_strip + 7) >> 3;
    c->strip_offsets[0] = 4096;
    #endif

    c->orientation = 1;
    c->planar_configuration = 1;
    c->x_resolution[0] = 72, c->x_resolution[1] = 1;
    c->y_resolution[0] = 72, c->y_resolution[1] = 1;
    c->resolution_unit = 2;

    #if 0
    // CFA tags
    c->cfa_repeat_pattern_dim[0] = 2; // cfa_repeat_rows = cfa_repeat_pattern_dim[0], cfa_repeat_cols = cfa_repeat_pattern_dim[1]
    c->cfa_repeat_pattern_dim[1] = 2;
    c->cfa_pattern = (uint8_t*)malloc(c->cfa_repeat_pattern_dim[0] * c->cfa_repeat_pattern_dim[1] * sizeof(uint8_t)); // # of values is cfa_repeat_rows * cfa_repeat_cols.
    c->cfa_pattern[0] = 1; // g
    c->cfa_pattern[1] = 2; // b
    c->cfa_pattern[2] = 0; // r
    c->cfa_pattern[3] = 1; // g
    // DNG-specific tags
    c->dng_version[0] = 1, c->dng_version[1] = 1;
    #endif

    c->unique_camera_model = (uint8_t*)malloc(8);
    memcpy(c->unique_camera_model, "VEEVEEV", 8);

    *s = c;
}
int write_tiff_tag(IOContext *s, uint16_t tag, uint16_t type, uint32_t count, uint8_t *values, uint32_t *offset)
{
    int size, pos;
    DEntry entry = { tag, type, count, 0 };

    size = count * typesize_table[type];
    if (size > 4) {
        entry.value = *offset;
        io_write(s, &entry, sizeof(DEntry));
        pos = io_tell(s);
        io_seek(s, *offset, SEEK_SET);
        io_write(s, values, size);
        io_seek(s, pos, SEEK_SET);
        *offset += size;
    } else {
        memcpy(&entry.value, values, size); // entry.value = *((uint32_t*)values);
        io_write(s, &entry, sizeof(DEntry));
    }
}
int write_tiff(TIFFContext *c, char *filename)
{
    struct stat st;
    int size;
    int i, ret;
    uint8_t *p;
    uint16_t byteorder, version;
    uint32_t offset;
    uint16_t number = 17;
    IOContext *s;

    if ((ret = io_open(&s, filename, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
        fprintf(stderr, "Error: failed to open file \"%s\"\n", filename);
        return -1;
    }

    // tiff file header固定长度是14字节
    byteorder = 0x4949;
    version = 42;
    offset = 8;
    io_write(s, &byteorder, 2);
    io_write(s, &version, 2);
    io_write(s, &offset, 4);

    // entries number
    number = 16;
    io_write(s, &number, 2);

    offset = 8 + 2 + number * sizeof(DEntry) + 4;

    write_tiff_tag(s, 0xfe, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->new_subfile_type, &offset);
    write_tiff_tag(s, 0x100, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->image_width, &offset);
    write_tiff_tag(s, 0x101, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->image_length, &offset);
    write_tiff_tag(s, 0x106, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->photo_interp, &offset);
    write_tiff_tag(s, 0x115, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->samples_per_pixel, &offset);
    write_tiff_tag(s, 0x102, TIFF_DATA_TYPE_SHORT, c->samples_per_pixel, (uint8_t*)c->bits_per_sample, &offset);
    write_tiff_tag(s, 0x103, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->compression, &offset);
    write_tiff_tag(s, 0x112, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->orientation, &offset);
    write_tiff_tag(s, 0x11c, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->planar_configuration, &offset);
    write_tiff_tag(s, 0x11a, TIFF_DATA_TYPE_RATIONAL, 1, (uint8_t*)c->x_resolution, &offset);
    write_tiff_tag(s, 0x11b, TIFF_DATA_TYPE_RATIONAL, 1, (uint8_t*)c->y_resolution, &offset);
    write_tiff_tag(s, 0x128, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->resolution_unit, &offset);
    write_tiff_tag(s, 0x116, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->rows_per_strip, &offset);
    write_tiff_tag(s, 0x117, TIFF_DATA_TYPE_LONG, c->strips_per_image, (uint8_t*)c->strip_byte_counts, &offset);
    write_tiff_tag(s, 0x111, TIFF_DATA_TYPE_LONG, c->strips_per_image, (uint8_t*)c->strip_offsets, &offset);
    write_tiff_tag(s, 0xc614, TIFF_DATA_TYPE_BYTE, strlen((char*)c->unique_camera_model) + 1, (uint8_t*)c->unique_camera_model, &offset);

    p = c->image_data;
    for (i = 0; i < c->strips_per_image; p += c->strip_byte_counts[i], ++i) {
        io_seek(s, c->strip_offsets[i], SEEK_SET);
        io_write(s, p, c->strip_byte_counts[i]);
    }
}

int init_dng_ctx(TIFFContext **s, uint8_t *bayer, int width, int height)
{
    TIFFContext *c;
    int i;

    c = (TIFFContext *)malloc(sizeof(TIFFContext));
    memset(c, 0, sizeof(TIFFContext));
    c->image_data = bayer;
    c->new_subfile_type = 0;
    c->image_width = width; // width
    c->image_length = height; // height
    c->samples_per_pixel = 1;
    c->bits_per_sample = (uint16_t*)malloc(sizeof(uint16_t)); // # of values is samples_per_pixel. when these values are "Vendor Unique", samples_per_pixel > 32767.
    c->bits_per_sample[0] = BITS_PER_SAMPLE;
    c->compression = 1;
    c->photo_interp = 32803; // photometric_interpretation. 1: BlackIsZero(grayscale images); 2: RGB; 6: YCbCr; 32803: CFA
    c->rows_per_strip = ROWS_PER_STRIP;
    c->strips_per_image = (c->image_length + c->rows_per_strip - 1) / c->rows_per_strip;
    c->strip_byte_counts = (uint32_t*)malloc(c->strips_per_image * sizeof(uint32_t)); // # of values is strips_per_image.
    c->strip_offsets = (uint32_t*)malloc(c->strips_per_image * sizeof(uint32_t)); // # of values is strips_per_image. strips_per_image = (image_length + rows_per_strip - 1) / rows_per_strip.
    int byte_counts_per_strip = (c->image_width * BITS_PER_SAMPLE * c->rows_per_strip + 7) >> 3;
    c->strip_offsets[0] = 4096;
    for (i = 0; i < c->strips_per_image - 1; ++i) {
        c->strip_byte_counts[i] = byte_counts_per_strip;
        c->strip_offsets[i + 1] = c->strip_offsets[i] + byte_counts_per_strip;
    }
    c->strip_byte_counts[i] = ((c->image_width * BITS_PER_SAMPLE * c->image_length + 7) >> 3) - byte_counts_per_strip * (c->strips_per_image - 1);
    c->orientation = 1;
    c->planar_configuration = 1;
    c->x_resolution[0] = 72, c->x_resolution[1] = 1;
    c->y_resolution[0] = 72, c->y_resolution[1] = 1;
    c->resolution_unit = 2;
    c->cfa_repeat_pattern_dim[0] = 2; // cfa_repeat_rows = cfa_repeat_pattern_dim[0], cfa_repeat_cols = cfa_repeat_pattern_dim[1]
    c->cfa_repeat_pattern_dim[1] = 2;
    c->cfa_pattern = (uint8_t*)malloc(c->cfa_repeat_pattern_dim[0] * c->cfa_repeat_pattern_dim[1] * sizeof(uint8_t)); // # of values is cfa_repeat_rows * cfa_repeat_cols.
    c->cfa_pattern[0] = 1; // g
    c->cfa_pattern[1] = 2; // b
    c->cfa_pattern[2] = 0; // r
    c->cfa_pattern[3] = 1; // g
    // DNG-specific tags
    c->dng_version[0] = 1, c->dng_version[1] = 1;
    c->unique_camera_model = (uint8_t*)malloc(8);
    memcpy(c->unique_camera_model, "VEEVEEV", 8);
    c->linearization_table_num = 256;
    c->linearization_table = (uint16_t*)malloc(c->linearization_table_num * sizeof(uint16_t));
    for (i = 0; i < c->linearization_table_num; ++i) c->linearization_table[i] = i;
    c->color_matrix1[0][0] = 19456; c->color_matrix1[0][1] = 10000;
    c->color_matrix1[4][0] = 12288; c->color_matrix1[4][1] = 10000;
    c->color_matrix1[8][0] = 12288; c->color_matrix1[8][1] = 10000;
    c->calibration_illuminant1 = 255;

    *s = c;
}
int write_dng(TIFFContext *c, char *filename)
{
    struct stat st;
    int size;
    int i, ret;
    uint8_t *p;
    uint16_t byteorder, version;
    uint32_t offset;
    uint16_t number = 17;
    IOContext *s;

    if ((ret = io_open(&s, filename, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
        fprintf(stderr, "Error: failed to open file \"%s\"\n", filename);
        return -1;
    }

    // tiff file header固定长度是14字节
    byteorder = 0x4949;
    version = 42;
    offset = 8;
    io_write(s, &byteorder, 2);
    io_write(s, &version, 2);
    io_write(s, &offset, 4);

    // entries number
    number = 24;
    io_write(s, &number, 2);

    offset = 8 + 2 + number * sizeof(DEntry) + 4;

    write_tiff_tag(s, 0xfe, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->new_subfile_type, &offset);
    write_tiff_tag(s, 0x100, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->image_width, &offset);
    write_tiff_tag(s, 0x101, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->image_length, &offset);
    write_tiff_tag(s, 0x106, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->photo_interp, &offset);
    write_tiff_tag(s, 0x115, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->samples_per_pixel, &offset);
    write_tiff_tag(s, 0x102, TIFF_DATA_TYPE_SHORT, c->samples_per_pixel, (uint8_t*)c->bits_per_sample, &offset);
    write_tiff_tag(s, 0x103, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->compression, &offset);
    write_tiff_tag(s, 0x112, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->orientation, &offset);
    write_tiff_tag(s, 0x11c, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->planar_configuration, &offset);
    write_tiff_tag(s, 0x11a, TIFF_DATA_TYPE_RATIONAL, 1, (uint8_t*)c->x_resolution, &offset);
    write_tiff_tag(s, 0x11b, TIFF_DATA_TYPE_RATIONAL, 1, (uint8_t*)c->y_resolution, &offset);
    write_tiff_tag(s, 0x128, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->resolution_unit, &offset);
    write_tiff_tag(s, 0x116, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&c->rows_per_strip, &offset);
    write_tiff_tag(s, 0x117, TIFF_DATA_TYPE_LONG, c->strips_per_image, (uint8_t*)c->strip_byte_counts, &offset);
    write_tiff_tag(s, 0x111, TIFF_DATA_TYPE_LONG, c->strips_per_image, (uint8_t*)c->strip_offsets, &offset);
    write_tiff_tag(s, 0x828d, TIFF_DATA_TYPE_SHORT, 2, (uint8_t*)c->cfa_repeat_pattern_dim, &offset);
    write_tiff_tag(s, 0x828e, TIFF_DATA_TYPE_BYTE, c->cfa_repeat_pattern_dim[0] * c->cfa_repeat_pattern_dim[1], (uint8_t*)c->cfa_pattern, &offset);
    write_tiff_tag(s, 0xc612, TIFF_DATA_TYPE_BYTE, 4, (uint8_t*)c->dng_version, &offset);
    write_tiff_tag(s, 0xc614, TIFF_DATA_TYPE_BYTE, strlen((char*)c->unique_camera_model) + 1, (uint8_t*)c->unique_camera_model, &offset);

    short default_crop_origin[2] = { 2, 2 }, default_crop_size[2] = { c->image_width - 4, c->image_length - 4 };
    write_tiff_tag(s, 0xc61f, TIFF_DATA_TYPE_SHORT, 2, (uint8_t*)default_crop_origin, &offset);
    write_tiff_tag(s, 0xc620, TIFF_DATA_TYPE_SHORT, 2, (uint8_t*)default_crop_size, &offset);

    //write_tiff_tag(s, 0xc621, TIFF_DATA_TYPE_SIGNED_RATIONAL, 9, (uint8_t*)c->color_matrix1, &offset);
    /*int32_t color_matrix1[9][2] = {
        { 10469, 10000 },
        { -5314, 10000 },
        { 1280, 10000 },
        { -4326, 10000 },
        { 12176, 10000 },
        { 2419, 10000 },
        { -886, 10000 },
        { 2473, 10000 },
        { 7160, 10000 },
    };*/
    int32_t color_matrix1[9][2] = {
        { 14361, 4096 },
        { -6554, 4096 },
        { -2228, 4096 },
        { -3891, 4096 },
        { 8098, 4096 },
        { 246, 4096 },
        { 231, 4096 },
        { -807, 4096 },
        { 4825, 4096 },
    };
    write_tiff_tag(s, 0xc621, TIFF_DATA_TYPE_SIGNED_RATIONAL, 9, (uint8_t*)color_matrix1, &offset);

    // 非必要tags
    //write_tiff_tag(s, 0xc618, TIFF_DATA_TYPE_SHORT, c->linearization_table_num, (uint8_t*)c->linearization_table, &offset);
    // 似乎white_level值需要满足一定条件
    //uint32_t white_level = 255;
    //write_tiff_tag(s, 0xc61d, TIFF_DATA_TYPE_LONG, 1, (uint8_t*)&white_level, &offset);
    //write_tiff_tag(s, 0xc65a, TIFF_DATA_TYPE_SHORT, 1, (uint8_t*)&c->calibration_illuminant1, &offset);

    p = c->image_data;
    for (i = 0; i < c->strips_per_image; p += c->strip_byte_counts[i], ++i) {
        io_seek(s, c->strip_offsets[i], SEEK_SET);
        io_write(s, p, c->strip_byte_counts[i]);
    }
}
