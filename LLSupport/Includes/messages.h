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

/* These are declarations for the bulk of the strings used by the
   program.  The contents of the string is generally documented in a
   comment following the declaration.  *BUT* over time there will
   probably be 'drift' between the comment and the actual message.
   When in doubt, check gedlib/messages.c
 */

#define SS (STRING)
#define ZST STRING

extern ZST qSiddbse;      /* "You must identify a database." */
extern ZST qSidldir;      /* "What directory holds (or will hold) the LifeLines database? (? to list)" */
extern ZST qSidldrp;      /* "enter path: " */
extern ZST qSnodbse;      /* "No LifeLines database found." */
extern ZST qScrdbse;      /* "Do you want to create a database at '%s'?" */
extern ZST qSbdlkar;      /* "Argument to lock (-l) must be y or n." */
extern ZST qSnormls;      /* "Cannot remove last person from family." */
	/* summary of options (compiled with Finnish as optional) */
extern ZST qSusgFinnOpt;  /* "lines [-adkrwiflntcuFxoCzI] [database]   # Use -F for Finnish database" */
	/* summary of options (compiled with Finnish as mandatory) */
extern ZST qSusgFinnAlw;  /* "lines [-adkrwiflntcuxoCzI] [database]   # Finnish database" */
	/* summary of options (normal compile -- no Finnish support) */
extern ZST qSusgNorm;     /* "lines [-adkrwiflntcuxoCzI] [database]" */

extern ZST qSronlya;      /* "The database is read only; you may not add records." */
extern ZST qSronlye;      /* "The database is read only; you may not change records." */
extern ZST qSronlym;      /* "The database is read only; you may not merge records." */
extern ZST qSronlyr;      /* "The database is read only; you may not remove records." */
extern ZST qSronly;       /* "The database is read only." */

extern ZST qSidbrws;      /* "Name, key, refn, list, or @:" */
extern ZST qSidkyrfn;     /* "enter key or refn: " */
extern ZST qSidcrmv;      /* "Please choose the child to remove from the family." */
extern ZST qSidsrmv;      /* "Please choose the spouse/parent to remove from the family." */
extern ZST qSidcrmf;      /* "From which family is the child to be removed?" */
extern ZST qSidsrmf;      /* "From which family is the spouse/parent to be removed?" */
extern ZST qSidfrmv;      /* "Identify family (enter nothing to identify by individual members)." */
extern ZST qSidfrsp;      /* "Identify a spouse of the family, if known." */
extern ZST qSidfrch;      /* "Identify a child of the family, if known." */
extern ZST qSid1csw;      /* "Identify the first child to swap." */
extern ZST qSid2csw;      /* "Identify the second child to swap." */
extern ZST qSidcrdr;      /* "Identify the child to reorder." */
extern ZST qSid1fsw;      /* "Identify the first family/spouse to swap." */
extern ZST qSid2fsw;      /* "Identify the second family/spouse to swap." */
extern ZST qSidsbrs;      /* "Please choose the spouse/parent to browse to." */
extern ZST qSid1sbr;      /* "Please choose the first spouse/parent to browse to." */
extern ZST qSid2sbr;      /* "Please choose the second spouse/parent to browse to." */
extern ZST qSidcbrs;      /* "Please choose the child to browse to." */
extern ZST qSid1cbr;      /* "Please choose the first child to browse to." */
extern ZST qSid2cbr;      /* "Please choose the second child to browse to." */
extern ZST qSidfbrs;      /* "Please choose the family to browse to." */
extern ZST qSidfamk;      /* "Enter Family Number to Browse to" */
extern ZST qSid1fbr;      /* "Please choose the first family to browse to." */
extern ZST qSid2fbr;      /* "Please choose the second family to browse to." */
extern ZST qSidhbrs;      /* "Please choose the father/husband to browse to." */
extern ZST qSid1hbr;      /* "Please choose the first father/husband to browse to." */
extern ZST qSid2hbr;      /* "Please choose the second father/husband to browse to." */
extern ZST qSidwbrs;      /* "Please choose the mother/wife to browse to." */
extern ZST qSid1wbr;      /* "Please choose the first mother/wife to browse to." */
extern ZST qSid2wbr;      /* "Please choose the second mother/wife to browse to." */
extern ZST qSidcswp;      /* "Identify a parent in the family having children swapped." */
extern ZST qSidfswp;      /* "Whose families/spouses are to be swapped?" */
extern ZST qSidprnt;      /* "Identify one of the child's parents, if known." */

extern ZST qSscnrfn;      /* "Enter pattern to match against refn." */

extern ZST qSnotonei;     /* "Please choose from among these people." */
extern ZST qSnotonex;     /* "Please choose from among these records." */
extern ZST qSiscnew;      /* "Is this the new child? " */
extern ZST qSissnew;      /* "Is this the new spouse? " */
extern ZST qSifonei;      /* "Is this the person?  Select if so." */
extern ZST qSifonex;      /* "Is this the record?  Select if so." */
extern ZST qSidcfam;      /* "Select the child the new child precedes or select last." */
extern ZST qSidpnxt;      /* "Please identify next person to browse to." */
extern ZST qSidnxt;       /* "Please identify record to browse to." */
extern ZST qSidspse;      /* "Please identify one of the spouses." */
extern ZST qSidplst;      /* "Please identify person or persons to browse to." */
extern ZST qSidfcop;      /* "Please choose family create operation." */
extern ZST qSentnam;      /* "Do you want to enter another name?" */

