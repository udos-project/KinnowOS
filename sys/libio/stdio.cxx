#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <svc.h>

/// @todo The C-library should attempt to perform reads/writes of remainders
/// for example if a fputs call, which uses fread under the hood, only writes 1/3 of
/// what it was told to, the current implementation thinks "Well, that's it, my job is
/// done, I hate all userland programs". And that's not very good for our fellow *nix-y
/// programs.

FILE _files[FOPEN_MAX] = {0};

FILE *stdin = nullptr;
FILE *stdout = nullptr;
FILE *stderr = nullptr;
FILE *stdprn = nullptr;

STDAPI FILE *fopen(const char *__restrict__ name, const char *__restrict__ mode)
{
    int oflag = 0;
    int idx;

    /* Parse mode string, see: https://pubs.opengroup.org/onlinepubs/007904875/functions/fopen.html
     * for a reference on parsing the mode string */
    /** @todo The reference says update and writing as if they were different, however we don't
     * treat those differently? */
    if(*mode == 'r') {
        oflag = O_RDONLY;
        mode++;
        if(*mode == '+') {
            oflag = O_RDWR;
        }
    } else if(*mode == 'w') {
        oflag = O_CREAT | O_TRUNC;
        mode++;
        if(*mode == '+') {
            oflag = O_CREAT | O_TRUNC;
        }
    } else if(*mode == 'a') {
        oflag = O_APPEND | O_CREAT;
        if(*mode == '+') {
            oflag = O_APPEND | O_CREAT;
        }
    }

    idx = open(name, oflag);
    if(idx < 0) {
        errno = idx;
        return nullptr;
    }
    return &_files[idx];
}

STDAPI size_t fwrite(const void *__restrict__ buf, size_t size, size_t n_elem, FILE *__restrict__ fp)
{
    int r;
    assert(fp != nullptr);
    r = (int)io_svc(SVC_VFS_WRITE, (uintptr_t)fp->handle, (uintptr_t)buf, (uintptr_t)size * n_elem);
    if(r < 0) {
        errno = -EBUSY;
        return 0;
    }
    return (size_t)r;
}

/// @brief Reads a binary buffer from a stream
/// @param buf 
/// @param size 
/// @param n_elem 
/// @param fp 
/// @return size_t 
STDAPI size_t fread(void *__restrict__ buf, size_t size, size_t n_elem, FILE *__restrict__ fp)
{
    int r;
    assert(fp != nullptr);
    r = (int)io_svc(SVC_VFS_READ, (uintptr_t)fp->handle, (uintptr_t)buf, (uintptr_t)size * n_elem);
    if(r < 0) {
        errno = -EBUSY;
        return 0;
    }
    return (size_t)r;
}

STDAPI int fputc(int ch, FILE *fp) {
    int r;
    assert(fp != nullptr);
    r = (fwrite(&ch, sizeof ch, 1, fp) == sizeof ch) ? 0 : -1;
    if(r < 0) {
        errno = -EBUSY;
        return errno;
    }
    return r;
}

STDAPI int fputs(const char *__restrict__ msg, FILE *__restrict__ fp) {
    size_t len = strlen(msg);
    int r;
    assert(fp != nullptr);
    r = (int)fwrite(msg, 1, len, fp);
    if(r < 0) {
        errno = -EBUSY;
        return errno;
    }
    return r;
}

STDAPI int fgetc(FILE *fp)
{
    char ch;
    int r;
    r = (fread(&ch, sizeof ch, 1, fp) == sizeof ch) ? 0 : -1;
    return r;
}

STDAPI char *fgets(char *__restrict__ str, int size, FILE *__restrict__ fp)
{
    int r;
    r = (int)fread(str, (size_t)size, 1, fp);
    if(r < 0) {
        errno = -EBUSY;
        return nullptr;
    }
    return str;
}

STDAPI int feof(FILE *fp)
{
    int ch;
    ch = fgetc(fp);
    fseek(fp, -1, SEEK_CUR);
    return (ch == EOF) ? 1 : 0;
}

STDAPI int vfprintf(FILE *__restrict__ fp, const char *__restrict__ fmt, va_list args) {
    char tmpbuf[BUFSIZ];
    size_t len;
    int r;
    vsnprintf(tmpbuf, sizeof tmpbuf, fmt, args);
    len = strlen(tmpbuf);
    r = (fwrite(tmpbuf, 1, len, fp) == len) ? 0 : -1;
    return r;
}

STDAPI int fprintf(FILE *__restrict__ fp, const char *__restrict__ fmt, ...) {
    int r;
    va_list args;

    va_start(args, fmt);
    r = vfprintf(fp, fmt, args);
    va_end(args);
    return r;
}

