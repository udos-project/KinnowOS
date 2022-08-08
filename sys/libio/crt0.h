#ifndef __LIBIO_CRT0_H__
#define __LIBIO_CRT0_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <bits/features.h>
#include <svc.h>

extern program_data_block pdb_area;
STDAPI extern int main(int argc, char **argv, char **envp);
STDAPI extern void _init(void);
STDAPI extern void _fini(void);
STDAPI extern void _start(void) __NORETURN;
STDAPI extern void _exit(int status) __NORETURN;

#ifdef __cplusplus
}
#endif

#endif
