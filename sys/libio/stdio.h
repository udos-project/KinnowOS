#ifndef __LIBIO_STDIO_H__
#define __LIBIO_STDIO_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <bits/features.h>

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>
#include <limits.h>

#define FOPEN_MAX 16
#define BUFSIZ 512
#define FILENAME_MAX 24

#define L_tmpnam 255
#define TMP_MAX 1

#define EOF (-1)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct _FILE {
    int flags;
    void *handle;
} FILE;

/* Internal variables */
extern FILE _files[FOPEN_MAX];

/* Standard I/O streams */
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
extern FILE *stdprn; /* Non-standard but Turbo C provides it on DOS */

/* FIle stream I/O */
FILE *fopen(const char *__restrict__ name, const char *__restrict__ mode);
size_t fwrite(const void *__restrict__ buf, size_t size, size_t n_elem, FILE *__restrict__ fp);
size_t fread(void *__restrict__ buf, size_t size, size_t n_elem, FILE *__restrict__ fp);
int fputc(int ch, FILE *fp);
int fputs(const char *__restrict__ msg, FILE *__restrict__ fp);
int fgetc(FILE *fp);
char *fgets(char *__restrict__ str, int size, FILE *__restrict__ fp);
int feof(FILE *fp);
int vfprintf(FILE *__restrict__ fp, const char *__restrict__ fmt, va_list args);
int fprintf(FILE *__restrict__ fp, const char *__restrict__ fmt, ...);
int fseek(FILE *fp, long offset, int whence);
int fseeko(FILE *fp, off_t offset, int whence);
int ferror(FILE *fp);
int rewind(FILE *fp);
int fflush(FILE *fp);
long ftell(FILE *fp);
int fclose(FILE *fp);

/* Implicitly directed to stdout */
int puts(const char *msg);
int putc(int ch, FILE *fp);
int putchar(int ch);
char *gets(char *msg, int size);
int getc(FILE *fp);
int ungetc(int c, FILE *stream);
int getchar(void);
int vprintf(const char *fmt, va_list args);
int printf(const char *fmt, ...);
#ifdef DEBUG
int _dprintf(const char *fmt, ...);
#   define dprintf(...) _dprintf(__FILE__ ": " __VA_ARGS__)
#else
#   define dprintf(fmt, ...)
#endif
int vsnprintf(char *s, size_t n, const char *fmt, va_list args);
int vsprintf(char *s, const char *fmt, va_list args);
int snprintf(char *s, size_t n, const char *fmt, ...);
int sprintf(char *s, const char *fmt, ...);

int remove(const char *name);
int rename(const char *oldname, const char *newname);

/* Pipes */
FILE *popen(const char *cmd, const char *mode);
int pclose(FILE *fp);

int perror(const char *msg);

FILE *tmpfile(void);
char *tmpnam(char *s);

#ifdef __cplusplus
}
#endif

#endif
