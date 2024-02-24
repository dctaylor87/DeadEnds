/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*=============================================================
 * nodeio.c -- I/O between nodes & files or strings
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/

#if defined(DEADENDS)

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"

#include "list.h"
#include "zstr.h"
#include "refnindex.h"
#include "gnode.h"
#include "translat.h"
#include "feedback.h"
#include "date.h"
#include "xlat.h"
#include "messages.h"
#include "readwrite.h"
#include "editing.h"
#include "splitjoin.h"
#include "codesets.h"
#include "refns.h"
#include "ll-node.h"

#else

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "feedback.h"
#include "metadata.h"
#include "date.h"
#include "xlat.h"
#include "cache.h"
#include "messages.h"

#endif

/*********************************************
 * local function prototypes, alphabetical
 *********************************************/

/* alphabetical */
static void prefix_file(FILE *fp, XLAT tt);
static STRING swrite_node(INT levl, NODE node, STRING p);
static STRING swrite_nodes(INT levl, NODE node, STRING p);
static BOOLEAN should_write_bom(void);
static void write_fam_to_file(NODE fam, CNSTRING file);
static void write_indi_to_file(NODE indi, CNSTRING file);
static void write_node(INT levl, FILE *fp, XLAT ttm,
	NODE node, BOOLEAN indent);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

#if 0		   /* XXX depends heavily on cache, needs a rewrite XXX */
/*=================================================
 * file_to_record -- Convert GEDCOM file to in-memory record
 *
 * fname:[IN]  name of file that holds GEDCOM record
 * ttm:  [IN]  character translation table
 * pmsg: [OUT] possible error message
 * pemp: [OUT] set true if file is empty
 * returns addref'd record
 *===============================================*/
RECORD
file_to_record (STRING fname, XLAT ttm, STRING *pmsg, BOOLEAN *pemp)
{
	NODE node = file_to_node(fname, ttm, pmsg, pemp);
	if (!node)
		return 0;
	if (nxref(node)) {
		CACHEEL cel = node_to_cacheel_old(node);
		RECORD rec = get_record_for_cel(cel);
		return rec;
	} else {
		RECORD rec = create_record_for_unkeyed_node(node);
		return rec;
	}
}
#endif

#if 0
/*============================================
 * node_to_file -- Convert tree to GEDCOM file
 *==========================================*/
BOOLEAN
node_to_file (INT levl,       /* top level */
              NODE node,      /* root node */
              STRING fname,   /* file */
              BOOLEAN indent, /* indent? */
              TRANTABLE tt)   /* char map */
{
	FILE *fp;
	if (!(fp = fopen(fname, LLWRITETEXT))) {
		llwprintf("Could not open file: `%s'\n", fname);
		return FALSE;
	}
	write_nodes(levl, fp, tt, node, indent, TRUE, TRUE);
	fclose(fp);
	return TRUE;
}
#endif

/*========================================
 * write_node -- Write NODE to GEDCOM file
 *
 * INT levl:       [in] level
 * FILE *fp:       [in] file
 * TRANTABLE tt    [in] char map
 * NODE node:      [in] node
 * BOOLEAN indent: [in]indent?
 *======================================*/
static void
write_node (INT levl, FILE *fp, XLAT ttm, NODE node,
	BOOLEAN indent)
{
	char out[MAXLINELEN+1];
	STRING p;
	if (indent) {
		INT i;
		for (i = 1;  i < levl;  i++)
			fprintf(fp, "  ");
	}
	fprintf(fp, FMT_INT, levl);
	if (nxref(node)) fprintf(fp, " %s", nxref(node));
	fprintf(fp, " %s", ntag(node));
	if ((p = nval(node))) {
		if (ttm) {
			translate_string(ttm, nval(node), out, MAXLINELEN+1);
			p = out;
		}
		fprintf(fp, " %s", p);
	}
	fprintf(fp, "\n");
}
/*==========================================
 * write_nodes -- Write NODEs to GEDCOM file
 *========================================*/
void
write_nodes (INT levl,       /* level */
             FILE *fp,       /* file */
             XLAT ttm,   /* char map */
             NODE node,      /* root */
             BOOLEAN indent, /* indent? */
             BOOLEAN kids,   /* output kids? */
             BOOLEAN sibs)   /* output sibs? */
{
	if (!node) return;
	write_node(levl, fp, ttm, node, indent);
	if (kids)
		write_nodes(levl+1, fp, ttm, nchild(node), indent, TRUE, TRUE);
	if (sibs)
		write_nodes(levl, fp, ttm, nsibling(node), indent, kids, TRUE);
}
/*====================================
 * swrite_node -- Write NODE to string
 *==================================*/
static STRING
swrite_node (INT levl,       /* level */
             NODE node,      /* node */
             STRING p)       /* write string */
{
	char scratch[600];
	STRING q = scratch;
	snprintf(q, sizeof(scratch), FMT_INT " ", levl);
	q += strlen(q);
	if (nxref(node)) {
		strcpy(q, nxref(node));
		q += strlen(q);
		*q++ = ' ';
	}
	strcpy(q, ntag(node));
	q += strlen(q);
	if (nval(node)) {
		*q++ = ' ';
		strcpy(q, nval(node));
		q += strlen(q);
	}
	*q++ = '\n';
	*q = 0;
	strcpy(p, scratch);
	return p + strlen(p);
}
/*=====================================
 * swrite_nodes -- Write tree to string
 *===================================*/
