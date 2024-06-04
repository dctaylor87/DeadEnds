/* 
   Copyright (c) 2000-2005 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * pedigree.c -- Display the pedigree browse screen
 *  Several related drawing routines to draw information trees
 *  Ancestor tree, descendant tree, GEDCOM tree
 * NB: Part of curses GUI version
 *   Created: 2000/10 by Perry Rapp
 *==============================================================*/


#include <ansidecl.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(DEADENDS)
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"

#include "refnindex.h"
#include "gnode.h"
#include "recordindex.h"
#include "rfmt.h"
#include "sequence.h"
#include "uiprompts.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "ll-list.h"
#include "lineage.h"
#include "codesets.h"
#include "gstrings.h"
#include "de-strings.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase
#else

#include "llstdlib.h"
#include "liflines.h"
#include "mystring.h"
#include "lloptions.h"

#include "llinesi.h"
#include "gedcom.h"
#include "indiseq.h"

#endif
/*********************************************
 * global/exported variables
 *********************************************/

/*********************************************
 * external/imported variables
 *********************************************/

/*********************************************
 * local types
 *********************************************/

/* tree built for display */
struct displaynode_s
{
	struct displaynode_s * firstchild;
	struct displaynode_s * nextsib;
	CString key;	/* used by anc/desc trees */
	String str; /* used by extended gedcom node trees */
};
typedef struct displaynode_s *DISPNODE;
typedef String (*LINEPRINT_FNC)(int width, void * param);
/* parameters for anc/desc trees */
typedef struct indi_print_param_s
{
	CString key;
} *INDI_PRINT_PARAM;
/* parameters for gedcom node traversal */
typedef struct node_print_param_s
{
	GNode *node;
	int gdvw;
} *NODE_PRINT_PARAM;
/* parameters for gedcom text expanded node traversal */
typedef struct node_text_print_param_s
{
	DISPNODE tn;
} *NODE_TEXT_PRINT_PARAM;

/*********************************************
 * local enums & defines
 *********************************************/

#define GENS_MAX 7
#define GENS_MIN 2

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static DISPNODE add_children(GNode *indi, int gen, int maxgen, int * count);
static DISPNODE add_parents(GNode *indi, int gen, int maxgen, int * count);
static DISPNODE alloc_displaynode(void);
static void append_to_text_list(List *list, String text, int width, bool newline);
static void check_scroll_max(CANVASDATA canvas);
static void count_nodes(GNode *node, int gen, int maxgen, int * count);
static void draw_gedcom_text(RecordIndexEl *rec, CANVASDATA canvas, bool reuse, int indent);
static void free_displaynode(DISPNODE tn);
static void free_dispnode_tree(DISPNODE tn);
static void free_entire_tree(void);
static int get_indent(void);
static String indi_lineprint(int width, void * param);
static String node_lineprint(int width, void * param);
static void print_to_screen(int gen, int indent, int * row, LINEPRINT_FNC, void *lpf_param, CANVASDATA canvas);
static List *text_to_list (String text, int width, int whattofree);
static String tn_lineprint(int width, void * param);
static void trav_bin_in_print_tn(DISPNODE tn, int * row, int gen, int indent, CANVASDATA canvas);
static void trav_pre_print_nod(GNode *node, int * row, int gen, int indent, CANVASDATA canvas, int gdvw);
static void trav_pre_print_tn(DISPNODE tn, int * row, int gen, int indent, CANVASDATA canvas);
static void trav_pre_print_tn_str(DISPNODE tn, int * row, int gen, int indent, CANVASDATA canvas);
static void set_scroll_max(CANVASDATA canvas, int count);

/*********************************************
 * local variables
 *********************************************/

static int Gens = 4;
static int Ancestors_mode = 1;
static int ScrollMax = 0;
static DISPNODE Root = 0; /* cached tree we display */

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================================
 * alloc_displaynode -- get new displaynode
 *  In preparation for using a block allocator
 *===============================================*/
static DISPNODE
alloc_displaynode (void)
{
	DISPNODE tn = (DISPNODE)stdalloc(sizeof(*tn));
	tn->firstchild = NULL;
	tn->nextsib = NULL;
	tn->key = 0;
	tn->str = NULL;
	return tn;
}
/*========================================
 * free_displaynode -- return displaynode to free-list
 * (see alloc_displaynode comments)
 *======================================*/
