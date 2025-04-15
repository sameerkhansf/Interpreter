//
// Created by natem on 4/4/2025.
//

#include <cassert>
#include <iostream>
#include "AST.h"
#include <stack>
using namespace std;

AST::AST(CST *cst, symbolTable * ST)
{
    node * iter = cst->root();
    ASTnode * parent = nullptr, *child = nullptr, * _node = head;
    this->ST = ST;

    while (iter)
    {
        int origLineNum = lineNum;
        if (match(iter, "function") || (match(iter, "procedure")))
        {
            child = parseFunctionDeclaration(iter);
        }
        else if (isDatatype(iter))
        {
            child = parseVarDeclaration(iter);
        }
        else
        {

             nextNode(iter);
             printf("INVALID STATEMENT ENCOUNTERED\n");
        }

        insert(parent, _node, child, origLineNum);
    }
    head = parent;
}


void AST::insert(ASTnode *& parent,  ASTnode *& _node, ASTnode *& child, int origLineNum)
{
    if (child) {
        if (origLineNum == lineNum)
            insertSibling(parent, _node, child);
        else
            insertChild(parent, _node, child);
    }
}


void AST::insertChild(ASTnode *& parent, ASTnode *& curNode, ASTnode *& _node )
{
    if (!_node)
        return;

    if (!parent)
    {
        parent = _node;
        curNode = getLastNode(_node);
    }
    else
    {
        assert(curNode);
        curNode->child(_node);
        curNode = getLastNode(_node);
    }
    _node = nullptr;
}




void AST::insertSibling(ASTnode *& parent, ASTnode *& curNode, ASTnode *& _node )
{

    if (!_node)
        return;


    if (!parent)
    {
        parent = _node;
        curNode = getLastNode(_node);
    }
    else
    {
        assert(curNode);
        curNode->sibling(_node);
        curNode = getLastNode(_node);
    }
    _node = nullptr;
}


void AST::nextNode(node *& iter)
{
    if (iter) {
        if (iter->sibling())
            iter = iter->sibling();
        else {
            iter = iter->child();
            lineNum++;
        }
    }
}


bool AST::isDatatype(node * iter)
{
    if (iter)
    {
        if (iter->content() == "int" || iter->content() == "char" || iter->content() == "bool")
            return true;
    }
    return false;
}


bool AST::match(node * iter, const string& str)
{
    if (iter && iter->content() == str)
    {
        return true;
    }
    return false;
}


ASTnode * AST::parseVarDeclaration(node *& iter)
{
    int scope = iter->scope();
    iter = iter->sibling();
    string varName = iter->content();
    auto * declarationNode = new ASTnode(DECLARATION, ST->retrieveNode(varName, scope));

    parseToGivenChar(";", iter); // parse to ;
    nextNode(iter);
    return declarationNode;

}


void AST::parseToEndl(node *& iter)
{
    if (iter)
    {
        while (iter->sibling())
            iter = iter->sibling();
        iter = iter->child();
        lineNum++;
    }
}


ASTnode * AST::parseFunctionDeclaration(node *& iter)
{

    int originalLineNum = lineNum;
    int scope = iter->scope();

    if (iter->content() == "function")
        iter = iter->sibling(); // iterate to return type
    iter = iter->sibling(); // iterate to name

    string funcName = iter->content();
    auto * declarationNode = new ASTnode(DECLARATION, ST->retrieveNode(funcName, scope));
    auto * _node = declarationNode;

    parseToGivenChar(")", iter);
    nextNode(iter);

    auto * child = parseBlockStatement(iter);
    insert(declarationNode, _node, child, originalLineNum);

    return declarationNode;

}



ASTnode* AST::parseBlockStatement( node *& iter) {

    int origLineNum = lineNum;
    auto * parent = new ASTnode(BEGIN_BLOCK, nullptr);
    auto * _node = parent;
    nextNode(iter);

    ASTnode* body = parseCompoundStatement(iter);
    insert(parent, _node, body, origLineNum);

    auto * child = new ASTnode(END_BLOCK, nullptr);

    insert(parent, _node, child, origLineNum);
     nextNode(iter);


    return parent;
}


void AST::parseToGivenChar(const string& character, node *& iter)
{
    if (iter)
    {
        while (iter && iter->content() != character)
        {
            if (iter->sibling())
                iter = iter->sibling();
            else if (iter->child()) {
                iter = iter->child();
                lineNum++;
            }
            else
                break;
        }
    }
}


