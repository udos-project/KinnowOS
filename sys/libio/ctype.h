#ifndef __LIBIO_CTYPE_H__
#define __LIBIO_CTYPE_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

int islower(int x);
int isupper(int x);
int isdigit(int x);
int isalpha(int x);
int isalnum(int x);
int toupper(int x);
int tolower(int x);
int isspace(int x);
int ispunct(int x);
int isxdigit(int x);

#define isprint(x) ((isalnum(x) || ispunct(x) || isblank(x)) ? 1 : 0)
#define isgraph(x) (!(isprint(x)) ? 1 : 0)

#ifdef __cplusplus
}
#endif

#endif
