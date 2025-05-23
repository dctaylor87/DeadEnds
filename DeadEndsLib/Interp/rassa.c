// DeadEnds
//
// rassa.c handles printing output from DeadEnds script programs.
//
// Created by Thomas Wetmore on 10 February 2024.
// Last changed on 17 May 2025.

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/param.h>		/* MAXPATHLEN */

#include "standard.h"
#include "hashtable.h"
#include "gedcom.h"
#include "interp.h"
#include "evaluate.h"
#include "builtintable.h"
#include "denls.h"
#include "messages.h"
#include "ask.h"
#include "locales.h"		/* CALLBACK_FNC, needed by lloptions.h */
#include "lloptions.h"		/* getdeoptstr */
#include "feedback.h"

#define MAXROWS 512
#define MAXCOLS 512
#define MAXPAGESIZE (MAXROWS*MAXCOLS)
int __cols = 0, __rows = 0;
int curcol = 1, currow = 1;
int outputmode = BUFFERED;
static String pagebuffer = null; // buffer for PAGEMODE.
static char linebuffer[1024];
static int linebuflen = 0;
static String bufptr = linebuffer;
String outfilename;

String noreport = (String) "No report was generated.";
//String whtout = (String) "What is the name of the output file?";

// initrassa initializes script program output.
void initrassa(void) {
	outputmode = BUFFERED;
	linebuflen = 0;
	bufptr = linebuffer;
	curcol = 1;
}

// finishrassa finalizes script program output.
//void finishrassa(void) {
//	if (outputmode == BUFFERED && linebuflen > 0 && Poutfp) {
//		fwrite(linebuffer, linebuflen, 1, Poutfp);
//		linebuflen = 0;
//		bufptr = linebuffer;
//		curcol = 1;
//	}
//}

// __pagemode switches script program output to page mode.
// usage: pagemode(INT, INT) -> VOID
PValue __pagemode(PNode* pnode, Context* context, bool* errflg) {
	PNode* arg = pnode->arguments;
	PValue pvalue = evaluate(arg, context, errflg);
	if (*errflg || pvalue.type != PVInt) {
		*errflg = true;
		scriptError(pnode, "the cols argument to pagemode must be an integer.");
		return nullPValue;
	}
	int cols = (int) pvalue.value.uInt;
	arg = arg->next;
	pvalue = evaluate(arg, context, errflg);
	if (*errflg || pvalue.type != PVInt) {
		*errflg = true;
		scriptError(pnode, "the rows argument to pagemode must be an integer.");
		return nullPValue;
	}
	int rows = (int) pvalue.value.uInt;
	*errflg = true;
	if (cols < 1 || cols > MAXCOLS || rows < 1 || rows > MAXROWS) {
		scriptError(pnode, "the value of rows or cols to pagemode is out of range.");
		return nullPValue;
	}
	*errflg = false;
	outputmode = PAGEMODE;
	__rows = rows;
	__cols = cols;
	if (pagebuffer) stdfree(pagebuffer);
	pagebuffer = (String) stdalloc(__rows*__cols);
	memset(pagebuffer, ' ', __rows*__cols);
	return nullPValue;
}

// __linemode switches script program output to line mode.
// usage: linemode() -> VOID
PValue __linemode(PNode* pnode ATTRIBUTE_UNUSED,
		  Context* context ATTRIBUTE_UNUSED,
		  bool* errflg) {
	outputmode = BUFFERED;
	linebuflen = 0;
	bufptr = linebuffer;
	curcol = 1;
	*errflg = false;
	return nullPValue;
}

