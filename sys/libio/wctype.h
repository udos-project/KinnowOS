#ifndef __WCTYPE_H__
#define __WCTYPE_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

typedef int wint_t;
typedef int wctrans_t;
typedef int wctype_t;

int iswlower(wint_t x);
int iswupper(wint_t x);
int iswdigit(wint_t x);
int iswalpha(wint_t x);
int iswalnum(wint_t x);
int wtoupper(wint_t x);
int wtolower(wint_t x);
int iswspace(wint_t x);
int iswpunct(wint_t x);
int iswxdigit(wint_t x);

#define iswprint(x) ((iswalnum(x) || iswpunct(x) || iswblank(x)) ? 1 : 0)
#define iswgraph(x) (!(iswprint(x)) ? 1 : 0)

#ifdef __cplusplus
}
#endif

#endif
