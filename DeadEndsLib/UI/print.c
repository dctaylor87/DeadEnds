/* uiio_printf -- provide UIIO version of *printf

   everthing gets redirected to an underlying function */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"		/* ASSERT */
#include "errors.h"

#include "messages.h"
#include "feedback.h"

#include "uiio.h"
#include "uiioi.h"

UIIO *current_uiio;

/* message -- handle generic message
   delegates to msg_outputv
   TODO: replace with msg_error/info/status */

void
message (CString fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  msg_outputv(MSG_ERROR, fmt, args);
  va_end(args);
}

/* msg_error -- handle error message
   delegates to msg_outputv */

void
msg_error (CString fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  msg_outputv(MSG_ERROR, fmt, args);
  va_end(args);
}

/* msg_info -- handle regular messages
   delegates to msg_outputv */

void
msg_info (CString fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  msg_outputv(MSG_INFO, fmt, args);
  va_end(args);
}

/* msg_status -- handle transitory/status messages
   delegates to msg_outputv */
void
msg_status (CString fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  msg_outputv(MSG_STATUS, fmt, args);
  va_end(args);
}

/* print an error log */

void
msg_errorlog (ErrorLog *errorLog)
{
  if (! errorLog)
    {
      msg_output (MSG_ERROR, "Error log does not exist.\n");
      return;
    }
  else if (lengthList (errorLog) == 0)
    {
      msg_output (MSG_ERROR, "Error log is empty.\n");
      return;
    }
  FORLIST (errorLog, element)
    Error *error = (Error *) element;
    msg_output (MSG_ERROR, "error in '%s' line %d: %s.\n",
		(error->fileName ? error->fileName : ""),
		error->lineNumber,
		(error->message ? error->message : "unknown"));
  ENDLIST
}

/* msg_output -- handle any message
   delegates to msg_outputv */

void
msg_output (MSG_LEVEL level, CString fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	msg_outputv(level, fmt, args);
	va_end(args);
}

void
msg_outputv (MSG_LEVEL level, CString fmt, va_list args)
{
  void (*func)(void *data, MSG_LEVEL level, CString fmt, va_list args);
  void *data;

  func = current_uiio->uiio_outputv_func;
  if (level == MSG_ERROR)
    data = current_uiio->uiio_error_data;
  else
    data = current_uiio->uiio_output_data;

  (*func)(data, level, fmt, args);
}

/* llwprintf -- Called as wprintf(fmt, arg, arg, ...) */

void
llwprintf (CString fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  llvwprintf(fmt, args);
  va_end(args);
}
