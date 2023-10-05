//
//  DeadEnds
//
//  functable.c -- Table of the built-in functions in the DeadEnds programming language.
//
//  Created by Thomas Wetmore on 10 January 2023.
//  Last changed on 17 June 2023.
//

#include "standard.h"
#include "symboltable.h"
#include "gedcom.h"
#include "interp.h"

extern PValue __add(PNode*, SymbolTable*, bool*);
extern PValue __addnode(PNode*, SymbolTable*, bool*);
extern PValue __addtoset(PNode*, SymbolTable*, bool*);
extern PValue __alpha(PNode*, SymbolTable*, bool*);
extern PValue __ancestorset(PNode*, SymbolTable*, bool*);
extern PValue __and(PNode*, SymbolTable*, bool*);
extern PValue __baptism(PNode*, SymbolTable*, bool*);
extern PValue __birth(PNode*, SymbolTable*, bool*);
extern PValue __burial(PNode*, SymbolTable*, bool*);
extern PValue __capitalize(PNode*, SymbolTable*, bool*);
extern PValue __card(PNode*, SymbolTable*, bool*);
extern PValue __child(PNode*, SymbolTable*, bool*);
//extern PValue __children(PNode*, SymbolTable*, bool*);  // NEW TO DEADENDS.
extern PValue __childset(PNode*, SymbolTable*, bool*);
extern PValue __choosechild(PNode*, SymbolTable*, bool*);
extern PValue __choosefam(PNode*, SymbolTable*, bool*);
extern PValue __chooseindi(PNode*, SymbolTable*, bool*);
extern PValue __choosespouse(PNode*, SymbolTable*, bool*);
extern PValue __choosesubset(PNode*, SymbolTable*, bool*);
extern PValue __col(PNode*, SymbolTable*, bool*);
extern PValue __concat(PNode*, SymbolTable*, bool*);
extern PValue __copyfile(PNode*, SymbolTable*, bool*);
extern PValue __createnode(PNode*, SymbolTable*, bool*);
extern PValue __d(PNode*, SymbolTable*, bool*);
extern PValue __database(PNode*, SymbolTable*, bool*);
extern PValue __date(PNode*, SymbolTable*, bool*);
extern PValue __dateformat(PNode*, SymbolTable*, bool*);
extern PValue __dayformat(PNode*, SymbolTable*, bool*);
extern PValue __death(PNode*, SymbolTable*, bool*);
extern PValue __decr(PNode*, SymbolTable*, bool*);
extern PValue __deletefromset(PNode*, SymbolTable*, bool*);
extern PValue __deletenode(PNode*, SymbolTable*, bool*);
extern PValue __dequeue(PNode*, SymbolTable*, bool*);
extern PValue __descendentset(PNode*, SymbolTable*, bool*);
extern PValue __difference(PNode*, SymbolTable*, bool*);
extern PValue __div(PNode*, SymbolTable*, bool*);
extern PValue __empty(PNode*, SymbolTable*, bool*);
extern PValue __eq(PNode*, SymbolTable*, bool*);
extern PValue __eqstr(PNode*, SymbolTable*, bool*);
extern PValue __exp(PNode*, SymbolTable*, bool*);
extern PValue __extractdate(PNode*, SymbolTable*, bool*);
extern PValue __extractnames(PNode*, SymbolTable*, bool*);
extern PValue __extractplaces(PNode*, SymbolTable*, bool*);
extern PValue __extracttokens(PNode*, SymbolTable*, bool*);
extern PValue __f(PNode*, SymbolTable*, bool*);
extern PValue __fam(PNode*, SymbolTable*, bool*);
extern PValue __father(PNode*, SymbolTable*, bool*);
extern PValue __female(PNode*, SymbolTable*, bool*);
extern PValue __firstchild(PNode*, SymbolTable*, bool*);
extern PValue __firstfam(PNode*, SymbolTable*, bool*);
extern PValue __firstindi(PNode*, SymbolTable*, bool*);
extern PValue __fnode(PNode*, SymbolTable*, bool*);
extern PValue __fullname(PNode*, SymbolTable*, bool*);
extern PValue __ge(PNode*, SymbolTable*, bool*);
extern PValue __gengedcom(PNode*, SymbolTable*, bool*);
extern PValue __genindiset(PNode*, SymbolTable*, bool*);
extern PValue __getel(PNode*, SymbolTable*, bool*);
extern PValue __getfam(PNode*, SymbolTable*, bool*);
extern PValue __getindi(PNode*, SymbolTable*, bool*);
extern PValue __getindiset(PNode*, SymbolTable*, bool*);
extern PValue __getint(PNode*, SymbolTable*, bool*);
extern PValue __getrecord(PNode*, SymbolTable*, bool*);
extern PValue __getstr(PNode*, SymbolTable*, bool*);
extern PValue __gettoday(PNode*, SymbolTable*, bool*);
extern PValue __givens(PNode*, SymbolTable*, bool*);
extern PValue __gt(PNode*, SymbolTable*, bool*);
extern PValue __husband(PNode*, SymbolTable*, bool*);
extern PValue __incr(PNode*, SymbolTable*, bool*);
extern PValue __index(PNode*, SymbolTable*, bool*);
extern PValue __indi(PNode*, SymbolTable*, bool*);
extern PValue __indiset(PNode*, SymbolTable*, bool*);
extern PValue __inode(PNode*, SymbolTable*, bool*);
extern PValue __insert(PNode*, SymbolTable*, bool*);
extern PValue __intersect(PNode*, SymbolTable*, bool*);
extern PValue __key(PNode*, SymbolTable*, bool*);
extern PValue __keysort(PNode*, SymbolTable*, bool*);
extern PValue __lastchild(PNode*, SymbolTable*, bool*);
extern PValue __le(PNode*, SymbolTable*, bool*);
extern PValue __length(PNode*, SymbolTable*, bool*);
extern PValue __lengthset(PNode*, SymbolTable*, bool*);
extern PValue __linemode(PNode*, SymbolTable*, bool*);
extern PValue __list(PNode*, SymbolTable*, bool*);
extern PValue __lock(PNode*, SymbolTable*, bool*);
extern PValue __long(PNode*, SymbolTable*, bool*);
extern PValue __lookup(PNode*, SymbolTable*, bool*);
extern PValue __lower(PNode*, SymbolTable*, bool*);
extern PValue __lt(PNode*, SymbolTable*, bool*);
extern PValue __male(PNode*, SymbolTable*, bool*);
extern PValue __marriage(PNode*, SymbolTable*, bool*);
extern PValue __menuchoose(PNode*, SymbolTable*, bool*);
extern PValue __mod(PNode*, SymbolTable*, bool*);
extern PValue __monthformat(PNode*, SymbolTable*, bool*);
extern PValue __mother(PNode*, SymbolTable*, bool*);
extern PValue __mul(PNode*, SymbolTable*, bool*);
extern PValue __name(PNode*, SymbolTable*, bool*);
extern PValue __namesort(PNode*, SymbolTable*, bool*);
extern PValue __nchildren(PNode*, SymbolTable*, bool*);
extern PValue __ne(PNode*, SymbolTable*, bool*);
extern PValue __neg(PNode*, SymbolTable*, bool*);
extern PValue __newfile(PNode*, SymbolTable*, bool*);
extern PValue __nextfam(PNode*, SymbolTable*, bool*);
extern PValue __nextindi(PNode*, SymbolTable*, bool*);
extern PValue __nextsib(PNode*, SymbolTable*, bool*);
extern PValue __nfamilies(PNode*, SymbolTable*, bool*);
extern PValue __nl(PNode*, SymbolTable*, bool*);
extern PValue __not(PNode*, SymbolTable*, bool*);
extern PValue __nspouses(PNode*, SymbolTable*, bool*);
extern PValue __or(PNode*, SymbolTable*, bool*);
extern PValue __ord(PNode*, SymbolTable*, bool*);
extern PValue __outfile(PNode*, SymbolTable*, bool*);
extern PValue __pagemode(PNode*, SymbolTable*, bool*);
extern PValue __pageout(PNode*, SymbolTable*, bool*);
extern PValue __parent(PNode*, SymbolTable*, bool*);
extern PValue __parents(PNode*, SymbolTable*, bool*);
extern PValue __parentset(PNode*, SymbolTable*, bool*);
extern PValue __place(PNode*, SymbolTable*, bool*);
extern PValue __pn(PNode*, SymbolTable*, bool*);
extern PValue __pop(PNode*, SymbolTable*, bool*);
extern PValue __pos(PNode*, SymbolTable*, bool*);
extern PValue __prevfam(PNode*, SymbolTable*, bool*);
extern PValue __previndi(PNode*, SymbolTable*, bool*);
extern PValue __prevsib(PNode*, SymbolTable*, bool*);
extern PValue __print(PNode*, SymbolTable*, bool*);
extern PValue __push(PNode*, SymbolTable*, bool*);
extern PValue __qt(PNode*, SymbolTable*, bool*);
extern PValue __reference(PNode*, SymbolTable*, bool*);
extern PValue __requeue(PNode*, SymbolTable*, bool*);
extern PValue __rjustify(PNode*, SymbolTable*, bool*);
extern PValue __roman(PNode*, SymbolTable*, bool*);
extern PValue __rot(PNode*, SymbolTable*, bool*);
extern PValue __row(PNode*, SymbolTable*, bool*);
extern PValue __save(PNode*, SymbolTable*, bool*);
extern PValue __savenode(PNode*, SymbolTable*, bool*);
extern PValue __set(PNode*, SymbolTable*, bool*);
extern PValue __setel(PNode*, SymbolTable*, bool*);
extern PValue __sex(PNode*, SymbolTable*, bool*);
extern PValue __short(PNode*, SymbolTable*, bool*);
extern PValue __sibling(PNode*, SymbolTable*, bool*);
extern PValue __siblingset(PNode*, SymbolTable*, bool*);
extern PValue __soundex(PNode*, SymbolTable*, bool*);
extern PValue __space(PNode*, SymbolTable*, bool*);
extern PValue __spouseset(PNode*, SymbolTable*, bool*);
extern PValue __stddate(PNode*, SymbolTable*, bool*);
extern PValue __strcmp(PNode*, SymbolTable*, bool*);
extern PValue __strlen(PNode*, SymbolTable*, bool*);
extern PValue __strsoundex(PNode*, SymbolTable*, bool*);
extern PValue __strtoint(PNode*, SymbolTable*, bool*);
extern PValue __sub(PNode*, SymbolTable*, bool*);
extern PValue __substring(PNode*, SymbolTable*, bool*);
extern PValue __surname(PNode*, SymbolTable*, bool*);
extern PValue __system(PNode*, SymbolTable*, bool*);
extern PValue __table(PNode*, SymbolTable*, bool*);
extern PValue __tag(PNode*, SymbolTable*, bool*);
extern PValue __title(PNode*, SymbolTable*, bool*);
extern PValue __trim(PNode*, SymbolTable*, bool*);
extern PValue __trimname(PNode*, SymbolTable*, bool*);
extern PValue __union(PNode*, SymbolTable*, bool*);
extern PValue __uniqueset(PNode*, SymbolTable*, bool*);
extern PValue __unlock(PNode*, SymbolTable*, bool*);
extern PValue __upper(PNode*, SymbolTable*, bool*);
extern PValue __value(PNode*, SymbolTable*, bool*);
extern PValue __valuesort(PNode*, SymbolTable*, bool*);
extern PValue __version(PNode*, SymbolTable*, bool*);
extern PValue __wife(PNode*, SymbolTable*, bool*);
extern PValue __xref(PNode*, SymbolTable*, bool*);
extern PValue __year(PNode*, SymbolTable*, bool*);