static STRING
swrite_nodes (INT levl,       /* level */
              NODE node,      /* root */
              STRING p)       /* write string */
{
	while (node) {
		p = swrite_node(levl, node, p);
		if (nchild(node))
			p = swrite_nodes(levl + 1, nchild(node), p);
		node = nsibling(node);
	}
	return p;
}
/*=========================================
 * node_to_string -- Convert tree to string
 *=======================================*/
STRING
node_to_string (NODE node)      /* root */
{
	INT len = tree_strlen(0, node) + 1;
	STRING str;
	if (len <= 0) return NULL;
	str = (STRING) stdalloc(len);
	(void) swrite_nodes(0, node, str);
	return str;
}
/*=====================================
 * prefix_file_for_edit - write BOM prefix to file if appropriate
 *  This handles prepending BOM (byte order mark) to UTF-8 files
 *===================================*/
void
prefix_file_for_edit (FILE *fp)
{
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	prefix_file(fp, ttmo);
}

#if !defined(DEADENDS)
/*=====================================
 * prefix_file_for_gedcom - write BOM prefix to file if appropriate
 *  This handles prepending BOM (byte order mark) to UTF-8 files
 *===================================*/
void
prefix_file_for_gedcom (FILE *fp)
{
	XLAT ttmo = transl_get_predefined_xlat(MINGD);
	prefix_file(fp, ttmo);
}
/*=====================================
 * prefix_file_for_report - write BOM prefix to file if appropriate
 *  This handles prepending BOM (byte order mark) to UTF-8 files
 *===================================*/
void
prefix_file_for_report (FILE *fp)
{
	XLAT ttmo = transl_get_predefined_xlat(MINRP);
	prefix_file(fp, ttmo);
}
#endif

/*=====================================
 * prefix_file - write BOM prefix to file if appropriate
 *  This handles prepending BOM (byte order mark) to UTF-8 files
 *===================================*/
static void
prefix_file (FILE *fp, XLAT tt)
{
	if (!should_write_bom())
		return;
	if (is_codeset_utf8(xl_get_dest_codeset(tt)))
	{
		/* 3-byte UTF-8 byte order mark */
		char bom[] = "\xEF\xBB\xBF";
		fwrite(bom, strlen(bom), 1, fp);
	}
}
/*=====================================
 * write_indi_to_file - write node tree into GEDCOM
 * (no user interaction)
 *===================================*/
void
write_indi_to_file (NODE indi, CNSTRING file)
{
	FILE *fp;
	XLAT ttmo = transl_get_predefined_xlat(MINED);
#if defined(DEADENDS)
	GNode *name, *refn, *sex, *body, *famc, *fams;
#else
	NODE name, refn, sex, body, famc, fams;
#endif

	ASSERT(fp = fopen(file, LLWRITETEXT));
	prefix_file(fp, ttmo);

	split_indi_old(indi, &name, &refn, &sex, &body, &famc, &fams);
	write_nodes(0, fp, ttmo, indi, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, name, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, refn, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, sex,   TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, body , TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, famc,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, fams,  TRUE, TRUE, TRUE);
	fclose(fp);
	join_indi(indi, name, refn, sex, body, famc, fams);
}
/*=====================================
 * write_bom - whether to write Unicode BOM
 * (byte order marker)
 *===================================*/
static BOOLEAN
should_write_bom (void)
{
#ifdef WIN32
	return TRUE;
#else
	return FALSE;
#endif
}
/*=====================================
 * write_indi_to_file_for_edit - write node tree into GEDCOM
 *  file to be edited
 * (no user interaction)
 *===================================*/
void
#if defined(DEADENDS)
write_indi_to_file_for_edit (NODE indi, CNSTRING file, bool rfmt, Database *database)
#else
write_indi_to_file_for_edit (NODE indi, CNSTRING file, RFMT rfmt)
#endif
{
#if defined(DEADENDS)
	annotateWithSupplemental(indi, rfmt, database);
#else
	annotate_with_supplemental(indi, rfmt);
#endif
	write_indi_to_file(indi, file);
	resolve_refn_links(indi);
}
/*=====================================
 * write_indi_to_file_for_edit - write node tree into GEDCOM
 *  file to be edited
 * (no user interaction)
 *===================================*/
void
#if defined(DEADENDS)
write_fam_to_file_for_edit (NODE fam, CNSTRING file, bool rfmt, Database *database)
#else
write_fam_to_file_for_edit (NODE fam, CNSTRING file, RFMT rfmt)
#endif
{
	annotate_with_supplemental(fam, rfmt);
	write_fam_to_file(fam, file);
	resolve_refn_links(fam);
}
/*=====================================
 * write_fam_to_file -- write node tree into GEDCOM
 * (no user interaction)
 *===================================*/
static void
write_fam_to_file (NODE fam, CNSTRING file)
{
	FILE *fp;
	XLAT ttmo = transl_get_predefined_xlat(MINED);
#if defined(DEADENDS)
	GNode *refn, *husb, *wife, *chil, *body;
#else
	NODE refn, husb, wife, chil, body;
#endif

	ASSERT(fp = fopen(file, LLWRITETEXT));
	prefix_file(fp, ttmo);

	split_fam(fam, &refn, &husb, &wife, &chil, &body);
	write_nodes(0, fp, ttmo, fam,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, refn, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, husb,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, wife,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, body,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, chil,  TRUE, TRUE, TRUE);
	join_fam(fam, refn, husb, wife, chil, body);
	fclose(fp);
}