extern ZST qSntprnt;      /* "This person is not a spouse or parent in any family." */
extern ZST qSntchld;      /* "This person is not a child in any family." */
extern ZST qSnonamky;     /* "There is no one in the database with that name or key." */

extern ZST qSnofath;      /* "This person's father is not in the database." */
extern ZST qSnomoth;      /* "This person's mother is not in the database." */
extern ZST qSnospse;      /* "This person has no spouse in the database." */
extern ZST qSnoysib;      /* "This person has no younger sibling in the database." */
extern ZST qSnoosib;      /* "This person has no older sibling in the database." */
extern ZST qSnoprnt;      /* "This person's parents are not in the database." */
extern ZST qSnohusb;      /* "This family has no male spouse/parent in the database." */
extern ZST qSnowife;      /* "This family has no female spouse/parent in the database." */
extern ZST qSnocinf;      /* "There are no children in the database for this family." */
extern ZST qSnocofp;      /* "This person has no children in the database." */
extern ZST qSnochil;      /* "No such child." */
extern ZST qSnopers;      /* "No such person." */
extern ZST qSnorec;       /* "No such record." */
extern ZST qSnofam;       /* "No such family." */
extern ZST qSnosour;      /* "No sources present." */
extern ZST qSidsour;      /* "Please choose the source to view." */
extern ZST qSnoeven;      /* "No events present." */
extern ZST qSideven;      /* "Please choose the event to view." */
extern ZST qSnoothe;      /* "No others present." */
extern ZST qSidothe;      /* "Please choose the other to view." */
extern ZST qSnonote;      /* "No notes present." */
extern ZST qSidnote;      /* "Please choose the note to view." */
extern ZST qSnoptr;       /* "No references present." */
extern ZST qSidptr;       /* "Please choose the reference to view." */
extern ZST qSduprfn;      /* "Duplicated REFN - please choose." */

extern ZST qSnosex;       /* "New spouse/parent has unknown sex; can't add to family." */
extern ZST qSnotopp;      /* "The persons are not of opposite sex; can't make family." */
extern ZST qSntsinf;      /* "This person is not a spouse in the family; can't remove." */
extern ZST qSntcinf;      /* "This person is not a child in the family; can't remove." */
extern ZST qSunksex;      /* "This person's sex is not known; can't make family." */
extern ZST qShashsb;      /* "This family already has a husband/male parent." */
extern ZST qShaswif;      /* "This family already has a wife/female parent." */
extern ZST qShasbth;      /* "This family has both spouses/parents; can't add another." */
extern ZST qShasnei;      /* "This family has neither spouse/parent; can't remove." */
extern ZST qShaslnk;      /* "This family still has links; not removed." */

extern ZST qSidfbys;      /* "Choose a family by selecting a spouse/parent." */
extern ZST qSiredit;      /* "Do you want to edit the person again? (Otherwise changes will be discarded.)" */
extern ZST qSireditopt;   /* "Do you want to edit the person again?" */
extern ZST qSfredit;      /* "Do you want to edit the family again? (Otherwise changes will be discarded.)" */
extern ZST qSfreditopt;   /* "Do you want to edit the family again?" */
extern ZST qSrredit;      /* "Do you want to edit the source again? (Otherwise changes will be discarded.)" */
extern ZST qSrreditopt;   /* "Do you want to edit the source again?" */
extern ZST qSeredit;      /* "Do you want to edit the event again? (Otherwise changes will be discarded.)" */
extern ZST qSereditopt;   /* "Do you want to edit the event again?" */
extern ZST qSxredit;      /* "Do you want to edit the record again? (Otherwise changes will be discarded.)" */
extern ZST qSxreditopt;   /* "Do you want to edit the record again?" */
extern ZST qSidpedt;      /* "Whom do you want to edit?" */
extern ZST qSidredt;      /* "Which source record do you want to edit?" */
extern ZST qSideedt;      /* "Which event record do you want to edit?" */
extern ZST qSidxedt;      /* "What record do you want to edit?" */

extern ZST qScfpadd;      /* "Do you really want to add this person to the database?" */
extern ZST qScffadd;      /* "Do you really want to add this family to the database?" */
extern ZST qScfcadd;      /* "Do you really want to add this child to the family?" */
extern ZST qScfsadd;      /* "Do you really want to add this spouse/parent to the family?" */
extern ZST qScfradd;      /* "Do you really want to add this source to the database?" */
extern ZST qScfeadd;      /* "Do you really want to add this event to the database?" */
extern ZST qScfxadd;      /* "Do you really want to add this record to the database?" */
extern ZST qScfpupt;      /* "Do you really want to update this person?" */
extern ZST qScffupt;      /* "Do you really want to update this family?" */
extern ZST qScfrupt;      /* "Do you really want to update this source?" */
extern ZST qScfeupt;      /* "Do you really want to update this event?" */
extern ZST qScfxupt;      /* "Do you really want to update this record?" */
extern ZST qScfpdel;      /* "Are you sure you want to remove the person from the database?" */
extern ZST qScfodel;      /* "Are you sure you want to remove this record from the database?" */
extern ZST qScffdel;      /* "Remove this family record?" */
extern ZST qScffdeld;     /* "(Family %s (%s, %s)" */
extern ZST qScfpmrg;      /* "Do you really want to merge these two persons?" */
extern ZST qScffmrg;      /* "Do you really want to merge these two families?" */
extern ZST qScffswp;      /* "Do you really want to swap spouse orders?" */
extern ZST qScfchswp;     /* "Do you really want to reorder children?" */
extern ZST qScfcrmv;      /* "Do you really want to remove this child from his/her family?" */
extern ZST qScfsrmv;      /* "Do you really want to remove this spouse from his/her family?" */
extern ZST qSspover;      /* "Too many spouses to display full list" */
extern ZST qSmgsfam;      /* "These persons are children in different families." */
extern ZST qSmgconf;      /* "Are you sure you want to merge them?" */

