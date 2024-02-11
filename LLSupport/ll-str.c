#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"
#include "de-strings.h"

void
strfree (String *str)
{
  if (*str)
    {
      stdfree (*str);
      *str = NULL;
    }
}

void
strupdate (String *str, CString value)
{
  strfree (str);
  if (value)
    *str = strsave (value);
}