ASTnode* AST::parseCompoundStatement( node *& iter ) {

    int origLineNum = lineNum;
    ASTnode * parent = nullptr;
    ASTnode * _node = parent;
    ASTnode * child = nullptr;

    while (iter && iter->content()!= "}")
    {
        child = parseStatement(iter);
        insert(parent, _node, child, origLineNum);
        if (iter && iter->content() == ";")
        {
            nextNode(iter);
        }
    }

    return parent;
}


ASTnode* AST::getLastNode(ASTnode * root) {

    if (!root) return nullptr;

    auto * iter = root;

    while (iter) {

        while (iter->sibling())
            iter = iter->sibling();

        if (iter->child()) {
            iter = iter->child();
        } else {

            break;
        }
    }

    return iter;
}



ASTnode * AST::parseReturnStatement(node *& iter)
{

    int origLineNum = lineNum;
    auto * parent = new ASTnode("RETURN", nullptr);
    auto * _node = parent;
    iter = iter->sibling();
    auto * child = convertToPostFix(iter);
    insert(parent, _node, child, origLineNum);
    nextNode(iter);
    return parent;
}


ASTnode * AST::parseDoubleQuoteString(node *& iter)
{
    int origLineNum = lineNum;
    auto * parent = new ASTnode(iter->content(), nullptr);
    auto * _node = parent;
    ASTnode * child = nullptr;
    iter = iter->sibling();

    // get the inside string if it exists
    if (!match(iter, "\""))
    {
        child = new ASTnode(iter->content(), nullptr);
        insert(parent, _node, child, origLineNum);
        iter = iter->sibling();
    }

    child = new ASTnode(iter->content(), nullptr);
    insert(parent, _node, child, origLineNum);
    iter = iter->sibling();
    return parent;

}


ASTnode * AST::parseSingleQuoteString(node *& iter)
{
    int origLineNum = lineNum;
    auto * parent = new ASTnode(iter->content(), nullptr);
    auto * _node = parent;
    ASTnode * child = nullptr;
    iter = iter->sibling();

    // get the inside string if it exists
    if (!match(iter, "\'"))
    {
        child = new ASTnode(iter->content(), nullptr);
        insert(parent, _node, child, origLineNum);
        iter = iter->sibling();
    }

    child = new ASTnode(iter->content(), nullptr);
    insert(parent, _node, child, origLineNum);
    iter = iter->sibling();
    return parent;
}


ASTnode * AST::parsePrintfStatement(node *& iter)
{
    int origLineNum = lineNum;
    auto * parent = new ASTnode("printf", nullptr);
    auto * _node = parent;
    ASTnode * child = nullptr;
    iter = iter->sibling(); // iterate to (
    iter = iter->sibling(); // iterate past
    iter = iter->sibling(); // iterate past quote

    child = new ASTnode(iter->content(), nullptr); // catch string
    insert(parent, _node, child, origLineNum);
    iter = iter->sibling(); // iterate to end quote
    iter = iter->sibling(); // iterate past


    while (iter && iter->content() != ")")
    {
        if ((!match(iter, ",")) && (!match(iter, ",")))
        {
            child = new ASTnode(iter->content(), nullptr);
            insert(parent, _node, child, origLineNum);
        }
        iter = iter->sibling();

    }
    iter = iter->sibling();
    return parent;

}


ASTnode* AST::parseStatement(node *& iter) {


    if (match(iter, "if"))
    {
        return parseSelectionStatement(iter);
    }
    else if (match(iter, "{"))
    {
        return parseBlockStatement(iter);
    }
    else if (match(iter,  "while") ||
             match(iter,  "for"))
    {
        return parseIterationStatement(iter);
    }
    else if (match(iter,  "return"))
    {
        return parseReturnStatement(iter);
    }
    else if (match(iter,  "printf")) {
        return parsePrintfStatement(iter);
    }
    else if (isDatatype(iter)) {
        return parseVarDeclaration(iter);
    }
    else  {

        node * peek = iter->sibling();
        if (match(peek, "=") || match(peek, "["))
            return parseAssignmentStatement(iter);
        else
            return parseUserDefinedFunction(iter);
    }

}



