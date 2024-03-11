/* signals.h -- decls for functions in signals.c */

extern void load_signames (void);
extern void set_signals (void (*handler)(int));
extern char *get_signame (int sig);
