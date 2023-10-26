/* porting.h -- the purpose of this file is to ease porting between
   the Python code in LifeLines and the Python code in DeadEnds */

#if defined(DEADENDS)

typedef const char *	CNSTRING;

#define BOOLEAN		_Bool
#define NODE		GNode *

/* nxref, nflag, ncel are not used by Python code.  The fate of
   nrefcnt is yet to be determined -- it is used by Python code, but
   is not defined by DeadEnds.  */

#define nchild(node)	((node)->child)
#define nsibling(node)	((node)->sibling)
#define nparent(node)	((node)->parent)
#define nval(node)	((node)->value)
#define ntag(node)	((node)->tag)
#define nxref(node)	((node)->key)

#define nrefcnt_inc(node)	/* XXX currently no field, but we need this XXX*/
#define nrefcnt_dec(node)	/* XXX currently no field, but we need this XXX*/

#define TRACK_NODE_REFCNT_INC(node) /* empty -- no current ref counts */
#define TRACK_NODE_REFCNT_DEC(node) /* empty -- no current ref counts */

#define RECORD		RecordIndexEl *

#define nztop(record)	((record)->root)
#define nzkey(record)	((record)->key) /* *OR* record->root->key, which? */
#define nztype(record)	((record)->key[0])

#define DOSURCAP	true
#define NOSURCAP	false
#define REGORDER	true
#define SURFIRST	false

#define DOASK1		1
#define NOASK1		0

#define release_record(record)		/* empty --= no current ref counts */

#define FALSE		false	/* <stdbool.h> */
#define TRUE		true	/* <stdbool.h> */

#define INT		int	/* XXX */

#define indi_to_fath(node)		personToFather(node)
#define indi_to_moth(node)		personToMother(node)
#define indi_to_next_sib_old(node)	personToNextSibling(node)
#define indi_to_prev_sib_old(node)	personToPreviousSibling(node)
#define indi_to_famc(node)		personToFamilyAsChild(node)

#define create_temp_node(xref, tag, value, parent)	createGNode (xref, tag, value, parent) /* XXX */
#define set_temp_node(node, temp) /* XXX currently no nodes are marked temporary XXX */
#define is_temp_node(node)	0 /* XXX currently no nodes are marked temporary XXX */

#define getlloptstr(property, default)	(getenv(property) ? getenv(property) : default) /* XXX */

#define event_to_date(node,shorten)	eventToDate(node,shorten)
#define givens(name)			getGivenNames(name)
#define getasurname(name)		getSurname(name)
#define getsxsurname(name)		getSurname(name)
#define find_tag(node, tag)		findTag(node, tag)
#define trad_soundex(name)		soundex(name)

#define manip_name(name,caps,reg,len)	manipulateName(name,caps,reg,len)

#define name_string(str)		nameString(str)
#define trim_name(name,len)		trimName(name,len)

#define key_to_irecord(key)	keyToPersonRecord(key, theDatabase)
#define key_to_frecord(key)	keyToFamilyRecord(key, theDatabase)
#define key_to_srecord(key)	keyToSourceRecord(key, theDatabase)
#define key_to_erecord(key)	keyToEventRecord(key, theDatabase)
#define key_to_orecord(key)	keyToOtherRecord(key, theDatabase)
#define qkey_to_irecord(key)	keyToPersonRecord(key, theDatabase)
#define qkey_to_frecord(key)	keyToFamilyRecord(key, theDatabase)
#define qkey_to_srecord(key)	keyToSourceRecord(key, theDatabase)
#define qkey_to_erecord(key)	keyToEventRecord(key, theDatabase)
#define qkey_to_orecord(key)	keyToOtherRecord(key, theDatabase)

#define node_to_record(node)	_llpy_node_to_record(node)

/* we drop efmt -- era format */
#define do_format_date(str,dfmt,mfmt,yfmt,sfmt,efmt,cmplx) format_date(str,dfmt,mfmt,yfmt,sfmt,cmplx)
#endif

#if !defined(DEADENDS)

#define nrefcnt_inc(node)	nrefcnt(node)++
#define nrefcnt_dec(node)	nrefcnt(node)--

#endif
