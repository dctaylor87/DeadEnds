// DeadEnds
//
// interp.h is the header file for the DeadEnds script interpreter.
//
// Created by Thomas Wetmore on 8 December 2022.
// Last changed on 22 May 2025.

#ifndef interp_h
#define interp_h

#include "standard.h"
#include "pnode.h"
#include "symboltable.h"
#include "frame.h"
#include "context.h"
#include "pvalue.h"
#include "database.h"

// Forward references.
typedef struct PNode PNode;
typedef struct HashTable SymbolTable;
typedef struct Frame Frame;

#define CC 32 // 'Commutative constant'.

// InterpType enumerates the interpreter functions return types.
typedef enum InterpType {
    InterpError = 0, InterpOkay, InterpBreak, InterpContinue, InterpReturn
} InterpType;

// Report Interpreter.
void initializeInterpreter(Database*);
void initset(void);
void initrassa(void);
void parseProgram(CString fileName, CString searchPath, ErrorLog *errorLog);
void finishInterpreter(void);
void finishrassa(File*);
void progmessage(int, CString);

void interpScript(Database*, String);
InterpType interpret(PNode*, Context*, PValue*);
InterpType interpChildren(PNode*, Context*, PValue*);
InterpType interpSpouses(PNode*, Context*, PValue*);
InterpType interpFamilies(PNode*, Context*, PValue*);
InterpType interpFathers(PNode*, Context*, PValue*);
InterpType interpMothers(PNode*, Context*, PValue*);
InterpType interpParents(PNode*, Context*, PValue*);
InterpType interpFornotes(PNode*, Context*, PValue*);
InterpType interpForindi(PNode*, Context*, PValue*);
InterpType interpForfam(PNode*, Context*, PValue*);
InterpType interpForsour(PNode*, Context*, PValue*);
InterpType interpForeven(PNode*, Context*, PValue*);
InterpType interpForothr(PNode*, Context*, PValue*);
InterpType interpretSequenceLoop(PNode*, Context*, PValue*);
InterpType interpForList(PNode*, Context*, PValue*);
InterpType interpIfStatement(PNode*, Context*, PValue*);
InterpType interpWhileStatement(PNode*, Context*, PValue*);
InterpType interpProcCall(PNode*, Context*, PValue*);  // User-defined procedure calls.
InterpType interpTraverse(PNode*, Context*, PValue*);
InterpType interp_fornodes(PNode*, Context*, PValue*);

// Prototypes.
PNode *createPNode(int);
PValue evaluate(PNode*, Context*, bool*);
bool evaluateConditional(PNode*, Context*, bool*);
PValue evaluateBuiltin(PNode*, Context*, bool*);
PValue evaluateIdent(PNode*, Context*);
PValue evaluateUserFunc(PNode*, Context*, bool*);
GNode* evaluateGNode(PNode*, Context*, bool*);
//String formatDate(String, int, int, int, int, bool); // declared in date.h
PNode *iden_node(String);
bool iistype(PNode*, PNType);
int num_params(PNode*);
void scriptError(PNode*, String, ...);
//void show_pnode(PNode*);
//void show_pnodes(PNode*);
PNode* string_node(String);
int yylex(void);
int yyparse(ErrorLog *errorLog);

bool setScriptOutputFile (CString filename, bool append, CString *errorMessage);
void poutput(String);
//void interp_main(void);

// Program running state flags.
extern bool programParsing;
extern bool programRunning;
extern bool programDebugging;

// Debugging flags.
extern bool callTracing;
extern bool returnTracing;
extern bool symbolTableTracing;

extern void adjust_cols (String str);

#endif // interp_h
