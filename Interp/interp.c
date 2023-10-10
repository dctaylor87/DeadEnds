//
//  DeadEnds
//
//  interp.c -- The functions that interpret DeadEnds programs. The top function is interpret,
//    which is called on a sequence of program nodes. Depending on the node types, interpret
//    may interpret a node directly, or call a more specific function.
//
//  Created by Thomas Wetmore on 9 December 2022.
//  Last changed on 7 October 2023.
//

#include <stdarg.h>
#include "symboltable.h"
#include "functiontable.h"
#include "interp.h"
#include "pnode.h"
#include "evaluate.h"
#include "lineage.h"
#include "pvalue.h"
#include "database.h"

static bool debugging = false;

extern FunctionTable *procedureTable;  //  Table of user-defined procedures.
extern FunctionTable *functionTable;   //  Table of user-defined functions.
extern SymbolTable *globalTable;       //  Global symbol table.
Database *theDatabase;  // The database.

extern String pnodeTypes[];

bool programParsing = false;
bool programRunning = false;
bool programDebugging = false;

// Global variables that form the interface between the lexer, parser, and interpreter.
//--------------------------------------------------------------------------------------------------
FILE *Poutfp = null;  // File to write the program output to.
int Perrors = 0;      // Number of errors encountered during parsing.

//String ierror = (String) "Error: file \"%s\": line %d: ";

// initializeInterpreter -- Initialize the interpreter.
//--------------------------------------------------------------------------------------------------
void initializeInterpreter(Database *database)
{
    Perrors = 0;
    theDatabase = database;
}

// finishInterpreter -- Finish the interpreter.
//--------------------------------------------------------------------------------------------------
void finishInterpreter(void) { }

//  remove_tables - Remove the interpreter's tables when no longer needed.
//--------------------------------------------------------------------------------------------------
//static void remove_tables(void)
//{
//    // The node block cleaner will free pnodes in procedureTable. TODO: FIGURE OUT WHAT THAT MEANS.
//    deleteHashTable(procedureTable);
//    // TODO: The lexer saved global names so they need to be freed.
//    deleteHashTable(globalTable);
//    // TODO: Are local variable IDENs leaking.
//    deleteHashTable(functionTable);
//}

