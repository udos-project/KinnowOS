/* sys/types.h
 *
 * Reference:
 * https://pubs.opengroup.org/onlinepubs/009696899/basedefs/sys/types.h.html
 */

#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__ 1

/* size_t, ssize_t */
#include <stddef.h>

typedef unsigned int u_int;
typedef unsigned long u_long;

typedef long clock_t;
typedef int clockid_t;
typedef int dev_t;

typedef long id_t; /* Should be able to hold a gid_t, uid_t or pid_t */
typedef short gid_t;
typedef short pid_t;
typedef short uid_t;

typedef int ino_t;
typedef int key_t;
typedef int mode_t;
typedef int nlink_t;
typedef long off_t;

typedef int pthread_attr_t;
typedef int pthread_barrier_t;
typedef int pthread_barrierattr_t;
typedef int pthread_cond_t;
typedef int pthread_condattr_t;
typedef int pthread_key_t;
typedef int pthread_mutex_t;
typedef int pthread_mutexattr_t;
typedef int pthread_once_t;
typedef int pthread_rwlock_t;
typedef int pthread_rwlockattr_t;
typedef int pthread_spinlock_t;
typedef int pthread_t;

typedef long time_t;
typedef long timer_t;
typedef long useconds_t;

typedef int trace_attr_t;
typedef int trace_event_id_t;
typedef int trace_event_set_t;
typedef int trace_id_t;

/* Posix Issue 5 */
typedef int blkcnt_t;
typedef int blksize_t;
typedef int fsblkcnt_t;
typedef int fsfilcnt_t;
typedef long suseconds_t;

#endif