// __newfile switches script program output to a new file.
// usage: newfile(STRING, BOOL) -> VOID
PValue __newfile(PNode* pnode, Context* context, bool* errflg) {
    PNode *arg = pnode->arguments;
    PValue pvalue = evaluate(arg, context, errflg);
    if (*errflg) return nullPValue;
    if (pvalue.type != PVString || strlen(pvalue.value.uString) == 0) {
        *errflg = true;
        scriptError(pnode, "first argument to newfile must be a string");
        return nullPValue;
    }
    String name = pvalue.value.uString;
    arg = arg->next;
    pvalue = evaluateBoolean(arg, context, errflg);
    if (*errflg) {
        scriptError(pnode, "second argument to newfile must be a boolean");
	return nullPValue;
    }
    bool aflag = pvalue.value.uBool;
#if 1
    closeFile(context->file);
    File *file = openFile(name, aflag ? "a" : "w");
    if (!file) {
      *errflg = true;
      scriptError(pnode, "could not open file %s", name);
      return nullPValue;
    }
    context->file = file;
#else
#if 1
	CString errorMessage = null;
	bool success = setScriptOutputFile (name, aflag, &errorMessage);
	if (! success) {
	  scriptError (pnode, "%s %s", errorMessage, name);
	  return nullPValue;
	}
#else
	if (Poutfp) {
		finishrassa();
		fclose(Poutfp);
		Poutfp = null;
	}
	outfilename = strsave(name);
	if (!(Poutfp = fopenPath(name, aflag?"a":"w", "." /*llprograms*/))) {
		scriptError(pnode, "Could not open file %s", name);
		return nullPValue;
	}
#endif
#endif
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
  if (! Poutfp) {
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
  if (context->file->isStdout)
    {
      CString dereports = getdeoptstr ("DEREPORTS", ".");
      char fname[MAXPATHLEN];
      CString ttl = _(qSoutarc); /* Enter name of output archive file. */
      CString prompt = _(qSwhtfname); /* enter file name:  */
      bool rtn = ask_for_output_filename (ttl, dereports, prompt, fname, sizeof(fname));
      if (! rtn)
	{
	  *errflg = true;
	  scriptError(pnode, "???");
	  return nullPValue;
	}
      File *file = openFile(fname, DEREADTEXT);
      if (! file)
	{
	  *errflg = true;
	  scriptError(pnode, noreport);
	  return nullPValue;
	}
      context->file = file;
      setbuf(file->fp, NULL);
    }
  *errflg = false;
  return createStringPValue(context->file->name);
}

#if 0
/* request_file -- Prompt user for file name
   returns open file pointer, or NULL if error
   handles error message.  */

static bool
request_file (bool *eflg)
{
  String rptdir = getlloptstr("DEREPORTS", ".");
  String fullpath=0;
  char fname[MAXPATHLEN];
  fname[0] = 0;
  bool rtn = rptui_ask_for_output_filename(_(qSwhtout), rptdir, _(qSwhtfname),
					   fname, sizeof (fname));
  if (! fname || ! fname[0])  {
    if (fname)
      prog_error(0, _("Report stopping due to error opening output file"));
    else
      prog_error(0, _("Report stopping due to lack of output file"));
    /* set error flag to stop interpreter */
    *eflg = true;
    /* set cancel flag to suppress traceback */
    rpt_cancelled = TRUE;
    strfree(&fname);
    return false;
  }
  if (outfilename)
    stdfree(outfilename);
  outfilename = fullpath;
  strfree(&fname);
  prefix_file_for_report(Poutfp);
  return true;
}
#endif

// __pos positions page output to a row and column.
// usage: pos(INT, INT) -> VOID
PValue __pos(PNode* pnode, Context* context, bool* errflg) {
	PNode *arg = pnode->arguments;
	int col = evaluateInteger(arg, context, errflg);
	if (*errflg) {
		scriptError(pnode, "The first argument to pos must be an integer");
		return nullPValue;
	}
	int row = evaluateInteger(arg->next, context, errflg);
	if (*errflg) {
		scriptError(pnode, "The second argument to pos must be an integer.");
		return nullPValue;
	}
	*errflg = true;
	if (outputmode != PAGEMODE || row < 1 || row > __rows ||
		col < 1 || col > __cols) {
		scriptError(pnode, "There is an error in the page mode, row or col values.");
		return nullPValue;
	}
	*errflg = false;
	currow = row;
	curcol = col;
	return nullPValue;
}

// __row positions output to the start of a row.
// usage: row(INT) -> VOID
PValue __row(PNode* pnode, Context* context, bool* errflg) {
	int row = evaluateInteger(pnode->arguments, context, errflg);
	if (*errflg) {
		scriptError(pnode, "The argument to row must be an integer.");
		return nullPValue;
	}
	*errflg = true;
	if (outputmode != PAGEMODE || row < 1 || row > __rows) {
		scriptError(pnode, "There is an error in the output mode or row value.");
		return nullPValue;
	};
	*errflg = false;
	currow = row;
	curcol = 1;
	return nullPValue;
}

// __col positions page output to specific column.
// usage: col(INT) -> VOID
PValue __col (PNode *pnode, Context *context, bool *errflg)
{
	int col = evaluateInteger(pnode->arguments, context, errflg);
	if (*errflg) {
		scriptError(pnode, "The argument to col must be an integer.");
		return nullPValue;
	}
	if (col < 1) col = 1;
	if (col > 100) col = 100;
	curcol = col;
	return nullPValue;
}

// __pageout outputs the current page and clears the page buffer.
// usage: pageout() -> VOID
PValue __pageout(PNode* pnode ATTRIBUTE_UNUSED,
		 Context* context,
		 bool* errflg) {
	char scratch[MAXCOLS+2];
	String p;
	int row, i;
	*errflg = true;
	if (outputmode != PAGEMODE) return nullPValue;
	*errflg = false;
	scratch[__cols] = '\n';
	scratch[__cols+1] = 0;
	p = pagebuffer;
	for (row = 1; row <= __rows; row++) {
		memcpy(scratch, p, __cols);
		for (i = __cols - 1; i > 0 && scratch[i] == ' '; i--)
			;
		scratch[i+1] = '\n';
		scratch[i+2] = 0;
		fputs(scratch, context->file->fp);
		p += __cols;
	}
	memset(pagebuffer, ' ', __rows*__cols);
	return nullPValue;
}

// adjustCols adjusts the column after printing a string
static void adjustCols(String string) {
	int c;
	while ((c = *string++)) {
		if (c == '\n')
			curcol = 1;
		else
			curcol++;
	}
}

// poutput outputs a string in the current mode.
//void poutput(String string) {
//	String p;
//	int c, len;
//	if (!string || *string == 0 || (len = (int) strlen(string)) <= 0) return;
////	if (!Poutfp) {
////		Poutfp = ask_for_file("w", whtout, &name, llreports);
////		if (!Poutfp)  {
////			message(noreport);
////			return;
////		}
////		setbuf(Poutfp, NULL);
////		outfilename = strsave(name);
////	}
//	switch (outputmode) {
//	case UNBUFFERED:
//		fwrite(string, len, 1, Poutfp);
//		adjustCols(string);
//		return;
//	case BUFFERED:
//		if (len > 1024) {
//			fwrite(linebuffer, linebuflen, 1, Poutfp);
//			fwrite(string, len, 1, Poutfp);
//			linebuflen = 0;
//			bufptr = linebuffer;
//			adjustCols(string);
//			return;
//		}
//		if (len + linebuflen > 1024) {
//			fwrite(linebuffer, linebuflen, 1, Poutfp);
//			linebuflen = 0;
//			bufptr = linebuffer;
//		}
//		linebuflen += len;
//		while ((c = *bufptr++ = *string++)) {
//			if (c == '\n')
//				curcol = 1;
//			else
//				curcol++;
//		}
//		--bufptr;
//		return;
//	case PAGEMODE:
//		p = pagebuffer + (currow - 1)*__cols + curcol - 1;
//		while ((c = *string++)) {
//			if (c == '\n') {
//				curcol = 1;
//				currow++;
//				p = pagebuffer + (currow - 1)*__cols;
//			} else {
//				if (curcol <= __cols && currow <= __rows)
//					*p++ = c;
//				curcol++;
//			}
//		}
//		return;
//	default:
//		FATAL();
//	}
//}