static void
free_displaynode (DISPNODE tn)
{
	tn->firstchild = NULL;
	tn->nextsib = NULL;
	tn->key = 0;
	if (tn->str) {
		stdfree(tn->str);
		tn->str = NULL;
	}
	stdfree(tn);
}
/*=================================================
 * add_children -- add children to tree recursively
 *===============================================*/
static DISPNODE
add_children (GNode *indi, int gen, int maxgen, int * count)
{
	DISPNODE tn = alloc_displaynode();
	DISPNODE tn0, tn1;
	int i;

	tn->key = personToKey(indi);
	tn->firstchild = 0;
	tn->nextsib = 0;
	(*count)++;

	if (gen < maxgen) {
		Sequence *childseq = personToChildren(indi, currentDatabase);
		if (childseq) {
			tn0=0;
			for (i=0; i<length_indiseq(childseq); i++) {
				GNode *child;
				CString childkey, childname;
				element_indiseq(childseq, i, &childkey, &childname);
				child = key_to_indi(childkey);
				tn1 = add_children(child, gen+1, maxgen, count);
				/* link new displaynode into tree we're building */
				if (tn0)
					tn0 = tn0->nextsib = tn1;
				else /* first child - first time thru loop */
					tn0 = tn->firstchild = tn1;
			}
			remove_indiseq(childseq);
		}
	}
	return tn;
}
/*===========================
 * text_to_list -- Split text into list of lines
 *  each no more than width
 *=========================*/
static List *
text_to_list (String text, int width, int whattofree)
{
	List *list = create_list2(whattofree);
	append_to_text_list(list, text, width, true);
	return list;
}
/*=================================================
 * append_to_text_list - add onto fixed width string list
 *  newline: flag to not append to last element of list
 *  we build each line in current, and add it to list as done
 *  ptr points to the undone part of text
 *  curptr points to the end (insertion point) of current
 * NB: We also build list from last to first, so client can use
 *  FORLIST traversal (which is backwards)
 *  TO DO: Break at whitespace
 *=================================================*/
static void
append_to_text_list (List *list, String text, int width, bool newline)
{
	String ptr = text;
	String temp, current, curptr;
	int len, curlen;
	if (!text || !text[0])
		return;
	/* pull off last line into temp, to append to */
	if (newline) {
		temp = NULL;
	} else {
		temp = pop_list(list);
		if (temp && (int)strlen(temp) >= width) {
			enqueue_list(list, temp);
			temp = NULL;
		}
	}
	current=stdalloc((width+1)*sizeof(char));
	current[0] = 0;
	curptr = current;
	curlen = width;
	if (temp) {
		llstrcatn(&curptr, temp, &curlen);
	}
	while (1) {
		len = strlen(ptr);
		if (!len) {
			/* done */
			if (current[0]) {
				enqueue_list(list, strsave(current));
			}
			stdfree(current);
			return;
		}
		if (len > curlen)
			len = curlen;
		temp = curptr;
		llstrcatn(&curptr, ptr, &curlen);
		ptr += (curptr - temp);
		if (!curlen) {
			/* filled up an item */
			enqueue_list(list, strsave(current));
			current[0] = 0;
			curptr = current;
			curlen = width;
		}
	}
}
/*=================================================
 * add_dnodes -- add dnodes to dnode tree
 *  recursively, traversing NODE tree & building corresponding
 *  dnode tree
 * if a line overflows, give it succeeding sibling dnodes
 * also, check for subordinate CONT & CONC dnodes to be assimilated
 *===============================================*/
