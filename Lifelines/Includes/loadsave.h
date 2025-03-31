/* warning: implicit declaration of function ‘approx_time’ [-Wimplicit-function-declaration] */
extern void write_header (FILE *fp);
extern void write_body (FILE *fp, Database *database);
extern void write_trailer (FILE *fp);

extern ZSTR approx_time (int seconds);
