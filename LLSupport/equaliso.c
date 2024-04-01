/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/*=============================================================
 * equaliso.c -- Equality operations on node trees
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 *   3.0.3 - 15 Aug 95
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(DEADENDS)
#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "refnindex.h"
#include "gnode.h"
#include "rfmt.h"
#include "xlat.h"
#include "editing.h"
#include "equaliso.h"
#else
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#endif

/*=========================================
 * equal_tree -- See if two trees are equal
 *=======================================*/
bool
equal_tree (GNode *root1,
            GNode *root2)
{
	String str1, str2;
	if (!root1 && !root2) return true;
	if (!root1 || !root2) return false;
	if (length_nodes(root1) != length_nodes(root2)) return false;
	while (root1) {
		if (nestr(ntag(root1), ntag(root2))) return false;
		str1 = nval(root1);
		str2 = nval(root2);
		if (str1 && !str2) return false;
		if (str2 && !str1) return false;
		if (str1 && str2 && nestr(str1, str2)) return false;
		if (!equal_tree(nchild(root1), nchild(root2))) return false;
		root1 = nsibling(root1);
		root2 = nsibling(root2);
	}
	return true;
}
/*=========================================
 * equal_node -- See if two nodes are equal
 *=======================================*/
bool
equal_node (GNode *node1,
            GNode *node2)
{
	String str1, str2;
	if (!node1 && !node2) return true;
	if (!node1 || !node2) return false;
	if (nestr(ntag(node1), ntag(node2))) return false;
	str1 = nval(node1);
	str2 = nval(node2);
	if (str1 && !str2) return false;
	if (str2 && !str1) return false;
	if (str1 && str2 && nestr(str1, str2)) return false;
	return true;
}
/*=================================================
 * iso_list -- See if two node lists are isomorphic
 *===============================================*/
bool
iso_list (GNode *root1,
          GNode *root2)
{
	int len1, len2;
	GNode *node1, *node2;
	if (!root1 && !root2) return true;
	if (!root1 || !root2) return false;
	len1 = length_nodes(root1);
	len2 = length_nodes(root2);
	if (len1 != len2) return false;
	if (len1 == 0) return true;
	node1 = root1;
	while (node1) {
		node2 = root2;
		while (node2) {
			if (equal_node(node1, node2))
				break;
			node2 = nsibling(node2);
		}
		if (!node2) return false;
		node1 = nsibling(node1);
	}
	return true;
}

/*====================================================
 * equal_nodes -- See if two node structures are equal
 *==================================================*/
bool
equal_nodes (GNode *root1,
             GNode *root2,
             bool kids,
             bool sibs)
{
	if (!root1 && !root2) return true;
	while (root1) {
		if (!equal_node(root1, root2)) return false;
		if (kids && !equal_nodes(nchild(root1), nchild(root2), 1, 1))
			return false;
		if (!sibs) return true;
		root1 = nsibling(root1);
		root2 = nsibling(root2);
	}
	return (root2 == NULL);
}
/*=======================================================
 * iso_nodes -- See if two node structures are isomorphic
 *=====================================================*/
bool
iso_nodes (GNode *root1,
           GNode *root2,
           bool kids,
           bool sibs)
{
	int len1, len2;
	GNode *node1, *node2;

	if (!root1 && !root2) return true;
	if (!root1 || !root2) return false;

	if (!kids && !sibs) return equal_node(root1, root2);
	if ( kids && !sibs)
		return equal_node(root1, root2) && iso_nodes(nchild(root1),
		    nchild(root2), 1, 1);

	len1 = length_nodes(root1);
	len2 = length_nodes(root2);
	if (len1 != len2) return false;
	if (len1 == 0) return true;

	node1 = root1;
	while (node1) {
		node2 = root2;
		while (node2) {
			if (iso_nodes(node1, node2, kids, 0))
				break;
			node2 = nsibling(node2);
		}
		if (!node2) return false;
		node1 = nsibling(node1);
	}
	return true;
}