static DISPNODE
add_dnodes (GNode *node, int gen, int indent, int maxgen, int * count, CANVASDATA canvas)
{
	DISPNODE tn;
	DISPNODE tn0, tn1, tn2;
	GNode *child, *anode;
	int width = (canvas->rect->right - canvas->rect->left) - 2 - gen*indent;
	static char line[MAXLINELEN], output[MAXLINELEN]; /* must be same size */
	String ptr=output;
	int leader;
	List *list=NULL;
	int mylen=sizeof(output), mylenorig;
	if (mylen>width)
		mylen = width;
	mylenorig = mylen;

	/* build xref & tag into line */
	line[0] = 0;
	ptr = line;
	mylen = sizeof(line);

	if (nxref(node)) {
		llstrcatn(&ptr, nxref(node), &mylen);
		llstrcatn(&ptr, " ", &mylen);
	}
	if (ntag(node)) {
		llstrcatn(&ptr, ntag(node), &mylen);
		llstrcatn(&ptr, " ", &mylen);
	}
	leader = ptr-line;
	width -= leader;
	if (width < 10) {
		/* insufficient space */
		return NULL;
	}

	/* output is available as scratch */

	list = text_to_list("", width, LISTDOFREE);
	if (nval(node)) {
		String valtxt = nval(node);
		append_to_text_list(list, valtxt, width, false); 
	}

	/* anode is first child */
	anode = nchild(node);
	/* check for text continuation nodes to assimilate */
	if (nchild(node)) {
		for ( ; anode && !nchild(anode); anode = nsibling(anode)) {
			bool newline=false;
			String valtxt=NULL;
			if (eqstr(ntag(anode), "CONC")) {
				append_to_text_list(list, " ", width, false);
				newline = false;
			} else if (eqstr(ntag(anode), "CONT")) {
				newline = true;
			} else {
				break;
			}
			valtxt = nval(anode);
			append_to_text_list(list, valtxt, width, newline); 
		}
	}
	/* anode is now first non-assimilated child */
	/*
	now add all list elements to tree as siblings
	first one will be tn, which we return as our result
	tn0 refers to previous one, for the nsibling links
	*/
	tn = tn0 = tn1 = 0;
	FORLIST(list, el)
		tn1 = alloc_displaynode();
		if (!tn) {
			int i;
			tn = tn1;
			/* ptr & mylen still point after leader */
			llstrcatn(&ptr, el, &mylen);
			/* put original line */
			tn1->str = strsave(line);
			/* now build leader we will keep reusing */
			for (i=0; i<leader; i++)
				line[i] = '.';
			line[leader-1] = ' ';
		} else {
			llstrcatn(&ptr, el, &mylen);
			tn1->str = strsave(line);
		}
		/* now we keep resetting ptr & mylen to after blank leader */
		/* so we keep reusing that leader we built in line earlier */
		ptr=line+leader;
		mylen=mylenorig-leader;
		tn1->firstchild = 0;
		tn1->nextsib = 0;
		if (tn0)
			tn0->nextsib = tn1;
		tn0 = tn1;
		(*count)++;
	ENDLIST
	/* special handling for empty list, which didn't get its leader */
	if (isEmptyList(list)) {
		tn1 = alloc_displaynode();
		tn = tn1;
		tn1->str = strsave(line);
		tn1->firstchild = 0;
		tn1->nextsib = 0;
		tn0 = tn1;
		(*count)++;
	}
	destroy_list(list);
	list=0;

	if (gen < maxgen) {
		/* our children hang off of tn2, which is last node of our
		sibling tree; tn0 is previously added child */
		tn2 = tn1;
		tn0 = 0;
		/* anode was last unassimilated child */
		for (child = anode; child; child = nsibling(child)) {
			tn1 = add_dnodes(child, gen+1, indent, maxgen, count, canvas);
			if (!tn1)
				continue; /* child was skipped */
			/* link new displaynode into tree we're building */
			if (tn0)
				tn0 = tn0->nextsib = tn1;
			else /* first child - first time thru loop */
				tn0 = tn2->firstchild = tn1;
			/* child displaynode might have (overflow or assimilated) siblings */
			while (tn0->nextsib)
				tn0 = tn0->nextsib;
		}
	}
	return tn;
}
/*=================================================
 * count_nodes -- count descendent nodes
 *===============================================*/
static void
count_nodes (GNode *node, int gen, int maxgen, int * count)
{
	(*count)++;

	if (gen < maxgen) {
		GNode *child;
		for (child = nchild(node); child; child = nsibling(child)) {
			count_nodes(child, gen+1, maxgen, count);
		}
	}
}
/*===============================================
 * add_parents -- add parents to tree recursively
 *=============================================*/
static DISPNODE
add_parents (GNode *indi, int gen, int maxgen, int * count)
{
	DISPNODE tn = alloc_displaynode();
	tn->key = personToKey(indi);
	tn->firstchild = 0;
	tn->nextsib = 0;
	(*count)++;
	if (gen<maxgen) {
		tn->firstchild = 
			add_parents(personToFather(indi, currentDatabase), gen+1, maxgen, count);
		if (tn->key)
			indi=keyToPerson(tn->key, currentDatabase);
		tn->firstchild->nextsib = 
			add_parents(personToMother(indi, currentDatabase), gen+1, maxgen, count);
	}
	return tn;
}
/*=====================================
 * print_to_screen -- print output line
 *  using caller-supplied print function
 *  this routine handles scroll & indent
 *  gen:       [IN]  Current generation (starts at 0)
 *  row:       [IN/OUT]  Current row (advanced here)
 *  fnc:       [IN]  (lineprint) callback to produce string for line
 *  lpf_param: [IN]  opaque data for (lineprint) callback
 *  canvas:    [IN]  size & handle for output canvas
 * Created: 2000/12/07, Perry Rapp
 *===================================*/
