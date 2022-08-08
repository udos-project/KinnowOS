#ifndef __LIBIO_SETJMP_H__
#define __LIBIO_SETJMP_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void *jmp_buf[1];

void _longjmp(jmp_buf env, int val);
#define longjmp(env, val) _longjmp(env, val)

int _setjmp(jmp_buf env);
#define setjmp(env) _setjmp(env)

#ifdef __cplusplus
}
#endif

#endif
