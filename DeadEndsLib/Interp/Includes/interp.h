// DeadEnds
//
// interp.h -- Header for the DeadEnds language interpreter.
//
// Created by Thomas Wetmore on 8 December 2022.
// Last changed on 18 April 2024.

#ifndef interp_h
#define interp_h

typedef struct PNode PNode;
typedef struct HashTable SymbolTable;

#include "standard.h"
#include "pnode.h"
#include "pvalue.h"
#include "symboltable.h"
#include "database.h"

// InterpType is the enumeration of interpreter return types.
typedef enum InterpType {
    InterpError = 0, InterpOkay, InterpBreak, InterpContinue, InterpReturn
} InterpType;

// Context is the data type that holds the context in which interpretation takes place.
typedef struct Context {
    SymbolTable *symbolTable;
    Database *database;
} Context;

// Report Interpreter.
void initializeInterpreter(Database*);
void initset(void);
void initrassa(void);
void parseProgram(CString fileName, CString searchPath);
void finishInterpreter(void);
void finishrassa(void);
void progmessage(char*);

Context *createContext(SymbolTable*, Database*);
void deleteContext(Context*);

InterpType interpret(PNode*, Context*, PValue*);
InterpType interpChildren(PNode*, Context*, PValue*);
InterpType interpSpouses(PNode*, Context*, PValue*);
InterpType interpFamilies(PNode*, Context*, PValue*);
InterpType interpFathers(PNode*, Context*, PValue*);
InterpType interpMothers(PNode*, Context*, PValue*);
InterpType interpParents(PNode*, Context*, PValue*);
InterpType interp_fornotes(PNode*, Context*, PValue*);
InterpType interp_fornodes(PNode*, Context*, PValue*);
InterpType interpForindi(PNode*, Context*, PValue*);
InterpType interp_forsour(PNode*, Context*, PValue*);
InterpType interp_foreven(PNode*, Context*, PValue*);
InterpType interp_forothr(PNode*, Context*, PValue*);
InterpType interpForFam(PNode*, Context*, PValue*);
InterpType interpretSequenceLoop(PNode*, Context*, PValue*);
InterpType interpForList(PNode*, Context*, PValue*);
InterpType interpIfStatement(PNode*, Context*, PValue*);
InterpType interpWhileStatement(PNode*, Context*, PValue*);
InterpType interpProcCall(PNode*, Context*, PValue*);  // User-defined procedure calls.
InterpType interpTraverse(PNode*, Context*, PValue*);

// Prototypes.
//void assignIdent(SymbolTable*, String, PValue);
int bool_to_int(bool);
double bool_to_float(bool);
PNode *createPNode(int);
PValue evaluate(PNode*, Context*, bool*);
bool evaluateConditional(PNode*, Context*, bool*);
PValue evaluateBuiltin(PNode*, Context*, bool*);
PValue evaluateIdent(PNode*, Context*, bool*);
PValue evaluateUserFunc(PNode*, Context*, bool*);
//GNode* evaluateFamily(PNode*, Context*, bool*);
GNode* evaluateGNode(PNode*, Context*, bool*);
//void extract_date(String, int*, int*, int*, int*, String*);
//String format_date(String, int, int, int, int, bool); // declared in date.h
void free_all_pnodes(void);
void free_pnode_tree(PNode*);
PNode *iden_node(String);
bool iistype(PNode*, PNType);
int num_params(PNode*);
void scriptError(PNode*, String, ...);
//void show_one_pnode(PNode*);
void show_pnode(PNode*);
void show_pnodes(PNode*);
PNode* string_node(String);
//PValue valueOfIdent(SymbolTable*, String);
int yylex(void);
int yyparse(void);

void poutput(String);
//void interp_main(void);

extern void adjust_cols (String str);
extern bool programRunning;

#endif // interp_h
