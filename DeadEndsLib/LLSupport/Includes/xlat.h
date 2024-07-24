/* custom translation tables */

#define MEDIN 0 /* MEDIN: translate editor characters to internal */
#define MINED 1	/* MINED: translate internal characters to editor */
#define MGDIN 2	/* MGDIN: translate gedcom file characters to internal */
#define MINGD 3	/* MGDIN: translate internal characters to gedcom file */
#define MDSIN 4	/* MDSIN: translate display characters to internal */
#define MINDS 5	/* MINDS: translate internal characters to display */
#define MRPIN 6	/* MRPIN: translate report to internal characters */
#define MINRP 7	/* MINRP: translate internal characters to report */
#define MSORT 8	/* MSORT: custom sort table, characters to numeric order */
#define MCHAR 9	/* MCHAR: character table (translation result unused) */
#define MLCAS 10 /* MLCAS: translate character to lower-case (UNIMPLEMENTED) */
#define MUCAS 11 /* MUCAS: translate character to upper-case (UNIMPLEMENTED) */
#define MPREF 12 /* MPREF: prefix to skip for sorting (UNIMPLEMENTED) */
#define NUM_TT_MAPS 13	/* number of maps listed above */

struct tag_xlat;

typedef struct tag_xlat *XLAT;

extern XLAT transl_get_predefined_xlat(int trnum);

/* xlat.c */
extern bool xl_do_xlat(XLAT xlat, ZSTR zstr);
extern void xl_free_adhoc_xlats(void);
extern void xl_free_xlats(void);
#if 0
extern String xlat_get_description(XLAT xlat);
#else
extern ZSTR xlat_get_description(XLAT xlat);
#endif
extern CString xl_get_dest_codeset(XLAT xlat);
extern TRANTABLE xl_get_legacy_tt(XLAT xlat);
extern XLAT xl_get_null_xlat(void);
extern int xl_get_uparam(XLAT);
extern XLAT xl_get_xlat(CString src, CString dest, bool adhoc);
extern bool xl_is_xlat_valid(XLAT xlat);
extern void xl_load_all_dyntts(CString ttpath);
extern void xl_parse_codeset(CString codeset, ZSTR zcsname, List **subcodes);
extern void xl_release_xlat(XLAT xlat);
extern void xl_set_uparam(XLAT, int uparam);
extern void xlat_shutdown(void);
