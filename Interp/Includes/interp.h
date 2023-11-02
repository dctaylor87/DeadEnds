//
//  DeadEnds
//
//  interp.h -- Header for the DeadEnds language interpreter.
//
//  Created by Thomas Wetmore on 8 December 2022.
//  Last changed on 13 October 2023.
//

#ifndef interp_h
#define interp_h

#include "standard.h"
#include "pnode.h"
#include "pvalue.h"
#include "symboltable.h"

// InterpType -- Enumeration of the interpreter return types.
//--------------------------------------------------------------------------------------------------
typedef enum InterpType {
    InterpError = 0, InterpOkay, InterpBreak, InterpContinue, InterpReturn
} InterpType;

//extern Table testTable;

// Report Interpreter.
void initializeInterpreter(Database*);
void initset(void);
void initrassa(void);
void parseProgram(CString fileName, CString searchPath);
void finishInterpreter(void);
void finishrassa(void);
void progmessage(char*);

InterpType interpret(PNode*, SymbolTable*, PValue*);
InterpType interpChildren(PNode*, SymbolTable*, PValue*);
InterpType interpSpouses(PNode*, SymbolTable*, PValue*);
InterpType interpFamilies(PNode*, SymbolTable*, PValue*);
InterpType interpFathers(PNode*, SymbolTable*, PValue*);
InterpType interpMothers(PNode*, SymbolTable*, PValue*);
InterpType interpParents(PNode*, SymbolTable*, PValue*);
InterpType interp_fornotes(PNode*, SymbolTable*, PValue*);
InterpType interp_fornodes(PNode*, SymbolTable*, PValue*);
InterpType interpForindi(PNode*, SymbolTable*, PValue*);
InterpType interp_forsour(PNode*, SymbolTable*, PValue*);
InterpType interp_foreven(PNode*, SymbolTable*, PValue*);
InterpType interp_forothr(PNode*, SymbolTable*, PValue*);
InterpType interpForFam(PNode*, SymbolTable*, PValue*);
InterpType interp_indisetloop(PNode*, SymbolTable*, PValue*);
InterpType interpForList(PNode*, SymbolTable*, PValue*);
InterpType interpIfStatement(PNode*, SymbolTable*, PValue*);           // Interpret if statements.
InterpType interpWhileStatement(PNode*, SymbolTable*, PValue*);         // Interpret while loops.
InterpType interpProcCall(PNode*, SymbolTable*, PValue*);         // Interpret user-defined procedure calls.
InterpType interpTraverse(PNode*, SymbolTable*, PValue*);

// Prototypes.
//void assignIdent(SymbolTable*, String, PValue);
int bool_to_int(bool);
double bool_to_float(bool);
PNode *createPNode(int);
PValue evaluate(PNode*, SymbolTable*, bool*);
bool evaluateConditional(PNode*, SymbolTable*, bool*);
PValue evaluateBuiltin(PNode*, SymbolTable*, bool*);
PValue evaluateIdent(PNode*, SymbolTable*, bool*);
PValue evaluateUserFunc(PNode*, SymbolTable*, bool*);
PValue eval_and_coerce(int, PNode*, SymbolTable*, bool*);
//GNode* evaluateFamily(PNode*, SymbolTable*, bool*);
GNode* evaluateGNode(PNode*, SymbolTable*, bool*);
//void extract_date(String, int*, int*, int*, int*, String*);
String format_date(String, int, int, int, int, bool);
void free_all_pnodes(void);
void free_pnode_tree(PNode*);
PNode *iden_node(String);
bool iistype(PNode*, PNType);
int num_params(PNode*);
void prog_error(PNode*, String, ...);
//void show_one_pnode(PNode*);
void show_pnode(PNode*);
void show_pnodes(PNode*);
PNode* string_node(String);
//PValue valueOfIdent(SymbolTable*, String);
int yylex(void);
int yyparse(void);

//void poutput(String);
void interp_main(void);

#endif // interp_h
