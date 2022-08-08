#include <stddef.h>
#include <signal.h>
#include <errno.h>
#include <bits/features.h>

static sighandler_t _signals[8] = {
    nullptr, /* nullptr */
    nullptr, /* SIGINT */
    nullptr, /* SIGKILL */
    nullptr, /* SIGABRT */
    nullptr, /* SIGFPE */
    nullptr, /* SIGSEGV */
    nullptr, /* SIGTERM */
};

STDAPI sighandler_t signal(int signum, sighandler_t handler)
{
    sighandler_t prev_hdl;
    if(signum > (int)(sizeof(_signals) / sizeof(_signals[0]))) {
        errno = -EINVAL;
        return SIG_ERR;
    }

    prev_hdl = _signals[signum];
    _signals[signum] = handler;
    return prev_hdl;
}
