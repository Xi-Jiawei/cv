#include "utils.h"

char *get_file_name(const char* filepath)
{
    char *fname, *p, *q;
    int len = strlen(filepath), flen;

    p = q = (char*)(filepath + len - 1);
    while(*p != '/' && p >= filepath) p--;
    while(*q != '.' && q >= filepath) q--;
    if (p < q) {
        p += 1;
        flen = q - p;
        fname = (char*)malloc(flen + 1);
        memcpy(fname, p, flen);
        fname[flen] = 0;
    } else {
        p += 1;
        flen = len - (p - filepath);
        fname = (char*)malloc(flen + 1);
        memcpy(fname, p, flen);
        fname[flen] = 0;
    }

    return fname;
}

char *get_file_ext(const char* filepath)
{
    char *fext = NULL, *p;
    int len = strlen(filepath), extlen;

    p = (char*)(filepath + len - 1);
    while(*p != '.' && p >= filepath) p--;
    if (p > filepath) {
        p += 1;
        extlen = len - (p - filepath);
        fext = (char*)malloc(extlen + 1);
        memcpy(fext, p, extlen);
        fext[extlen] = 0;
    }

    return fext;
}

int match_ext(const char *file, const char *ext)
{
    char *p = get_file_ext(file);
    const char *q = ext;

    if (!p)
        return 0;

    while (*p && *q && *p == *q) {
        p++;
        q++;
    }
    if (*p || *q) return 0;

    return 1;
}

int io_open(IOContext **s, const char *filename, int flags)
{
    int fd;
    IOContext *c;

    fd = open(filename, flags, 0666);
    if (fd == -1)
        return ERROR(errno);

    c = (IOContext *)malloc(sizeof(IOContext));
    memset(c, 0, sizeof(IOContext));
    *s = c;
    c->fc = (FileContext *)malloc(sizeof(FileContext) + strlen(filename) + 1);
    c->fc->filename = (char *)&(c->fc[1]);
    strcpy(c->fc->filename, filename);
    c->fc->fd          = fd;
    c->fc->pos         = 0;
    c->fc->eof_reached = 0;

    return 0;
}

int io_close(IOContext *s)
{
    if(!s)
        return 0;
    if (s->fc) {
        close(s->fc->fd);
        free(s->fc);
    }
    free(s);
    return 0;
}

int io_feof(IOContext *s)
{
    if(!s)
        return 0;
    return s->fc->eof_reached;
}

int64_t io_seek(IOContext *s, int64_t offset, int whence)
{
    int64_t ret;

    if (whence == SEEK_SIZE) {
        struct stat st;
        ret = fstat(s->fc->fd, &st);
        return ret < 0 ? ERROR(errno) : (S_ISFIFO(st.st_mode) ? 0 : st.st_size);
    }

    ret = lseek(s->fc->fd, offset, whence);

    if (ret < 0) return ERROR(errno);
    if (whence == SEEK_SET) s->fc->pos = offset;
    else if (whence == SEEK_CUR) s->fc->pos += offset;
    return ret;
}

int64_t io_skip(IOContext *s, int64_t offset)
{
    return io_seek(s, offset, SEEK_CUR);
}

int64_t io_tell(IOContext *s)
{
    return io_seek(s, 0, SEEK_CUR);
}

int io_read(IOContext *s, void *buf, int size)
{
    int ret;
    ret = read(s->fc->fd, buf, size);
    if (ret == 0) {
        s->fc->eof_reached = 1;
        return ERROR_EOF; // ERROR(EAGAIN)
    } else if (ret < 0) {
        s->fc->eof_reached = 1;
        return ERROR(errno);
    }
    s->fc->pos += ret;
    return ret;
}

int io_write(IOContext *s, void *buf, int size)
{
    int ret;
    ret = write(s->fc->fd, buf, size);
    if (ret <= 0) {
        return ERROR(errno);
    }
    s->fc->pos += ret;
    return ret;
}
