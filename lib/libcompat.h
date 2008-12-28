#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "../src/check.h"

/* defines size_t */
#include <sys/types.h>

#if !HAVE_MALLOC
void *rpl_malloc (size_t n)
#endif /* !HAVE_MALLOC */

#if !HAVE_REALLOC
void *rpl_realloc (void *p, size_t n)
#endif /* !HAVE_REALLOC */

#if !HAVE_SLEEP
unsigned int sleep (unsigned int seconds);
#endif /* !HAVE_SLEEP */

#if !HAVE_STRDUP
char *strdup (const char *str)
#endif /* !HAVE_STRDUP */

#if !HAVE_STRSIGNAL
const char *strsignal(int sig);
#endif /* !HAVE_STRSIGNAL */

/* silence warnings about an empty library */
void ck_do_nothing (void);