extern PValue __noop(PNode*, SymbolTable*, bool*);

BuiltIn builtIns[] = {
    "add",        2,    32,    __add,
//    "addnode",    3,    3,    __addnode,
    "addtoset",    3,    3,    __addtoset,
    "alpha",    1,    1,    __alpha,
    "ancestorset",    1,    1,    __ancestorset,
    "and",        2,    32,    __and,
//    "atoi",        1,    1,    __strtoint,
    "baptism",    1,    1,    __baptism,
    "birth",      1,    1,    __birth,
    "burial",     1,    1,    __burial,
    "capitalize", 1,    1,    __capitalize,
    "card",       1,    1,    __card,
    "child",      1,    1,    __child,
//    "children", 1,  1,  __children,
    "childset",    1,    1,    __childset,
//    "choosechild",    1,    1,    __choosechild,
//    "choosefam",    1,    1,    __choosefam,
//    "chooseindi",    1,    1,    __chooseindi,
//    "choosespouse",    1,    1,    __choosespouse,
//    "choosesubset",    1,    1,    __choosesubset,
//    "col",        1,    1,    __col,
//    "concat",    2,    32,    __concat,
    "copyfile",    1,    1,     __copyfile,
    "createnode",    2,    2,   __createnode,
    "d",            1,    1,    __d,
    "database",     0,    1,    __noop,
//    "date",        1,    1,    __date,
    "dateformat",   1,    1,    __dateformat,
    "dayformat",    1,    1,    __dayformat,
    "death",        1,    1,    __death,
    "decr",         1,    1,    __decr,
//    "deletefromset",3,    3,    __deletefromset,
//    "deletenode",    1,    1,    __deletenode,
//    "dequeue",    1,    1,    __dequeue,
//    "dereference",    1,    1,    __getrecord,
    "descendantset",1,    1,    __descendentset,
    "descendentset",1,    1,    __descendentset,
    "difference",    2,    2,    __difference,
     "div",        2,    2,    __div,
     "empty",    1,    1,    __empty,
//    "enqueue",    2,    2,    __push,
    "eq",        2,    2,    __eq,
    "eqstr",    2,    2,    __eqstr,
    "exp",        2,    2,    __exp,
//    "extractdate",    4,    4,    __extractdate,
//    "extractnames",    4,    4,    __extractnames,
//    "extractplaces",3,    3,    __extractplaces,
//    "extracttokens",4,    4,    __extracttokens,
    "f",      1,    1,  __f,
    "fam",       1,    1,    __fam,
    "father",    1,    1,    __father,
    "female",    1,    1,    __female,
    "firstchild",    1,    1,    __firstchild,
//    "firstfam",    1,    1,    __firstfam,
    "firstindi",    0,    0,    __firstindi,
//    "fnode",    1,    1,    __fnode,
    "fullname",    4,    4,    __fullname,
    "ge",        2,    2,    __ge,
//    "gengedcom",    1,    1,    __gengedcom,
//    "genindiset",    2,    2,    __genindiset,
    "getel",    2,    2,    __getel,
//    "getfam",    1,    1,    __getfam,
//    "getindi",    1,    2,    __getindi,
//    "getindimsg",    2,    2,    __getindi,
//    "getindiset",    1,    2,    __getindiset,
//    "getint",    1,    2,    __getint,
//    "getintmsg",    2,    2,    __getint,
//    "getrecord",    1,    1,    __getrecord,
//    "getstr",    1,    2,    __getstr,
//    "getstrmsg",    2,    2,    __getstr,
//    "gettoday",    0,    0,    __gettoday,
    "givens",     1,    1,    __givens,
    "gt",         2,    2,    __gt,
    "husband",    1,    1,    __husband,
    "incr",       1,    1,    __incr,
//    "index",    3,    3,    __index,
    "indi",       1,    1,    __indi,
    "indiset",    1,    1,    __indiset,
    "inode",      1,    1,    __inode,
    "insert",    3,    3,    __insert,
    "intersect",  2,    2,    __intersect,
    "key",        1,    2,    __key,
    "keysort",    1,    1,    __keysort,
    "lastchild",  1,    1,    __lastchild,
    "le",         2,    2,    __le,
    "length",    1,    1,    __length,
    "lengthset",   1,    1,    __lengthset,
//    "linemode",    0,    0,    __linemode,
    "list",        1,    1,    __list,
    "lock",        1,    1,    __noop,
    "long",        1,    1,    __long,
    "lookup",    2,    2,    __lookup,
    "lower",    1,    1,    __lower,
    "lt",        2,    2,    __lt,
    "male",        1,    1,    __male,
    "marriage",    1,    1,    __marriage,
//    "menuchoose",    1,    2,    __menuchoose,
    "mod",        2,    2,    __mod,
    "monthformat",    1,    1,    __monthformat,
    "mother",    1,    1,    __mother,
    "mul",        2,    32,    __mul,
    "name",        1,    2,    __name,
    "namesort",    1,    1,    __namesort,
    "nchildren",    1,    1,    __nchildren,
    "ne",        2,    2,    __ne,
    "neg",        1,    1,    __neg,
//    "nestr",    2,    2,    __strcmp,
//    "newfile",    2,    2,    __newfile,
//    "nextfam",    1,    1,    __nextfam,
    "nextindi",    1,    1,    __nextindi,
    "nextsib",    1,    1,    __nextsib,
//    "nfamilies",    1,    1,    __nfamilies,
    "nl",        0,    0,    __nl,
    "not",        1,    1,    __not,
    "nspouses",    1,    1,    __nspouses,
    "or",        2,    32,    __or,
    "ord",        1,    1,    __ord,
//    "outfile",    0,    0,    __outfile,
//    "pagemode",    2,    2,    __pagemode,
//    "pageout",    0,    0,    __pageout,
    "parent",    1,    1,    __parent,
//    "parents",    1,    1,    __parents,
    "parentset",    1,    1,    __parentset,
    "place",    1,    1,    __place,
    "pn",        2,    2,    __pn,  // Outputs pronouns
    "pop",        1,    1,    __pop,
//    "pos",        2,    2,    __pos,
//    "prevfam",    1,    1,    __prevfam,
//    "previndi",    1,    1,    __previndi,
     "prevsib",    1,    1,    __prevsib,
//    "print",    1,    32,    __print,
    "push",        2,    2,    __push,
    "qt",        0,    0,    __qt,
//    "reference",    1,    1,    __reference,
//    "requeue",    2,    2,    __requeue,
//    "rjustify",    2,    2,    __rjustify,
    "roman",    1,    1,    __roman,
//    "root",        1,    1,    __rot,
//    "row",        1,    1,    __row,
//    "save",        1,    1,    __save,
//    "savenode",    1,    1,    __savenode,
    "set",        2,    2,    __set,
    "setel",    3,    3,    __setel,
    "sex",        1,    1,    __sex,
    "short",    1,    1,    __short,  // Short form of an event.
    "sibling",    1,    1,    __sibling,
    "siblingset",    1,    1,    __siblingset,
//    "soundex",    1,    1,    __soundex,
    "sp",        0,    0,    __space,
    "spouseset",    1,    1,    __spouseset,
    "stddate",    1,    1,    __stddate,
    "strcmp",    2,    2,    __strcmp,
//    "strconcat",    2,    32,    __concat,
    "strlen",    1,    1,    __strlen,
//    "strsave",    1,    1,    __save,
    "strsoundex",    1,    1,    __strsoundex,
    "strtoint",    1,    1,    __strtoint,
    "sub",        2,    2,    __sub,
//    "substring",    3,    3,    __substring,
    "surname",    1,    1,    __surname,
//    "system",    1,    1,    __system,
    "table",    1,    1,    __table,
    "tag",        1,    1,    __tag,
    "title",    1,    1,    __title,
//    "trim",        2,    2,    __trim,
    "trimname",    2,    2,    __trimname,
    "union",    2,    2,    __union,
    "uniqueset",    1,    1,    __uniqueset,
    "unlock",    1,    1,    __noop,
    "upper",    1,    1,    __upper,
    "value",    1,    1,    __value,
//    "valuesort",    1,    1,    __valuesort,
    "version",    0,    0,    __version,
    "wife",        1,    1,    __wife,
    "xref",        1,    1,    __xref,
    "year",        1,    1,    __year,
};

int nobuiltins = sizeof(builtIns)/sizeof(builtIns[0]);