extern ZST qSbadata;      /* "There is something wrong with the data." */
extern ZST qSidchld;      /* "Please identify the child." */
extern ZST qSidsbln;      /* "Please identify one of the child's siblings." */
extern ZST qSidsadd;      /* "Identify spouse/parent to add to an existing family." */
extern ZST qSidsinf;      /* "Identify spouse/parent already in family, if known." */
extern ZST qSkchild;      /* "Identify child already in family." */
extern ZST qSiscinf;      /* "This person is already a child in a family. Add anyway?" */
extern ZST qSidsps1;      /* "Identify a spouse/parent for the new family." */
extern ZST qSidsps2;      /* "Identify the second spouse/parent, if known." */
extern ZST qStwohsb;      /* "Both families must have husbands/fathers." */
extern ZST qStwowif;      /* "Both families must have wives/mothers." */

extern ZST qSids2fm;      /* "Identify spouse/parent in second family, if known." */
extern ZST qSidc2fm;      /* "Identify child in second family." */
extern ZST qSidp2br;      /* "Identify second person to browse to." */

extern ZST qScrtcfm;      /* "Create a family with this person as a child." */
extern ZST qScrtsfm;      /* "Create a family with this person as a spouse/parent." */
extern ZST qSless2c;      /* "This family has less than two children; can't swap." */
extern ZST qSless2f;      /* "This person is a spouse/parent in less than two families." */
extern ZST qSparadox;     /* "Something impossible happened. Contact tech support." */

extern ZST qSokcswp;      /* "The two children were swapped." */
extern ZST qSokfswp;      /* "The two families were swapped." */
extern ZST qSokcrmv;      /* "The child was removed from his/her family." */
extern ZST qSoksrmv;      /* "The spouse was removed from his/her family." */

extern ZST qSnopmrg;      /* "A person cannot be merged with him/herself." */
extern ZST qSnofmrg;      /* "A family cannot be merged with itself." */
extern ZST qSnoqmrg;      /* "Two persons with different parents cannot be merged." */
extern ZST qSnoxmrg;      /* "Two parents of different sexes cannot be merged." */
extern ZST qSdhusb;       /* "The families have different fathers/husbands; cannot merge." */
extern ZST qSdwife;       /* "The families have different wives/mothers; cannot merge." */
extern ZST qSidpdel;      /* "Who do you want to remove from the database?" */
extern ZST qSidodel;      /* "What record do you want to remove from the database?" */
extern ZST qSmklast;      /* "Place the child last in the family." */
extern ZST qSabverr;      /* "Error in abbreviations file." */
extern ZST qSuoperr;      /* "Error in user options file." */
extern ZST qScmperr;      /* "Error in character mapping file." */
extern ZST qSsepch;       /* "(Separator is %s)" */
extern ZST qSaredit;      /* "Do you want to re-edit it?" */

extern ZST qSgdpadd;      /* "%s was added to the database." */
extern ZST qSgdcadd;      /* "%s was added as a child." */
extern ZST qSgdsadd;      /* "%s was added as a spouse and/or parent." */
extern ZST qSgdfadd;      /* "The new family was added to the database." */
extern ZST qSgdpmod;      /* "%s was modified in the database." */
extern ZST qSgdfmod;      /* "The family was modified in the database." */
extern ZST qSgdrmod;      /* "The source was modified in the database." */
extern ZST qSgdemod;      /* "The event was modified in the database." */
extern ZST qSgdxmod;      /* "The record was modified in the database." */

extern ZST qSnofopn;      /* "Could not open file %s." */
extern ZST qSfn2long;     /* "Filepath too long." */

extern ZST qSmrkrec;      /* "Please mark a record first." */

extern ZST qSlstnam;      /* "The current list is now named %s." */
extern ZST qSlstnon;      /* "The current list is not named." */
extern ZST qSlstwht;      /* "What should the name of this list be?" */
extern ZST qSlstnad;      /* "No persons were added to the current list." */
extern ZST qSlstpad;      /* "What persons or list do you want to add to the current list?" */
extern ZST qSlstbot;      /* "You are at the bottom of the list." */
extern ZST qSlsttop;      /* "You are at the top of the list." */
extern ZST qSlstnew;      /* "New persons were added to the current list." */

extern ZST qSbadttnum;    /* "System error: illegal map code" */
extern ZST qSnosuchtt;    /* "No such translation table in this database" */

