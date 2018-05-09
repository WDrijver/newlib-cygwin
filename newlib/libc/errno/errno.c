/* The errno variable is stored in the reentrancy structure.  This
   function returns its address for use by the macro errno defined in
   errno.h.  */
#define __NO_INLINE__
#include <errno.h>
#include <reent.h>

#ifndef _REENT_ONLY

__stdargs int *
__errno ()
{
  return &_REENT->_errno;
}

#endif
