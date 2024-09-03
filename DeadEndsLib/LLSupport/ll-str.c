#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>

#include "standard.h"
#include "denls.h"
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

/* allocsubbytes -- Return substring (by byte counts)
   assumes valid inputs
   returns alloc'd memory
   start is 0-based start byte, len is # bytes
   strictly at the byte level
   client is responsible for codeset
   Created: 2001/08/02 (Perry Rapp) */

String
allocsubbytes (String s, int start, int num)
{
	String substr;
	substr = stdalloc(num+1);
	strncpy(substr, &s[start], num);
	substr[num] = 0;
	return substr;
}
