#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "standard.h"
#include "errors.h"
#include "feedback.h"
#include "database.h"
#include "sequence.h"
#include "uiio.h"
#include "uiioi.h"

bool uiio_pre_database_init (UIIO *uiio, bool runningInterpreter)
{
  if (uiio->uiio_pre_database_init)
    return (*uiio->uiio_pre_database_init)(runningInterpreter);

  return true;			/* nothing to do, so we succeed */
}

void uiio_post_database_init (UIIO *uiio)
{
  if (uiio->uiio_post_database_init)
    (*uiio->uiio_post_database_init)();
}

void uiio_main_loop (UIIO *uiio)
{
  if (uiio->uiio_main_loop)
    (*uiio->uiio_main_loop)();
}

void uiio_shutdown_ui (UIIO *uiio, bool pause)
{
  if (uiio->uiio_shutdown)
  (*uiio->uiio_shutdown)(pause);
}

int uiio_input (UIIO *uiio, char **buffer, int *length, char **err_msg)
{
  if (uiio->uiio_input_func)
    return (*uiio->uiio_input_func)(uiio->uiio_input_data, buffer, length, err_msg);

  /* XXX compute error message to be returned XXX */
  return (-1);
}

int uiio_output (UIIO *uiio, char *buffer, char **err_msg)
{
  if (uiio->uiio_output_func)
    return (*uiio->uiio_output_func)(uiio->uiio_output_data, buffer, err_msg);

  /* XXX compute error message to be returned XXX */
  return (-1);
}

int uiio_error (UIIO *uiio, char *buffer, char **err_msg)
{
  if (uiio->uiio_error_func)
    return (*uiio->uiio_error_func)(uiio->uiio_error_data, buffer, err_msg);

  /* XXX compute error message to be returned XXX */
  return (-1);
}

/* XXX call the init function for each know UIIO instance XXX */
bool uiio_init (UIIO *uiio)
{
  if (uiio->uiio_init)
    return (*uiio->uiio_init)();

  return true;
}

void uiio_printf (UIIO *uiio, char *format, ...)
{
}

#if 0
int uiio_ask_for_char_msg(UIIO *uiio, CString msg, CString ttl, CString prmpt, CString ptrn)
{
  if (uiio->uiio_ask_for_char_msg)
    return (*uiio->uiio_ask_for_char_msg)(msg, ttl, prmpt, ptrn);

  return 0;			/* not supported */
}

bool uiio_ask_for_filename_impl(UIIO *uiio, CString ttl, CString path, CString prmpt, String buffer, int buflen)
{
  if (uiio->uiio_ask_for_filename_impl)
    return (*uiio->uiio_ask_for_filename_impl)(ttl, path, prmpt, buffer, buflen);

  return false;			/* not supported */
}

bool uiio_ask_for_string(UIIO *uiio, CString ttl, CString prmpt, String buffer, int buflen)
{
  if (uiio->uiio_ask_for_string)
    return (*uiio->uiio_ask_for_string)(ttl, prmpt, buffer, buflen);

  return false;			/* not supported */
}

bool uiio_ask_for_string2(UIIO *uiio, CString ttl1, CString ttl2, CString prmpt, String buffer, int buflen)
{
  if (uiio->uiio_ask_for_string2)
    return (*uiio->uiio_ask_for_string2)(uiio, ttl1, ttl2, prmpt, buffer, buflen);

  return false;			/* not supported */
}

int uiio_chooseFromArray(UIIO *uiio, CString ttl, int no, String *pstrngs)
{
  if (uiio->uiio_chooseFromArray)
    return (*uiio->uiio_chooseFromArray)(ttl, no, pstrngs);

  return (-1);			/* not supported */
}

Sequence *uiio_invoke_search_menu(UIIO *uiio)
{
  if (uiio->uiio_invoke_search_menu)
    return (*uiio->uiio_invoke_search_menu)();

  retrun null;			/* not supported */
}

void uiio_llwprintf()
{
  if (uiio->uiio_llwprintf)
    (*uiio->uiio_llwprintf)();
}

void uiio_view_array(UIIO *uiio, CString ttl, int no, String *pstrngs)
{
  if (uiio->uiio_view_array)
    (*uiio->uiio_view_array)(ttl, no, pstrngs);
}

bool uiio_ask_for_program(UIIO *uiio, CString mode, CString ttl, String *pfname, CString path, CString ext, bool picklist)
{
  if (uiio->uiio_ask_for_program)
    return (*uiio->uiio_ask_for_program)(mode, ttl, pfname, path, ext, picklist);

  return false;			/* not supported */
}

int uiio_chooseOneOrListFromSequence(UIIO *uiio, CString ttl, Sequence *seq, bool multi, enum SequenceType type)
{
  if (uiio->uiio_chooseOneOrListFromSequence)
    return (*uiio->uiio_chooseOneOrListFromSequence)(uiio, ttl, seq, multi, type);

  return 0;			/* not supported */
}

#endif
