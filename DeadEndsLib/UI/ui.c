#include <stdbool.h>

#include "standard.h"
#include "errors.h"
#include "feedback.h"
#include "database.h"
#include "sequence.h"
#include "ask.h"
#include "uiioi.h"
#include "uiio.h"
#include "ui.h"

int ask_for_char_msg (CString msg, CString ttl,
		      CString prmpt, CString ptrn)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_ask_for_char_msg)
    return (uiio->uiio_ask_for_char_msg)(msg, ttl, prmpt, ptrn);

  return 0;			/* cancelled, unsupported */
}

bool ask_for_filename_impl (CString ttl, CString path,
			    CString prmpt,
			    String buffer, int buflen)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_ask_for_filename_impl)
    return (uiio->uiio_ask_for_filename_impl)(ttl, path, prmpt, buffer, buflen);

  return false;			/* cancelled, unsupported */
}

int chooseFromArray (CString ttl, int no, String *pstrngs)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_chooseFromArray)
    return (uiio->uiio_chooseFromArray)(ttl, no, pstrngs);

  return 0;			/* cancelled, unsupported */
}

Sequence *invoke_search_menu (void)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_invoke_search_menu)
    return (uiio->uiio_invoke_search_menu)();

  return 0;			/* cancelled, unsupported */
}

int prompt_stdout (CString prompt)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_prompt_stdout)
    return (uiio->uiio_prompt_stdout)(prompt);

  return 0;			/* cancelled, unsupported */
}

void view_array (CString ttl, int no, String *pstrngs)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_view_array)
    (uiio->uiio_view_array)(ttl, no, pstrngs);

  return;
}

void llvwprintf (CString fmt, va_list args)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_llvwprintf)
    (uiio->uiio_llvwprintf)(fmt, args);

  return;
}

bool ask_for_string (CString ttl, CString prmpt,
		     String buffer, int buflen)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_ask_for_string)
    return (uiio->uiio_ask_for_string)(ttl, prmpt, buffer, buflen);

  return false;			/* cancelled, unsupported */
}

bool ask_for_string2 (CString ttl1, CString ttl2, CString prmpt,
		      String buffer, int buflen)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_ask_for_string2)
    return (uiio->uiio_ask_for_string2)(ttl1, ttl2, prmpt, buffer, buflen);

  return false;			/* cancelled, unsupported */
}

bool ask_for_program (CString mode, CString ttl, String *pfname,
		      CString path, CString ext, bool picklist)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_ask_for_program)
    return (uiio->uiio_ask_for_program)(mode, ttl, pfname, path, ext, picklist);

  return false;			/* cancelled, unsupported */
}

int chooseOneOrListFromSequence (CString ttl, Sequence *seq, bool multi,
				 enum SequenceType type)
{
  UIIO *uiio = current_uiio;
  if (uiio->uiio_chooseOneOrListFromSequence)
    return (uiio->uiio_chooseOneOrListFromSequence)(ttl, seq, multi, type);

  return 0;			/* cancelled, unsupported */
}