ASTnode * AST::parseIterationStatement(node *& iter)
{
    int origLineNum = lineNum;

    ASTnode * parent = nullptr, * _node = parent, * child = nullptr;


    if (match(iter, "while")) {
        parent = new ASTnode("while", nullptr);
        _node = parent;
        iter = iter->sibling();

        ASTnode* condition = convertToPostFix(iter);
        insert(parent,_node, condition, origLineNum);

        nextNode(iter);

        if (match(iter, "{")) {
            ASTnode* block = parseBlockStatement(iter);
            insert(parent, _node, block, origLineNum);
        } else {
            ASTnode* stmt = parseStatement(iter);
            insert(parent,_node, stmt, origLineNum);
        }
        return parent;
    }

    if (match(iter, "for")) {

        iter = iter->sibling();

        parent = new ASTnode("FOR EXPRESSION 1", nullptr);
        _node = parent;
        iter = iter->sibling();

        ASTnode* init = convertToPostFix(iter);
        insert(parent, _node, init, origLineNum);
        lineNum++;


        child = new ASTnode("FOR EXPRESSION 2", nullptr);
        insert(parent, _node, child, origLineNum);
        iter = iter->sibling();
        origLineNum = lineNum;

        ASTnode* condition = convertToPostFix(iter);
        insert(parent,_node, condition, origLineNum);
        nextNode(iter);
        lineNum++;


        child = new ASTnode("FOR EXPRESSION 3", nullptr);
        insert(parent, _node, child, origLineNum);
        origLineNum = lineNum;

        ASTnode* update = convertToPostFix(iter);
        insert(parent,_node, update, origLineNum);
        origLineNum = lineNum;
        lineNum++;

        nextNode(iter);

        if (match(iter, "{")) {
            ASTnode* block = parseBlockStatement(iter);
            insert(parent,_node, block, origLineNum);
        } else {
            ASTnode* stmt = parseStatement(iter);
            insert(parent,_node, stmt, origLineNum);
        }

        return parent;
    }
    return nullptr; // should not get called
}



ASTnode * AST::parseUserDefinedFunction(node *& iter)
{
    int origLineNum = lineNum;
    string prev = iter->content();
    auto * parent = new ASTnode(iter->content(), ST->retrieveNode(prev, iter->scope()));
    auto * _node = parent;
    ASTnode * child = nullptr;
    iter = iter->sibling();

    while (iter && prev != ")") {
        child = new ASTnode(iter->content(), nullptr);
        insert(parent, _node, child, origLineNum);
        prev = iter->content();
        iter = iter->sibling();
    }
    return parent;
}


ASTnode * AST::parseAssignmentStatement(node *& iter)
{

    auto * assignmentNode = new ASTnode(ASSIGNMENT, ST->retrieveNode(iter->content(), iter->scope()));
    auto * _node = assignmentNode;
    auto * child = convertToPostFix(iter);
    insert(assignmentNode, _node, child, lineNum);

    return assignmentNode;
}


ASTnode * AST::parseSelectionStatement(node *& iter)
{
    int origLineNum = lineNum;

    auto * parent = new ASTnode("if", nullptr);
    auto * _node = parent;
    ASTnode* child = nullptr;
    iter = iter->sibling();

    child = convertToPostFix(iter);
    insert(parent, _node, child, origLineNum);
    origLineNum = lineNum;
    nextNode(iter);


    if (match(iter, "{")) {
        ASTnode* block = parseBlockStatement(iter);
        insert(parent, _node, block ,origLineNum);
    }
    else {
        ASTnode* stmt = parseStatement(iter);
        insert(parent,_node, stmt,origLineNum);
    }


    if (match(iter, "else")) {

        child = new ASTnode("else", nullptr);
        insert(parent, _node, child, origLineNum);
        origLineNum = lineNum;
        nextNode(iter);


        if (match(iter, "{")) {
            ASTnode* block = parseBlockStatement(iter);
            insert(parent,_node, block,origLineNum);
        } else {
            ASTnode* stmt = parseStatement(iter);
            insert(parent,_node, stmt,origLineNum);
        }
    }

    return parent;
}


bool AST::isOperator(node * iter)
{
    return (iter->content() == "+" || iter->content() == "-" || iter->content() == "*" ||
            iter->content() == "/" || iter->content() == "%" || iter->content() == "!" ||
            iter->content() == "&&" || iter->content() == "||" || iter->content() == "==" ||
            iter->content() == "!=" || iter->content() == "<="  || iter->content() == ">=" ||
            iter->content() == "<" || iter->content() == ">" || iter->content() == "=" );
}


