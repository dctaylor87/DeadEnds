/* options.h -- these are the variables that are set during the
   parsing of the command line that are accessed by other files */

#if !defined(_OPTIONS_H_)
#define _OPTIONS_H_

extern int winx;		/* screen size, columns */
extern int winy;		/* screen size, rows */

extern bool debugmode;		/* no signal handling, so we can get coredump */

extern bool keyflag;		/* show key values */
extern bool traditional;	/* use traditional family rules */

extern List *exprogs;
extern StringTable *exargs;
extern String progout;
extern bool graphical;
extern String configfile;

extern String ImportLog;	/* file location of import errors */

extern bool python_interactive;
extern bool have_python_scripts;

//extern CString usageSummary;

extern int alldone;		/* completion flag */
#endif
