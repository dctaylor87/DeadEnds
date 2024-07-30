// DeadEnds
//
// pnode.c holds the functions that manage PNodes (program nodes).
//
// Created by Thomas Wetmore on 14 December 2022.
// Last changed on 28 July 2024.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "refnindex.h"
#include "pnode.h"
#include "standard.h"
#include "hashtable.h"
#include "functiontable.h"
#include "gedcom.h"
#include "interp.h"
#include "parse.h"

static bool debugging = false;

// pnodeTypes are String names for the program node types useful for debugging.
String pnodeTypes[] = {
    "", "ICons", "FCons", "SCons", "Ident", "If", "While", "Break", "Continue", "Return",
    "ProcDef", "ProcCall", "FuncDef", "FuncCall", "BltinCall", "Traverse", "Nodes", "Families",
    "Spouses", "Children", "Indis", "Fams", "Sources", "Events", "Others", "List", "Set",
    "Fathers", "Mothers", "FamsAsChild", "Notes"
};

extern FunctionTable *functionTable;  // parse.c

// showPNode shows a PNode useful for debugging.
void showPNode(PNode* pnode) {
    printf("%s ", pnodeTypes[pnode->type]);
    switch (pnode->type) {
        case PNICons:     printf("%ld\n", pnode->intCons); break;
        case PNFCons:     printf("%g\n", pnode->floatCons); break;
        case PNSCons:     printf("%s\n", pnode->stringCons); break;
        case PNProcCall:  printf("%s()\n", pnode->procName); break;
        case PNFuncCall:  printf("%s()\n", pnode->funcName); break;
        case PNBltinCall: printf("%s()\n", pnode->funcName); break;
        case PNIdent:     printf("%s\n", pnode->identifier); break;
        case PNProcDef:   printf("%s\n", pnode->procName); break;
        case PNFuncDef:   printf("%s\n", pnode->funcName); break;
        case PNIf:
        case PNWhile:
        case PNBreak:
        case PNContinue:
        case PNReturn:
        case PNTraverse:
        case PNNodes:
        case PNFamilies:
        case PNSpouses:
        case PNChildren:
        case PNIndis:
        case PNFams:
        case PNSources:
        case PNEvents:
        case PNOthers:
        case PNList:
        case PNSequence:
        case PNFathers:
        case PNMothers:
        case PNFamsAsChild:
        case PNNotes:
        default: printf("\n"); break;
    }
}

static void setParents(PNode* list, PNode* parent); // Set the parents of a PNode list.

// allocPNode allocates a PNode and sets the pType, pFileName and pLineNum fields.
static PNode* allocPNode(int type) {
    PNode* node = (PNode*) stdalloc(sizeof(*node));
    if (debugging) {
        printf("allocPNode(%d) %s, %d\n", type, currentFileName, currentLine);
    }
    if (! node)
      return NULL;
    // make sure all fields we don't explicitly initialize are zero
    bzero ((void *)node, sizeof (*node));

    node->type = type;
    node->fileName = strsave(currentFileName); // TODO: MEMORY!!!!!!!!!
    node->lineNumber = currentLine; // Overwritten by the yacc m production?
    return node;
}

// iconsPNode creates an integer PNode with a C long.
PNode* iconsPNode(long intConstant) {
    PNode *node = allocPNode(PNICons);
    node->intCons = intConstant;
    return node;
}

// fconsPNode creates a float PNode with a C double.
PNode* fconsPNode(double floatConstant) {
    PNode *pnode = allocPNode(PNFCons);
    pnode->floatCons = floatConstant;
    return pnode;
}

// sconsPNode creates a String PNode.
PNode* sconsPNode(String string) {
    PNode *pnode = allocPNode(PNSCons);
    pnode->stringCons = string;
    return pnode;
}

// ifPNode creates an if statement PNode.
PNode* ifPNode (PNode* cond, PNode* tnode, PNode* enode) {
    PNode *pnode = allocPNode(PNIf);
    pnode->condExpr = cond;
    pnode->thenState = tnode;
    pnode->elseState = enode;
    setParents(tnode, pnode);
    setParents(enode, pnode);
    return pnode;
}

