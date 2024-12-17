/* user interface input/output interface
   how do we interact with the user? */

/* This is for the *INTERNALS*.  For the API, please see hdrs/uiio.h instead */

/* XXX we might want to rethink how to do i/o here! XXX */

struct uiio
{
  CString uiio_name;		/* for use in error and debug messages */

  void *uiio_input_data;
  void *uiio_output_data;
  void *uiio_error_data;

  /* tentative argument lists and return values */

  /* XXX might want to add a void* argument or a pointer to a defined
     structure.  Currently all communication is via external variables
     -- which is not a good interface.  Since there are potentially
     multiple front-ends communicating with multiple back ends, it
     should be defined...*/
  void (*uiio_main_loop)(void);

  /* if pause is true, an error occurred and we should give the user a
     chance to read a previously displayed message.  */
  void (*uiio_shutdown)(bool pause);

  int (*uiio_input_func)(void *data, char **buffer, int *length, char **err_msg);
  int (*uiio_output_func)(void *data, const char *buffer, char **err_msg);
  int (*uiio_error_func)(void *data, const char *buffer, char **err_msg);

  /* Called by the msg_* functions.  For errors, we use uiio_error_data,
     for status and info we use uiio_output_data */
  void (*uiio_outputv_func)(void *data,
			    MSG_LEVEL level, CString fmt, va_list args);
};

extern const char *ui_prompt;
