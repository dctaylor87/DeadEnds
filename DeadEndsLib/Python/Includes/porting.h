/* porting.h -- the purpose of this file is to ease porting between
   the Python code in LifeLines and the Python code in DeadEnds */

//#if defined(DEADENDS)

//typedef const char *	CNSTRING;

//#define BOOLEAN		_Bool
#define SURCAPTYPE	_Bool
//#define NODE		GNode *

#define TRACK_NODE_REFCNT_INC(node) /* empty -- no current ref counts */
#define TRACK_NODE_REFCNT_DEC(node) /* empty -- no current ref counts */

//#define RECORD		RecordIndexEl *

#define nztop(record)	((record)->root)
#define nzkey(record)	((record)->root->key)
#define nztype(record)	recordType((record)->root)

#define DOSURCAP	true
#define NOSURCAP	false
#define REGORDER	true
#define SURFIRST	false

#define DOASK1		1
#define NOASK1		0

//#define release_record(record)		/* empty --= no current ref counts */

//#define FALSE		false	/* <stdbool.h> */
//#define TRUE		true	/* <stdbool.h> */

#define INT		int	/* XXX */

//#define indi_to_fath(node)		personToFather(node, database)
//#define indi_to_moth(node)		personToMother(node, database)
//#define indi_to_next_sib_old(node)	personToNextSibling(node, database)
//#define indi_to_prev_sib_old(node)	personToPreviousSibling(node, database)
//#define indi_to_famc(node)		personToFamilyAsChild(node, database)

//#define getlloptstr(property, default)	(getenv(property) ? getenv(property) : default) /* XXX */

//#define event_to_date(node,shorten)	eventToDate(node,shorten)
//#define event_to_plac(node,shorten)	eventToPlace(node,shorten)
//#define givens(name)			getGivenNames(name)
//#define getasurname(name)		getSurname(name)
//#define getsxsurname(name)		getSurname(name)
//#define find_tag(node, tag)		findTag(node, tag)
//#define trad_soundex(name)		soundex(name)

//#define manip_name(name,caps,reg,len)	manipulateName(name,caps,reg,len)

//#define name_string(str)		nameString(str)
//#define trim_name(name,len)		trimName(name,len)

//#define key_to_irecord(key)	keyToPersonRecord(key, database)
//#define key_to_frecord(key)	keyToFamilyRecord(key, database)
//#define key_to_srecord(key)	keyToSourceRecord(key, database)
//#define key_to_erecord(key)	keyToEventRecord(key, database)
//#define key_to_orecord(key)	keyToOtherRecord(key, database)
//#define qkey_to_irecord(key)	keyToPersonRecord(key, database)
//#define qkey_to_frecord(key)	keyToFamilyRecord(key, database)
//#define qkey_to_srecord(key)	keyToSourceRecord(key, database)
//#define qkey_to_erecord(key)	keyToEventRecord(key, database)
//#define qkey_to_orecord(key)	keyToOtherRecord(key, database)

#define rmvat(key)		(key)

#define node_to_record(node)	_llpy_node_to_record(node, database)
//#define copy_nodes(node, children, siblings)	copyNodes(node, children, siblings)

/* we drop efmt -- era format */
#define do_format_date(str,dfmt,mfmt,yfmt,sfmt,efmt,cmplx) formatDate(str,dfmt,mfmt,yfmt,sfmt,cmplx)

//#endif

//#if !defined(DEADENDS)
//
//#define nrefcnt_inc(node)	nrefcnt(node)++
//#define nrefcnt_dec(node)	nrefcnt(node)--
//
//#endif
