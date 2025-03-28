#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"		/* ASSERT */
#include "errors.h"

#include "feedback.h"
#include "recordindex.h"
#include "sequence.h"

#include "screen.h"
#include "messages.h"
#include "mycurses.h"
#include "codesets.h"
#include "denls.h"
//#include "llinesi.h"
#include "screen.h"
#include "zstr.h"
#include "translat.h"
#include "curses-ui.h"		/* curses_outputv */

#include "uiio.h"
#include "uiioi.h"

#include "stringtable.h"
#include "options.h"		/* alldone */

/* forward references */

static bool curses_ui_pre_db_init (bool runningInterpreter);

static void curses_ui_post_db_init (void);

static void curses_ui_main_loop (void);

static void uiio_curses_shutdown (bool pause);

static int
curses_input (void *data, char **buffer, int *length, char **err_msg);

static int
curses_output (void *data, const char *buffer, char **err_msg);

static int
curses_error (void *data, const char *buffer, char **err_msg);
#if 0				/* for now... */
static void
curses_outputv (void *data, char **err_msg,
		MSG_LEVEL level, CString fmt, va_list args);
static void append_to_msg_list (String msg);
static void display_status (String text);
#endif

/* local variables */

static struct uiio _uiio_curses =
  {
    "CURSES",			/* name */
    0,				/* input data */
    0,				/* output data */
    0,				/* error data */
    curses_ui_pre_db_init,	/* pre database init */
    curses_ui_post_db_init,	/* post database init */
    curses_ui_main_loop,	/* main loop */
    uiio_curses_shutdown,	/* shutdown */
    curses_input,		/* input func */
    curses_output,		/* output func */
    curses_error,		/* error func */
    curses_outputv		/* outputv func */
  };

UIIO *uiio_curses = &_uiio_curses;

static int
curses_input (void *data, char **buffer, int *length, char **err_msg)
{
  ASSERT (0);			/* not yet implemented */
}

static int
curses_output (void *data, const char *buffer, char **err_msg)
{
  ASSERT (0);			/* not yet implemented */
}

static int
curses_error (void *data, const char *buffer, char **err_msg)
{
  ASSERT (0);			/* not yet implemented */
}

#if 0	   /* for now... */
/* curses_outputv -- output message varargs style arguments
   Actually all other msg functions delegate to here.
   @level:     -1=error,0=info,1=status
   @fmt:   [IN]  printf style format string
   @args:  [IN]  vprintf style varargs
   Puts into message list and/or into status area */
 
static void
curses_outputv (ARG_UNUSED(void *data), ARG_UNUSED(char **err_msg),
		MSG_LEVEL level, CString fmt, va_list args)
{
  char buffer[250];
  String ptr;
  unsigned int width = MAINWIN_WIDTH-5;
  /* prefix errors & infos with * and space respectively */
  switch(level) {
  case MSG_ERROR:
    buffer[0] = '*';
    ptr = &buffer[1];
    break;
  case MSG_INFO:
    buffer[0] = ' ';
    ptr = &buffer[1];
    break;
  default:
    ptr = buffer;
    break;
  }
  /* now make string to show/put on msg list */
  destrncpyvf(ptr, sizeof(buffer)-1, uu8, fmt, args);
  /* first handle transitory/status messages */
  if (level==MSG_STATUS) {
    if (lock_std_msg)
      return; /* can't display it, status bar is locked */
    if (status_showing[0] && !status_transitory) {
      /* we are overwriting something important
	 so it is already on the msg list, we just need to make
	 sure the msg list gets displayed */
      if (!viewing_msgs)
	msg_flag = true;
    }
    display_status(buffer);
    return;
  }
  /* everything important goes onto msg list */
  append_to_msg_list(buffer);
  /* update flag about whether we need to show msg list to user */
  /* being careful in case we are currently *in* the msg list
     show routine */
  if (!viewing_msgs && (lengthList(msg_list)>1 || lock_std_msg)) {
    msg_flag = true;
  }
  /* now put it to status area if appropriate */
  if (!lock_std_msg) {
    if (strlen(buffer)>width) {
      buffer[width-4]=0;
      strcat(buffer, "...");
      /*
	TODO: This doesn't make sense until the msg list handles long strings
	if (!viewing_msgs)
	msg_flag = true;
      */
    }
    display_status(buffer);
  }
}

/* append_to_msg_list -- put msg on the msg list

   This is a list that we show the user
   when the current command completes,
   unless it only had one item, and it got
   put on the status bar, and it wasn't too wide. */
 
static void
append_to_msg_list (String msg)
{
  if (!msg_list)
    msg_list = createList (NULL, NULL, free, false);
  enqueueList(msg_list, strsave(msg));
}
/* display_status -- put string in status line
   We don't touch the status_transitory flag
   That is caller's responsibility. */
 
static void
display_status (String text)
{
  UIWINDOW uiwin = main_win;
  WINDOW *win = uiw_win(uiwin);
  int row;
  /* first store it */
  destrncpy(status_showing, text, sizeof(status_showing), uu8);
  if ((int)strlen(text)>ll_cols-6) {
    status_showing[ll_cols-8] = 0;
    strcat(status_showing, "...");
  }
  /* then display it */
  row = ll_lines-2;
  clear_hseg(win, row, 2, ll_cols-2);
  wmove(win, row, 2);
  mvccwaddstr(win, row, 2, status_showing);
  place_cursor_main();
  wrefresh(win);
}
#endif

static bool curses_ui_pre_db_init (bool runningInterpreter)
{
  if (! runningInterpreter)
    {
      /* start (n)curses and create windows */
      char errmsg[512];
      if (! init_screen(errmsg, sizeof(errmsg)))
	{
	  /* init_screen failed -- give the bad news */
	  endwin();
	  fprintf(stderr, "%s", errmsg);
	  return false;
	}
      set_screen_graphical(graphical);
    }
  return true;
}

static void curses_ui_post_db_init (void)
{
  if (!int_codeset[0]) {
    msg_info("%s", _("Warning: database codeset unspecified"));
  } else if (!transl_are_all_conversions_ok()) {
    msg_info("%s", _("Warning: not all conversions available"));
  }

  init_show_module();
  init_browse_module();
}

static void
curses_ui_main_loop (void)
{
  while (! alldone)
    main_menu ();
}

void
uiio_curses_init (void)
{
  /* for now, there is nothing to do -- curses initialization is
     handled elsewhere */
}

static void
uiio_curses_shutdown (bool pause)
{
  term_screen();
  if (pause) /* if error, give user a second to read it */
    sleep(1);
  /* Terminate Curses UI */
  if (!isendwin())
    endwin();
}
