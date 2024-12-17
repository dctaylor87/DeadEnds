/* ll-gedcom.h -- GEDCOM related macros, most are adapted from
   LifeLines hdrs/gedcom.h and hdrs/gedcom_macros.h. */
#define FORFAMS(indi,fam,num,database)	\
	{\
	GNode *frec=0;	\
	GNode *__node = findTag(nchild(indi),"FAMS");\
	GNode *fam=0;\
	String __key=0;\
	num = 0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "FAMS")) break;\
		__key = nval(__node);\
		__node = nsibling(__node);\
		++num;\
		if (!__key || !(frec=keyToFamilyRecord(__key, database)) || !(fam=nztop(frec))) { \
			continue;\
		}\
		{

#define ENDFAMS \
		}\
		releaseRecord(frec); \
	}}