/* GEDCOM file */
extern ZST qSgdnadd;      /* "Because of errors the GEDCOM file was not loaded.\n" */
extern ZST qSdboldk;      /* "No errors; adding records with original keys..." */
extern ZST qSdbnewk;      /* "No errors; adding records with new keys..." */
extern ZST qScfoldk;      /* "Use original keys from GEDCOM file?" */
extern ZST qSproceed;     /* "Proceed?" */
extern ZST qSoutarc;      /* "Enter name of output archive file." */
extern ZST qSoutfin;      /* "Database `%s' has been saved in `%s'." */
extern ZST qSmouttt;      /* "Enter name of translation table file to write" */
extern ZST qSmintt;       /* "Enter name of translation table file to read" */
extern ZST qSmisixr;      /* "Line %d: The person defined here has no key: skipped." */
extern ZST qSmisfxr;      /* "Line %d: The family defined here has no key." */
extern ZST qSmulper;      /* "Lines %d and %d: Person %s is multiply defined: skipped." */
extern ZST qSmulfam;      /* "Lines %d and %d: Family %s is multiply defined." */
extern ZST qSmatper;      /* "Line %d: Person %s has an incorrect key: skipped." */
extern ZST qSmatfam;      /* "Line %d: Family %s has an incorrect key." */
extern ZST qSundper;      /* "Person %s is referred to but not defined." */
extern ZST qSundfam;      /* "Family %s is referred to but not defined." */
extern ZST qSundsrc;      /* "Source %s is referred to but not defined." */
extern ZST qSundevn;      /* "Event %s is referred to but not defined." */
extern ZST qSbadlev;      /* "Line %d: This line has a level number that is too large." */
extern ZST qSnoname;      /* "Line %d: Person defined here has no name." */
#if 0
STRING noxref = SS "Line %d: This record has no cross reference value.";
#endif

	/* Option at bottom of list, if none in list are desired */
extern ZST qSextchoos;     /* "<Choose outside this list>" */
	/* What report program to run ? */
extern ZST qSwhatrpt;     /* "What is the name of the program?" */
extern ZST qSwhatgedc;    /* "Please enter the name of the GEDCOM file." */

extern ZST qSwhtout;      /* "What is the name of the output file?" */
extern ZST qSopt2long;    /* "Malformed configuration file: line too long." */
extern ZST qSunsupunix;   /* "Unsupported file encoding (no multibyte encodings except UTF-8)." */
extern ZST qSunsupuniv;    /* "Unsupported file encoding: %s." */

/* misc prompts */
extern ZST qSchoostrttl;  /* "Enter string for program" */
extern ZST qSaskstr;      /* "enter string: " */
extern ZST qSaskint;      /* "enter integer:" */
extern ZST qSasknam;      /* "enter name: " */
extern ZST qShitkey;      /* "Strike any key to continue." */

/* new records prototypes */
	/* (all-caps words are in GEDCOM language -- don't change) */
extern ZST qSdefsour;     /* "0 SOUR\n1 REFN\n1 TITL Title\n1 AUTH Author" */
extern ZST qSdefeven;     /* "0 EVEN\n1 REFN\n1 DATE\n1 PLAC\n1 INDI\n  2 NAME\n  2 ROLE\n1 SOUR" */
extern ZST qSdefothr;     /* "0 XXXX\n1 REFN" */
/* end new record prototypes */

/* node.c errors */
extern ZST qSfileof;      /* "The file is as positioned at EOF." */
extern ZST qSreremp;      /* "Line %d: This line is empty; EOF?" */
extern ZST qSrerlng;      /* "Line %d: This line is too long." */
extern ZST qSrernlv;      /* "Line %d: This line has no level number." */
extern ZST qSrerinc;      /* "Line %d: This line is incomplete." */
extern ZST qSrerbln;      /* "Line %d: This line has a bad link." */
extern ZST qSrernwt;      /* "Line %d: This line needs white space before tag." */
extern ZST qSrerilv;      /* "Line %d: This line has an illegal level." */
extern ZST qSrerwlv;      /* "The record begins at wrong level." */

/* &&begin signals */
extern ZST qScoredump;    /* "\nAborting now. Core dump? [y/n]" */
extern ZST qSprogsig;     /* "Looks like a program was running.\nCheck file %1 around line %2.\n" */
extern ZST qSsignal;      /* "signal %1: %2" */
	/* system signal#0 name */
extern ZST qSsig00;       /* "SIGNAL 0" */
extern ZST qSsig01;       /* "HANGUP" */
extern ZST qSsig02;       /* "INTERRUPT" */
extern ZST qSsig03;       /* "QUIT" */
extern ZST qSsig04;       /* "ILLEGAL INSTRUCTION" */
extern ZST qSsig05;       /* "TRACE TRAP" */
extern ZST qSsig06;       /* "ABORT" */
extern ZST qSsig07;       /* "EMT INST" */
extern ZST qSsig08;       /* "FLOATING POINT EXCEPTION" */
extern ZST qSsig09;       /* "KILL" */
extern ZST qSsig10;       /* "BUS ERROR" */
extern ZST qSsig11;       /* "SEGMENTATION ERROR" */
extern ZST qSsig12;       /* "SYSTEM CALL ERROR" */
extern ZST qSsig13;       /* "PIPE WRITE" */
extern ZST qSsig14;       /* "ALARM CLOCK" */
	/* system signal#15 name -- presumably user used UNIX kill command to stop lifelines */
