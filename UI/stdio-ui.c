#include <ansidecl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(DEADENDS)

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"		/* ASSERT */
//#include "llnls.h"
#include "feedback.h"
#include "messages.h"

#else

#include "llstdlib.h"		/* __fatal */
#include "standard.h"		/* ASSERT */
#include "feedback.h"
#include "messages.h"

#endif

#include "uiio.h"
#include "uiioi.h"

/* forward references */

static int
stdio_input (void *data, char **buffer, int *length, char **err_msg);

static int
stdio_output (void *data, const char *buffer, char **err_msg);

static void
stdio_outputv (void *data, MSG_LEVEL level, STRING fmt, va_list args);

/* local variables */

static struct uiio _uiio_stdio =
  {
    0,
    0,
    0,
    stdio_input,
    stdio_output,
    stdio_output,
    stdio_outputv
  };

UIIO *uiio_stdio = &_uiio_stdio;

/* XXX this should be moved elsewhere, renamed, and passed as an argument.  XXX */
const char *ui_prompt = "cli> "; /* XXX */

/* stdio_input -- on success (return 0), the input line is placed into
   a newly malloc'ed buffer -- it is the caller's responsibility to
   free the buffer when done with it. On failure, -1 is returned and a
   static (not malloc'ed) error message is placed into *err_msg. */

/* XXX TODO (FUTURE): replace -1 with an error number so that caller
   can progamatically do something depending upon what the error is...
   For now, we don't know of anything to do besides report the problem
   to the user... XXX */

static int
stdio_input (void *data, char **buffer, int *length, char **err_msg)
{
  size_t getline_length = 0;	/* size of malloc'ed buffer */
  ssize_t status;		/* negative for error, otherwise length */

  *buffer = NULL;
  status = getline (buffer, &getline_length, (FILE *)data);
  if (status < 0)
    {
      /* NOTE: strerror translates into current locale */
      *err_msg = strerror (errno);
      return (-1);
    }
  *err_msg = NULL;

  /* if there is a terminating newline, strip it */
  if ((*buffer)[status - 1] == '\n')
    {
      (*buffer)[status - 1] = '\0';
      status--;
    }
  ASSERT (status <= (ssize_t)__INT_MAX__);
  *length = (int)status;

  /* XXX double check -- does caller expect 0 or count?  Also, how to
     flag error vs EOF?  XXX */
  return (0);
}

static int
stdio_output (void *data, const char *buffer, char **err_msg)
{
  FILE *fp = (FILE *)data;
  int status;

  status = fputs (buffer, fp);
  if (status >= 0)
    {
      status = 0;
      *err_msg = NULL;
    }
  else
    {
      status = -1;		/* XXX is this what we want to return? XXX */
      *err_msg = strerror (errno);
    }
  return (status);
}

static void
stdio_outputv (void *data, MSG_LEVEL level, STRING fmt, va_list args)
{
  FILE *file = (FILE *)data;

  /* distinguish between error, info, and status messages */
  switch (level)
    {
    case MSG_ERROR:
      fputc ('*', file);
      break;
    case MSG_INFO:
      fputc (' ', file);
      break;
    default:
      break;
    }
  vfprintf (file, fmt, args);
  /* over 99% of messages do not have a trailing newline! */
  fputc ('\n', file);
}

void
uiio_stdio_init (void)
{
  _uiio_stdio.uiio_input_data = (void *)stdin;
  _uiio_stdio.uiio_output_data = (void *)stdout;
  _uiio_stdio.uiio_error_data = (void *)stderr;
}