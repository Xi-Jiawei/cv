#ifndef __UTILS_H
#define __UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

#define ABS(a) (a < 0 ? -(a) : a)
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLIP(x,min,max) MAX(min, MIN(max, x))
#define CLIP255(x) MAX(0, MIN(255, x))
#define CLAMP(x,min,max) MAX(min, MIN(max, x))
#define CLAMP255(x) MAX(0, MIN(255, x))

#define CLIP4095(x) CLIP(x,0,4095)
#define CLAMP4095(x) CLAMP(x,0,4095)

#define MKSHORTTAG(a,b) (a | (b << 8))
#define MKBESHORTTAG(a,b) (b | (a << 8))

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#define ERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))

#define ERROR_EOF ERRTAG( 'E','O','F',' ') ///< End of file

#define ERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#define UNERROR(e) (-(e)) ///< Returns a POSIX error code from a library function error return value.

/**
 * ORing this as the "whence" parameter to a seek function causes it to
 * return the filesize without seeking anywhere. Supporting this is optional.
 * If it is not supported then the seek function will return <0.
 */
#define SEEK_SIZE 0x10000

/**
 * @name URL open modes
 * The flags argument to avio_open must be one of the following
 * constants, optionally ORed with other flags.
 * @{
 */
#define IO_FLAG_READ  1                                      /**< read-only */
#define IO_FLAG_WRITE 2                                      /**< write-only */
#define IO_FLAG_READ_WRITE (IO_FLAG_READ|IO_FLAG_WRITE)      /**< read-write pseudo flag */

#ifdef __GNUC__
#    define GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
#    define GCC_VERSION_AT_MOST(x,y)  (__GNUC__ < (x) || __GNUC__ == (x) && __GNUC_MINOR__ <= (y))
#else
#    define GCC_VERSION_AT_LEAST(x,y) 0
#    define GCC_VERSION_AT_MOST(x,y)  0
#endif

#ifndef av_always_inline
#if GCC_VERSION_AT_LEAST(3,1)
#    define av_always_inline __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#    define av_always_inline __forceinline
#else
#    define av_always_inline inline
#endif
#endif

/* standard file protocol */
typedef struct FileContext {
    int fd;                 /**< FILE descriptation */
    int64_t pos;            /**< position in the file */
    char *filename;         /**< specified URL */
    int eof_reached;        /**< true if was unable to read due to error or eof */
#if HAVE_DIRENT_H
    DIR *dir;
#endif
} FileContext;

typedef struct IOContext {
    FileContext *fc;

    unsigned char *buffer;  /**< Start of the buffer. */
    int buffer_size;        /**< Maximum buffer size */
    unsigned char *buf_ptr; /**< Current position in the buffer */
    unsigned char *buf_end;
} IOContext;

char *get_file_name(const char* filepath);
char *get_file_ext(const char* filepath);
int match_ext(const char *file, const char *ext);

int io_open(IOContext **s, const char *filename, int flags);
int io_close(IOContext *s);
int io_feof(IOContext *s);
int64_t io_seek(IOContext *s, int64_t offset, int whence);
int64_t io_skip(IOContext *s, int64_t offset);
int64_t io_tell(IOContext *s);
int io_read(IOContext *s, void *buf, int size);
int io_write(IOContext *s, void *buf, int size);

#endif /* __UTILS_H */