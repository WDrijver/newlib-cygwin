#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include "_ansi.h"
#include <sys/cdefs.h>
#include <sys/signal.h>

_BEGIN_STD_C

typedef int	sig_atomic_t;		/* Atomic entity type (ANSI) */
#if __BSD_VISIBLE
typedef _sig_func_ptr sig_t;		/* BSD naming */
#endif
#if __GNU_VISIBLE
typedef _sig_func_ptr sighandler_t;	/* glibc naming */
#endif

#define SIG_DFL ((_sig_func_ptr)0)	/* Default action */
#define SIG_IGN ((_sig_func_ptr)1)	/* Ignore action */
#define SIG_ERR ((_sig_func_ptr)-1)	/* Error return */

struct _reent;

__stdargs _sig_func_ptr _signal_r (struct _reent *, int, _sig_func_ptr);
__stdargs int	_raise_r (struct _reent *, int);

#ifndef _REENT_ONLY
__stdargs _sig_func_ptr signal (int, _sig_func_ptr);
__stdargs int	raise (int);
__stdargs void	psignal (int, const char *);
#endif

_END_STD_C

#endif /* _SIGNAL_H_ */
