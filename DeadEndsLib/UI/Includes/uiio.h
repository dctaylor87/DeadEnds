/* user interface input output -- public interface */

struct uiio;
typedef struct uiio UIIO;

extern void uiio_main_loop (UIIO *uiio);

/* if pause is true, an error occurred and we should give the user a
   chance to read a previously displayed message.  */
extern void uiio_shutdown_ui (UIIO *uiio, bool pause);

extern int uiio_input (UIIO *uiio, char **buffer, int *length, char **err_msg);
extern int uiio_output (UIIO *uiio, char *buffer, char **err_msg);
extern int uiio_error (UIIO *uiio, char *buffer, char **err_msg);
extern void uiio_init (void);

extern void uiio_printf (UIIO *uiio, char *format, ...);

extern void uiio_stdio_init (void);
extern void uiio_curses_init (void);

extern UIIO *uiio_curses;
extern UIIO *uiio_stdio;
extern UIIO *current_uiio;
