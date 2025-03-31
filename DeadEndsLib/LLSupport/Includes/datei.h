#ifndef datei_h_included
#define datei_h_included


/* GEDCOM dates */
struct tag_gdate {
	int calendar;
	struct tag_dnum year;
	struct tag_dnum month;
	struct tag_dnum day;
	int mod; /* apparently unused, but referenced by extractdatestr :( */
	int eratime; /* eg, AD, BC */
};
struct tag_gdateval {
	struct tag_gdate date1;
	struct tag_gdate date2; /* used by period/from_to & range/bet_and */
	int type;
	int subtype;
	int valid; /* -1=bad syntax, 0=freeform, 1=perfect GEDCOM date */
	String text; /* copy of original */
};
/* typedef struct tag_gdateval *GDATEVAL; */ /* in date.h */

enum { GDV_PERIOD=1, GDV_RANGE, GDV_DATE, GDV_APPROX  }; /* type */
enum { GDVP_FROM=1, GDVP_TO, GDVP_FROM_TO }; /* period subtype */
enum { GDVR_BEF=1, GDVR_AFT, GDVR_BET, GDVR_BET_AND }; /* range subtype */
enum { GDVA_ABT=1, GDVA_EST, GDVA_CAL }; /* approx subtype */
enum { GDV_GREGORIAN=1, GDV_JULIAN, GDV_HEBREW, GDV_FRENCH, GDV_ROMAN, GDV_CALENDARS_IX };
enum { GDV_AD=1, GDV_BC };
enum {
	GDV_V_INVALID=1, /* eg, two calendars, or two years, or AND without preceding BETWEEN */
	GDV_V_GOOD=2, /* without problem */
	GDV_V_FREEFORM=3, /* not perfectly parsed */
	GDV_V_PHRASE=4 /* parenthesized GEDCOM DATE PHRASE, eg, "(winter of '73)" */
};

/* GEDCOM keywords */
enum { GD_ABT=1, GD_EST, GD_CAL, GD_BEF, GD_AFT, GD_BET, GD_AND, GD_FROM, GD_TO, GD_END1 };
enum { GD_BC=GD_END1, GD_AD, GD_END2 };

/* custom picture strings for complex dates (0 means use default).
   These change when language changes.
   These should be stdalloc'd (unless 0). */
extern String cmplx_custom[ECMPLX_END];
/* generated picture strings for complex dates
   eg, "BEFORE", "Before", "before", "BEF" 
   These change when language changes.
   These are stdalloc'd. */
extern String cmplx_pics[ECMPLX_END][6];

/* custom picture string for ymd date format */
extern String date_pic;

/* generated month names (for Gregorian/Julian months) */
extern String calendar_pics[GDV_CALENDARS_IX];

typedef String MONTH_NAMES[6];

extern String roman_lower[];
extern String roman_upper[];

extern MONTH_NAMES months_gj[12];
extern MONTH_NAMES months_fr[13];
extern MONTH_NAMES months_heb[13];

struct gedcom_keywords_s {
	String keyword;
	int value;
};

/* GEDCOM keywords (fixed, not language dependent) */
extern struct gedcom_keywords_s gedkeys[];


//extern TABLE keywordtbl;
extern bool lang_changed;


void initialize_if_needed(void);


#endif /* datei_h_included */


