extern void parseArguments (int argc, char *argv[], CString optString);

/* program name for messages */
extern CString ProgName;

/* if there is a config file or command line option of this name, it
   has the name of the crashlog */
extern CString crashlog_optname;

/* default crash log file name */
extern CString crashlog_default;

/* First line of usage message */
extern CString usage_summary;