/** @todo fseek() */
STDAPI int fseek(FILE *fp, long offset, int whence)
{
    int r;
    assert(fp != nullptr);
    r = (int)_rawioctl(fp->handle, 0x03, offset, whence);
    if(r < 0) {
        errno = -EBUSY;
        return errno;
    }
    return r;
}

/** @todo fseeko() */
STDAPI int fseeko(FILE *fp, off_t offset, int whence)
{
    assert(fp != nullptr);
    return fseek(fp, (long)offset, whence);
}

STDAPI int ferror(FILE *fp)
{
    assert(fp != nullptr);
    /** @todo ferror() */
    return 0;
}

/// @brief Rewind back to the start
/// @param fp File to rewind
/// @return int Result of operation
STDAPI int rewind(FILE *fp)
{
    assert(fp != nullptr);
    return fseek(fp, 0, SEEK_SET);
}

STDAPI int fflush(FILE *fp)
{
    int r;
    assert(fp != nullptr);
    r = (int)io_svc(SVC_VFS_FLUSH, (uintptr_t)fp->handle, 0, 0);
    return r;
}

STDAPI long ftell(FILE *fp)
{
    long off;
    int r;

    r = _rawioctl(fp->handle, 0x02, &off);
    if(r < 0) {
        return (long)r;
    }
    return off;
}

STDAPI int fclose(FILE *fp)
{
    assert(fp != nullptr);
    io_svc(SVC_VFS_CLOSE, (uintptr_t)fp->handle, 0, 0);
    fp->handle = nullptr;
    return 0;
}

STDAPI int puts(const char *msg) {
    int r;
    assert(msg != nullptr);
    r = fputs(msg, stdout);
    return r;
}

STDAPI int putc(int ch, FILE *fp) {
    int r;
    r = fputc(ch, fp);
    return r;
}

STDAPI int putchar(int ch) {
    int r;
    r = fputc(ch, stdout);
    return r;
}

/// @brief Non-standard, gets a string from stdout
/// @param str Buffer to place obtained string
/// @param size Max. size of string
/// @return char* Always equal to buffer if anything was read
STDAPI char *gets(char *str, int size) {
    assert(str != nullptr);
    return fgets(str, size, stdout);
}

STDAPI int getc(FILE *fp) {
    return fgetc(fp);
}

/** @todo ungetc() */
STDAPI int ungetc(int c, FILE *stream)
{
    return EOF;
}

STDAPI int getchar(void) {
    return fgetc(stdout);
}

STDAPI int vprintf(const char *fmt, va_list args) {
    int r;
    r = vfprintf(stdout, fmt, args);
    return r;
}

STDAPI int printf(const char *fmt, ...) {
    int r;
    va_list args;
    va_start(args, fmt);
    r = vfprintf(stdout, fmt, args);
    va_end(args);
    return r;
}

STDAPI int _dprintf(const char *fmt, ...)
{
    char tmpbuf[BUFSIZ];
    va_list args;
    va_start(args, fmt);
    vsnprintf(tmpbuf, sizeof tmpbuf, fmt, args);
    io_svc(SVC_PRINT_DEBUG, (uintptr_t)tmpbuf, 0, 0);
    va_end(args);
    return 0;
}