// whilePNode creates a while loop PNode.
PNode* whilePNode(PNode* cond, PNode* body) {
    PNode *node = allocPNode(PNWhile);
    node->condExpr = cond;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// breakPNode creates a break PNode.
PNode* breakPNode(void) { return allocPNode(PNBreak); }

// continuePNode creates a continue PNode.
PNode *continuePNode(void) { return allocPNode(PNContinue); }

// returnPNode create a return PNode with optional return expression.
PNode* returnPNode(PNode *args) {
    PNode *node = allocPNode(PNReturn);
    if (args) { node->returnExpr = args; }
    return node;
}

// procDefPNode creates a user-defined proc definition node.
PNode* procDefPNode(String name, PNode* parms, PNode* body) {
    PNode *node = allocPNode(PNProcDef);
    node->procName = name;
    node->parameters = parms;
    node->procBody = body;
    setParents(body, node);
    return node;
}

// procCallPNode creates a user-defined proc call node.
PNode* procCallPNode(String name, PNode *args) {
    PNode *node = allocPNode(PNProcCall);
    node->procName = strsave(name);
    node->arguments = args;
    return node;
}

// funcDefPNode creates a user-defined function definition node.
PNode* funcDefPNode(String name, PNode* parms, PNode* body) {
    PNode *node = allocPNode(PNFuncDef);
    node->funcName = name;
    node->parameters = parms;
    node->funcBody = body;
    setParents(body, node);
    return node;
}

// funcCallPNode creates a builtin or user-defined function call program node.
PNode* funcCallPNode(String name, PNode* alist) {
    if (isInHashTable(functionTable, name)) { // User-defined.
        PNode *node = allocPNode(PNFuncCall);
        node->funcName = name;
        node->arguments = alist;
        node->funcBody = searchFunctionTable(functionTable, name);
        return node;
    }

    // Not user-defined; should be a builtin.
    int lo = 0;
    int hi = nobuiltins - 1;
    bool found = false;
    int r;
    int md = 0;
    while (lo <= hi) {
        md = (lo + hi) >> 1;
        if ((r = nestr(name, builtIns[md].name)) < 0)
            hi = md - 1;
        else if (r > 0)
            lo = md + 1;
        else {
            found = true;
            break;
        }
    }
    if (found) { // Is a builtin
        int n;
        if ((n = num_params(alist)) < builtIns[md].minParams || n > builtIns[md].maxParams) {
            printf("%s: must have %d to %d parameters.\n", name,
                   builtIns[md].minParams, builtIns[md].maxParams);
            Perrors++;
        }
        PNode *node = allocPNode(PNBltinCall);
        node->funcName = name;
        node->arguments = alist;
        node->builtinFunc = builtIns[md].func;
        return node;
    }
    printf("%s: undefined function.\n", name);
    Perrors++; // Is undefined.
    PNode *node = allocPNode(PNFuncCall);
    node->funcName = name;
    node->parameters = alist;
    node->funcBody = null;
    return node;
}

// traversePNode creates a traverse loop PNode to traverses a tree of GNodes. snode and levv are
// the names of the idents holding the current node's name and the current level.
PNode* traversePNode (PNode* nexpr, String snode, String levv, PNode* body) {
    PNode *node = allocPNode(PNTraverse);
    node->gnodeExpr = nexpr;
    node->gnodeIden = snode;
    node->levelIden = levv;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// fornodesPNode creates a fornodes loop PNode to iterate the children of a GNode. nvar is the
// name of the ident holding the current child's name.
PNode* fornodesPNode (PNode* nexpr, String nvar, PNode* body) {
    PNode *node = allocPNode(PNNodes);
    node->gnodeExpr = nexpr;
    node->gnodeIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// familiesPNode creates a family loop PNode to loop through the families a person is a spouse in.
// fvar is the family ident; svar is the spouse ident; nvar is the counter ident.
PNode* familiesPNode(PNode* pexpr, String fvar, String svar, String count, PNode* body) {
    PNode *node = allocPNode(PNFamilies);
    node->personExpr = pexpr;
    node->familyIden = fvar;
    node->spouseIden = svar;
    node->countIden = count;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// spousesPNode create a spouse loop PNode that loops through the spouses of a person. svar is
// spouse ident; fvar is the family ident; nvar is the counter ident.
PNode* spousesPNode (PNode* pexpr, String svar, String fvar, String count, PNode* body) {
    PNode *node = allocPNode(PNSpouses);
    node->personExpr = pexpr;
    node->spouseIden = svar;
    node->familyIden = fvar;
    node->countIden = count;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// childrenPNode createa a children loop PNode that loops through the chidren in a family.
// Called by yyparse() on rule reduction; fexpr is the family expression; cvar is the child
// loop var; nvar is the counter; body is the root PNode of the loop body.
PNode *childrenPNode (PNode *fexpr, String cvar, String nvar, PNode *body) {
    PNode *node = allocPNode(PNChildren);
    node->familyExpr = fexpr;
    node->childIden = cvar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

//  forindiPNode -- Create a forindi PNode to iterate every Person in the Database.
//    usage: forindi(INDI_V, INT_V) { body }
//    fields: pPersonIden, pCountIden, pLoopState
//--------------------------------------------------------------------------------------------------
PNode *forindiPNode (String ivar, String nvar, PNode *body)
//  ivar -- Person identifier.
//  nvar -- Counter identifier.
//  body -- First PNode of the loop body.
{
    PNode *node = allocPNode(PNIndis);
    node->personIden = ivar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

//  forfamPNode -- Create a forfam program node loop to iterate every family in the database.
//--------------------------------------------------------------------------------------------------
PNode *forfamPNode (String fvar, String nvar, PNode *body)
//  fvar -- Family identifier.
//  nvar -- Counter identifier.
//  body -- First PNode of the loop body.
{
    PNode *node = allocPNode(PNFams);
    node->familyIden = fvar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// forsourPNode -- Create a forsour PNode loop to iterate every Source in the Database.
//--------------------------------------------------------------------------------------------------
PNode *forsourPNode (String svar, String nvar, PNode *body)
// svar -- Source identifier.
// nvar -- Counter identifier.
// body -- First PNode of the loop body.
{
    PNode *node = allocPNode(PNSources);
    node->sourceIden = svar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// forevenPNode -- Create foreven loop node
//--------------------------------------------------------------------------------------------------
PNode *forevenPNode(String evar, String nvar, PNode *body)
// evar -- Event identifier.
// nvar -- Counter identifier.
// body -- First PNode of the loop body.
{
    PNode *node = allocPNode(PNEvents);
    node->eventIden = evar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// forothrPNode -- Create forothr loop node
//--------------------------------------------------------------------------------------------------
PNode *forothrPNode (String ovar, String nvar, PNode *body)
// ovar -- Other identifier.
// nvar -- Counter identifier.
// body -- First PNode of the loop body.
{
    PNode *node = allocPNode(PNOthers);
    node->otherIden = ovar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// forlistPNode -- Create a list loop PNode.
// Usage:
//--------------------------------------------------------------------------------------------------
PNode *forlistPNode (PNode *lexpr, String evar, String nvar, PNode *body)
// iexpr -- PNode expression that evaluates to a List.
// evar -- Element identifier.
// nvar -- Counter identifier.
// body -- First PNode of the loop body.
{
    PNode *node = allocPNode(PNList);
    node->listExpr = lexpr;
    node->elementIden = (void*) evar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// forindisetPNode -- Create an index loop PNode.
//--------------------------------------------------------------------------------------------------
PNode *forindisetPNode(PNode *iexpr, String ivar, String vvar, String nvar, PNode *body)
// iexpr expr
// ivar    person
// vvar    value
// nvar    counter
// body    body
{
    PNode *node = allocPNode(PNSequence);
    node->sequenceExpr = iexpr;
    node->elementIden = ivar;
    node->valueIden = vvar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// fornotesPNode -- Create fornotes loop node
//--------------------------------------------------------------------------------------------------
PNode *fornotesPNode (PNode *nexpr, String vvar, PNode *body)
// nexpr -- PNode expression that evaluates to a GNode.
// vvar -- value
// body -- First PNode of loop body.
{
//    PNode *node = allocPNode(INOTES);
//    node->pnexpr = nexpr;
//    istrng(node) = (Word) vvar;
//    node->pbody = body;
//    setParents(body, node);
//    return node;
    return null;
}

// iden_node -- Create an identifier PNode.
// TODO: FIGURE OUTWHAT TO DO WITH THIS.
//--------------------------------------------------------------------------------------------------
PNode *iden_node(String identifier)
// iden -- String to embed in a PNode.
{
    PNode *pnode = allocPNode(PNIdent);
    pnode->identifier = identifier;
    return pnode;
}

// fathersPNode -- Create fathers loop node.
//--------------------------------------------------------------------------------------------------
PNode *fathersPNode(PNode *pexpr, String pvar, String fvar, String nvar, PNode *body)
//  pexpr -- Person PNode expression.
//  pvar -- Father identifier.
//  fvar -- Family identifier.
//  nvar -- Count identifier.
//  body -- First PNode of body.
{
    PNode *node = allocPNode(PNFathers);
    node->personExpr = pexpr;
    node->fatherIden = pvar;
    node->familyIden = fvar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// mothersPNode -- Create mothers loop node.
//--------------------------------------------------------------------------------------------------
PNode *mothersPNode(PNode *pexpr, String pvar, String fvar, String nvar, PNode *body)
//  pexpr -- Person PNode expression.
//  pvar -- Mother identifier.
//  fvar -- Family identifier.
//  nvar -- Count identifier.
//  body -- First PNode of loop.
{
    PNode *node = allocPNode(PNMothers);
    node->personExpr = pexpr;
    node->motherIden = pvar;
    node->familyIden = fvar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// parentsPNode -- Create parents loop node.
//--------------------------------------------------------------------------------------------------
PNode *parentsPNode(PNode *pexpr, String fvar, String nvar, PNode *body)
//  pexpr -- Person PNode expression.
//  fvar -- Family identifier.
//  nvar -- Count identifier.
//  body -- First PNode of loop.
{
    PNode *node = allocPNode(PNFamsAsChild);
    node->personExpr = pexpr;
    node->familyIden = fvar;
    node->countIden = nvar;
    node->loopState = body;
    setParents(body, node);
    return node;
}

// setParents -- Set the parent node for a list of nodes.
//--------------------------------------------------------------------------------------------------
static void setParents (PNode *list, PNode *parent)
//  list -- List of program nodes to be assigned a parent.
//  parent -- Node to be made the parent.
{
    while (list) {
        list->parent = parent;
        list = list->next;
    }
}

//  freePNodes -- Free the program nodes rooted at the given program node.
//    TODO: Write me.
//    NOTE: This is more complicated than it seem at first. Just like there is a separate
//      function for allocating each type of program node, there should be a separate function
//      for freeing each tyhpe of program node.
//--------------------------------------------------------------------------------------------------
void freePNodes(PNode *pnode)
{
    while (pnode) {
        switch (pnode->type) {
            case PNICons: break;
            case PNFCons: break;
            case PNSCons:
            case PNIdent:
                stdfree(pnode->stringCons);
                break;
            case PNIf:
                freePNodes(pnode->condExpr);
                freePNodes(pnode->thenState);
                freePNodes(pnode->elseState);
                break;
            case PNWhile:
                freePNodes(pnode->condExpr);
                freePNodes(pnode->loopState);
                break;
            case PNBreak:
            case PNContinue:
                break;
            case PNReturn:
                if (pnode->returnExpr)
                    freePNodes(pnode->returnExpr);
                break;
            case PNProcDef:
                stdfree(pnode->procName);
                freePNodes(pnode->parameters);
                freePNodes(pnode->procBody);
                break;
            case PNProcCall:
                stdfree(pnode->procName);
                freePNodes(pnode->arguments);
                break;
            case PNFuncDef:
                stdfree(pnode->funcName);
                freePNodes(pnode->parameters);
                freePNodes(pnode->funcBody);
                break;
            case PNFuncCall:
            case PNBltinCall:
                stdfree(pnode->funcName);
                freePNodes(pnode->arguments);
                break;
            case PNTraverse:
                freePNodes(pnode->gnodeExpr);
                stdfree(pnode->gnodeIden);
                stdfree(pnode->levelIden);
                freePNodes(pnode->loopState);
                break;
            case PNNodes:
                freePNodes(pnode->gnodeExpr);
                stdfree(pnode->gnodeIden);
                freePNodes(pnode->loopState);
                break;
            case PNFamilies:
                freePNodes(pnode->personExpr);
                stdfree(pnode->familyIden);
                stdfree(pnode->spouseIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNSpouses:
                freePNodes(pnode->personExpr);
                stdfree(pnode->spouseIden);
                stdfree(pnode->familyIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNChildren:
                freePNodes(pnode->familyExpr);
                stdfree(pnode->childIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNIndis:
                stdfree(pnode->personIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNFams:
                stdfree(pnode->familyIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNSources:
                stdfree(pnode->sourceIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNEvents:
                stdfree(pnode->eventIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNOthers:
                stdfree(pnode->otherIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNList:
                freePNodes(pnode->listExpr);
                stdfree(pnode->elementIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNSequence:
                freePNodes(pnode->sequenceExpr);
                stdfree(pnode->elementIden);
                stdfree(pnode->valueIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNTable:
            case PNFathers:
                freePNodes(pnode->personExpr);
                stdfree(pnode->fatherIden);
                stdfree(pnode->familyIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNMothers:
                freePNodes(pnode->personExpr);
                stdfree(pnode->motherIden);
                stdfree(pnode->familyIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNFamsAsChild:
                freePNodes(pnode->personExpr);
                stdfree(pnode->familyIden);
                stdfree(pnode->countIden);
                freePNodes(pnode->loopState);
                break;
            case PNNotes:
                break;
        }
        pnode = pnode->next;
    }
}