extern ZST qSsig15;       /* "TERMINATE FROM KILL" */
extern ZST qSsig16;       /* "USER SIGNAL 1" */
extern ZST qSsig17;       /* "USER SIGNAL 2" */
extern ZST qSsig18;       /* "DEATH OF CHILD" */
extern ZST qSsig19;       /* "POWER-FAIL RESTART" */
extern ZST qSsig20;       /* "WINDOW CHANGE" */
extern ZST qSsigunk;      /* "Unknown signal" */
/* &&end signals */

/* browse display stuff */
extern ZST qSdspl_indi;   /* "person" */
extern ZST qSdspa_resi;   /* ", of " */
extern ZST qSdspl_fath;   /* "father" */
extern ZST qSdspl_moth;   /* "mother" */
extern ZST qSdspl_spouse; /* "spouse" */
extern ZST qSdspl_child;  /* "child" */
/* &&begin display abbreviations */
	/* m.: married */
extern ZST qSdspa_mar;    /* "m. " */
	/* eng. : engaged */
extern ZST qSdspa_eng;    /* "eng. " */
	/* mc. : marriage contract */
extern ZST qSdspa_marc;   /* "m.c. " */

	/* div.: divorced */
extern ZST qSdspa_div;    /* "div. " */
	/* b.: born */
extern ZST qSdspa_bir;    /* "b. " */
	/* bap.: baptized */
extern ZST qSdspa_chr;    /* "bap. " */
	/* d.: died */
extern ZST qSdspa_dea;    /* "d. " */
	/* bur.: buried */
extern ZST qSdspa_bur;    /* "bur. " */
	/* cb.: child born */
extern ZST qSdspa_chbr;   /* "cb. " */
/* &&end display abbreviations, begin long forms */
extern ZST qSdspl_mar;    /* "married: " */
extern ZST qSdspl_eng;    /* "engaged: " */
extern ZST qSdspl_marc;   /* "marr.ct: " */
extern ZST qSdspl_bir;    /* "born: " */
extern ZST qSdspl_chr;    /* "bapt: " */
extern ZST qSdspl_dea;    /* "died: " */
extern ZST qSdspl_bur;    /* "buri: " */
/* &&end display long forms */

/* editing errors */
extern ZST qSbadind;      /* "You cannot edit the INDI line in a person record." */
extern ZST qSbadfmc;      /* "You cannot edit the FAMC line in a person record." */
extern ZST qSbadfms;      /* "You cannot edit the FAMS lines in a person record." */
extern ZST qSbadfam;      /* "You cannot edit the FAM line in a family record." */
extern ZST qSbadhsb;      /* "You cannot edit the HUSB line in a family record." */
extern ZST qSbadwif;      /* "You cannot edit the WIFE line in a family record." */
extern ZST qSbadchl;      /* "You cannot edit the CHIL lines in a family record." */
extern ZST qSbademp;      /* "The record is empty." */
extern ZST qSbadin0;      /* "The record does not begin with an INDI line." */
extern ZST qSbadfm0;      /* "The record does not begin with a FAM line." */
extern ZST qSbadsr0;      /* "The record does not begin with a SOUR line." */
extern ZST qSbadev0;      /* "The record does not begin with an EVEN line." */
extern ZST qSbadothr0;    /* "INDI, FAM, SOUR, EVEN records may not be other records." */
extern ZST qSbadmul;      /* "The record contains multiple level 0 lines." */
extern ZST qSbadenm;      /* "This person record has bad GEDCOM name syntax." */
extern ZST qSbadparsex;   /* "You cannot change the sex of a parent." */
extern ZST qSbadirefn;    /* "REFN key is already in use." */

extern ZST qStag2lng2cnc; /* "Tag is too long to connect automatically." */
	/* I,F,S,E,X are conventional letters, so leave them as is) */
extern ZST qSdbrecords;   /* "Database records" */
extern ZST qSdbrecstats;  /* FMT_INT "I, " FMT_INT "F, " FMT_INT "S," FMT_INT "E, " FMT_INT "X" */

/* menus */
extern ZST qSmtitle;      /* "LifeLines %s - Genealogical DB and Programming System" */
extern ZST qScright;      /* "Copyright(c) 1991 to 1996, by T. T. Wetmore IV" */
extern ZST qSdbname;      /* "Current Database - %s" */
extern ZST qSplschs;      /* "Please choose an operation:" */
extern ZST qSmn_unkcmd;   /* "Not valid command" */

/* prompt, full list, yes list */
extern ZST qSaskynq;      /* "enter y (yes) or n (no): " */
extern ZST qSaskynyn;     /* "yYnN" */ /* valid chars for yes/no answer */
extern ZST qSaskyY;       /* "yY" */ /* chars meaning yes answer */

/* browse menu titles */
extern ZST qSttlindibrw;  /* "LifeLines -- Person Browse Screen (* toggles menu)" */
extern ZST qSttlfambrw;   /* "LifeLines -- Family Browse Screen (* toggles menu)" */
extern ZST qSttl2perbrw;  /* "LifeLines -- Two Person Browse Screen (* toggles menu)" */
extern ZST qSttl2fambrw;  /* "LifeLines -- Two Family Browse Screen (* toggles menu)" */
extern ZST qSttlauxbrw;   /* "LifeLines -- Auxiliary Browse Screen (* toggles menu)" */
extern ZST qSttllstbrw;   /* "LifeLines -- List Browse Screen (* toggles menu)" */

