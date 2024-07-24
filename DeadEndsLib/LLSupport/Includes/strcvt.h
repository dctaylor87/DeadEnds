extern bool isnumeric(String);
extern ZSTR ll_tocapitalizedz(String s, int utf8);
extern ZSTR ll_tolowerz(CString s, int utf8);
extern ZSTR ll_totitlecasez(String, int utf8);
extern ZSTR ll_toupperz(CString s, int utf8);
extern void set_utf8_casing(ZSTR (*ufnc)(CString), ZSTR (*lfnc)(CString));
extern String upperascii_s(String str);
