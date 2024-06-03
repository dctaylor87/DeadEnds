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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-04-25 J.F.Chandler */
/*==========================================================
 * import.c -- Import GEDCOM file to LifeLines database
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 29 Aug 94    3.0.2 - 10 Dec 94
 *   3.0.3 - 14 Jan 96
 *========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "sequence.h"
#include "gedcheck.h"
#include "liflines.h"

#include "llinesi.h"
#include "loadsave.h"
#include "feedback.h"
#include "impfeed.h"
#include "codesets.h"
#include "zstr.h"
#include "messages.h"


/*********************************************
 * external/imported variables
 *********************************************/

/* external data set by check_stdkeys() */

extern int gd_itot;	/* total number of individuals */
extern int gd_ftot;	/* total number of families */
extern int gd_stot;	/* total number of sources */
extern int gd_etot;	/* total number of events */
extern int gd_xtot;	/* total number of others */
extern int gd_imax;	/* maximum individual key number */
extern int gd_fmax;	/* maximum family key number */
extern int gd_smax;	/* maximum source key number */
extern int gd_emax;	/* maximum event key number */
extern int gd_xmax;	/* maximum other key number */

/*********************************************
 * local types
 *********************************************/

typedef struct gd_metadata_s {
	ZSTR sour_system_id;
	ZSTR sour_system_version;
	ZSTR sour_system_name;
	ZSTR sour_system_data_name;
	ZSTR date;
	ZSTR filename;
	ZSTR gedcom_version;
	ZSTR gedcom_form;
	ZSTR charset;
	ZSTR charset_version;
	ZSTR language;
} *GD_METADATA;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static bool do_import(IMPORT_FEEDBACK ifeed, FILE *fp);
static bool is_lossy_conversion(const char * cs_src, const char * cs_dest);
static bool is_unicode_encoding_name(const char * codeset);
static void restore_record(NODE node, int type, int num);
static String translate_key(String);
static bool translate_values(NODE, void *);

/*********************************************
 * local variables
 *********************************************/

static int gd_reuse = 1;/* reuse original keys in GEDCOM file if possible */

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================================
 * import_from_file -- Read GEDCOM file to database
 *  ifeed: [IN]  output methods
 *  fp:    [I/O] GEDCOM file whence to load data
 *===============================================*/
bool
import_from_gedcom_file (IMPORT_FEEDBACK ifeed, FILE *fp)
{
	bool rtn=false;

	flineno = 0;
	rtn = do_import(ifeed, fp);

	return rtn;
}
/*=================================================
 * do_import -- Read GEDCOM file to database
 *  ifeed: [IN]  output methods
 *  fp:    [I/O] GEDCOM file whence to load data
 *===============================================*/