/* list menu */

extern ZST qSchlistx;     /* "Commands:   Select by number, u Page Up, d Page Down, q Quit" */
extern ZST qSvwlistx;     /* "Commands:   u Page Up, d Page Down, q Quit" */
extern ZST qSerrlist;     /* "Messages:" */

/* adding new xref */
extern ZST qSdefttl;      /* "Please choose from the following options:" */
extern ZST qSnewrecis;    /* "New record is %s" */
extern ZST qSautoxref;    /* "Insert xref automatically at bottom of current record." */
extern ZST qSeditcur;     /* "Edit current record now to add xref manually." */
extern ZST qSgotonew;     /* "Browse new record (without adding xref)." */
extern ZST qSstaycur;     /* "Return to current record (without adding xref)." */

/* misc */
extern ZST qSunksps;      /* "Spouse unknown" */
extern ZST qSnohist;      /* "No more history" */
extern ZST qSbadhistcnt;  /* "Bad history count" */
extern ZST qSbadhistcnt2; /* "Bad backup history count" */
extern ZST qSbadhistlen;  /* "Bad history length" */
extern ZST qShistclr;     /* "Delete history (%d entries)?" */
extern ZST qSdataerr;     /* "Error accessing data" */
extern ZST qSidhist;      /* "Choose from history" */
extern ZST qSnofandl;     /* "Cannot combine forceopen (-f) and lock (-l) flags." */
extern ZST qSiddefpath;   /* "Default path: " */
extern ZST qSmisskeys;    /* "WARNING: missing keys" */
extern ZST qSbadkeyptr;   /* "This does not point to another record in the database!" */
extern ZST qSwhtfname;    /* "enter file name" */
extern ZST qSwhtfnameext; /* "enter file name (*%s)" */
extern ZST qSnosuchrec;   /* "There is no record with that key or reference." */
extern ZST qSbaddb;       /* "Database was corrupt." */

/* translation table errors */
extern ZST qSbaddec;      /* "Bad decimal number format." */
extern ZST qSbadhex;      /* "Bad hexidecimal number format." */
extern ZST qSnorplc;      /* "No replacement string on line." */
extern ZST qSnoorig;      /* "No original string on line." */
extern ZST qSbadesc;      /* "Bad escape format." */
extern ZST qSmaperr;      /* "%s: line %d (entry %d): %s" */

/* many menus */
extern ZST qSmn_quit;     /* "q  Return to main menu" */
extern ZST qSmn_ret;      /* "q  Return to previous menu" */
extern ZST qSmn_exit;     /* "q  Quit program" */
extern ZST qSmn_changedb; /* "Q  Quit current database" */

/* &&begin main menu (70 chars after spaces) */
extern ZST qSmn_mmbrws;   /* "b  Browse the persons in the database" */
extern ZST qSmn_mmsear;   /* "s  Search database" */
extern ZST qSmn_mmadd;    /* "a  Add information to the database" */
extern ZST qSmn_mmdel;    /* "d  Delete information from the database" */
extern ZST qSmn_mmrpt;    /* "r  Generate report by entering report name" */
extern ZST qSmn_mmprpt;   /* "p  Pick a report from list and run" */
extern ZST qSmn_mmcset;   /* "c  Character set options" */
extern ZST qSmn_mmtt;     /* "t  Modify character translation tables" */
extern ZST qSmn_mmut;     /* "u  Miscellaneous utilities" */
extern ZST qSmn_mmex;     /* "x  Handle source, event and other records" */

/* &&end main menu, begin utility menu */
extern ZST qSmn_uttl;     /* "What utility do you want to perform?" */
extern ZST qSmn_utsave;   /* "s  Save the database in a GEDCOM file" */
extern ZST qSmn_utread;   /* "r  Read in data from a GEDCOM file" */
extern ZST qSmn_utgdchoo; /* "R  Pick a GEDCOM file and read in" */
extern ZST qSmn_utkey;    /* "k  Find a person's key value" */
extern ZST qSmn_utkpers;  /* "i  Identify a person from key value" */
extern ZST qSmn_utdbstat; /* "d  Show database statistics" */
extern ZST qSmn_utmemsta; /* "m  Show memory statistics" */
extern ZST qSmn_utplaces; /* "e  Edit the place abbreviation file" */
extern ZST qSmn_utusropt; /* "o  Edit the user options file" */

/* &&end utility menu, begin extra menu */
extern ZST qSmn_xttl;     /* "What activity do you want to perform?" */
extern ZST qSmn_xxbsour;  /* "s  Browse source records" */
extern ZST qSmn_xxbeven;  /* "e  Browse event records" */
extern ZST qSmn_xxbothr;  /* "x  Browse other records" */
extern ZST qSmn_xxasour;  /* "1  Add a source record to the database" */
extern ZST qSmn_xxesour;  /* "2  Edit source record from the database" */
extern ZST qSmn_xxaeven;  /* "3  Add an event record to the database" */
extern ZST qSmn_xxeeven;  /* "4  Edit event record from the database" */
extern ZST qSmn_xxaothr;  /* "5  Add an other record to the database" */
extern ZST qSmn_xxeothr;  /* "6  Edit other record from the database" */

