
/* stdstrng.c */
extern INT chartype (INT c);
extern BOOLEAN isnumch (INT c);
extern BOOLEAN iswhite (INT c);
extern BOOLEAN islinebreak (INT c);
extern BOOLEAN isletter (INT c);
extern BOOLEAN isasciiletter (INT c);
extern INT ll_toupper (INT c);
extern INT ll_tolower (INT c);
extern BOOLEAN eqstr_ex (CNSTRING s1, CNSTRING s2);
extern char *llstrncpy (char *dest, const char *src, size_t n, int utf8);
extern char *llstrncpyf (char *dest, size_t n, int utf8, const char * fmt, ...);
extern char *llstrncpyvf (char *dest, size_t n, int utf8, const char * fmt, va_list args);
extern INT ll_atoi (CNSTRING str, INT defval);
extern void stdstring_hardfail (void);
extern int make8char (int c);

/* strapp.c */
extern char *llstrapps (char *dest, size_t limit, int utf8, const char *src);
extern char *llstrappc (char *dest, size_t limit, char ch);
extern char *llstrappf (char * dest, int limit, int utf8, const char * fmt, ...);
extern char *llstrappvf (char * dest, int limit, int utf8, const char * fmt, va_list args);

/* strset.c */
extern char *llstrsets (char *dest, size_t limit, int utf8, const char *src);
extern char *llstrsetc (char *dest, size_t limit, char ch);
extern char *llstrsetf (char * dest, int limit, int utf8, const char * fmt, ...);
extern char *llstrsetvf (char * dest, int limit, int utf8, const char * fmt, va_list args);

/* strutf8.c */
extern INT utf8len (char ch);
extern size_t str8chlen (CNSTRING str);
extern STRING find_prev_char (STRING ptr, INT * width, STRING limit, int utf8);
extern INT next_char32 (STRING * ptr, int utf8);
extern void skip_BOM (STRING * pstr);
extern void unicode_to_utf8 (INT wch, char * utf8);
extern void chopstr_utf8 (STRING str, size_t index, BOOLEAN utf8);
extern void limit_width (STRING str, size_t width, BOOLEAN utf8);

/* strwhite.c */
extern STRING trim (STRING str, INT len);
extern void striptrail (STRING p);
extern void skipws (STRING * ptr);
extern BOOLEAN allwhite (STRING p);
extern void chomp (STRING str);

/* appendstr.c */

extern void appendstr(STRING * pdest, INT * len, int utf8, CNSTRING src);
/* The author of appendstr, writes: llstrcatn is a bad name, because
   its prototype is different from strcatn! */
#define llstrcatn(dest, src, len) appendstr(dest, len, uu8, src)

/* ll-str.c */

extern void strfree (String *str);
extern void strupdate (String *str, CString value);