static bool
do_import (IMPORT_FEEDBACK ifeed, FILE *fp)
{
	NODE node, conv;
	XLAT ttm = 0;
	String msg;
	bool emp;
	int nindi = 0, nfam = 0, neven = 0;
	int nsour = 0, nothr = 0, type, num = 0;
	int totkeys = 0, totused = 0;
	char msgbuf[80];
	bool succeeded=false;
	String str,unistr=0;
	ZSTR zerr=0;
	TABLE metadatatab = createStringTable();
	String gdcodeset=0;
	int warnings=0;

	/* start by assuming default */
	strupdate(&gdcodeset, gedcom_codeset_in);

/*	rptui_init(); *//* clear ui time counter */

/* Open and validate GEDCOM file */
	if ((unistr=check_file_for_unicode(fp)) && !eqstr(unistr, "UTF-8")) {
		msg_error(_(qSunsupuniv), unistr);
		goto end_import;
	}
	if (eqstr_ex(unistr, "UTF-8")) {
		strupdate(&gdcodeset, "UTF-8");
	}

	if (!scan_header(fp, metadatatab, &zerr)) {
		msg_error("%s", zs_str(zerr));
		goto end_import;
	}

	if ((str = valueof_str(metadatatab, "GEDC.FORM"))!= NULL) {
		if (!eqstr(str, "LINEAGE-LINKED")) {
			if (!ask_yes_or_no_msg(
				_("This is not a lineage linked GEDCOM file.")
				, _("Proceed anyway?")
				))
				goto end_import;
		}
	}
	if (!unistr && (str = valueof_str(metadatatab, "CHAR"))!= NULL) {
		/* if no BOM, use file's declared encoding if present */
		strupdate(&gdcodeset, str);
	}

	/* TODO: Push this codeset question down to after the validation, where we can know if
	the incoming file happened to really be all ASCII */

	if (!int_codeset[0]) {
		/* TODO: ask if user would like to adopt codeset of incoming file, if we found it */
		if (!ask_yes_or_no_msg(
			_("No current internal codeset, so no codeset conversion can be done")
			, _("Proceed without codeset conversion?")
			))
			goto end_import;
	}

	/* Warn if lossy code conversion likely */
	if (gdcodeset[0] && int_codeset[0]) {
		if (is_lossy_conversion(gdcodeset, int_codeset)) {
			ZSTR zstr=zs_new();
			zs_setf(zstr, _("Lossy codeset conversion (from <%s> to <%s>) likely")
				, gdcodeset, int_codeset);
			if (!ask_yes_or_no_msg(
				zs_str(zstr)
				, _("Proceed anyway?")
				))
				goto end_import;
		}
	}

	/* validate */
	if (ifeed && ifeed->validating_fnc)
		(*ifeed->validating_fnc)();

	if (!validate_gedcom(ifeed, fp)) {
		if (ifeed && ifeed->error_invalid_fnc)
			(*ifeed->error_invalid_fnc)(_(qSgdnadd));
		goto end_import;
	}
	warnings = validate_get_warning_count();
	if (warnings) {
		ZSTR zstr=zs_new();
		zs_setf(zstr, _pl(FMT_INT " warning during import",
			          FMT_INT " warnings during import", warnings), warnings);
		if (!ask_yes_or_no_msg(zs_str(zstr), _(qSproceed))) {
			goto end_import;
		}
	}

	if (gdcodeset[0] && int_codeset[0]) {
retry_input_codeset:
		ttm = transl_get_xlat(gdcodeset, int_codeset);
		if (!transl_is_xlat_valid(ttm)) {
			ZSTR zstr=zs_new();
			char csname[64];
			bool b;
			transl_release_xlat(ttm);
			ttm = 0;
			zs_setf(zstr, _("Cannot convert codeset (from <%s> to <%s>)")
				, gdcodeset, int_codeset);
			b = ask_for_string(zs_str(zstr)
				, _("Enter codeset to assume (* for none)")
				, csname, sizeof(csname)) && csname[0];
			zs_free(&zstr);
			if (!b)
				goto end_import;
			if (!eqstr(csname, "*")) {
				strupdate(&gdcodeset, csname);
				goto retry_input_codeset;
			}
		}
	}
	
	if((num_indis() > 0)
		|| (num_fams() > 0)
		|| (num_sours() > 0)
		|| (num_evens() > 0)
		|| (num_othrs() > 0)) gd_reuse = false;
	else if((gd_reuse = check_stdkeys())) {
		totused = gd_itot + gd_ftot + gd_stot + gd_etot + gd_xtot;
		totkeys = gd_imax + gd_fmax + gd_smax + gd_emax + gd_xmax;
		if((totkeys-totused) > 0) {
			int delkeys = totkeys-totused;
			snprintf(msgbuf, sizeof(msgbuf)
				, _pl("Using original keys, " FMT_INT " deleted record will be in the database."
					, "Using original keys, " FMT_INT " deleted records will be in the database."
					, delkeys)
				, delkeys
				);
		}
		else strcpy(msgbuf, " ");
		gd_reuse = ask_yes_or_no_msg(msgbuf, _(qScfoldk));
/*
TODO: why were these here ?
		touchwin(uiw_win(stdout_win));
		wrefresh(uiw_win(stdout_win));
*/
	}

	/* start loading the file */
	rewind(fp);

	/* test for read-only database here */

	/* tell user we are beginning real part of import */
	if (ifeed && ifeed->beginning_import_fnc) {
		if(gd_reuse)
			(*ifeed->beginning_import_fnc)(_(qSdboldk));
		else
			(*ifeed->beginning_import_fnc)(_(qSdbnewk));
	}


/* Add records to database */
	node = convert_first_fp_to_node(fp, false, ttm, &msg, &emp);
	while (node) {
		if (!(conv = node_to_node(node, &type))) {
			freeGNodes(node);
			node = next_fp_to_node(fp, false, ttm, &msg, &emp);
			continue;
		}
		switch (type) {
		case INDI_REC: num = ++nindi; break;
		case FAM_REC:  num = ++nfam;  break;
		case EVEN_REC: num = ++neven; break;
		case SOUR_REC: num = ++nsour; break;
		case OTHR_REC: num = ++nothr; break;
		default: FATAL();
		}
		restore_record(conv, type, num);
		if (ifeed && ifeed->added_rec_fnc)
			ifeed->added_rec_fnc(nxref(conv)[1], ntag(conv), num);
		freeGNodes(node);
		node = next_fp_to_node(fp, false, ttm, &msg, &emp);
	}
	if (msg) {
		msg_error("%s", msg);
	}
	if(gd_reuse && ((totkeys - totused) > 0)) {
		if (ifeed && ifeed->adding_unused_keys_fnc)
			(*ifeed->adding_unused_keys_fnc)();
		addmissingkeys(INDI_REC);
		addmissingkeys(FAM_REC);
		addmissingkeys(EVEN_REC);
		addmissingkeys(SOUR_REC);
		addmissingkeys(OTHR_REC);
	}
	succeeded = true;

end_import:
	validate_end_import();
	zs_free(&zerr);
	destroy_table(metadatatab);
	strfree(&gdcodeset);
	return succeeded;
}
/*=============================================
 * restore_record -- Restore record to database
 *===========================================*/