/* &&end utility menu, &&begin translation table menu */
extern ZST qSmn_tt_ttl;   /* "Translation Tables" */
extern ZST qSmn_tt_edit;  /* "e  edit individual tables (in db)" */
extern ZST qSmn_tt_load;  /* "l  load a table from a file (into db)" */
extern ZST qSmn_tt_save;  /* "s  save a table to a file (from db)" */
extern ZST qSmn_tt_exp;   /* "x  export all tables (from db to files)" */
extern ZST qSmn_tt_imp;   /* "i  import all tables (from files into db)" */
extern ZST qSmn_tt_dir;   /* "export/import directory:" */

/* menus for translation tables */
extern ZST qSmn_edttttl;  /* "Which character mapping do you want to edit?" */
extern ZST qSmn_svttttl;  /* "Which character mapping do you want to save?" */

/* not yet implemented choices */
extern ZST qSmn_notimpl;  /* "Not implemented yet" */

/* &&begin add menu */
extern ZST qSmn_add_ttl;  /* "What do you want to add?" */
extern ZST qSmn_add_indi; /* "p  Person - add new person to the database" */
extern ZST qSmn_add_fam;  /* "f  Family - create family record from one or two spouses" */
extern ZST qSmn_add_chil; /* "c  Child - add a child to an existing family" */
extern ZST qSmn_add_spou; /* "s  Spouse - add a spouse to an existing family" */

/* &&end add menu, begin delete menu */
extern ZST qSmn_del_ttl;  /* "What do you want to remove?" */
extern ZST qSmn_del_chil; /* "c  Child - remove a child from his/her family" */
extern ZST qSmn_del_spou; /* "s  Spouse - remove a spouse from a family" */
extern ZST qSmn_del_indi; /* "i  Individual - remove a person completely" */
extern ZST qSmn_del_fam;  /* "f  Family - remove a family completely" */
extern ZST qSmn_del_any;  /* "o  Other - remove other record completely" */

/* &&end delete menu, begin search menu */
extern ZST qSmn_sea_ttl;  /* "How would you like to find a record?" */

/* &&end search menu, begin scan status strings */
extern ZST qSsts_sca_ful; /* "Performing full name scan" */
extern ZST qSsts_sca_fra; /* "Performing name fragment scan" */
extern ZST qSsts_sca_ref; /* "Performing refn scan" */
extern ZST qSsts_sca_src; /* "Performing source scan" */
extern ZST qSsts_sca_non; /* "No records found in scan" */

/* &&complex date strings (A=abbrev, B=full)*/
extern ZST qSdatea_abtA;  /* "abt %1" */
extern ZST qSdatea_abtB;  /* "about %1" */
extern ZST qSdatea_estA;  /* "est %1" */
extern ZST qSdatea_estB;  /* "estimated %1" */
extern ZST qSdatea_calA;  /* "cal %1" */
extern ZST qSdatea_calB;  /* "calculated %1" */
extern ZST qSdatep_fromA; /* "fr %1" */
extern ZST qSdatep_fromB; /* "from %1" */
extern ZST qSdatep_toA;   /* "to %1" */
extern ZST qSdatep_toB;   /* "to %1" */
extern ZST qSdatep_frtoA; /* "fr %1 to %2" */
extern ZST qSdatep_frtoB; /* "from %1 to %2" */
extern ZST qSdater_befA;  /* "bef %1" */
extern ZST qSdater_befB;  /* "before %1" */
extern ZST qSdater_aftA;  /* "aft %1" */
extern ZST qSdater_aftB;  /* "after %1" */
extern ZST qSdater_betA;  /* "bet %1 and %2" */
extern ZST qSdater_betB;  /* "between %1 and %2" */
	/* &&origin/era trailers */
	/* B.C. = Before Christ (calendar) */
extern ZST qSdatetrl_bcA; /* "B.C." */
	/* BC = Before Christ (calendar) */
extern ZST qSdatetrl_bcB; /* "BC" */
	/* B.C.E. = Before Common Era (calendar) */
extern ZST qSdatetrl_bcC; /* "B.C.E." */
	/* BCE. = Before Common Era (calendar) */
extern ZST qSdatetrl_bcD; /* "BCE" */
	/* A.D. = Anno Domini (calendar) */
extern ZST qSdatetrl_adA; /* "A.D." */
	/* AD = Anno Domini (calendar) */
extern ZST qSdatetrl_adB; /* "AD" */
	/* C.E. = Common Era (calendar) */
extern ZST qSdatetrl_adC; /* "C.E." */
	/* CE = Common Era (calendar) */
extern ZST qSdatetrl_adD; /* "CE" */
	/* &&calendar pics */
	/* Julian calendar year */
extern ZST qScaljul;      /* "%1J" */
	/* Hebrew calendar year */
extern ZST qScalheb;      /* "%1 HEB" */
	/* French Republic calendar year */
extern ZST qScalfr;       /* "%1 FR" */
	/* Roman calendar year -- Anno Urbe Condite ? */
extern ZST qScalrom;      /* "%1 AUC" */
	/* &&Gregorian/Julian months */