//  interpret -- Interpret a list of program nodes. If a return statement is encountered this
//    function returns at that point with the return value as the last parameter. The language
//    allows expressions at the statement level, so top level expressions are also interpreted.
//    Output goes to the output file when any statement or top level expression evaluates to a
//    string.
//------------------------------------------------------------------------------------------------
InterpType interpret(PNode *programNode, SymbolTable *symbolTable, PValue *returnValue)
//  programNode -- First program node in a possible list of nodes to interpret.
//  symbolTtable -- Current symbol table.
//  returnValue -- Possible return value.
{
    ASSERT(programNode && symbolTable);
    bool errorFlag = false;
    InterpType returnCode;
    PValue pvalue;

    //  While there are program nodes in the list left to interpret...
    while (programNode) {
        if (programDebugging) {
            printf("interpret:%d: ", programNode->lineNumber);
            showPNode(programNode);
        }

        //  Use the program node's type to decide what to do.
        switch (programNode->type) {

            //  Strings are interpreted by writing them to the output file.
            case PNSCons:
                printf("%s", (String) programNode->stringCons);
                break;

            //  Integer and floating constants are ignored at the top level.
            case PNICons:
            case PNFCons:
                break;
                
            //  Identifiers are interpreted by looking them up in the symbol tables; if they
            //    have a string value, that value is written to the output.
            case PNIdent:
                pvalue = evaluateIdent(programNode, symbolTable, &errorFlag);
                if (errorFlag) {
                    prog_error(programNode, "error evaluating an identifier");
                    return InterpError;
                }
                if (pvalue.type == PVString && pvalue.value.uString)
                    printf("%s", pvalue.value.uString);
                break;

            //  Builtin function calls are interpreted by interpreting the function, and if it
            //    returns a string, writes the string to the output file.
            case PNBltinCall:
                pvalue = evaluateBuiltin(programNode, symbolTable, &errorFlag);
                if (errorFlag) {
                    prog_error(programNode, "error calling built-in function: %s", programNode->funcName);
                    return InterpError;
                }
                if (pvalue.type == PVString && pvalue.value.uString)
                    printf("%s", pvalue.value.uString);
                break;

            //  User procedures are interpreted by binding arguments to their parameters and
            //    then interpreting the procedure's statements.
            case PNProcCall:
               switch (returnCode = interpProcCall(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  User function calls are interpreted by evaluating the call and if it returns
            //    a string writing it to the output file. This applies to statement level
            //    function calls only, those not in expressions. Function calls in expressions
            //    are handled by the evaluator.
            case PNFuncCall:
                pvalue = evaluateUserFunc(programNode, symbolTable, &errorFlag);
                if (errorFlag) return InterpError;
                if (pvalue.type == PVString && pvalue.value.uString) {
                    printf("%s", pvalue.value.uString);
                    stdfree(pvalue.value.uString);  // The pvalue's string is in the heap.
                }
                break;

            //  User function and procedure definitions are illegal during interpretation.
            case PNFuncDef:
            case PNProcDef:
            case PNTable:
            case PNNotes:  //  Maybe this is legitimate?
                FATAL();

            //  Interpret a children loop statement.
            case PNChildren:
                switch (returnCode = interpChildren(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Interpret a spouses loop statement.
            case PNSpouses:
                switch (returnCode = interpSpouses(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            // Iterate though the families a person is a spouse in.
            case PNFamilies:
                switch (returnCode = interpFamilies(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Loop through a person's fathers via the FAMC links.
            case PNFathers:
                switch (returnCode = interpFathers(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Loop through a person's mothers via the FAMC links.
            case PNMothers:
                switch (returnCode = interpMothers(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Loop througn the families a person is in as a child.
            case PNFamsAsChild:
                switch (returnCode = interpParents(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            // Loop through the contents of a sequence.
            case PNSequence:
                switch (returnCode = interp_indisetloop(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Loop through all persons in the database.
            case PNIndis:
                switch (returnCode = interpForindi(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            // Iterate through all families in the database.
            case PNFams:
                switch (returnCode = interpForFam(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default:
                        return returnCode;
                }
                break;

            //  Iterate through all sources in the database.
            case PNSources:
                switch (returnCode = interp_forsour(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Iterate through all events in the database.
            case PNEvents:
                switch (returnCode = interp_foreven(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Loop through all 'other' records in the database.
            case PNOthers:
                switch (returnCode = interp_forothr(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Loop through all elements of a list. All elements will be PValue*'s.
            case PNList:
                switch (returnCode = interpForList(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;
//            case INOTES:
//                switch (irc = interp_fornotes(node, stab, pval)) {
//                    case INTOKAY:
//                        break;
//                    case INTERROR:

//                        return INTERROR;
//                    default:
//                        return irc;
//                }
//                break;
            case PNNodes:
                switch (returnCode = interp_fornodes(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;
            case PNTraverse:
                switch (returnCode = interpTraverse(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Interpret an if statement.
            case PNIf:
                switch (returnCode = interpIfStatement(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            //  Inter*pret a while statement.
            case PNWhile:
                switch (returnCode = interpWhileStatement(programNode, symbolTable, returnValue)) {
                    case InterpOkay: break;
                    case InterpError: return InterpError;
                    default: return returnCode;
                }
                break;

            // Intepret a break statement.
            case PNBreak:
                return InterpBreak;

            // Interpret a continue statement.
            case PNContinue:
                return InterpContinue;

            //  Interpret a return statement.
            case PNReturn:
                if (programNode->returnExpr)
                    *returnValue = evaluate(programNode->returnExpr, symbolTable, &errorFlag);
                return InterpReturn;

//            default:
//                printf("itype(node) is %d\n", itype(node));
//                printf("HUH, HUH, HUH, HUNH!\n");
//                return INTERROR;
        }

        //  Move to the next statement to interpret.
        programNode = programNode->next;
    }
    return InterpOkay;
}

//  interpChildren -- Interpret child loop. Loops through the children of a family.
//    usage: children(FAM, INDI_V, INT_V) {...}
//    fields: pFamilyExpr, pChildIden, pCountIden, pLoopState
//--------------------------------------------------------------------------------------------------
InterpType interpChildren (PNode *pnode, SymbolTable *symtab, PValue* pval)
//  node -- The children loop program node to interpret.
//  stab -- The current symbol table.
//  pval -- Possible return value.
{
    bool eflg = false;
    GNode *fam =  evaluateFamily(pnode->familyExpr, symtab, &eflg);
    if (eflg || !fam || nestr(fam->tag, "FAM")) {
        prog_error(pnode, "the first argument to children must be a family");
        return InterpError;
    }
    FORCHILDREN(fam, chil, nchil) {
        assignValueToSymbol(symtab, pnode->childIden, PVALUE(PVPerson, uGNode, chil));
        assignValueToSymbol(symtab, pnode->countIden, PVALUE(PVInt, uInt, nchil));
        InterpType irc = interpret(pnode->loopState, symtab, pval);
        switch (irc) {
            case InterpContinue:
            case InterpOkay: goto a;
            case InterpBreak: return InterpOkay;
            default: return irc;
        }
    a:	;
    } ENDCHILDREN
    return InterpOkay;
}

//  interpSpouses -- Interpret spouse loop. Loops through the spouses of a person.
//    usage: spouses(INDI, INDI_V, FAM_V, INT_V) {...}
//    fields: pPersonExpr, pSpouseIden, pFamilyIden, pCountIden, pLoopState
//--------------------------------------------------------------------------------------------------
InterpType interpSpouses(PNode *pnode, SymbolTable *symtab, PValue *pval)
//  node -- The spouses program node.
//  stab -- Local symbol table.
//  pval -- Possible return value.
{
    bool eflg = false;
    GNode *indi = evaluatePerson(pnode->personExpr, symtab, &eflg);
    if (eflg || !indi || nestr(indi->tag, "INDI")) {
        prog_error(pnode, "the first argument to spouses must be a person");
        return InterpError;
    }
    FORSPOUSES(indi, spouse, fam, nspouses) {
        assignValueToSymbol(symtab, pnode->spouseIden, PVALUE(PVPerson, uGNode, spouse));
        assignValueToSymbol(symtab, pnode->familyIden, PVALUE(PVFamily, uGNode, fam));
        assignValueToSymbol(symtab, pnode->countIden, PVALUE(PVInt, uInt, nspouses));

        InterpType irc = interpret(pnode->loopState, symtab, pval);
        switch (irc) {
            case InterpContinue:
            case InterpOkay: goto b;
            case InterpBreak: return InterpOkay;
            default: return irc;
        }
    b:	;
    } ENDSPOUSES
    return InterpOkay;
}

//  interpFamilies -- Interpret family loop (families a person is in as a spouse).
//    usage: families(INDI, FAM_V, INDI_V, INT_V) {...}
//    fields: pPersonExpr, pFamilyIden, pSpouseIden, pCountIden, pLoopState
//--------------------------------------------------------------------------------------------------
InterpType interpFamilies(PNode *node, SymbolTable* stab, PValue *pval)
{
    bool eflg = false;
    GNode *indi = evaluatePerson(node->personExpr, stab, &eflg);
    if (eflg || !indi || nestr(indi->tag, "INDI")) {
        prog_error(node, "the first argument to families must be a person");
        return InterpError;
    }
    FORFAMSS(indi, fam, spouse, nfams) {
        assignValueToSymbol(stab, node->familyIden, PVALUE(PVFamily, uGNode, fam));
        assignValueToSymbol(stab, node->spouseIden, PVALUE(PVPerson, uGNode, spouse));
        assignValueToSymbol(stab, node->countIden, PVALUE(PVInt, uInt, nfams));
        InterpType irc = interpret(node->loopState, stab, pval);
        switch (irc) {
            case InterpContinue:
            case InterpOkay: goto c;
            case InterpBreak: return InterpOkay;
            default: return irc;
        }
    c:	;
    }
    ENDFAMSS
    return InterpOkay;
}

//
// interpFathers -- Interpret fathers loop. Most persons will only have one father in a
//    database, so most of the time the loop body will interpreted once.
//--------------------------------------------------------------------------------------------------
InterpType interpFathers(PNode *node, SymbolTable* stab, PValue *pval)
{
    bool eflg = false;
    GNode *indi = evaluatePerson(node->personExpr, stab, &eflg);
    if (eflg || !indi || nestr(indi->tag, "INDI")) {
        prog_error(node, "the first argument to fathers must be a person");
        return InterpError;
    }
    FORFAMCS(indi, fam, husb, wife, nfams) {
        assignValueToSymbol(stab, node->familyIden, PVALUE(PVFamily, uGNode, fam));
        assignValueToSymbol(stab, node->fatherIden, PVALUE(PVFamily, uGNode, husb));
        assignValueToSymbol(stab, node->countIden, PVALUE(PVInt, uInt, nfams));
        InterpType irc = interpret(node->loopState, stab, pval);
        switch (irc) {
            case InterpContinue:
            case InterpOkay: goto d;
            case InterpBreak: return InterpOkay;
            default: return irc;
        }
    d:	    ;
    }  ENDFAMCS
    return InterpOkay;
}

//  interpMothers -- Interpret mothers loop
//--------------------------------------------------------------------------------------------------
InterpType interpMothers (PNode *node, SymbolTable *stab, PValue *pval)
{
    bool eflg = false;
    GNode *indi = evaluatePerson(node->personExpr, stab, &eflg);
    if (eflg || !indi || nestr(indi->tag, "INDI")) {
        prog_error(node, "the first argument to mothers must be a person");
        return InterpError;;
    }
    FORFAMCS(indi, fam, husb, wife, nfams) {
        //  Assign the current loop identifier valujes to the symbol table.
        assignValueToSymbol(stab, node->familyIden, PVALUE(PVFamily, uGNode, fam));
        assignValueToSymbol(stab, node->motherIden, PVALUE(PVFamily, uGNode, wife));
        assignValueToSymbol(stab, node->countIden, PVALUE(PVInt, uInt, nfams));

        // Intepret the body of the loop.
        InterpType irc = interpret(node->loopState, stab, pval);
        switch (irc) {
            case InterpContinue:
            case InterpOkay: goto d;
            case InterpBreak: return InterpOkay;
            default: return irc;
        }
    d:        ;
    }  ENDFAMCS
    return InterpOkay;
}

//  interpParents -- Interpret parents loop; this loops over all families a person is a child in.
//    TODO: Does this exist in LifeLines?
//--------------------------------------------------------------------------------------------------
InterpType interpParents(PNode *node, SymbolTable *stab, PValue *pval)
{
    bool eflg = false;
    InterpType irc;
    GNode *indi = evaluatePerson(node->personExpr, stab, &eflg);
    if (eflg || !indi || nestr(indi->tag, "INDI")) {
        prog_error(node, "the first argument to parents must be a person");
        return InterpError;
    }
    FORFAMCS(indi, fam, husb, wife, nfams) {
        assignValueToSymbol(stab, node->familyIden, PVALUE(PVFamily, uGNode, fam));
        assignValueToSymbol(stab, node->countIden,  PVALUE(PVInt, uInt, nfams));
        irc = interpret(node->loopState, stab, pval);
        switch (irc) {
            case InterpContinue:
            case InterpOkay: goto f;
            case InterpBreak: return InterpOkay;
            default: return irc;
        }
    f:	;
    }
    ENDFAMCS
    return InterpOkay;
}
//
//// interp_fornotes -- Interpret NOTE loop
////--------------------------------------------------------------------------------------------------
InterpType interp_fornotes(PNode *node, SymbolTable *stab, PValue *pval)
{
    ASSERT(node && stab);
    bool eflg = false;
    InterpType irc;
    GNode *root = evaluateGNode(node, stab, &eflg);
    if (eflg) {
        prog_error(node, "1st arg to fornotes must be a record line");
        return InterpError;
    }
    if (!root) return InterpOkay;
    FORTAGVALUES(root, "NOTE", sub, vstring) {
        assignValueToSymbol(stab, node->gnodeIden, PVALUE(PVString, uString, vstring));
        irc = interpret(node->loopState, stab, pval);
        switch (irc) {
            case InterpContinue:
            case InterpOkay:
                goto g;
            case InterpBreak:
                return InterpOkay;
            default:
                return irc;
        }
    g:      ;
    } ENDTAGVALUES
    return InterpOkay;
}

//  interp_fornodes -- Interpret fornodes loop. Loops through the children of a Gedcom node.
//    usage: fornodes(NODE, NODE_V) {...}
//    fields: pGNodeExpr, pNodeIden, pLoopState
//--------------------------------------------------------------------------------------------------
InterpType interp_fornodes(PNode *node, SymbolTable *stab, PValue *pval)
{
    bool eflg = false;
    GNode *root = evaluateGNode(node->gnodeExpr, stab, &eflg);
    if (eflg || !root) {
        prog_error(node, "the first argument to fornodes must be a Gedcom node/line");
        return InterpError;
    }
    GNode *sub = root->child;
    while (sub) {
        assignValueToSymbol(stab, node->gnodeIden, PVALUE(PVGNode, uGNode, sub));
        InterpType irc = interpret(node->loopState, stab, pval);
        switch (irc) {
            case InterpContinue:
            case InterpOkay:
                sub = sub->sibling;
                continue;
            case InterpBreak: return InterpOkay;
            default:
                return irc;
        }
    }
    return InterpOkay;
}

//  interpForindi -- Interpret the forindi loop statement.
//    usage: forindi(INDI_V, INT_V) {...}
//    fields: pPersonIden, pCountIden, pLoopState.
//--------------------------------------------------------------------------------------------------
InterpType interpForindi (PNode *node, SymbolTable *stab, PValue *pval)
//  node -- Program node of the forindi statement.
//  stab -- Symbol table.
//  pval -- Possible return value.
{
    int numPersons = numberPersons(theDatabase);
    int numMisses = 0;
    char scratch[10];

    for (int i = 1; i <= numPersons; i++) {
        sprintf(scratch, "I%d", i);
        GNode *person = keyToPerson(scratch, theDatabase);
        if (person) {
            assignValueToSymbol(stab, node->personIden, PVALUE(PVPerson, uGNode, person));
            assignValueToSymbol(stab, node->countIden, PVALUE(PVInt, uInt, i));
            InterpType irc = interpret(node->loopState, stab, pval);
            switch (irc) {
                case InterpContinue:
                case InterpOkay: continue;
                case InterpBreak:
                case InterpReturn: goto e;
                case InterpError: return InterpError;
            }
        } else {
            numMisses++;
        }
    }

    //  Remove the loop variales from the symbol table before returning.
    //  MNOTE: The elements get removed from the table only in one case.
e:  removeFromHashTable(stab, node->personIden);
    removeFromHashTable(stab, node->countIden);
    return InterpOkay;
}
/////*========================================+
//// * interp_forsour -- Interpret forsour loop
//// *  usage: forsour(SOUR_V,INT_V) {...}
//// *=======================================*/
InterpType interp_forsour (PNode *node, SymbolTable *stab, PValue *pval)
{
////    NODE sour;
////    static char key[MAXKEYWIDTH];
////    STRING record;
////    INTERPTYPE irc;
////    INT len, count = 0;
////    INT scount = 0;
////    insert_pvtable(stab, inum(node), PINT, 0);
////    while (TRUE) {
////        printkey(key, 'S', ++count);
////        if (!(record = retrieve_record(key, &len))) {
////            if(scount < num_sours()) continue;
////            break;
////        }
////        if (!(sour = stringToNodeTree(record))) continue;
////        scount++;
////        insert_pvtable(stab, ielement(node), PSOUR,
////                       sour_to_cacheel(sour));
////        insert_pvtable(stab, inum(node), PINT, (VPTR)count);
////        irc = interpret((PNODE) ibody(node), stab, pval);
////        free_nodes(sour);
////        stdfree(record);
////        switch (irc) {
////            case INTCONTINUE:
////            case INTOKAY:
////                continue;
////            case INTBREAK:
////                return INTOKAY;
////            default:
////                return irc;
////        }
////    }
////    return INTOKAY;
    return InterpOkay;
}
//

//  interp_foreven -- Interpret the foreven loop.
//    usage: foreven(EVEN_V,INT_V) {...}
//--------------------------------------------------------------------------------------------------
InterpType interp_foreven (PNode *node, SymbolTable *stab, PValue *pval)
{
    int numEvents = numberEvents(theDatabase);
    int numMisses = 0;
    char scratch[10];

    for (int i = 1; i <= numEvents; i++) {
        sprintf(scratch, "E%d", i);
        GNode *event = keyToEvent(scratch, theDatabase);
        if (event) {
            assignValueToSymbol(stab, node->eventIden, PVALUE(PVEvent, uGNode, event));
            assignValueToSymbol(stab, node->countIden, PVALUE(PVInt, uInt, i));
            InterpType irc = interpret(node->loopState, stab, pval);
            switch (irc) {
                case InterpContinue:
                case InterpOkay: continue;
                case InterpBreak:
                case InterpReturn: goto e;
                case InterpError: return InterpError;
            }
        } else {
            numMisses++;
        }
    }

    //  Remove the loop variales from the symbol table before returning.
    //  MNOTE: The elements get removed from the table only in one case.
e:  removeFromHashTable(stab, node->personIden);
    removeFromHashTable(stab, node->countIden);
    return InterpOkay;
}
/////*========================================+
//// * interp_forothr -- Interpret forothr loop
//// *  usage: forothr(OTHR_V,INT_V) {...}
//// *=======================================*/
InterpType interp_forothr(PNode *node, SymbolTable *stab, PValue *pval)
{
    int numOthers = numberOthers(theDatabase);
    int numMisses = 0;
    char scratch[10];

    for (int i = 1; i <= numOthers; i++) {
        sprintf(scratch, "X%d", i);
        GNode *event = keyToEvent(scratch, theDatabase);
        if (event) {
            assignValueToSymbol(stab, node->otherIden, PVALUE(PVEvent, uGNode, event));
            assignValueToSymbol(stab, node->countIden, PVALUE(PVInt, uInt, i));
            InterpType irc = interpret(node->loopState, stab, pval);
            switch (irc) {
                case InterpContinue:
                case InterpOkay: continue;
                case InterpBreak:
                case InterpReturn: goto e;
                case InterpError: return InterpError;
            }
        } else {
            numMisses++;
        }
    }

    //  Remove the loop variales from the symbol table before returning.
    //  MNOTE: The elements get removed from the table only in one case.
e:  removeFromHashTable(stab, node->personIden);
    removeFromHashTable(stab, node->countIden);
    return InterpOkay;
    return InterpOkay;
}
/////*======================================+
//// * interpForFam -- Interpret forfam loop
//// *  usage: forfam(FAM_V,INT_V) {...}
//// *=====================================*/
InterpType interpForFam (PNode *node, SymbolTable *stab, PValue *pval)
{
////    NODE fam;
////    static char key[MAXKEYWIDTH];
////    STRING record;
////    INTERPTYPE irc;
////    INT len, count = 0;
////    INT fcount = 0;
////    insert_pvtable(stab, inum(node), PINT, (VPTR)count);
////    while (TRUE) {
////        printkey(key, 'F', ++count);
////        if (!(record = retrieve_record(key, &len))) {
////            if(fcount < num_fams()) continue;
////            break;
////        }
////        if (!(fam = stringToNodeTree(record))) continue;
////        fcount++;
////        insert_pvtable(stab, ielement(node), PFAM,
////                       (VPTR) fam_to_cacheel(fam));
////        insert_pvtable(stab, inum(node), PINT, (VPTR)count);
////        irc = interpret((PNODE) ibody(node), stab, pval);
////        free_nodes(fam);
////        stdfree(record);
////        switch (irc) {
////            case INTCONTINUE:
////            case INTOKAY:
////                continue;
////            case INTBREAK:
////                return INTOKAY;
////            default:
////                return irc;
////        }
////    }
////    return INTOKAY;
    return InterpOkay;
}

//  interpretSequenceLoop -- Interpret a sequence loop.
//    usage: forindiset(SET, INDI_V, ANY_V, INT_V) { }
//    fields: pSequenceExpr, pElementIden, pCountIden, pLoopState
//--------------------------------------------------------------------------------------------------
InterpType interp_indisetloop(PNode *pnode, SymbolTable *symbolTable, PValue *pval)
{
    bool eflg = false;
    InterpType irc;
    // The sequence expression field must be a sequence.
    PValue val = evaluate(pnode->sequenceExpr, symbolTable, &eflg);
    if (eflg || val.type != PVSequence) {
        prog_error(pnode, "the first argument to forindiset must be a set");
        return InterpError;
    }
    Sequence *seq = val.value.uSequence;

    // Start the loop
    FORSEQUENCE(seq, el, ncount) {

        // Update the current person in the symbol table.
        GNode *indi = keyToPerson(el->key, theDatabase);
        assignValueToSymbol(symbolTable, pnode->elementIden, PVALUE(PVPerson, uGNode, indi));

        // Update the current person's value in the symbol table.
        PValue pvalue = el->value ? (PValue) {el->value->type, el->value->value} :
        nullPValue;
        assignValueToSymbol(symbolTable, pnode->valueIden, pvalue);

        // Update the loop counter in the symbol table.
        assignValueToSymbol(symbolTable, pnode->countIden, PVALUE(PVInt, uInt, ncount));

        // Interpret the body of the loop.
        switch (irc = interpret(pnode->loopState, symbolTable, pval)) {
            case InterpContinue:
            case InterpOkay: goto h;
            case InterpBreak: return InterpOkay;
            default: return irc;
        }
    h:	;
    }
    ENDSEQUENCE
    return InterpOkay;
}

//  interpIfStatement -- Interpret an if statement.
//    usage: if ([VAR,] COND) { THEN } [{ else ELSE }]
//    fields: pCondExpr, pThenState, pElseState
//--------------------------------------------------------------------------------------------------
InterpType interpIfStatement(PNode *pnode, SymbolTable *symtab, PValue *rvalue)
//  pnode -- Program node holding the if statement.
//  symtab -- Symbol table.
//  rvalue -- Possible return value.
{
    ASSERT(pnode && pnode->type == PNIf && symtab);

    // Evaluate the conditional expression.
    bool eflg = false;
    bool cond = evaluateConditional(pnode->condExpr, symtab, &eflg);
    if (eflg) return InterpError;

    // If the condition is true interpret the then clause.
    if (cond) return interpret(pnode->thenState, symtab, rvalue);

    // If the condition is false and there is an else clause interpret the else clause.
    if (pnode->elseState) return interpret(pnode->elseState, symtab, rvalue);

    // Else there was no else clause so return okay.
    return InterpOkay;
}

//  interpWhileStatement -- Interpret a while statement.
//--------------------------------------------------------------------------------------------------
InterpType interpWhileStatement (PNode *node, SymbolTable *stab, PValue *pval)
//  node -- Program node holding a while statement.
//  stab -- Symbol table.
//  pvalue --
{
    ASSERT(node && node->type == PNWhile && stab);

    // Loop.
    bool eflg = false;
    while (true) {
        // Evaluate the condition.
        bool cond = evaluateConditional(node->condExpr, stab, &eflg);

        // If there was an error evaluating the condition, return with an error.
        if (eflg) return InterpError;

        // If the condition is false, the loop is over so return okay.
        if (!cond) return InterpOkay;

        // Interpret the body of the loop and switch on the return code.
        InterpType irc;
        switch (irc = interpret(node->loopState, stab, pval)) {

            // For continue and okay codes, return to top for another loop.
            case InterpContinue:
            case InterpOkay:
                continue;

            // For a break or return code, break out of the loop.
            case InterpBreak:
                return InterpOkay;

            // For any other code return the code; this includes InterReturn.
            default:
                return irc;
        }
    }
}

//  interpProcCall -- Interpret a procedure call statement. The fields used in the program nodes
//    are pProcName for the procedure name, pArguments for the arguments, pParameters for the
//    parameters and pProcBody for the procedure statements. This function first
//    retrieves the procedure definition program node from the procedure table. It then evaluates
//    the list of arguments and binds their values to the parameters in the symbol table.
//    It then calls interpret on the first statement of the body.
//--------------------------------------------------------------------------------------------------
InterpType interpProcCall(PNode *programNode, SymbolTable *symbolTable, PValue *pval)
//  programNode -- Program node with user-procedure call.
//  symbolTable -- Symbol table.
//  pval --
{
    ASSERT(programNode && programNode->type == PNProcCall && symbolTable);
    if (programDebugging) {
        printf("interpProcCall: %d: %s\n", programNode->lineNumber, programNode->procName);
    }

    //  Look up the procedure in the procedure table.
    PNode *procedure = searchFunctionTable(procedureTable, programNode->procName);
    if (!procedure) {
        printf("``%s'': undefined procedure\n", programNode->procName);
        return InterpError;
    }

    //  Create a symbol table for the procedure.
    SymbolTable *newSymbolTable = createSymbolTable();
    if (debugging)
        printf("interpProcCall: creating symbol table %p for %s\n", newSymbolTable,
               programNode->procName);
    PNode *argument = programNode->arguments;  // First argument to the procedure.
    PNode *parameter = procedure->parameters;  // First parameter of the procedure.

    //  Bind the argument values to the parameters.
    while (argument && parameter) {
        bool eflg = false;

        //  Evaluate the current argument and return if there is an error.
        PValue value = evaluate(argument, symbolTable, &eflg);
        if (eflg) return InterpError;

        //  Add the argument to the symbol table for the current parameter. Create a copy of
        //    the argument's PValue on the heap.
        assignValueToSymbol(newSymbolTable, parameter->identifier, value);

        // Step to the next argument and parameter.
        argument = argument->next;
        parameter = parameter->next;
    }

    // Check for mismatch in the numbers of arguments and parameters.
    if (argument || parameter) {
        printf("``%s'': mismatched args and params\n", programNode->procName);
        deleteHashTable(newSymbolTable);  // Get rid of that new symbol table.
        return InterpError;
    }

    if (debugging) {
        printf("Symbol Table after binding args and parms:\n");
        showSymbolTable(newSymbolTable);
    }

    // Interpret the body of the procedure using the new symbol table.
    InterpType returnCode = interpret(procedure->procBody, newSymbolTable, pval);
    deleteHashTable(newSymbolTable);
    switch (returnCode) {
        case InterpReturn:
        case InterpOkay: return InterpOkay;
        case InterpBreak:
        case InterpContinue:
        case InterpError:
        default: return InterpError;
    }
    return InterpError;
}

//  interpTraverse -- Interpret the traverse statement. This traverses a Gedcom node tree or
//    subtree. This function adds two entries to the local symbol table for the two loop
//    variables, and removes them when the loop finishes.
//    usage: traverse(GNode expr, GNode ident, int ident) {...}
//    fields: pGNodeExpr, pLevelIden, pGNodeIden.
//--------------------------------------------------------------------------------------------------
#define MAXTRAVERSEDEPTH 100
InterpType interpTraverse(PNode *traverseNode, SymbolTable *symbolTable, PValue *returnValue)
//  traverseNode -- Program node holding a traverse statement.
//  symbolTable -- Local symbol table.
//  returnValue -- Possible return value.
{
    ASSERT(traverseNode && symbolTable);
    // Get the Gedcom node that will be the root node of the traverse.
    bool errorFlag = false;
    GNode *root = evaluateGNode(traverseNode->gnodeExpr, symbolTable, &errorFlag);
    if (errorFlag || !root) {
        prog_error(traverseNode, "the first argument to traverse must be a Gedcom line");
        return InterpError;
    }

    //  Create symbol table entries for the level and node loop variables.
    //  TODO: Should we check that these identifiers are not already in the symbol table?
    assignValueToSymbol(symbolTable, traverseNode->levelIden, PVALUE(PVInt, uInt, 0));
    assignValueToSymbol(symbolTable, traverseNode->gnodeIden, PVALUE(PVGNode, uGNode, root));

    //  Normally getValueOfIden is used to get the value of an identifier from a symbol table.
    //    In this case, however, I want direct access to the PValues stored in the table. This
    //    is because each iteration updates the values of two identifiers. The proper way to
    //    do this is to do assignments each time. But I decided to keep pointers to the PValues
    //    inside the symbol table and change them directly. So I'm using searchHashTable
    //    instead of getValueOfIden to get write access to those PValues. The variables level
    //    and node below point into the symbol table.
    PValue *level = ((Symbol*) searchHashTable(symbolTable, traverseNode->levelIden))->value;
    PValue *node = ((Symbol*) searchHashTable(symbolTable, traverseNode->gnodeIden))->value;
    ASSERT(node && level);

    // Create the stack of Gedcom nodes that hold the path to the current node.
    GNode *snode, *nodeStack[MAXTRAVERSEDEPTH];
    InterpType irc;
    InterpType returnIrc = InterpOkay;

    // Load the stack with the root and start traversing.
    int lev = 0;
    nodeStack[lev] = snode = root;
    while (true) {
        // Update the symbol table with the current variable values.
        node->value.uGNode = snode;
        level->value.uInt = lev;

        // Interpret the body of the loop.
        switch (irc = interpret(traverseNode->loopState, symbolTable, returnValue)) {
            case InterpContinue:
            case InterpOkay: break;
            case InterpBreak:
                returnIrc = InterpOkay;
                goto a;
            default:
                returnIrc = irc;
                goto a;
        }

        // Modify lev and the stack for the next traverse.
        // If the current node has a child go down.
        if (snode->child) {
            snode = nodeStack[++lev] = snode->child;
            continue;
        }
        // If current node has a sibling, go there.
        if (snode->sibling) {
            snode = nodeStack[lev] = snode->sibling;
            continue;
        }
        // If current node has no child or sibling, pop until either reach top or find a sibling.
        while (--lev >= 0 && !(nodeStack[lev])->sibling)
            ;
        if (lev < 0) break;
        snode = nodeStack[lev] = (nodeStack[lev])->sibling;
    }
a:  removeFromHashTable(symbolTable, traverseNode->levelIden);
    removeFromHashTable(symbolTable, traverseNode->gnodeIden);
    return returnIrc;
}

//  prog_error -- Report a run time program error.
//--------------------------------------------------------------------------------------------------
void prog_error(PNode *gnode, String fmt, ...)
{
    va_list args;
    printf("\nError in \"%s\" at line %d: ", gnode->fileName, gnode->lineNumber);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(".\n");
}
