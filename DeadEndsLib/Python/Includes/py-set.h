/* these macros might ought to be moved into gedcom_macros.h */

#define NEXT_FAMC(indi)	findTag(indi, "FAMC")

/* FORSPOUSES_RECORD -- iterator over all FAMS nodes and all spouses
   of 'record'.  spouse is an addref'd RECORD and needs to be
   release_record'ed if it is not saved. */

#define FORSPOUSES_RECORD(record,spouse_r) \
	{\
	  GNode *_fam = 0; \
	  int _num = 0; \
	  GNode *_indi = nztop (record);	\
	  FORSPOUSES(_indi,_spouse,_fam,_num) \
	    spouse_r = node_to_record (_spouse);

#define ENDSPOUSES_RECORD \
	  ENDSPOUSES \
	}

/* FORCHILDREN_RECORD -- iterate over all the children of a family.
   fam (input) is a RECORD, child (output) is a RECORD -- which has
   been addref'd.  If you are not saving a reference to child, you
   need to do a release_record on it */

#define FORCHILDREN_RECORD(fam,child,database)	\
	{\
        GNode *__node = findTag(nchild(nztop(fam)), "CHIL");	\
	RecordIndexEl *child=0;\
	String __key=0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "CHIL")) break;\
		__key = nval(__node);\
		__node = nsibling(__node);\
		if (!__key || !(child = keyToPersonRecord(__key, database))) {	\
			continue;\
		}\
		{

#define ENDCHILDREN_RECORD \
		}\
	}}


/* FORFAMS_RECORD -- iterate over all FAMS nodes of indi this is an
   optimization of FORFAMSS for cases where spouse is not used or is
   computed in other ways.  */

#define FORFAMS_RECORD(indi,fam) \
	{\
	RecordIndexEl *fam=0; \
	GNode *__node = findTag(nchild(nztop(indi)),"FAMS");	\
	String __key=0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "FAMS")) break;\
		__key = nval(__node);\
		__node = nsibling(__node);\
		if (!__key || !(fam=keyToFamilyRecord(__key, database))) { \
			continue;\
		}\
		{

#define ENDFAMS_RECORD \
		}\
		releaseRecord(fam); \
	}}


/* FORFAMSPOUSES_RECORD -- iterate over all spouses in one family (All
   husbands and wives).

   NOTE: spouse record has been addref'd.  If you don't store it, you
   need to call releaseRecord on it.*/

#define FORFAMSPOUSES_RECORD(fam,spouse, database)	\
	{\
	GNode *__node = nchild(nztop(fam));	\
	RecordIndexEl *spouse=0;\
	String __key=0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "HUSB") && !eqstr(ntag(__node), "WIFE")) {\
			__node = nsibling(__node);\
			continue;\
		}\
		__key = nval(__node);\
		if (!__key || !(spouse = keyToPersonRecord(__key, database))) {	\
			__node = nsibling(__node);\
			continue;\
		}\
		{

#define ENDFAMSPOUSES_RECORD \
		}\
		__node = nsibling(__node);\
	}}

/* FORFAMSPOUSES -- iterate over all spouses in one family (All
   husbands and wives).  */

#define FORFAMSPOUSES(fam,spouse,database)	\
	{\
	GNode *__node = nchild(fam);	\
	GNode *spouse=0;\
	CString __key=0;\
	while (__node) {\
		if (!eqstr(ntag(__node), "HUSB") && !eqstr(ntag(__node), "WIFE")) {\
			__node = nsibling(__node);\
			continue;\
		}\
		__key = nval(__node);\
		if (!__key || !(spouse = keyToPerson(__key, database->recordIndex))) {	\
			__node = nsibling(__node);\
			continue;\
		}\
		{

#define ENDFAMSPOUSES \
		}\
		__node = nsibling(__node);\
	}}
