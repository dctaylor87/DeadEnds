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
/*============================================================
 * choose.c -- Implements the choose operations
 * Copyright(c) 1992-4 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 06 Dec 94    3.0.3 - 08 May 95
 *==========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "refnindex.h"
#include "recordindex.h"
#include "rfmt.h"
#include "sequence.h"
#include "ask.h"
#include "feedback.h"
#include "errors.h"
#include "liflines.h"
#include "choose.h"
#include "ui.h"
#include "ll-sequence.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

/*=================================================
 * chooseChild -- Choose child of person or family
 *  irec: [IN] parent (may be null if fam provided)
 *  frec:  [IN] family (may be null if indi provided)
 *  msg0: [IN] message to display if no children
 *  msgn: [IN] title for choosing child from list
 *  ask1: [IN] whether to prompt if only one child
 *===============================================*/
RecordIndexEl *
chooseChild (RecordIndexEl *irec, RecordIndexEl *frec, CString msg0, CString msgn, ASK1Q ask1)
{
  RecordIndexEl *rec=0;
  Sequence *seq=0;

  if (irec)
    {
      ASSERT (! irec->parent);
      seq = personToChildren(nztop(irec), currentDatabase->recordIndex);
    }
  if (! irec && frec)
    {
      ASSERT (! frec->parent);
      seq = familyToChildren(nztop(frec), currentDatabase->recordIndex);
    }
  if (!seq)
    {
      msg_error("%s", msg0);
      return NULL;
    }
  rec = chooseFromSequence(seq, ask1, msgn, msgn);
  deleteSequence(seq);
  return rec;
}
/*========================================
 * chooseSpouse -- Choose person's spouse
 *  irec: [IN]  known person (gives up if this is null)
 *  msg0: [IN] message to display if no spouses
 *  msgn: [IN] title for choosing spouse from list
 *  asks if multiple
 *======================================*/
RecordIndexEl *
chooseSpouse (RecordIndexEl *irec, CString msg0, CString msgn)
{
  RecordIndexEl *rec=0;
  Sequence *seq=0;

  if (!irec) return NULL;
  ASSERT (! irec->parent);
  if (!(seq = indi_to_spouses(nztop(irec))))
    {
      msg_error("%s", msg0);
      return NULL;
    }
  rec = chooseFromSequence(seq, NOASK1, NULL, msgn);
  deleteSequence(seq);
  return rec;
}
/*========================================
 * chooseSource -- Choose any referenced source from some,
 *  presumably top level, node
 *  always asks
 *======================================*/
RecordIndexEl *
chooseSource (RecordIndexEl *current, CString msg0, CString msgn)
{
  Sequence *seq;
  RecordIndexEl *rec;
  if (! current)
    return NULL;
  ASSERT (! current->parent);
    if (!(seq = GNodeToSources(nztop(current), currentDatabase)))
      {
	msg_error("%s", msg0);
	return NULL;
      }
  rec = chooseFromSequence(seq, DOASK1, msgn, msgn);
  deleteSequence(seq);
  return rec;
}

/*========================================
 * chooseNote -- Choose any referenced note from some,
 *  presumably top level, node
 *  always asks
 * Created: 2001/02/11, Perry Rapp
 *======================================*/
RecordIndexEl *
chooseNote (RecordIndexEl *current, CString msg0, CString msgn)
{
  Sequence *seq;
  RecordIndexEl *rec;
  if (! current)
    return NULL;

  ASSERT (! current->parent);
  if (!(seq = GNodeToNotes(nztop(current), currentDatabase)))
    {
      msg_error("%s", msg0);
      return NULL;
    }
  rec = chooseFromSequence(seq, DOASK1, msgn, msgn);
  deleteSequence(seq);
  return rec;
}

/*========================================
 * choosePointer -- Choose any reference (pointer) from some,
 *  presumably top level, node
 *  always asks
 * Created: 2001/02/24, Perry Rapp
 * Returns addref'd record
 *======================================*/
RecordIndexEl *
choosePointer (RecordIndexEl *current, CString msg0, CString msgn)
{
  Sequence *seq;
  RecordIndexEl *rec;
  if (! current)
    return NULL;
  ASSERT (! current->parent);
  if (!(seq = GNodeToPointers(nztop(current), currentDatabase)))
    {
      msg_error("%s", msg0);
      return NULL;
    }
  rec = chooseFromSequence(seq, DOASK1, msgn, msgn);
  deleteSequence(seq);
  return rec;
}

/*==========================================================
 * chooseFamily -- Choose family from person's FAMS/C lines
 *  asks if multiple
 * irec: [IN]  person of interest
 * msg0: [IN]  message to display if no families
 * msgn: [IN]  title if need to choose which family
 * fams: [IN]  want spousal families of indi ? (or families indi is child in)
 *========================================================*/
RecordIndexEl *
chooseFamily (RecordIndexEl *irec, CString msg0, CString msgn, bool fams)
{
  RecordIndexEl *rec=0;
  ASSERT (irec && ! irec->parent);
  Sequence *seq = indi_to_families(nztop(irec), fams);
  if (!seq)
    {
      if (msg0)
	msg_error("%s", msg0);
      return NULL;
    }
  rec = chooseFromSequence(seq, NOASK1, NULL, msgn);
  deleteSequence(seq);
  return rec;
}

/*===================================================
 * chooseFather -- Choose father of person or family
 * irec: [IN]  person of interest if non-null
 * frec: [IN]  family of interest if non-null
 * msg0: [IN]  message to display if no fathers
 * msgn: [IN]  title if need to choose which father
 * ask1: [IN]  whether or not to prompt if only one father found
 *=================================================*/
RecordIndexEl *
chooseFather (RecordIndexEl *irec, RecordIndexEl *frec, CString msg0, CString msgn, ASK1Q ask1)
{
  RecordIndexEl *rec=0;
  Sequence *seq=0;

  if (irec)
    {
      ASSERT (! irec->parent);
      seq = personToFathers(nztop(irec), currentDatabase->recordIndex);
    }
  if (!irec && frec)
    {
      ASSERT (! frec->parent);
	seq = familyToFathers(nztop(frec), currentDatabase->recordIndex);
    }
  if (!seq)
    {
      msg_error("%s", msg0);
      return NULL;
    }
  rec = chooseFromSequence(seq, ask1, msgn, msgn);
  deleteSequence(seq);
  return rec;
}

/*===================================================
 * chooseMother -- Choose mother of person or family
 * irec: [IN]  person of interest if non-null
 * frec: [IN]  family of interest if non-null
 * msg0: [IN]  message to display if no mothers
 * msgn: [IN]  title if need to choose which mother
 * ask1: [IN]  whether or not to prompt if only one mother found
 *=================================================*/
RecordIndexEl *
chooseMother (RecordIndexEl *irec, RecordIndexEl *frec, CString msg0, CString msgn, ASK1Q ask1)
{
  RecordIndexEl *rec=0;
  Sequence *seq=0;

  if (irec)
    {
      ASSERT (! irec->parent);
	seq = personToMothers(nztop(irec), currentDatabase->recordIndex);
    }
  if (!irec && frec)
    {
      ASSERT (! frec->parent);
	seq = familyToMothers(nztop(frec), currentDatabase->recordIndex);
    }
  if (!seq)
    {
      msg_error("%s", msg0);
      return NULL;
    }
  rec = chooseFromSequence(seq, ask1, msgn, msgn);
  deleteSequence(seq);
  return rec;
}