static void
print_to_screen (int gen, int indent, int * row, LINEPRINT_FNC fnc
	, void * lpf_param, CANVASDATA canvas)
{
	char buffer[140], *ptr=buffer;
	String line;
	int mylen = sizeof(buffer);
	int width = canvas->rect->right - canvas->rect->left;
	/* NODE indi = 0; */
	int drow = *row - canvas->scroll; /* effective display row */
	int i, overflow=0;
	if (mylen > width-2)
		mylen = width-2;
	if (drow >= canvas->rect->top && drow <= canvas->rect->bottom) {
		/* in range to display */
		/* check if it is a top or bottom row & there is more beyond */
		if (drow == canvas->rect->top && canvas->scroll>0)
			overflow=1;
		if (drow == canvas->rect->bottom && canvas->scroll<ScrollMax)
			overflow=1;
		strcpy(ptr, "");
		for (i=0; i<gen*indent; i++)
			llstrcatn(&ptr, " ", &mylen);
		/* call thru fnc pointer to make string */
		line = (*fnc)(mylen, lpf_param);
		llstrcatn(&ptr, line, &mylen);
		/* tell canvas to put line out */
		(*canvas->line)(canvas, drow, canvas->rect->left, buffer, overflow);
	}
	(*row)++;
}
/*=================================
 * indi_lineprint -- print an indi line
 *  in an ancestral or descendant tree
 * returns static buffer
 * Created: 2001/01/27, Perry Rapp
 * Does internal-to-display translation
 *===============================*/
static String
indi_lineprint (int width, void * param)
{
	INDI_PRINT_PARAM ipp = (INDI_PRINT_PARAM)param;
	GNode *indi=0;
	if (ipp->key)
		indi = keyToPerson(ipp->key, currentDatabase);
	return indi_to_ped_fix(indi, width);
}
/*=================================
 * node_lineprint -- print a node line
 *  in a node tree
 * This is used in gedcom view & extended
 *  gedcom views
 * returns static buffer
 * Created: 2001/01/27, Perry Rapp
 *===============================*/
static String
node_lineprint (int width, void * param)
{
	static char line[120];
	String ptr=line;
	int mylen=sizeof(line);
	NODE_PRINT_PARAM npp = (NODE_PRINT_PARAM)param;
	GNode *node=npp->node;

	if (mylen>width)
		mylen=width;
	if (nxref(node)) {
		llstrcatn(&ptr, nxref(node), &mylen);
		llstrcatn(&ptr, " ", &mylen);
	}
	if (ntag(node)) {
		llstrcatn(&ptr, ntag(node), &mylen);
		llstrcatn(&ptr, " ", &mylen);
	}
	if (nval(node)) {
		llstrcatn(&ptr, nval(node), &mylen);
		llstrcatn(&ptr, " ", &mylen);
	}
	if (npp->gdvw == GDVW_EXPANDED && nval(node)) {
		String key = rmvat(nval(node)), str;
		if (key) {
			str = generic_to_list_string(NULL, key, mylen, ",", false, true, currentDatabase);
			llstrcatn(&ptr, " : ", &mylen);
			llstrcatn(&ptr, str, &mylen);
		}
	}
	return line;
}
/*=================================
 * tn_lineprint -- print a displaynode line
 *  this simply uses the string buffer of the displaynode
 * Created: 2001/04/15, Perry Rapp
 *===============================*/
static String
tn_lineprint (ATTRIBUTE_UNUSED int width, void * param)
{
	NODE_TEXT_PRINT_PARAM ntpp = (NODE_TEXT_PRINT_PARAM)param;

	return ntpp->tn->str;
}
/*=================================
 * trav_pre_print_tn -- traverse displaynode tree,
 *  printing indis in preorder
 * Created: 2000/12/07, Perry Rapp
 *===============================*/
