#ifndef __BITS_H
#define __BITS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "tiff.h"

int scaleraw(TIFFDirectory *td, int dst_width, int dst_height);

#endif /* __BITS_H */