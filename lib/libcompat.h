#ifndef LIBCOMPAT_H
#define LIBCOMPAT_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define GCC_VERSION_AT_LEAST(major, minor) \
((__GNUC__ > (major)) || \
 (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
#else
#define GCC_VERSION_AT_LEAST(major, minor) 0
#endif

#if GCC_VERSION_AT_LEAST(2,95)
#define CK_ATTRIBUTE_UNUSED __attribute__ ((unused))
#else
#define CK_ATTRIBUTE_UNUSED              
#endif /* GCC 2.95 */

#if GCC_VERSION_AT_LEAST(2,5)
#define CK_ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
#define CK_ATTRIBUTE_NORETURN
#endif /* GCC 2.5 */

#if _MSC_VER
#include <WinSock2.h> /* struct timeval, API used in gettimeofday implementation */
#include <io.h> /* read, write */
#include <process.h> /* getpid */
#endif /* _MSC_VER */

/* defines size_t */
#include <sys/types.h>

/* provides assert */
#include <assert.h>

/* defines FILE */
#include <stdio.h>

/* defines exit() */
#include <stdlib.h>

/* provides localtime and struct tm */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* !HAVE_SYS_TIME_H */
#include <time.h>

/* declares fork(), _POSIX_VERSION.  according to Autoconf.info,
   unistd.h defines _POSIX_VERSION if the system is POSIX-compliant,
   so we will use this as a test for all things uniquely provided by
   POSIX like sigaction() and fork() */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

/* declares pthread_create and friends */
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

/* replacement functions for broken originals */
#if !HAVE_DECL_ALARM
unsigned int alarm (unsigned int seconds);
#endif /* !HAVE_DECL_ALARM */

#if !HAVE_MALLOC
void *rpl_malloc (size_t n);
#endif /* !HAVE_MALLOC */

#if !HAVE_REALLOC
void *rpl_realloc (void *p, size_t n);
#endif /* !HAVE_REALLOC */

/* functions that may be undeclared */
#if !HAVE_DECL_FILENO && !HAVE__FILENO
int fileno (FILE *stream);
#elif !HAVE_FILENO && HAVE__FILENO
#define fileno _fileno;
#endif /* !HAVE_DECL_FILENO && !HAVE__FILENO */

#if !HAVE_GETPID && HAVE__GETPID
#define getpid _getpid;
#endif /* !HAVE_GETPID && HAVE__GETPID */

#if !HAVE_GETTIMEOFDAY
int gettimeofday (struct timeval *tv, void* tz);
#endif /* !HAVE_LOCALTIME_R */

#if !HAVE_DECL_LOCALTIME_R
#if !defined(localtime_r)
struct tm *localtime_r (const time_t *clock, struct tm *result);
#endif
#endif /* !HAVE_DECL_LOCALTIME_R */

#if !HAVE_DECL_PIPE && !HAVE__PIPE
 int pipe (int *fildes);
#elif !HAVE_DECL_PIPE && HAVE__PIPE
#define pipe _pipe;
#endif /* !HAVE_DECL_PIPE && HAVE__PIPE */

#if !HAVE_DECL_PUTENV && !HAVE__PUTENV
 int putenv (const char *string);
#elif !HAVE_DECL_PUTENV && HAVE__PUTENV
#define putenv _putenv;
#endif /* HAVE_DECL_PUTENV && !HAVE__PUTENV */

#if !HAVE_READ && HAVE__READ
#define read _read
#endif /* !HAVE_READ && HAVE__READ */

#if !HAVE_DECL_SETENV
int setenv (const char *name, const char *value, int overwrite);
#endif /* !HAVE_DECL_SETENV */

/* our setenv implementation is currently broken */
#if !HAVE_DECL_SETENV
#define HAVE_WORKING_SETENV 0
#else
#define HAVE_WORKING_SETENV 1
#endif

#if !HAVE_DECL_SLEEP
unsigned int sleep (unsigned int seconds);
#endif /* !HAVE_DECL_SLEEP */

#if !HAVE_DECL_STRDUP && !HAVE__STRDUP
 char *strdup (const char *str);
#elif !HAVE_DECL_STRDUP && HAVE__STRDUP
#define strdup _strdup;
#endif /* !HAVE_DECL_STRDUP && HAVE__STRDUP */

#if !HAVE_DECL_STRSIGNAL
const char *strsignal (int sig);
#endif /* !HAVE_DECL_STRSIGNAL */

#if !HAVE_DECL_UNSETENV
int unsetenv (const char *name);
#endif /* !HAVE_DECL_UNSETENV */

#if !HAVE_WRITE && HAVE_WRITE
#define write _write
#endif /* !HAVE_WRITE && HAVE__WRITE */

/* 
 * On systems where clock_gettime() is not available, or
 * on systems where some clocks may not be supported, the
 * definition for CLOCK_MONOTONIC and CLOCK_REALTIME may not
 * be available. These should define which type of clock
 * clock_gettime() should use. We define it here if it is
 * not defined simply so the reimplementation can ignore it.
 *
 * We set the values of these clocks to some (hopefully)
 * invalid value, to avoid the case where we define a
 * clock with a valid value, and unintentionally use
 * an actual good clock by accident.
 */
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC -1
#endif
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME -1
#endif

#ifndef HAVE_LIBRT

#ifdef STRUCT_TIMESPEC_DEFINITION_MISSING
/*
 * The following structure is defined in POSIX 1003.1 for times
 * specified in seconds and nanoseconds. If it is not defined in
 * time.g, then we need to define it here
 */
struct timespec {
   time_t   tv_sec;
   long     tv_nsec;
};
#endif /* STRUCT_TIMESPEC_DEFINITION_MISSING */

#ifdef STRUCT_ITIMERSPEC_DEFINITION_MISSING
/* 
 * The following structure is defined in POSIX.1b for timer start values and intervals.
 * If it is not defined in time.h, then we need to define it here.
 */
struct itimerspec
{
    struct timespec it_interval;
    struct timespec it_value;
};
#endif /* STRUCT_ITIMERSPEC_DEFINITION_MISSING */

/* 
 * Do a simple forward declaration in case the struct is not defined.
 * In the versions of timer_create in libcompat, sigevent is never
 * used.
 */
struct sigevent;

int clock_gettime(clockid_t clk_id, struct timespec *ts);
int timer_create(int clockid, struct sigevent *sevp, timer_t *timerid);
int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec * old_value);
int timer_delete(timer_t timerid);
#endif /* HAVE_LIBRT */

/*
 * The following checks are to determine if the system's
 * snprintf (or its variants) should be replaced with
 * the C99 compliant version in libcompat.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#if HAVE_STDARG_H
#include <stdarg.h>
#if !HAVE_VSNPRINTF
int rpl_vsnprintf(char *, size_t, const char *, va_list);
#endif
#if !HAVE_SNPRINTF
int rpl_snprintf(char *, size_t, const char *, ...);
#endif
#if !HAVE_VASPRINTF
int rpl_vasprintf(char **, const char *, va_list);
#endif
#if !HAVE_ASPRINTF
int rpl_asprintf(char **, const char *, ...);
#endif
#endif /* HAVE_STDARG_H */

/* silence warnings about an empty library */
void ck_do_nothing (void) CK_ATTRIBUTE_NORETURN;

#endif /* !LIBCOMPAT_H */
