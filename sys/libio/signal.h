#ifndef __LIBIO_SIGNAL_H__
#define __LIBIO_SIGNAL_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

typedef int sigset_t;
typedef int sig_atomic_t;

typedef void (*sighandler_t)(int);

#define SIG_ERR ((sighandler_t)-1) /* Error return */
#define SIG_DFL ((sighandler_t)0) /* Default */
#define SIG_IGN ((sighandler_t)1) /* Ignore */

/* ISO C99 */
#define SIGINT 1
#define SIGKILL 2
#define SIGABRT 3
#define SIGFPE 4
#define SIGSEGV 5
#define SIGTERM 6

sighandler_t signal(int signum, sighandler_t handler);

#ifdef __cplusplus
}
#endif

#endif
