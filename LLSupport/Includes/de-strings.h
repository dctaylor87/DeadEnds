
/* stdstrng.c */
extern int chartype (int c);
extern bool isnumch (int c);
extern bool iswhite (int c);
extern bool islinebreak (int c);
extern bool isletter (int c);
extern bool isasciiletter (int c);
extern int ll_toupper (int c);
extern int ll_tolower (int c);
extern bool eqstr_ex (CString s1, CString s2);
extern char *llstrncpy (char *dest, const char *src, size_t n, int utf8);
extern char *llstrncpyf (char *dest, size_t n, int utf8, const char * fmt, ...);
extern char *llstrncpyvf (char *dest, size_t n, int utf8, const char * fmt, va_list args);
extern int ll_atoi (CString str, int defval);
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
extern int utf8len (char ch);
extern size_t str8chlen (CString str);
extern String find_prev_char (String ptr, int * width, String limit, int utf8);
extern int next_char32 (String * ptr, int utf8);
extern void skip_BOM (String * pstr);
extern void unicode_to_utf8 (int wch, char * utf8);
extern void chopstr_utf8 (String str, size_t index, bool utf8);
extern void limit_width (String str, size_t width, bool utf8);

/* strwhite.c */
extern String trim (String str, int len);
extern void striptrail (String p);
extern void skipws (String * ptr);
extern bool allwhite (String p);
extern void chomp (String str);

/* appendstr.c */

extern void appendstr(String * pdest, int * len, int utf8, CString src);
/* The author of appendstr, writes: llstrcatn is a bad name, because
   its prototype is different from strcatn! */
#define llstrcatn(dest, src, len) appendstr(dest, len, uu8, src)

/* ll-str.c */

extern void strfree (String *str);
extern void strupdate (String *str, CString value);
extern String allocsubbytes (String s, int start, int num);
