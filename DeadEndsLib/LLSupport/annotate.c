/* created by David Taylor */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "standard.h"

#include "hashtable.h"
#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "zstr.h"
#include "gstrings.h"
#include "gedcom.h"
#include "recordindex.h"	/* releaseRecord */

#include "annotate.h"
#include "locales.h"		/* needed by lloptions.h */
#include "lloptions.h"
#include "xref.h"
#include "iterate.h"

static void annotate_node (GNode *node, bool expand_refns,
			   bool annotate_pointers, bool rfmt,
			   Database *database);

/* annotateWithSupplemental -- Expand any references that have REFNs
   This converts, eg, "@S25@" to "<1850.Census>" Used for editing.  */

/* modifies tree in place -- perhaps it should copy the tree? */
void
annotateWithSupplemental (GNode *node, bool rfmt, Database *database)
{
  bool expand_refns = (getdeoptint("ExpandRefnsDuringEdit", 0) > 0);
  bool annotate_pointers = (getdeoptint("AnnotatePointers", 0) > 0);
  GNode *child=0;
#if defined(DEADENDS)
  ITERGNODETREE(child)
    annotate_node (child, expand_refns, annotate_pointers, rfmt, database);
  ENDITERGNODETREE
#else
  struct tag_node_iter nodeit;

  /* annotate all descendant nodes */
  begin_node_it(node, &nodeit);
  while ((child = next_node_it_ptr(&nodeit)) != NULL) {
    annotate_node(child, expand_refns, annotate_pointers, rfmt, database);
  }
#endif
}

/* annotate_node -- Alter a node by
   expanding refns (eg, "@S25@" to "<1850.Census>")
   annotating pointers (eg, "@I1@" to "@I1@ {{ John/SMITH }}")
   Used during editing.  */
static void
annotate_node (GNode *node, bool expand_refns,
	       bool annotate_pointers, bool rfmt,
	       Database *database)
{
	CString key=0;
	GNode *rec=0;

	key = value_to_xref(nval(node));
	if (!key) return;
	
	rec = getRecord (key, database->recordIndex);
	if (!rec) return;
	
	if (expand_refns) {
		GNode *refn = REFN(rec);
		char buffer[60];
		/* if there is a REFN, and it fits in our buffer,
		and it doesn't have any (confusing) > in it */
		if (refn && nval(refn) && !strchr(nval(refn), '>')
			&& strlen(nval(refn))<=sizeof(buffer)-3) {
			/* then replace, eg, @S25@, with, eg, <1850.Census> */
			buffer[0]=0;
			strcpy(buffer, "<");
			strcat(buffer, nval(refn));
			strcat(buffer, ">");
			stdfree(nval(node));
			nval(node) = strsave(buffer);
		}
	}

	if (annotate_pointers) {
		String str = generic_to_list_string(rec, key, 60, ", ", rfmt, false, database);
		ZSTR zstr = zs_news(nval(node));
		zs_apps(zstr, " {{");
		zs_apps(zstr, str);
		zs_apps(zstr, " }}");
		stdfree(nval(node));
		nval(node) = strsave(zs_str(zstr));
		zs_free(&zstr);
	}

	/* release the (temporary) record created in key_possible_to_record() */
	releaseRecord(rec);
}