extern ZST qSmon_gj1A;    /* "jan" */
extern ZST qSmon_gj1B;    /* "january" */
extern ZST qSmon_gj2A;    /* "feb" */
extern ZST qSmon_gj2B;    /* "february" */
extern ZST qSmon_gj3A;    /* "mar" */
extern ZST qSmon_gj3B;    /* "march" */
extern ZST qSmon_gj4A;    /* "apr" */
extern ZST qSmon_gj4B;    /* "april" */
/* Put short form for may (don't use **) */
extern ZST qSmon_gj5A;    /* "**may" */
extern ZST qSmon_gj5B;    /* "may" */
extern ZST qSmon_gj6A;    /* "jun" */
extern ZST qSmon_gj6B;    /* "june" */
extern ZST qSmon_gj7A;    /* "jul" */
extern ZST qSmon_gj7B;    /* "july" */
extern ZST qSmon_gj8A;    /* "aug" */
extern ZST qSmon_gj8B;    /* "august" */
extern ZST qSmon_gj9A;    /* "sep" */
extern ZST qSmon_gj9B;    /* "september" */
extern ZST qSmon_gj10A;   /* "oct" */
extern ZST qSmon_gj10B;   /* "october" */
extern ZST qSmon_gj11A;   /* "nov" */
extern ZST qSmon_gj11B;   /* "november" */
extern ZST qSmon_gj12A;   /* "dec" */
extern ZST qSmon_gj12B;   /* "december" */
	/* &&Hebrew months */
extern ZST qSmon_heb1A;   /* "tsh" */
extern ZST qSmon_heb1B;   /* "tishri" */
extern ZST qSmon_heb2A;   /* "csh" */
extern ZST qSmon_heb2B;   /* "cheshvan" */
extern ZST qSmon_heb3A;   /* "ksl" */
extern ZST qSmon_heb3B;   /* "kislev" */
extern ZST qSmon_heb4A;   /* "tvt" */
extern ZST qSmon_heb4B;   /* "tevet" */
extern ZST qSmon_heb5A;   /* "shv" */
extern ZST qSmon_heb5B;   /* "shevat" */
extern ZST qSmon_heb6A;   /* "adr" */
extern ZST qSmon_heb6B;   /* "adar" */
extern ZST qSmon_heb7A;   /* "ads" */
extern ZST qSmon_heb7B;   /* "adar sheni" */
extern ZST qSmon_heb8A;   /* "nsn" */
extern ZST qSmon_heb8B;   /* "nisan" */
extern ZST qSmon_heb9A;   /* "iyr" */
extern ZST qSmon_heb9B;   /* "iyar" */
extern ZST qSmon_heb10A;  /* "svn" */
extern ZST qSmon_heb10B;  /* "sivan" */
extern ZST qSmon_heb11A;  /* "tmz" */
extern ZST qSmon_heb11B;  /* "tamuz" */
extern ZST qSmon_heb12A;  /* "aav" */
extern ZST qSmon_heb12B;  /* "av" */
extern ZST qSmon_heb13A;  /* "ell" */
extern ZST qSmon_heb13B;  /* "elul" */
	/* &&French Republic months */
extern ZST qSmon_fr1A;    /* "vend" */
extern ZST qSmon_fr1B;    /* "vendemiaire" */
extern ZST qSmon_fr2A;    /* "brum" */
extern ZST qSmon_fr2B;    /* "brumaire" */
extern ZST qSmon_fr3A;    /* "frim" */
extern ZST qSmon_fr3B;    /* "frimaire" */
extern ZST qSmon_fr4A;    /* "nivo" */
extern ZST qSmon_fr4B;    /* "nivose" */
extern ZST qSmon_fr5A;    /* "pluv" */
extern ZST qSmon_fr5B;    /* "pluviose" */
extern ZST qSmon_fr6A;    /* "vent" */
extern ZST qSmon_fr6B;    /* "ventose" */
extern ZST qSmon_fr7A;    /* "germ" */
extern ZST qSmon_fr7B;    /* "germinal" */
extern ZST qSmon_fr8A;    /* "flor" */
extern ZST qSmon_fr8B;    /* "floreal" */
extern ZST qSmon_fr9A;    /* "prai" */
extern ZST qSmon_fr9B;    /* "prairial" */
extern ZST qSmon_fr10A;   /* "mess" */
extern ZST qSmon_fr10B;   /* "messidor" */
extern ZST qSmon_fr11A;   /* "ther" */
extern ZST qSmon_fr11B;   /* "thermidor" */
extern ZST qSmon_fr12A;   /* "fruc" */
extern ZST qSmon_fr12B;   /* "fructidor" */
extern ZST qSmon_fr13A;   /* "comp" */
extern ZST qSmon_fr13B;   /* "jour_complementairs" */

/* Copyright, warranty, contact information, etc. */

extern ZST qSgen_copyright; /* "Copyright (C) 1991-2019 Thomas T. Wetmore IV and contributors." */
extern ZST qSgen_warranty;  /* "This is free software; see the source for copying conditions.  There is NO\n"
                               "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." */
extern ZST qSgen_license;   /* "Lifelines is distributed under an X/MIT Open Source license.\n"
                               "\tSee file LICENSE in the program directory." */
extern ZST qSgen_website;   /* "For more information, visit our web site at https://lifelines.github.io/lifelines/" */
extern ZST qSgen_bugreport; /* "Please report bugs to https://github.com/lifelines/lifelines/issues" */
