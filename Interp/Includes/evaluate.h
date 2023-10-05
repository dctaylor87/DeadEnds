//
//  DeadEnds
//
//  evaluate.h
//
//  Created by Thomas Wetmore on 15 December 2022.
//  Last changed on 30 March 2023.
//

#ifndef evaluate_h
#define evaluate_h

#include "symboltable.h"   // Table.
#include "pnode.h"   // PNode.
#include "gnode.h"   // GNode.
#include "pvalue.h"  // PValue.

// TODO: Should some of these be made static inside evaluate.c?
PValue evaluate(PNode*, SymbolTable*, bool* errflag);          // Generic evaluator.
PValue evaluateIdent(PNode*, SymbolTable*, bool* errflag);     // Evaluate an identifier.
PValue evaluateBuiltin(PNode*, SymbolTable*, bool* errflag);   // Evaluate a built-in.
PValue evaluateUserFunc(PNode*, SymbolTable*, bool* errflag);  // Evaluate a user function.
GNode* evaluatePerson(PNode*, SymbolTable*, bool* errflag);    // Evaluate and return a person.
GNode* evaluateFamily(PNode*, SymbolTable*, bool* errflg);     // Evaluate and return a family.
PValue evaluateBoolean(PNode*, SymbolTable*, bool* errflg);    // Evaluate a boolean expression.

#endif /* evaluate_h */
