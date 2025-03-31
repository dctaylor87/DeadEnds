/* expandvars.c -- expand variables -- expands ~, ~user, $VAR ${VAR}
   Both ~ and ~user are only recognized at the start of the string.
   By contrast, $ is recognized anywhere.  */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#if defined(HAVE_PWD_H)
#include <pwd.h>
#endif

#include "standard.h"
#include "denls.h"
#include "sys_inc.h"

#include "list.h"
#include "errors.h"
#include "messages.h"
#include "de-strings.h"
#include "stringtable.h"
#include "options.h"

#include "expandvars.h"

/* errors:

   . system error (generic system error)
   . errno (system call failed, error in errno)
   . no memory (malloc failed)
   . user not found (~ and ~user)
   . variable not found (for $VAR and ${VAR}
   . syntax (for ${ without }, for $ at end, $ followed by non [a-zA-Z0-9_]
   . others?
*/

/* expandVariables --
   input -- the string to be expanded
   output -- on success: the expanded string
             on failure: NULL
   details:  on success: not modified
             on failure: details on the failure,
	     interpretation depends on return value.  */

enum ExpandErrorCode
expandVariables (CString input, String *output, CString *details)
{
  int ndx = 0;
  String result = 0;

  /* tilde is special, but ONLY as the first character. */
  if (input[0] == '~')
    {
      if ((input[1] == '/') || (input[1] == '\0'))
	{
	  /* if present, use $HOME, otherwise use '/' */
	  result = getenv ("HOME");
	  if (! result)
	    result = "/";
	  result = strsave (result);
	}
      else
	{
	  /* user name is present */
	  for (ndx = 1; input[ndx]; ndx++)
	    {
	      if ((input[ndx] == ':') || (input[ndx] == '/'))
		break;
	    }
	  /* we have the end of the user name */
	  char user[ndx + 1];

	  memcpy (user, &input[1], ndx);
	  user[ndx] = 0;

	  /* now, get the user's home directory */
	  CString homedir = 0;
	  struct passwd *pw;
	  while (pw = getpwent())
	    {
	      if (eqstr (pw->pw_name, user))
		homedir = pw->pw_dir;
	    }
	  if (! homedir)
	    {
	      /* XXX error -- user not found! XXX */
	    }
	  result = strsave (homedir);
	}
      /* XXX do we need the strsave's above?  For final result?  Yes,
	 but this is intermediate... XXX */
    }

  for (; input[ndx]; ndx++)
    {
      CString var_start = 0;
      CString var_end;
      int have_brace = 0;

      if (input[ndx] == '$')
	{
	  have_brace = 0;

	  /* start of a variable use */
	  if (input[ndx + 1] == '{')
	    {
	      have_brace = 1;
	      ndx++;
	    }
	  var_start = &input[ndx + 1];
	}
      if (var_start && *var_start)
	{
	  if (*var_start == '}')
	    {
	      /* XXX error -- we have ${} with no content XXX */
	    }
	  for (var_end = var_start; *var_end; var_end++)
	    {
	      if ((*var_end == '}') || (*var_end == '\0'))
		break;
	      else if (isalnum(*var_end))
		var_end++;
	      else
		break;
	    }
	}
    }
}
