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

extern XLAT transl_get_predefined_xlat(INT trnum);
