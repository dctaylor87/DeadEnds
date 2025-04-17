#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "standard.h"
#include "errors.h"
#include "feedback.h"
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

