//
//  DeadEnds Library
//  rassa.c handles the page mode builtins for DeadEnds script programs.
//
//  Created by Thomas Wetmore on 10 February 2024.
//  Last changed on 18 June 2025.
//

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/param.h>		/* MAXPATHLEN */

#include "standard.h"
#include "context.h"
#include "errors.h"
#include "evaluate.h"
#include "file.h"
#include "gedcom.h"
#include "hashtable.h"
#include "pnode.h"
#include "interp.h"
#include "pvalue.h"

#include "builtintable.h"
#include "denls.h"
#include "messages.h"
#include "ask.h"
#include "locales.h"		/* CALLBACK_FNC, needed by lloptions.h */
#include "lloptions.h"		/* getdeoptstr */
#include "feedback.h"
#include "frame.h"

#define MAXROWS 512
#define MAXCOLS 512

/* forward references */
static bool request_file (Context *context, CString *msg);

// XXX do we need this? XXX
void finishrassa (File *file ATTRIBUTE_UNUSED)
{
}

//static void pageout_1 (FILE *);

// __pagemode switches script program output to page mode.
// usage: pagemode(INT, INT) -> VOID
// First parameter is number of rows, second is number of columns.
PValue __pagemode(PNode* pnode, Context* context, bool* errflg) {
    PNode* arg = pnode->arguments;
    PValue pvalue = evaluate(arg, context, errflg);
    if (*errflg || pvalue.type != PVInt) {
	*errflg = true;
	scriptError(pnode, "the rows argument to pagemode must be an integer.");
	return nullPValue;
    }
    int rows = (int) pvalue.value.uInt;
    arg = arg->next;
    pvalue = evaluate(arg, context, errflg);
    if (*errflg || pvalue.type != PVInt) {
	*errflg = true;
	scriptError(pnode, "the columns argument to pagemode must be an integer.");
	return nullPValue;
    }
    int cols = (int) pvalue.value.uInt;
    if (cols < 1 || cols > MAXCOLS || rows < 1 || rows > MAXROWS) {
	*errflg = true;
	scriptError(pnode, "the value of rows or cols to pagemode is out of range.");
	return nullPValue;
    }
    File* file = context->file;
    if (! file) {
      /* we do not have an output file, get one! */
      CString msg = 0;
      bool rtn = request_file (context, &msg);
      if (! rtn) {
	*errflg =- true;
	scriptError (pnode, msg);
	return nullPValue;
      }
      file = context->file;
    }
    // If in page mode remove the old Page before creating a new one.
    if (file->mode == pageMode) deletePage(file->page);
    file->mode = pageMode;
    file->page = createPage(rows, cols);
    return nullPValue;
}

// __linemode switches script program output to line mode.
// usage: linemode() -> VOID
PValue __linemode(PNode* pnode, Context* context, bool* errflg) {
    File* file = context->file;
    if (! file) {
      /* we do not have an output file, get one! */
      CString msg = 0;
      bool rtn = request_file (context, &msg);
      if (! rtn) {
	*errflg = true;
	scriptError (pnode, msg);
	return nullPValue;
      }
      file = context->file;
    }
    // Do nothing if already in line mode.
    if (file->mode == lineMode) return nullPValue;
    // Switch to line mode; assume the last page has been written by a pageout().
	file->mode = lineMode;
    deletePage(file->page);
    file->page = null;
    return nullPValue;
}

// __newfile switches script program output to a new file.
// usage: newfile() -> VOID
// usage: newfile(STRING[, BOOL]) -> VOID
PValue __newfile(PNode* pnode, Context* context, bool* errflg) {
    PNode* arg = pnode->arguments;
    // Revert to standard output if there are no arguments.
    if (!arg) {
        finishrassa (context->file); /* needed for page mode */
        closeFile(context->file);
        context->file = stdOutputFile();
        return nullPValue;
    }
    // The first argument must be a non-empty string (file name).
    PValue pvalue = evaluate(arg, context, errflg);
    if (*errflg) return nullPValue;
    if (pvalue.type != PVString || !pvalue.value.uString || strlen(pvalue.value.uString) == 0) {
        *errflg = true;
        scriptError(pnode, "first argument to newfile must be a non-empty string");
        return nullPValue;
    }
    String name = pvalue.value.uString;
    // Default mode is write
    char* mode = "w";
    // The optional second argument can specify append mode.
    arg = arg->next;
    if (arg) {
        pvalue = evaluateBoolean(arg, context, errflg);
        if (*errflg) {
            scriptError(pnode, "second argument to newfile() must be a boolean");
            return nullPValue;
        }
        if (pvalue.value.uBool) mode = "a";
    }

    finishrassa (context->file); /* needed for page mode */
    // Open the new file.
    File* file = openFile(name, mode);
    if (!file) {
      *errflg = true;
      scriptError(pnode, "could not open file: %s", name);
      return nullPValue;
    }
    // Close the previous file and switch.
    closeFile(context->file);
    context->file = file;
    return nullPValue;
}

