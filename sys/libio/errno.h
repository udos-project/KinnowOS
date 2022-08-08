#ifndef __LIBIO_ERRNO_H__
#define __LIBIO_ERRNO_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

/* Reference for error defines:
 * https://linux.die.net/man/3/errno
 */
#define E2BIG 1
#define EACCES 2
#define EADDRINUSE 3
#define EADDRNOTAVAIL 4
#define EAFNOSUPPORT 5
#define EAGAIN 6
#define EALREADY 7
#define EBADE 8
#define EBADF 9
#define EBADFD 10
#define EBADMSG 11
#define EBADR 12
#define EBADRQC 13
#define EBADSLT 14
#define EBUSY 15
#define ECANCELED 16
#define ECHILD 17
#define ECHRNG 18
#define ECOMM 19
#define ECONNABORTED 20
#define ECONNREFUSED 21
#define ECONNRESET 22
#define EDEADLK 23
#define EDESTADDRREQ 25
#define EDOM 26
#define EDQUOT 27
#define EEXIST 28
#define EFAULT 29
#define EFBIG 30
#define EHOSTDOWN 31
#define EHOSTUNREACH 32
#define EHWPOISON 33
#define EIDRM 34
#define EILSEQ 35
#define EINPROGRESS 36
#define EINTR 37
#define EINVAL 38
#define EIO 39
#define EISCONN 40
#define EISDIR 41
#define EISNAM 42
#define EKEYEXPIRED 43
#define EKEYREJECTED 44
#define EKEYREVOKED 45
#define EL2HLT 46
#define EL2NSYNC 47
#define EL3HLT 48
#define EL3RST 49
#define ELIBACC 50
#define ELIBBAD 51
#define ELIBMAX 52
#define ELIBSCN 53
#define ELIBEXEC 54
#define ELNRANGE 55
#define ELOOP 56
#define EMEDIUMTYPE 57
#define EMFILE 58
#define EMLINK 59
#define EMSGSIZE 60
#define EMULTIHOP 61
#define ENAMETOOLONG 62
#define ENETDOWN 63
#define ENETRESET 64
#define ENETUNREACH 65
#define ENFILE 66
#define ENOANO 67
#define ENOBUFS 68
#define ENODATA 69
#define ENODEV 70
#define ENOENT 71
#define ENOEXEC 72
#define ENOKEY 73
#define ENOLCK 74
#define ENOLINK 75
#define ENOMEDIUM 76
#define ENOMEM 77
#define ENOMSG 78
#define ENONET 79
#define ENOPKG 80
#define ENOPROTOOPT 81
#define ENOSPC 82
#define ENOSR 83
#define ENOSTR 84
#define ENOSYS 85
#define ENOTBLK 86
#define ENOTCONN 87
#define ENOTDIR 88
#define ENOTEMPTY 89
#define ENOTRECOVERABLE 90
#define ENOTSOCK 91
#define ENOTSUP 92
#define ENOTTY 93
#define ENOTUNIQ 94
#define ENXIO 95
#define EOVERFLOW 97
#define EOWNERDEAD 98
#define EPERM 99
#define EPFNOSUPPORT 100
#define EPIPE 101
#define EPROTO 102
#define EPROTONOSUPPORT 103
#define EPROTOTYPE 104
#define ERANGE 105
#define EREMCHG 106
#define EREMOTE 107
#define EREMOTEIO 108
#define ERESTART 109
#define ERFKILL 110
#define EROFS 111
#define ESHUTDOWN 112
#define ESPIPE 113
#define ESOCKTNOSUPPORT 114
#define ESRCH 115
#define ESTALE 116
#define ESTRPIPE 117
#define ETIME 118
#define ETIMEDOUT 119
#define ETOOMANYREFS 120
#define ETXTBSY 121
#define EUCLEAN 122
#define EUNATCH 123
#define EUSERS 124
#define EXDEV 126
#define EXFULL 127

/* Aliases */
#define EDEADLOCK EDEADLK
#define EWOULDBLOCK EBUSY
#define EOPNOTSUPP EOPNOTSUP

/* ISO C recommends not doing this but who cares? */
extern int errno;

#ifdef __cplusplus
}
#endif

#endif