bool AST::isOpString(const string& iter)
{
    return (iter == "+" || iter == "-" || iter == "*" ||
            iter == "/" || iter == "%" || iter == "!" ||
            iter == "&&" || iter == "||" || iter == "==" ||
            iter == "!=" || iter == "<="  || iter == ">=" ||
            iter == "<" || iter == ">" || iter == "=" );
}


int AST::precedence(const string& op)
{
    if (op == "!")
        return 1;
    else if (op == "*" || op == "/" || op == "%")
        return 2;
    else if (op == "+" || op == "-")
        return 3;
    else if (op == "<" || op == ">" || op == ">=" || op == "<=")
        return 4;
    else if (op == "==" || op == "!=")
        return 5;
    else if (op == "&&")
        return 6;
    else if (op == "||")
        return 7;
    else
        return 8; // assignment
}


bool AST::isLeftAssociative(const string& op)
{
    if (op == "!" || op == "=" || op == "+=" || op == "-=") {
        return false;
    }
    return true;
}



ASTnode * AST::convertToPostFix(node *& iter)
{
    stack<string> opStack;
    ASTnode * parent = nullptr, *child = nullptr;
    auto * _node = parent;

    int origLineNum = lineNum;
    while (iter && iter->sibling() && !match(iter, "{") && !match(iter, ";"))
    {

        if (!isOperator(iter) && !match(iter, "(") && !match(iter, ")") )
        {

            if (match(iter, "'"))
            {
                child = parseSingleQuoteString(iter);
                insert(parent, _node, child, origLineNum);
                continue;
            }
            else if (match(iter, "\""))
            {
                child = parseDoubleQuoteString(iter);
                insert(parent, _node, child, origLineNum);
                continue;
            }
            else
                child = new ASTnode(iter->content(), nullptr);
            insert(parent, _node, child, origLineNum);

            // user - defined function
            string prev;
            auto * peek = iter->sibling();
            if (match(peek, "("))
            {
                iter = iter->sibling();
                while (iter && prev != ")") {
                    child = new ASTnode(iter->content(), nullptr);
                    insert(parent, _node, child, origLineNum);
                    prev = iter->content();
                    iter = iter->sibling();
                }
                continue;

            }
        }
        else if (isOperator(iter))
        {
            string curOp = iter->content();
            int curPrec = precedence(curOp);

            while (!opStack.empty() && isOpString(opStack.top()) &&
            (curPrec > precedence(opStack.top()) || (curPrec == precedence(opStack.top()) && isLeftAssociative(curOp)) ))
            {
                child = new ASTnode(opStack.top(), nullptr);
                insert(parent, _node, child, origLineNum);
                opStack.pop();
            }
            opStack.push(curOp);
        }
        else if (match(iter, "("))
        {
            opStack.emplace("(");
        }
        else if (match(iter, ")"))
        {

            while (opStack.top() != "(")
            {
                child = new ASTnode(opStack.top(), nullptr);
                insert(parent, _node, child, origLineNum);
                opStack.pop();
            }
            opStack.pop();
        }
        nextNode(iter);
        origLineNum = lineNum;
    }

    while (!opStack.empty())
    {
        if (opStack.top() != "(" && opStack.top() != ")") {
            child = new ASTnode(opStack.top(), nullptr);
            insert(parent, _node, child, origLineNum);
        }
        opStack.pop();
    }
    return parent;

}



/***
 * This outputs the contents of a CST in a human-readable way
 * @param outputFile file to hold the contents of the CST
 */
void AST::output(const string& outputFile)
{
    ofstream outFS;
    outFS.open(outputFile);

    if (!outFS.is_open())
    {
        std::cout << "ERROR OPENING " << outputFile << endl;
        exit(3);
    }

    traverseAST(head,outFS);
    if (head)
        outFS << "null";

    outFS.close();

}



#include <iomanip>
#define ORIGINAL_LENGTH 1
int lineWid = ORIGINAL_LENGTH;


/***
 * This traverse the AST and outputs the tokens in their tree
 * structure
 * @param head root node of the tree
 * @param outFS output file stream
 */
void AST::traverseAST(ASTnode * head, ofstream& outFS)
{
    if (!head)
        return;
    outFS << "[" << head->content() << "]->";
    if (head->sibling())
        lineWid += head->content().size() + 4;
    traverseAST(head->sibling(), outFS);

    if (head->child())
    {
        outFS << "null\n\n" << setw(lineWid);
        traverseAST(head->child(), outFS);
        lineWid = ORIGINAL_LENGTH;

    }

}