static void
trav_pre_print_tn (DISPNODE tn, int * row, int gen, int indent, CANVASDATA canvas)
{
	DISPNODE n0;
	struct indi_print_param_s ipp;
	ipp.key = tn->key;
	/* all display printing passes thru generic print_to_screen,
	which handles scrolling */
	print_to_screen(gen, indent, row, &indi_lineprint, &ipp, canvas);
	for (n0=tn->firstchild; n0; n0=n0->nextsib)
		trav_pre_print_tn(n0, row, gen+1, indent, canvas);
}
/*=================================
 * trav_pre_print_tn_str -- traverse displaynode str tree,
 *  printing displaynode buffers in preorder
 * Created: 2001/04/15, Perry Rapp
 *===============================*/
static void
trav_pre_print_tn_str (DISPNODE tn, int * row, int gen, int indent, CANVASDATA canvas)
{
	DISPNODE n0;
	struct node_text_print_param_s ntpp;
	ntpp.tn = tn;
	/* all display printing passes thru generic print_to_screen,
	which handles scrolling */
	print_to_screen(gen, indent, row, &tn_lineprint, &ntpp, canvas);
	for (n0=tn->firstchild; n0; n0=n0->nextsib)
		trav_pre_print_tn_str(n0, row, gen+1, indent, canvas);
}
/*=================================
 * trav_pre_print_nod -- traverse node tree,
 *  printing nodes in preorder
 * Created: 2001/01/27, Perry Rapp
 *===============================*/
static void
trav_pre_print_nod (GNode *node, int * row, int gen, int indent,
	CANVASDATA canvas, int gdvw)
{
	GNode *child;
	struct node_print_param_s npp;
	npp.node = node;
	npp.gdvw = gdvw;
	/* all display printing passes thru generic print_to_screen,
	which handles scrolling */
	print_to_screen(gen, indent, row, &node_lineprint, &npp, canvas);
	for (child=nchild(node); child; child=nsibling(child))
		trav_pre_print_nod(child, row, gen+1, indent, canvas, gdvw);
}
/*===========================================
 * trav_bin_in_print_tn -- traverse binary tree,
 *  printing indis in inorder
 * Created: 2000/12/07, Perry Rapp
 *=========================================*/
static void
trav_bin_in_print_tn (DISPNODE tn, int * row, int gen, int indent, CANVASDATA canvas)
{
	struct indi_print_param_s ipp;
	ipp.key = tn->key;
	if (tn->firstchild)
		trav_bin_in_print_tn(tn->firstchild, row, gen+1, indent, canvas);
	/* all display printing passes thru generic print_to_screen,
	which handles scrolling */
	print_to_screen(gen, indent, row, &indi_lineprint, &ipp, canvas);
	if (tn->firstchild && tn->firstchild->nextsib)
		trav_bin_in_print_tn(tn->firstchild->nextsib, row, gen+1, indent, canvas);
}
/*======================================================
 * set_scroll_max -- compute max allowable scroll based on
 *  number of rows in this pedigree tree
 *  canvas:  [I/O] canvas data from client (we adjust scroll)
 *  count:   [IN]  # of items being displayed
 * Created: 2000/12/07, Perry Rapp
 *====================================================*/
static void
set_scroll_max (CANVASDATA canvas, int count)
{
	int hgt = canvas->rect->bottom - canvas->rect->top + 1;
	ScrollMax = count - hgt;
	if (ScrollMax<0) ScrollMax=0;
}
/*======================================================
 * check_scroll_max -- ensure current scroll is within range
 *  canvas [I/O] canvas data from client (we adjust scroll member)
 *====================================================*/
static void
check_scroll_max( CANVASDATA canvas)
{
	if (canvas->scroll > ScrollMax) canvas->scroll = ScrollMax;
	if (canvas->scroll < 0) canvas->scroll = 0;
}
/*======================================================
 * get_indent -- screen indent for treeview display
 *====================================================*/
static int
get_indent (void)
{
	int indent = getdeoptint("GedcomDisplayIndent", 6);
	if (indent<0 || indent>9) indent = 6;
	return indent;
}
/*=========================================================
 * draw_descendants -- build descendant tree & print it out
 *  Build a displaynode tree then print it out in
 *  preorder traversal
 * Created: 2000/12/07, Perry Rapp
 *=======================================================*/