#if 0
bool
setScriptOutputFile (CString filename, bool append, CString *errorMessage)
{
  if (Poutfp) {
    finishrassa();
    fclose(Poutfp);
    Poutfp = null;
  }
  if (filename) {
    outfilename = strsave(filename);
    if (! (Poutfp = fopenPath (filename, (append ? "a" : "w"), "." /*llprograms*/))) {
      *errorMessage = "Could not open file";
      return false;
    }
  } else
    outfilename = null;

  return true;
}
#endif

// __print prints a list of expresseion values to the stdout window.
// usage: print([STRING]+,) -> VOID
PValue __print (PNode *pnode, Context *context, bool *errflg)
{
#if 0
  // __outfile does this, so we do it for consistency
  if (! context->file) {
    String dereports = getdeoptstr ("DEREPORTS", ".");
    Poutfp = ask_for_output_file("w", qSwhtout, &outfilename, dereports, NULL);
    if (! Poutfp) {
      *errflg = true;
      scriptError (pnode, noreport);
      return nullPValue;
    }
    setbuf(Poutfp, NULL);
  }
#endif

  int count = 0;
  for (PNode *arg = pnode->arguments; arg; arg = arg->next)
    {
      count++;
      PValue str = evaluate (arg, context, errflg);
      if (*errflg || (str.type != PVString))
	{
	  *errflg = true;
	  printf("arg %d is of type %d\n", count, str.type);
	  scriptError (pnode, "argument number %d to print is not a string", count);
	  return nullPValue;
	}
#if 1
      llwprintf ("%s", str.value.uString);
#else
      fprintf (Poutfp, "%s", str.value.uString);
#endif
  }
  return nullPValue;
}

// __outfile returns the name of the script output file.
// usage: outfile() -> STRING
PValue __outfile(PNode* pnode, Context* context, bool* errflg)
{
  if (! context->file || context->file->isStdout)
    {
      /* we do not have an output file, get one! */
      CString msg = 0;
      bool rtn = request_file (context, &msg);
      if (! rtn)
	{
	  *errflg = true;
	  scriptError (pnode, msg);
	  return nullPValue;
	}
    }
  return createStringPValue(context->file->name);
}

/* request_file -- Prompt user for report output file name.
   Returns success or failure.  */

static bool
request_file (Context *context, CString *msg)
{
  CString dereports = getdeoptstr ("DEREPORTS", ".");
  char fname[MAXPATHLEN];
  CString ttl = _(qSoutarc); /* Enter name of output archive file. */
  CString prompt = _(qSwhtfname); /* enter file name:  */
  bool rtn = ask_for_output_filename (ttl, dereports, prompt, fname, sizeof(fname));
  if (! rtn)
    {
      *msg = "error asking user for output file name";
      return false;
    }
  File *file = openFile(fname, DEWRITETEXT);
  if (! file)
    {
      *msg = "Error opening output file.  No report was generated.";
      return false;
    }
  context->file = file;
  setbuf(file->fp, NULL);
  //prefix_file_for_report(context->file->fp);
  return true;
}

// __pos positions page output to a row and column.
// usage: pos(INT, INT) -> VOID
PValue __pos(PNode* pnode, Context* context, bool* errflg) {
    // The file must be in page mode.
    File* file = context->file;
    if (file->mode != pageMode) {
        scriptError(pnode, "The output file must in page mode");
        *errflg = true;
        return nullPValue;
    }
    PNode *arg = pnode->arguments;
    // Get the column value.
	int row = evaluateInteger(arg, context, errflg);
	if (*errflg) {
		scriptError(pnode, "The first argument to pos must be an integer");
		return nullPValue;
	}
    // Get the row value.
	int col = evaluateInteger(arg->next, context, errflg);
	if (*errflg) {
		scriptError(pnode, "The second argument to pos must be an integer.");
		return nullPValue;
	}
    Page* page = file->page;
	if (row < 1 || row > page->nrows ||	col < 1 || col > page->ncols) {
		scriptError(pnode, "row or col is out of range");
        *errflg = true;
		return nullPValue;
	}
	page->currow = row;
	page->curcol = col;
	return nullPValue;
}

// __row positions output to the start of a row.
// usage: row(INT) -> VOID
PValue __row(PNode* pnode, Context* context, bool* errflg) {
    // The file must be in page mode.
    File* file = context->file;
    if (file->mode != pageMode) {
        scriptError(pnode, "The output file must in page mode");
        *errflg = true;
        return nullPValue;
    }
	int row = evaluateInteger(pnode->arguments, context, errflg);
	if (*errflg) {
		scriptError(pnode, "the argument to row must be an integer");
		return nullPValue;
	}
    Page* page = file->page;
	if (row < 1 || row > page->nrows) {
		scriptError(pnode, "the row value is out of range");
        *errflg = true;
		return nullPValue;
	}
    page->currow = row;
    page->curcol = 1;
	return nullPValue;
}

