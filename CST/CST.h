//
// Created by natem on 2/28/2025.
//

#ifndef ASSIGNMENT3_CST_H
#define ASSIGNMENT3_CST_H
#include "../tokenize/token.h"
#include "../symbolTable/symbolTable.h"
#include "node.h"
#include <fstream>

#define IDENTIFIER "IDENTIFIER"
#define STRING "STRING"
#define INTEGER "INTEGER"
#define CHAR "CHAR"

void traverseCST(node * head, ofstream& outFS); // used to output the CST

class CST
{
public:

    // constructor/destructor
    CST(token * head, const string& outputFile, symbolTable * ST);
    ~CST() { deleteNodes(head); }

    // insertion/traversal functions
    void insert(node *& parent, node *& curNode, node *& _node, node * lastNode, int origLineNum);
    void insertSibling(node *& parent, node *& curNode, node *& _node, node * lastNode);
    void insertChild(node *& parent, node *& curNode, node *& _node, node * lastNode);
    void outputTraversal(const string& outputFile);

    // parsing functions
    node * parse(token *& iter, ofstream&);
    node * parseSingleQuoteString(token *& iter, ofstream&);
    node * parseDoubleQuoteString(token *& iter, ofstream&);
    node * parseFunction(token *& iter, ofstream&);
    node * parseNumericalExpression(token *& iter, ofstream&);
    node * parseBooleanExpression(token *& iter, ofstream&);
    node * parseInitializationExpression(token *& iter, ofstream&);
    node * parseExpression(token *& iter, ofstream&);
    node * parseSelectionStatement(token *& iter, ofstream&);
    node * parseIterationStatement(token *& iter, ofstream&);
    node * parseAssignmentStatement(token *& iter, ofstream&);
    node * parsePrintfStatement(token *& iter, ofstream&);
    node * parseGetCharFunction(token *& iter, ofstream&);
    node * parseUserDefinedFunction(token *& iter, ofstream&);
    node * parseDeclarationStatement(token *& iter, ofstream&);
    node * parseReturnStatement(token *& iter, ofstream&);
    node * parseStatement(token *& iter, ofstream&);
    node * parseCompoundStatement(token *& iter, ofstream&);
    node * parseBlockStatement(token *& iter, ofstream&);
    node * parseParameterList(token *& iter, ofstream&);
    node * parseProcedureDeclaration(token *& iter, ofstream&);
    node * parseRelationalStatement(token *& iter, ofstream&);
    node * parseIdentifierAndIdentifierArrayList(token *& iter, ofstream&);
    node * parseIncrementStatement(token *& iter, ofstream&);
    node* parseFactor(token *& iter, ofstream&);
    bool match(token *& iter, const string& expected);
    node* parseTerm(token *& iter, ofstream&);
    bool matchType(token *& iter, const string& expected);
    bool isDatatype(token *& iter, ofstream&);
    static bool isBooleanOperator(token *& iter);
    node * getLastSibling(node *_node);
    void parseEndlToken(token *& iter);
    node * getLastNode(node * _node);
    void deleteNodes(node * _node);
    token * parseUntilEndL(token * iter);
    bool hasErrors() { return errorDetected; }
    symbolTable * createPL(node * iter, ofstream& outFS);
    node * root() { return head; }

private:
    node * head;
    string curFunction;
    int lineNum = 1;
    int curScope = 0;
    int prevScope = 0;
    bool errorDetected = false;
    symbolTable * ST;

};




#endif //ASSIGNMENT3_CST_H
