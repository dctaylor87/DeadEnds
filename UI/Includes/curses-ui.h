/* temporarily -- currently too many dependencies to move this from
   screen.c to curses-ui.c (where it really belongs as a static
   function) */

extern void
curses_outputv (void *data, MSG_LEVEL level, STRING fmt, va_list args);
