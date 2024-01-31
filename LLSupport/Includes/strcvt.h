extern BOOLEAN isnumeric(STRING);
extern ZSTR ll_tocapitalizedz(STRING s, INT utf8);
extern ZSTR ll_tolowerz(CNSTRING s, INT utf8);
extern ZSTR ll_totitlecasez(STRING, INT utf8);
extern ZSTR ll_toupperz(CNSTRING s, INT utf8);
extern void set_utf8_casing(ZSTR (*ufnc)(CNSTRING), ZSTR (*lfnc)(CNSTRING));
extern STRING upperascii_s(STRING str);
