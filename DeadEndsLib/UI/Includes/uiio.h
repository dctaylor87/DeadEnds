/* user interface input output -- public interface */

/* struct uiio is opaque to user code.  The internals of the structure
   are known only to code within the UI directory. */
struct uiio;
typedef struct uiio UIIO;

extern bool uiio_pre_database_init (UIIO *uiio, bool runningInterpreter);

extern void uiio_post_database_init (UIIO *uiio);

/* main loop contains the UI's equivalent of the read-eval-print loop */
extern void uiio_main_loop (UIIO *uiio);

/* if pause is true, an error occurred and we should give the user a
   chance to read a previously displayed message.  */
extern void uiio_shutdown_ui (UIIO *uiio, bool pause);

extern int uiio_input (UIIO *uiio, char **buffer, int *length, char **err_msg);

/* XXX Consider adding a length argument to the next two so that data
   which contains embedded NULs or is not NUL terminated can be
   handled.  XXX */

/* print the NUL terminated message on the UI's equivalent of stdout. */
extern int uiio_output (UIIO *uiio, char *buffer, char **err_msg);

/* print the NUL terminated message on the UI's equivalent of stderr. */
extern int uiio_error (UIIO *uiio, char *buffer, char **err_msg);

extern bool uiio_init (UIIO *uiio);

extern void uiio_printf (UIIO *uiio, char *format, ...);

/* XXX these should be removed from here and be entry points in the structure XXX */
extern bool uiio_stdio_init (void);
extern bool uiio_curses_init (void);

extern UIIO *uiio_curses;
extern UIIO *uiio_stdio;
extern UIIO *current_uiio;