static void
restore_record (NODE node, int type, ATTRIBUTE_UNUSED int num)
{
	String old, new, str, key;

	if (!node) return;
	ASSERT(old = nxref(node));
	new = translate_key(rmvat(old));
	switch (type) {
	case INDI_REC: break;
	case FAM_REC:  break;
	case EVEN_REC: break;
	case SOUR_REC: break;
	case OTHR_REC: break;
	default: FATAL();
	}
	if (nestr(old, new)) {
		stdfree(old);
		nxref(node) = strsave(new);
	}
	traverse_nodes(node, translate_values, 0);
	if (type == INDI_REC) {
		add_indi_no_cache(node);
		return;
	}
	resolve_refn_links(node);
	ASSERT(str = node_to_string(node));
	key = rmvat(nxref(node));
	ASSERT(store_record(key, str, strlen(str)));
	index_by_refn(node, key);
	stdfree(str);
}
/*==============================================================
 * translate_key -- Translate key from external to internal form
 *============================================================*/
String
translate_key (String key)    /* key does not have surrounding @ chars */
{
	ELMNT elm;
	int dex = xref_to_index(key);
	ASSERT(dex > -1);
	elm = index_data[dex];
	ASSERT(elm);
	switch (Type(elm)) {
	case INDI_REC:
		if (!New(elm)) New(elm) = strsave(newixref(key, gd_reuse));
		return New(elm);
	case FAM_REC:
		if (!New(elm)) New(elm) = strsave(newfxref(key, gd_reuse));
		return New(elm);
	case SOUR_REC:
		if (!New(elm)) New(elm) = strsave(newsxref(key, gd_reuse));
		return New(elm);
	case EVEN_REC:
		if (!New(elm)) New(elm) = strsave(newexref(key, gd_reuse));
		return New(elm);
	case OTHR_REC:
		if (!New(elm)) New(elm) = strsave(newxxref(key, gd_reuse));
		return New(elm);
	default:
		FATAL();
	}
	/* NOTREACHED */
	return NULL;
}
/*============================================================
 * translate_values -- Traverse function to translate pointers
 *==========================================================*/
static bool
translate_values (NODE node, ATTRIBUTE_UNUSED void *param)
{
	String new;
	if (!pointer_value(nval(node))) return true;
	new = translate_key(rmvat(nval(node)));
	stdfree(nval(node));
	nval(node) = strsave(new);
	return true;
}
/*============================================================
 * is_lossy_conversion -- Is this a lossy codeset conversion?
 * (we say it is if from Unicode to non-Unicode)
 *==========================================================*/
static bool
is_lossy_conversion (const char * cs_src, const char * cs_dest)
{
	/* if no destination, then no conversion, not lossy */
	if (!cs_dest || !cs_dest[0]) return false;
	if (eqstr_ex(cs_src, cs_dest)) {
		/* source same as destination, no conversion, not lossy */
		return false;
	} else {
		const char * cs_in = norm_charmap((char *)cs_src);
		const char * cs_out = norm_charmap((char *)cs_dest);
		/* conversion to unicode is not lossy */
		if (is_unicode_encoding_name(cs_out)) return false;
		/* if source is ASCII, assume all ok, not lossy */
		if (eqstr(cs_in, "ASCII")) return false;
		/* everything else assumed lossy */
		return true;
	}
}
/*============================================================
 * is_unicode_encoding_name -- Is this a unicode encoding?
 *==========================================================*/
static bool
is_unicode_encoding_name (const char * codeset)
{
	if (!codeset || !codeset[0]) return false;
	if (eqstr(codeset, "UTF-8")) return true;
	if (eqstr(codeset, "UCS-2LE")) return true;
	if (eqstr(codeset, "UCS-2BE")) return true;
	if (eqstr(codeset, "UTF-16LE")) return true;
	if (eqstr(codeset, "UTF-16BE")) return true;
	if (eqstr(codeset, "UTF-32")) return true;
	return false;
}
