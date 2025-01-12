#include "standard.h"
#include "uiio.h"

/* options recognized by getopt */
CString optString = "adkntu:x:o:zC:I:p:Pvh?";

/* program name for messages */
#if defined(WIN32)
CString ProgName = "Lines";
#else
CString ProgName = "llines";
#endif

/* which set of UIIO routines to use */
//uiio *current_uiio = uiio_curses;

/* if there is a config file or command line option of this name, it
   has the name of the crashlog */
CString crashlog_optname = "CrashLog_DeadEnds";
/* default crash log file name */
CString crashlog_default = "CrashLog_deadends.log";
