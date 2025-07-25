#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "standard.h"
#include "ll-list.h"

/* llFreeListElement -- this function exists solely to have the
   corresponding calls to free tracked when allocations are being
   logged.

   If you are creating a List and are thinking of passing 'free' as
   the delete function, pass this function instead.  */

void
llFreeListElement (void *ptr)
{
  stdfree (ptr);
}