void
pedigree_draw_descendants (RecordIndexEl *rec, CANVASDATA canvas, bool reuse)
{
	int gen=0;
	int row = canvas->rect->top;
	int indent = get_indent();
	/* build displaynode tree */
	if (!reuse) {
		int count=0;
		free_entire_tree();
		Root = add_children(nztop(rec), gen, Gens, &count);
		set_scroll_max(canvas, count);
	}
	check_scroll_max(canvas);
	/* preorder traversal */
	trav_pre_print_tn(Root, &row, gen, indent, canvas);
}
/*=========================================================
 * pedigree_draw_gedcom -- print out gedcom node tree
 * Created: 2001/01/27, Perry Rapp
 *=======================================================*/
void
pedigree_draw_gedcom (RecordIndexEl *rec, int gdvw, CANVASDATA canvas, bool reuse)
{
	int count=0, gen=0, row=canvas->rect->top;
	int indent = get_indent();
	if (gdvw == GDVW_TEXT) {
		draw_gedcom_text(rec, canvas, reuse, indent);
		return;
	}
	count_nodes(nztop(rec), gen, Gens, &count);
	set_scroll_max(canvas, count);
	check_scroll_max(canvas);
	/* preorder traversal */
	trav_pre_print_nod(nztop(rec), &row, gen, indent, canvas, gdvw);
}
/*=========================================================
 * draw_gedcom_text -- print out gedcom node tree in text wrapped view
 *  This builds a displaynode tree then displays it with a
 *  preorder traversal
 * Created: 2001/04/15, Perry Rapp
 *=======================================================*/
static void
draw_gedcom_text (RecordIndexEl *rec, CANVASDATA canvas, bool reuse, int indent)
{
	int gen=0;
	int row = canvas->rect->top;
	DISPNODE tn;
	if (!reuse) {
		int count=0;
		/* int skip=0; */
		free_entire_tree();
		Root = add_dnodes(nztop(rec), gen, indent, Gens, &count, canvas);
		set_scroll_max(canvas, count); 
	}
	check_scroll_max(canvas);
	/* preorder traversal */
	/* root may have siblings due to overflow/assimilation */
	for (tn=Root ; tn; tn = tn->nextsib) {
		trav_pre_print_tn_str(tn, &row, gen, indent, canvas);
	}
}
/*=====================================================
 * show_ancestors -- build ancestor tree & print it out
 *  Build a displaynode tree then print it out in
 *  inorder traversal
 * Created: 2000/12/07, Perry Rapp
 *===================================================*/
void
pedigree_draw_ancestors (RecordIndexEl *irec, CANVASDATA canvas, bool reuse)
{
	int gen=0;
	int row = canvas->rect->top;
	int indent = get_indent();
	/* build displaynode tree */
	if (!reuse) {
		int count=0;
		free_entire_tree();
		Root = add_parents(nztop(irec), gen, Gens, &count);
		set_scroll_max(canvas, count);
	}
	check_scroll_max(canvas);
	/* inorder traversal */
	trav_bin_in_print_tn(Root, &row, gen, indent, canvas);
}
/*===========================================
 * pedigree_toggle_mode -- toggle between 
 *  ancestral & descendant mode pedigree tree
 * Created: 2000/12/07, Perry Rapp
 *=========================================*/
void
pedigree_toggle_mode (void)
{
	Ancestors_mode = !Ancestors_mode;
}
/*======================================================
 * pedigree_increase_generations -- adjust # generations
 *  displayed in pedigree tree
 * Created: 2000/12/07, Perry Rapp
 *====================================================*/
void
pedigree_increase_generations (int delta)
{
	Gens += delta;
	if (Gens > GENS_MAX)
		Gens = GENS_MAX;
	else if (Gens < GENS_MIN)
		Gens = GENS_MIN;
}
/*===============================================================
 * free_dispnode_tree -- free a displaynode tree
 * Created: 2000/02/01, Perry Rapp
 *=============================================================*/
static void
free_dispnode_tree (DISPNODE tn)
{
	DISPNODE child, sib;
	ASSERT(tn);
	/* must get my links before I kill myself */
	child=tn->firstchild;
	sib=tn->nextsib;
	free_displaynode(tn);
	if (child)
		free_dispnode_tree(child);
	if (sib)
		free_dispnode_tree(sib);
}
/*===============================================================
 * free_entire_tree -- free a displaynode tree
 *  Root may have siblings because of overflow/assimilation
 * Created: 2000/04/15, Perry Rapp
 *=============================================================*/
static void
free_entire_tree (void)
{
	if (Root) {
		/* free_dispnode_tree frees both children & siblings */
		free_dispnode_tree(Root);
		Root = 0;
	}
}