STDAPI int vsnprintf(char *s, size_t n, const char *fmt, va_list args)
{
    size_t i = 0;

    s[0] = '\0';
    while(*fmt != '\0' && i < n - 1) {
        i = strlen(s);
        if(*fmt == '%') {
            char padchr = ' ';
            int rpad = 0;
			int base = 10;

            ++fmt;

            /* Check - modifier for right padding */
            if(*fmt == '-') {
                rpad = 1;
                ++fmt;
            }

            /* And the zero for "zero-padding" */
            if(*fmt == '0') {
                padchr = '0';
                ++fmt;
            }

            if(*fmt == 's') {
                const char *str = va_arg(args, const char *);
                size_t len = 0;

                if(str == nullptr) {
                    str = "nullptr";
                }

                len = strlen(str);
                if(len >= n - i) {
                    len = n - i;
                }
                strncat(s, str, n);
                fmt++;
            } else if(*fmt == 'c') {
                s[i++] = (char)va_arg(args, int);
                s[i] = '\0';
                fmt++;
            } else if(*fmt == 'u') {
                unsigned int val = va_arg(args, unsigned int);
                ltoa(val, &s[i], 10);
                fmt++;
            } else if(*fmt == 'p') {
                uintptr_t val = (uintptr_t)va_arg(args, void *);
                uptrtoa(val, &s[i], 16);
                fmt++;
            } else if(*fmt == 'f') {
                /** @todo Support formatting for f */
                /* va_args promotes float to double */
                float val = (float)va_arg(args, double);
                libio_llftoa((long double)val, &s[i]);
                fmt++;
            } else if(*fmt == 'x' || *fmt == 'X') {
                unsigned val = va_arg(args, unsigned);
                ultoa(val, &s[i], 16);
                if(*fmt == 'X') {
                    char *ptr = &s[i];
                    while(*ptr != '\0') {
                        *(ptr++) = toupper(*ptr);
                    }
                }
                fmt++;
            } else if(*fmt == 'o') {
                unsigned val = va_arg(args, unsigned);
                ultoa(val, &s[i], 8);
                fmt++;
            } else if(*fmt == 'b') {
                unsigned val = va_arg(args, unsigned);
                ultoa(val, &s[i], 2);
                fmt++;
            } else if(*fmt == 'l') {
                fmt++;
                if(*fmt == 'l') {
                    fmt++;
                    if(*fmt == 'f') {
                        /** @todo Support formatting for long double */
                        long double val = va_arg(args, long double);
                        libio_llftoa((long double)val, &s[i]);
                        fmt++;
                    } else if(*fmt == 'x') {
                        /** @todo Support formatting for ulong long */
                        unsigned long long val = va_arg(args, unsigned long long);
                        ultoa((unsigned long)val, &s[i], 16);
                        fmt++;
                    }
                } else if(*fmt == 'f') {
                    /** @todo Support formatting for double*/
                    double val = va_arg(args, double);
                    libio_llftoa((long double)val, &s[i]);
                    fmt++;
                } else if(*fmt == 'x') {
                    unsigned long val = va_arg(args, unsigned long);
                    ultoa(val, &s[i], 16);
                    fmt++;
                } else if(*fmt == 'o') {
                    unsigned long val = va_arg(args, unsigned long);
                    ultoa(val, &s[i], 8);
                    fmt++;
                } else {
                    long val = va_arg(args, long);
                    ltoa(val, &s[i], 16);
                    fmt++;
                }
            } else if(*fmt == 'i' || *fmt == 'd') {
                signed int val = va_arg(args, signed int);
                itoa(val, &s[i], 10);
                fmt++;
            } else if(*fmt == 'z' && *(fmt + 1) == 'u') {
                size_t val = va_arg(args, size_t);
                usizetoa(val, &s[i], 10);
                fmt += 2;
            } else if(*fmt == '%') {
                s[i++] = '%';
                s[i] = '\0';
                fmt++;
            }
        } else {
            s[i++] = *(fmt++);
            s[i] = '\0';
        }
    }
    return 0;
}

STDAPI int vsprintf(char *s, const char *fmt, va_list args)
{
    int r;
    r = vsnprintf(s, 80, fmt, args);
    return r;
}

STDAPI int snprintf(char *s, size_t n, const char *fmt, ...)
{
    va_list args;
    int r;

    va_start(args, fmt);
    r = vsnprintf(s, n, fmt, args);
    va_end(args);
    return r;
}

STDAPI int sprintf(char *s, const char *fmt, ...)
{
    va_list args;
    int r;

    va_start(args, fmt);
    r = vsnprintf(s, 80, fmt, args);
    va_end(args);
    return r;
}

/** @todo remove() */
STDAPI int remove(const char *name)
{
    io_svc(SVC_VFS_REMOVE_NODE, (uintptr_t)"/TAPE", (uintptr_t)name, 0);
    return 0;
}

/** @todo rename() */
STDAPI int rename(const char *oldname, const char *newname)
{
    while(1);
    return 0;
}

/** @todo popen() */
STDAPI FILE *popen(const char *cmd, const char *mode)
{
    while(1);
    return nullptr;
}

/** @todo pclose() */
STDAPI int pclose(FILE *fp)
{
    while(1);
    return 0;
}

STDAPI int perror(const char *msg)
{
    assert(msg != nullptr);
    return fprintf(stderr, "%s: %s\r\n", strerror(errno), msg);
}

STDAPI FILE *tmpfile(void)
{
    FILE *fp;
    char *newname;

    newname = tmpnam(nullptr);
    if(newname == nullptr) {
        return nullptr;
    }
    fp = fopen(newname, "w");
    return fp;
}

STDAPI char *tmpnam(char *s)
{
    if(s == nullptr) {
        return (char *)"/TAPE/WORK";
    }

    strcpy(s, "/TAPE/WORK");
    return s;
}