// __col positions page output to a specific column.
// usage: col(INT) -> VOID
PValue __col (PNode *pnode, Context *context, bool *errflg) {
    // The file must be in page mode.
    File* file = context->file;
    if (file->mode != pageMode) {
        scriptError(pnode, "The output file must in page mode");
        *errflg = true;
        return nullPValue;
    }
	int col = evaluateInteger(pnode->arguments, context, errflg);
	if (*errflg) {
		scriptError(pnode, "the argument to col must be an integer");
		return nullPValue;
	}
    Page* page = file->page;
    if (col < 1 || col > page->ncols) {
        scriptError(pnode, "the column value is out of range");
        *errflg = true;
        return nullPValue;
    }
    page->curcol = col;
	return nullPValue;
}

// __TYPEOF returns the type of its argument as a string.
PValue __TYPEOF(PNode* pnode, Context* context, bool *errflg) {
    PValue pvalue = evaluate(pnode->arguments, context, errflg);
    if (*errflg) return nullPValue;
    return createStringPValue(typeOf(pvalue));
}

// __SHOWSTACK prints the runtime stack.
// usage: showstack()
extern void showRuntimeStack(Context*, PNode*);
PValue __SHOWSTACK(PNode* pnode, Context* context, bool *errflg) {
    showRuntimeStack(context, pnode);
    return nullPValue;
}

// __SHOWFRAME prints the current frame.
// usage: showframe()
PValue __SHOWFRAME(PNode* pnode, Context* context, bool* errflg) {
    showFrame(context->frame);
    return nullPValue;
}

// __pageout outputs the current page and clears the page buffer.
// usage: pageout() -> VOID
PValue __pageout(PNode* pnode, Context* context, bool* errflg) {
    // The file must be in page mode.
    File* file = context->file;
    if (file->mode != pageMode) {
        scriptError(pnode, "The output file must in page mode");
        *errflg = true;
        return nullPValue;
    }
    Page* page = file->page;
    char scratch[page->ncols + 2];
    scratch[page->ncols] = '\n';
    scratch[page->ncols + 1] = 0;
    String p = page->buffer;
    for (int row = 0; row < page->nrows; row++) {
        memcpy(scratch, p, page->ncols);
        fputs(scratch, file->fp);
        p += page->ncols;
    }
    memset(page->buffer, ' ', page->nrows*page->ncols);
    return nullPValue;
}

#if 0
/* pageout_1 -- pageout helper.  Needs to be called by finishrassa if
   we are in pagemode -- if we are closing the output file, we need
   write out the current page.  */

static void pageout_1 (FILE *fp)
{
	char scratch[MAXCOLS+2];
	String p;
	int row, i;

	scratch[__cols] = '\n';
	scratch[__cols+1] = 0;
	p = pagebuffer;
	for (row = 1; row <= __rows; row++) {
		memcpy(scratch, p, __cols);
		for (i = __cols - 1; i > 0 && scratch[i] == ' '; i--)
			;
		scratch[i+1] = '\n';
		scratch[i+2] = 0;
		fputs(scratch, fp);
		p += __cols;
	}
	memset(pagebuffer, ' ', __rows*__cols);
}
#endif

// adjustCols adjusts the column after printing a string.
// NOTE: This looks buggy. NOTE: This looks buggy.
static void adjustCols(String string, Page* page) {
	int c;
	while ((c = *string++)) {
		if (c == '\n')
			page->curcol = 1;
		else
			page->curcol++;
	}
}

// poutput outputs a string to the current program output file in the current mode.
void oldpoutput(String string, Context* context) {
    File* file = context->file;
    // Handle line mode.
    if (file->mode == lineMode) {
        fprintf(file->fp, "%s", string);
        return;
    }

    // Handle page mode.
    if (file->mode != pageMode) FATAL();
    if (!string || *string == 0) return;
    Page* page = file->page;
    String buffer = page->buffer;
    String p = buffer + (page->currow - 1)*page->ncols + page->curcol - 1;
    int c;
    while ((c = *string++)) {
        if (c == '\n') {
            page->curcol = 1;
            page->currow++;
            p = buffer + (page->currow)*page->ncols;
        } else {
            if (page->curcol <= page->ncols && page->currow <= page->nrows) {
                *p++ = c;
                page->curcol++;
            }
        }
    }
}

// poutput outputs a string to the current program output file in the current mode.
void poutput(PNode *pnode, String string, Context* context, bool *errflg) {
    File* file = context->file;

    if (! file) {
      /* we do not have an output file, get one! */
      CString msg = 0;
      bool rtn = request_file (context, &msg);
      if (! rtn) {
	*errflg = true;
	scriptError (pnode, msg);
	return;
      }
      file = context->file;
    }

    // Handle line mode.
    if (file->mode == lineMode) {
        fprintf(file->fp, "%s", string);
        return;
    }

    // Sanity check for page mode.
    if (file->mode != pageMode || !string || *string == 0) return;

    Page* page = file->page;
    String buffer = page->buffer;

    int row = page->currow;
    int col = page->curcol;

    while (*string) {
        char c = *string++;

        if (c == '\n') {
            row++;
            col = 1;
            if (row > page->nrows) break; // Avoid writing past end of page
            continue;
        }

        if (row > page->nrows || col > page->ncols) continue; // Ignore overflows

        // Compute linear index and store character.
        int offset = (row - 1) * page->ncols + (col - 1);
        buffer[offset] = c;
        col++;
    }

    page->currow = row;
    page->curcol = col;
}

