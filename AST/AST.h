//
// Created by natem on 4/4/2025.
//

#ifndef ASSIGNMENT4_AST_H
#define ASSIGNMENT4_AST_H
#include "../CST/CST.h"
#include "ASTnode.h"


#define ASSIGNMENT "ASSIGNMENT"
#define BEGIN_BLOCK "BEGIN_BLOCK"
#define END_BLOCK "END_BLOCK"
#define DECLARATION "DECLARATION"


class AST {
public:
    AST(CST * cst, symbolTable * ST);

    void nextNode(node *& iter);
    void insert(ASTnode *& parent,  ASTnode *& _node, ASTnode *& child,  int origLineNum);
    void insertSibling(ASTnode *& parent, ASTnode *& _node, ASTnode *& child);
    void insertChild(ASTnode *& parent, ASTnode *& _node, ASTnode *& child);
    void parseToGivenChar(const string& character, node *& iter);
    void traverseAST(ASTnode * head, ofstream& outFS);
    void output(const string& output);

    static bool match(node * iter, const string& str);
    static bool isDatatype(node * iter);
    static bool isOperator(node * iter);
    static bool isOpString(const string& iter);
    static bool isLeftAssociative(const string& iter);
    static int precedence(const string& op);

    ASTnode * parseVarDeclaration(node *& iter);
    ASTnode * parseSelectionStatement(node *& iter);
    ASTnode * parseFunctionDeclaration(node *& iter);
    ASTnode * parseBlockStatement( node *& iter);
    ASTnode * parseCompoundStatement( node *& iter );
    ASTnode * parseStatement(node *& iter);
    ASTnode * parseAssignmentStatement(node *& iter);
    ASTnode * getLastNode(ASTnode * root);
    ASTnode * convertToPostFix(node *& iter);
    ASTnode * parseUserDefinedFunction(node *& iter);
    ASTnode * parseReturnStatement(node *& iter);
    ASTnode * parsePrintfStatement(node *& iter);
    ASTnode * parseString(node *& iter);
    ASTnode * parseIterationStatement(node *& iter);


private:
    ASTnode * head;
    symbolTable * ST;
    int lineNum = 1;
};


#endif //ASSIGNMENT4_AST_H
