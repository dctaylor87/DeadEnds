/* user interface input/output interface
   how do we interact with the user? */

/* This is for the *INTERNALS*.  For the API, please see hdrs/uiio.h instead */

/* XXX we might want to rethink how to do i/o here! XXX */

struct uiio
{
  void *uiio_input_data;
  void *uiio_output_data;
  void *uiio_error_data;

  /* tentative argument lists and return values */
  int (*uiio_input_func)(void *data, char **buffer, int *length, char **err_msg);
  int (*uiio_output_func)(void *data, const char *buffer, char **err_msg);
  int (*uiio_error_func)(void *data, const char *buffer, char **err_msg);

  /* for errors, we use uiio_error_data,
     for status and info we use uiio_output_data */
  void (*uiio_outputv_func)(void *data,
			    MSG_LEVEL level, CString fmt, va_list args);
};

extern const char *ui_prompt;
