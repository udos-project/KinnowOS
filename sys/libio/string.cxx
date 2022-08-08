#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <svc.h>
#include <errno.h>

/* Not static-inlined on C89 */
#if defined(__STDC__) && !defined(__STDC_VERSION__) /* C89 */
#   include "string.inl"
#else /* C99 */

#endif

/**
 * @brief Duplicate a string
 * 
 * @param s The string to duplicate
 * @return char* The new allocated duplicated string
 */
char *strdup(const char *s)
{
    auto *p = (char *)malloc(strlen(s) + 1);
    if(p == nullptr) {
        return nullptr;
    }
    strcpy(p, s);
    return p;
}

/**
 * @brief Duplicate a string up to n characters
 * 
 * @param s The string to duplicate
 * @param n Maximum number of characters
 * @return char* The new allocated duplicated string
 */
char *strndup(const char *s, size_t n)
{
    auto *p = (char *)malloc(min(strlen(s), n) + 1);
    if(p == nullptr) {
        return nullptr;
    }
    strncpy(p, s, n);
    return p;
}

/* Reference for error messages (some are missing):
 * https://linux.die.net/man/3/errno
 */
static const char *_strerror(int err)
{
    switch(abs(err)) {
    case E2BIG:
        return "Argument list too long";
    case EACCES:
        return "Permission denied";
    case EADDRINUSE:
        return "Address already in use";
    case EADDRNOTAVAIL:
        return "Address not available";
    case EAFNOSUPPORT:
        return "Address family not supported";
    case EAGAIN:
        return "Resource temporarily unavailable";
    case EALREADY:
        return "Connection already in progress";
    case EBADE:
        return "Invalid exchange";
    case EBADF:
        return "Bad file descriptor";
    case EBADFD:
        return "File descriptor in bad state";
    case EBADMSG:
        return "Bad message";
    case EBADR:
        return "Invalid request descriptor";
    case EBADRQC:
        return "Invalid request code";
    case EBADSLT:
        return "Invalid slot";
    case EBUSY:
        return "Device or resource busy";
    case ECANCELED:
        return "Operation canceled";
    case ECHILD:
        return "No child processes";
    case ECHRNG:
        return "Channel number out of range";
    case ECOMM:
        return "Communication error on send";
    case ECONNABORTED:
        return "Connection aborted";
    case ECONNREFUSED:
        return "Connection refused";
    case ECONNRESET:
        return "Connection reset";
    case EDEADLK:
        return "Resource deadlock avoided";
    case EDESTADDRREQ:
        return "Destination address required";
    case EDOM:
        return "Mathematics argument out of domain of function";
    case EDQUOT:
        return "Disk quota exceeded";
    case EEXIST:
        return "File exists";
    case EFAULT:
        return "Bad address";
    case EFBIG:
        return "File too large";
    case EHOSTDOWN:
        return "Host is down";
    case EHOSTUNREACH:
        return "Host is unreachable";
    case EHWPOISON:
        return "Hardware poisoned"; /** @todo Correct error message */
    case EIDRM:
        return "Identifier removed";
    case EILSEQ:
        return "Illegal byte sequence";
    case EINPROGRESS:
        return "Operation in progress";
    case EINTR:
        return "Interrupted function call";
    case EINVAL:
        return "Invalid argument";
    case EIO:
        return "Input/output error";
    case EISCONN:
        return "Socket is connected";
    case EISDIR:
        return "Is a directory";
    case EISNAM:
        return "Is a named type file";
    case EKEYEXPIRED:
        return "Key has expired ";
    case EKEYREJECTED:
        return "Key was rejected by service";
    case EKEYREVOKED:
        return "Key has been revoked";
    case EL2HLT:
        return "Level 2 halted";
    case EL2NSYNC:
        return "Level 2 not synchronized";
    case EL3HLT:
        return "Level 3 halted";
    case EL3RST:
        return "Level 3 halted";
    case ELIBACC:
        return "Cannot access a needed shared library";
    case ELIBBAD:
        return "Accessing a corrupted shared library";
    case ELIBMAX:
        return "Attempting to link in too many shared libraries";
    case ELIBSCN:
        return "lib section in a.out corrupted";
    case ELIBEXEC:
        return "Cannot exec a shared library directly";
    case ELNRANGE:
        return "Level not in range"; /** @todo Correct error message */
    case ELOOP:
        return "Too many levels of symbolic links";
    case EMEDIUMTYPE:
        return "Wrong medium type";
    case EMFILE:
        return "Too many open files";
    case EMLINK:
        return "Too many links";
    case EMSGSIZE:
        return "Message too long";
    case EMULTIHOP:
        return "Multihop attempted";
    case ENAMETOOLONG:
        return "Filename too long";
    case ENETDOWN:
        return "Network is down";
    case ENETRESET:
        return "Connection aborted by network";
    case ENETUNREACH:
        return "Network unreachable";
    case ENFILE:
        return "Too many open files in system";
    case ENOANO:
        return "No ANO"; /** @todo Correct error message */
    case ENOBUFS:
        return "No buffer space available";
    case ENODATA:
        return "No message is available on the STREAM head read queue";
    case ENODEV:
        return "No such device";
    case ENOENT:
        return "No such file or directory";
    case ENOEXEC:
        return "Exec format error";
    case ENOKEY:
        return "Required key not available";
    case ENOLCK:
        return "No locks available";
    case ENOLINK:
        return "Link has been severed";
    case ENOMEDIUM:
        return "No medium found";
    case ENOMEM:
        return "Not enough space";
    case ENOMSG:
        return "No message of the desired type";
    case ENONET:
        return "Machine is not on the network";
    case ENOPKG:
        return "Package not installed";
    case ENOPROTOOPT:
        return "Protocol not available";
    case ENOSPC:
        return "No space left on device";
    case ENOSR:
        return "No STREAM resources";
    case ENOSTR:
        return "Not a STREAM";
    case ENOSYS:
        return "Function not implemented";
    case ENOTBLK:
        return "Block device required";
    case ENOTCONN:
        return "The socket is not connected";
    case ENOTDIR:
        return "Not a directory";
    case ENOTEMPTY:
        return "Directory not empty";
    case ENOTRECOVERABLE:
        return "Not recoverable"; /** @todo Correct error message */
    case ENOTSOCK:
        return "Not a socket";
    case ENOTSUP:
        return "Operation not supported";
    case ENOTTY:
        return "Inappropriate I/O control operation";
    case ENOTUNIQ:
        return "Name not unique on network";
    case ENXIO:
        return "No such device or address";
    case EOVERFLOW:
        return "Value too large to be stored in data type";
    case EOWNERDEAD:
        return "Owner read"; /** @todo COrrect error message */
    case EPERM:
        return "Operation not permitted";
    case EPFNOSUPPORT:
        return "Protocol family not supported";
    case EPIPE:
        return "Broken pipe";
    case EPROTO:
        return "Protocol error";
    case EPROTONOSUPPORT:
        return "Protocol not supported";
    case EPROTOTYPE:
        return "Protocol wrong type for socket";
    case ERANGE:
        return "Result too large";
    case EREMCHG:
        return "Remote address changed";
    case EREMOTE:
        return "Object is remote";
    case EREMOTEIO:
        return "Remote I/O error";
    case ERESTART:
        return "Interrupted system call should be restarted";
    case ERFKILL:
        return "RF Kill"; /** @todo Correct error message */
    case EROFS:
        return "Read-only file system";
    case ESHUTDOWN:
        return "Cannot send after transport endpoint shutdown";
    case ESPIPE:
        return "Invalid seek";
    case ESOCKTNOSUPPORT:
        return "Socket type not supported";
    case ESRCH:
        return "No such process";
    case ESTALE:
        return "Stale file handle";
    case ESTRPIPE:
        return "Streams pipe error";
    case ETIME:
        return "Timer expired";
    case ETIMEDOUT:
        return "Connection timed out";
    case ETOOMANYREFS:
        return "Too many references"; /** @todo Correct error message */
    case ETXTBSY:
        return "Text file busy";
    case EUCLEAN:
        return "Structure needs cleaning";
    case EUNATCH:
        return "Protocol driver not attached";
    case EUSERS:
        return "Too many users";
    case EXDEV:
        return "Improper link";
    case EXFULL:
        return "Exchange full";
    default:
        return "No error";
    }
}

char *strerror(int err)
{
    return (char *)_strerror(err);
}

#if _POSIX_C_SOURCE >= 200809L
/**
 * @brief Copies a string s2 to s1 - however returns a pointer to the end
 * of the string rather than the start of it
 * 
 * @param s1 Destination string
 * @param s2 Source string
 * @return char* Pointer to the end of the string (NUL character)
 */
char *stpcpy(char *__restrict__ s1, const char *__restrict__ s2)
{
    strcpy(s1, s2);
    return s1 + strlen(s1);   
}
#endif
