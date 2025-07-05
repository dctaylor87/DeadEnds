#include "standard.h"
#include "uiio.h"

/* options recognized by getopt */
CString optString = "adkns:tx:o:C:I:p:Pvh?";

/* program name for messages */
CString ProgName = "llexec";
CString usage_summary = "llexec [-{adkntx:o:C:I:p:Pvh?}] [database]";

/* which set of UIIO routines to use */
//UIIO *current_uiio = uiio_stdio;

/* if there is a config file or command line option of this name, it
   has the name of the crashlog */
CString crashlog_optname = "CrashLog_DEexec";

/* default crash log file name */
CString crashlog_default = "CrashLog_DEexec.log";
