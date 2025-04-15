//
// Originally created by Nathan Mailloux on 2/28/2025.
//


#include "CST.h"
#include <cassert>
#include <iostream>
#include <utility>
#include <fstream>
using namespace std;


CST::CST(token * tokenHead, const string& outputFile, symbolTable * ST)
{
    ofstream outFS;
    this->ST = ST;

    // clear the file
    outFS.open(outputFile);
    outFS.close();

    outFS.open(outputFile);

    if (!outFS.is_open())
        throw runtime_error("CST FILE DID NOT OPEN\n");

    head = nullptr;
    head = parse(tokenHead, outFS);

    outFS.close();

}


void CST::insert(node *& parent, node *& curNode, node *& _node, node * lastNode, int origLineNum)
{
    if (_node) {
        if (origLineNum == lineNum)
            insertSibling(parent, curNode, _node, lastNode);
        else
            insertChild(parent, curNode, _node, lastNode);
    }
}



void CST::insertSibling(node *& parent, node *& curNode, node *& _node, node * lastNode)
{

    if (!_node)
        return;


    if (!parent)
    {
        parent = _node;
        curNode = lastNode;
    }
    else
    {
        assert(curNode);
        curNode->sibling(_node);
        curNode = lastNode;
    }
    _node = nullptr;
}


void CST::insertChild(node *& parent, node *& curNode, node *& _node, node * lastNode)
{

    if (!_node)
        return;

    if (!parent)
    {
        parent = _node;
        curNode = lastNode;
    }
    else
    {
        assert(curNode);
        curNode->child(_node);
        curNode = lastNode;
    }
    _node = nullptr;
}


void CST::deleteNodes(node * _node)
{

    node * temp = nullptr;
    while (_node)
    {
        temp = _node;
        if (_node->sibling())
        {
            _node = _node->sibling();
        }
        else if (_node->child())
        {
            _node = _node->child();
        }
        else
        {
            break;
        }
        delete temp;
    }
    delete _node;
}



token * CST::parseUntilEndL(token * iter)
{
    if (!iter)
        return iter;
    while (iter && (!iter->isEndlToken() || !iter->isScope()))
        iter = iter->next();
    return iter;
}


node* CST::getLastNode(node* root) {

    if (!root) return nullptr;

    node* iter = root;

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


token * lastToken(token * TL)
{
    while (TL && TL->next())
        TL = TL->next();
    return TL;
}


node * CST::parse(token *& iter, ofstream& outFS) {

    if (!iter)
        return nullptr;

    node * parent = nullptr, *_node = parent, * child = nullptr;
    int origLineNum = lineNum;

    while (iter)
    {
        origLineNum = lineNum;
        if (iter->isEndlToken())
        {
            if (iter->next()) {
                parseEndlToken(iter);
            }
            else {
                iter = iter->next();
                lineNum++;
            }
            continue;
        }
        if (match(iter, "procedure"))
            child = parseProcedureDeclaration(iter, outFS);
        else if (match(iter, "function"))
            child = parseFunction(iter, outFS);
        else if (isDatatype(iter, outFS))
            child =  parseDeclarationStatement(iter, outFS);
        else
        {
            errorDetected = true;
            outFS << "Syntax error on line " << lineNum << ": INVALID STATEMENT";
        }

        insert(parent, _node, child, getLastNode(child), origLineNum);
        outFS.flush();
        curScope = 0;

    }

    return parent;

}


node * CST::parseSingleQuoteString(token *& iter, ofstream& outFS) {

    if (!iter)
        return nullptr;
    int origLineNum = lineNum;

    if (!matchType(iter, "SINGLE_QUOTE"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED SINGLE QUOTE\n";}

    auto * parent = new node("\'", curScope);
    node * _node = parent;
    iter = iter->next();

    if (!matchType(iter, STRING) && !matchType(iter, STRING) &&
        !matchType(iter, "SINGLE_QUOTE"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED STRING/END QUOTE\n";}

    if (matchType(iter, STRING)) {
        auto *child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();
    }
    if (!match(iter, "\'"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED END QUOTE\n";}

    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);


    iter = iter->next();
    return parent;


}


void CST::parseEndlToken(token *& iter) {

    while (iter && (iter->isScope() || iter->isEndlToken())) {
        if (iter->isEndlToken())
            lineNum++;
        iter = iter->next();
    }

}


node * CST::parseAssignmentStatement(token *& iter, ofstream& outFS)
{

    // symbol table parameters
    string idName, idType = "datatype", datatype = "unknown";
    int size = 0;
    bool isArr = false;

    int origLineNum = lineNum;
    if (!matchType(iter, IDENTIFIER))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected identifier\n";}

    idName = iter->content();
    auto * parent = new node(iter->content(), curScope);
    node * child, * _node = parent;
    iter = iter->next();

    if (match(iter, "[")) {

        isArr = true;
        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);

        iter = iter->next();

        if (!matchType(iter, INTEGER))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected array index as integer\n";}

        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        if (!match(iter, "]"))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected closing ']' for array declaration\n";}

        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();
    }

//    // ensure the variable has been declared using a temporary ST node
//    auto * test = new stNode(idName, idType, datatype, isArr, size, curScope);
//    int foundScope;
//    if (!ST->inUse(test, foundScope))
//    {
//       errorDetected = true;
//       outFS << "Error on line " << lineNum << ": variable \"" << idName << "\" has not been declared\n";
//    }
//    delete test;

    if (!match(iter, "="))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ASSIGNMENT OPERATOR\n";}

    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    if (match(iter, "\""))
    {
        child = parseDoubleQuoteString(iter, outFS);
    }
    else if (match(iter, "'"))
    {
        child = parseSingleQuoteString(iter, outFS);
    }
    else if (matchType(iter, INTEGER))
    {
        child = parseExpression(iter, outFS);
    }
    else if (match(iter, "getChar"))
    {
        child = parseGetCharFunction(iter, outFS);
    }
    else if (matchType(iter, IDENTIFIER))
    {

        token * peek = iter->next();
        if (match(peek, "(")) {
            child = parseUserDefinedFunction(iter, outFS);
            insert(parent, _node, child , getLastNode(child), origLineNum);
            return parent;
        }
        else
            child = parseExpression(iter, outFS);
    }
    else
    {
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": invalid assignment statement\n";}
    }

    insert(parent, _node, child, getLastNode(child), origLineNum);

    if (!match(iter, ";"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected SEMICOLON\n";}

    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    parseEndlToken(iter);

    return parent;

}


node * CST::parseIncrementStatement(token *& iter, ofstream& outFS)
{
    int origLineNum = lineNum;
    if (!matchType(iter, IDENTIFIER))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected identifier\n";}

    auto * parent = new node(iter->content(), curScope);
    node * child, * _node = parent;
    iter = iter->next();

    if (!match(iter, "="))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ASSIGNMENT OPERATOR\n";}

    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();


    if (matchType(iter, INTEGER))
    {
        child = parseExpression(iter, outFS);
    }
    else if (matchType(iter, IDENTIFIER))
    {
        token * peek = iter->next();
        if (match(peek, "(")) {
            child = parseUserDefinedFunction(iter, outFS);
            insert(parent, _node, child , getLastNode(child), origLineNum);
            return parent;
        }
        else
            child = parseExpression(iter, outFS);
    }
    else
    {
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": invalid assignment statement\n";}
    }

    insert(parent, _node, child, getLastNode(child), origLineNum);

    return parent;

}


node * CST::parseDoubleQuoteString(token *& iter, ofstream& outFS) {

    int origLineNum = lineNum;
    if (!match(iter, "\""))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED DOUBLE QUOTE\n";}

    auto * parent = new node(iter->content(), curScope);
    node * _node = getLastNode(parent);
    iter = iter->next();


    if (!matchType(iter, STRING) && !matchType(iter, "DOUBLE_QUOTE"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED DOUBLE STRING/END QUOTE\n";}

    if (matchType(iter, STRING)) {
        auto *child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();
    }

    if (!match(iter, "\""))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": NO ENDING DOUBLE QUOTE\n";}

    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    return parent;

}


node* CST::parseCompoundStatement( token *& iter, ofstream& outFS) {

    int origLineNum = lineNum;
    if (!iter)
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": NO STATEMENT IN BRACKETS\n";}

    node* parent = nullptr;
    node* _node = parent;
    node * child = nullptr;

    while (iter && iter->content()!= "}") {
        child = parseStatement(iter, outFS);
        insert(parent, _node, child, getLastNode(child), origLineNum);
//        if (!match(iter, "}"))
//        {
//            origLineNum = lineNum;
//        }
        parseEndlToken(iter);
    }

    return parent;
}



node* CST::parseBlockStatement( token *& iter, ofstream& outFS) {

    int origLineNum = lineNum;
    if (!match(iter, "{"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected '{' to start block\n";}

    node* parent = new node(iter->content(), curScope);
    node* _node = parent;
    node* child;
    iter = iter->next();

    parseEndlToken(iter);

    node* body = parseCompoundStatement(iter, outFS);
    insert(parent, _node, body, getLastNode(body), origLineNum);

    parseEndlToken(iter);


    if (!match(iter, "}"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected '}' to close block\n";}

    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    return parent;
}



node* CST::parseStatement(token *& iter, ofstream& outFS) {

    int origLineNum = lineNum;
    if (!iter)
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": STATEMENT EXPECTED BUT IS NULL\n";}
    if (match(iter, "if"))
    {
        return parseSelectionStatement(iter, outFS);
    }
    else if (match(iter, "{"))
    {
        return parseBlockStatement(iter, outFS);
    }
    else if (match(iter,  "while") ||
             match(iter,  "for"))
    {
        return parseIterationStatement(iter, outFS);
    }
    else if (match(iter,  "return"))
    {
        return parseReturnStatement(iter, outFS);
    }
    else if (match(iter,  "printf")) {
        return parsePrintfStatement(iter, outFS);
    }
    else if (isDatatype(iter, outFS)) {
        return parseDeclarationStatement(iter, outFS);
    }
    else if (matchType(iter, IDENTIFIER)) {
        token * peek = iter->next();
        if (match(peek, "=") || match(peek, "["))
            return parseAssignmentStatement(iter, outFS);
        else
            return parseUserDefinedFunction(iter, outFS);
    }
    else if (iter->isEndlToken() || iter->isScope())
    {
        parseEndlToken(iter);
        return parseStatement(iter, outFS);
    }
    else {

        if (iter)
            iter = iter->next();
    }
    return nullptr;
}


node* CST::parsePrintfStatement( token *& iter, ofstream& outFS) {

    int origLineNum = lineNum;
    if (!match(iter, "printf"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected 'printf'\n";}

    node* parent = new node(iter->content(), curScope);
    node* _node = getLastNode(parent);
    node* child;
    iter = iter->next();


    if (!match(iter, "("))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected '(' after 'printf'\n";}
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();


    if (match(iter, "\"")) {
        node* str = parseDoubleQuoteString(iter, outFS);
        insert(parent,_node, str, getLastNode(str), origLineNum);
    }
    else if (match(iter, "'")) {
        node* str = parseSingleQuoteString(iter, outFS);
        insert(parent,_node, str, getLastNode(str), origLineNum);
    }
    else {
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected a quoted string after printf\n";}
    }


    if (match(iter, ",")) {
        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        node* args = parseIdentifierAndIdentifierArrayList(iter, outFS);
        insert(parent,_node, args, getLastNode(args), origLineNum);
    }


    if (!match(iter, ")"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ')' after printf arguments\n";}
    child = new node(")",curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();


    if (!match(iter, ";"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ';' after printf statement\n";}
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    return parent;
}


node* CST::parseIdentifierAndIdentifierArrayList( token *& iter, ofstream& outFS) {

    int origLineNum = lineNum;
    if (!matchType(iter, IDENTIFIER))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected identifier\n";}

    node* parent = new node(iter->content(), curScope);
    node* _node = getLastNode(parent);
    node* child;
    iter = iter->next();

    while (iter) {

        if (match(iter, "["))
        {
            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            if (!matchType(iter, INTEGER) && !match(iter, IDENTIFIER))
            { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected integer/variable in array brackets\n";}

            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            if (!match(iter, "]"))
            { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected closing ']'\n";}

            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();
        }


        if (match(iter, ","))
        {
            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            if (!matchType(iter, IDENTIFIER))
            { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": NO IDENTIFIER AFTER COMMA\n";}

            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

        }
        else
        {
            break;
        }
    }

    return parent;
}


node* CST::parseIterationStatement( token *& iter, ofstream& outFS) {

    int origLineNum = lineNum;
    if (!iter)
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Unexpected end of input in iteration statement\n";}

    node* parent = nullptr;
    node* _node = nullptr;
    node* child = nullptr;

    if (match(iter, "while")) {
        parent = new node("while", curScope);
        _node = parent;
        iter = iter->next();

        if (!match(iter, "("))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected '(' after 'while'\n";}

        child = new node("(", curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        node* condition = parseBooleanExpression(iter, outFS);
        insert(parent,_node, condition, getLastNode(condition), origLineNum);

        if (!match(iter, ")") && iter && !iter->isEndlToken())
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ')' after while condition\n";}

        if (!iter->isEndlToken()) {
            child = new node(")", curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();
        }
        origLineNum = lineNum;
        parseEndlToken(iter);

        if (match(iter, "{")) {
            node* block = parseBlockStatement(iter, outFS);
            insert(parent, _node, block, getLastNode(block), origLineNum);
        } else {
            node* stmt = parseStatement(iter, outFS);
            insert(parent,_node, stmt, getLastNode(stmt), origLineNum);
        }

        return parent;
    }

    if (match(iter, "for")) {
        parent = new node("for", curScope);
        _node = parent;
        iter = iter->next();

        if (!match(iter, "("))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected '(' after 'for'\n";}

        child = new node("(", curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        node* init = parseInitializationExpression(iter, outFS);
        insert(parent, _node, init, getLastNode(init), origLineNum);

        if (!match(iter, ";"))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ';' after initialization\n";}
        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        node* condition = parseBooleanExpression(iter, outFS);
        insert(parent,_node, condition, getLastNode(condition),origLineNum);

        if (!match(iter, ";"))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected second ';' in for-loop\n";}
        child = new node(";", curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        node* update = parseIncrementStatement(iter, outFS);
        insert(parent,_node, update, getLastNode(update),origLineNum);

        if (!match(iter, ")"))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ')' after for-loop header\n";}
        child = new node(")", curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        if (match(iter, "{")) {
            node* block = parseBlockStatement(iter, outFS);
            insert(parent,_node, block, getLastNode(block),origLineNum);
        } else {
            node* stmt = parseStatement(iter, outFS);
            insert(parent,_node, stmt, getLastNode(stmt),origLineNum);
        }

        return parent;
    }
    else
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected 'for' or 'while' at start of iteration statement\n";
    }
    return nullptr;
}


node* CST::parseInitializationExpression( token *& iter, ofstream& outFS) {

    int origLineNum = lineNum;
    parseEndlToken(iter);

    if (!matchType(iter, IDENTIFIER))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected identifier in initialization expression\n";}

    node * parent = new node(iter->content(), curScope);
    node * _node = getLastNode(parent);
    iter = iter->next();


    if (!match(iter, "="))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected '=' in initialization expression\n";}

    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();


    if (match(iter, "\"")) {
        child = parseDoubleQuoteString(iter, outFS);
    }

    else if (match(iter, "'")) {
        child = parseSingleQuoteString(iter, outFS);
    }

    else {
        child = parseExpression(iter, outFS);
    }

    insert(parent, _node, child, getLastNode(child), origLineNum);
    return parent;
}



node* CST::parseReturnStatement( token *& iter, ofstream& outFS)
{
    int origLineNum = lineNum;
    parseEndlToken(iter);
    if (!match(iter, "return"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected 'return'\n";}

    node* parent = new node(iter->content(), curScope);
    node* _node = parent, *child;
    iter = iter->next();

    if (!iter)
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Unexpected end of input after 'return'\n";}


    if (match(iter, "'")) {
        child = parseSingleQuoteString(iter, outFS);
    }

    else if (match(iter, "\"")) {
        child = parseDoubleQuoteString(iter, outFS);
    }

    else {
        child = parseExpression(iter, outFS);
    }
    insert(parent, _node, child, getLastNode(child), origLineNum);


    if (!match(iter, ";"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ';' at end of return statement\n";}

    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    parseEndlToken(iter);

    return parent;
}


node * CST::parseDeclarationStatement(token *& iter, ofstream& outFS)
{
    // symbol table parameters
    string idName, idType = "datatype", datatype;
    bool isArr = false;
    int arrSize = 0;


    int origLineNum = lineNum;
    if (!isDatatype(iter, outFS))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected datatype\n";}

    auto *parent = new node(iter->content(), curScope);
    datatype = iter->content(), curScope;
    node *_node = parent, *child = nullptr;
    iter = iter->next();


    while (true)
    {
        if (!matchType(iter, IDENTIFIER)) {
            { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected identifier in declaration\n";}
        }

        idName = iter->content(), curScope;
        node *idNode = new node(iter->content(), curScope);
        insert(parent,_node, idNode, getLastNode(idNode),origLineNum);
        iter = iter->next();


        if (match(iter, "[")) {

            isArr = true;

            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            if (!matchType(iter, INTEGER) ||
                ((matchType(iter, INTEGER) && iter->content()[0] == '-')))
            { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected array size as positive integer\n";}

            arrSize = stoi(iter->content());

            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            if (!match(iter, "]"))
            { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected closing ']' for array declaration\n";}

            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();
        }

        auto * variable = new stNode(idName, idType, datatype, isArr, arrSize, curScope);
        int foundScope;
        if (ST->inUse(variable, foundScope))
        {
            outFS << "Error on line " << lineNum << ": variable \"" << idName << "\" is already defined ";
            if (foundScope == 0)
                outFS << "globally\n";
            else
                outFS << "locally\n";
            errorDetected = true;
            delete variable;
        }
        else if (checkReserveWord(variable->idName(), VAR_RESERVED_WORDS))
        {
            outFS << "Syntax Error on line " << lineNum << ": the name " << idName << " cannot be used as it\n";
            outFS << " is a reserve word\n";
            errorDetected = true;
            delete variable;
        }
        else
            ST->insert(variable);


        if (match(iter, ",")) {
            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            isArr = false;
            arrSize = 0;
            continue;

        }

        break;

    }
    if (!match(iter, ";"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected semicolon to end declaration statement\n";}

    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    return parent;
}


node* CST::parseSelectionStatement( token *& iter, ofstream& outFS)
{

    int origLineNum = lineNum;
    if (!match(iter, "if"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected 'if' at start of selection statement\n";}

    auto* parent = new node(iter->content(), curScope);
    auto* _node = parent;
    node * child = nullptr;
    iter = iter->next();

    node* condition = parseBooleanExpression(iter, outFS);
    insert(parent,_node, condition, getLastNode(condition),origLineNum);
    origLineNum = lineNum;
    parseEndlToken(iter);


    if (match(iter, "{")) {
        node* block = parseBlockStatement(iter, outFS);
        insert(parent,_node, block, getLastNode(block),origLineNum);
    }
    else {
        node* stmt = parseStatement(iter, outFS);
        insert(parent,_node, stmt, getLastNode(stmt),origLineNum);
    }

    origLineNum = lineNum;
    parseEndlToken(iter);


    if (match(iter, "else")) {

        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        origLineNum = lineNum;
        parseEndlToken(iter);

        if (match(iter, "{")) {
            node* block = parseBlockStatement(iter, outFS);
            insert(parent,_node, block, getLastNode(block),origLineNum);
        } else {
            node* stmt = parseStatement(iter, outFS);
            insert(parent,_node, stmt, getLastNode(stmt),origLineNum);
        }
    }

    return parent;
}


node * CST::parseProcedureDeclaration(token *& iter, ofstream& outFS)
{


    string idName, idType = "procedure", datatype = "NOT APPLICABLE";
    prevScope++;
    curScope = prevScope;

    int origLineNum = lineNum;
    if (!match(iter, "procedure"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED PROCEDURE\n";}

    auto * parent = new node(iter->content(), curScope);
    node * _node = getLastNode(parent);
    iter = iter->next();


    if (!matchType(iter, IDENTIFIER))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED IDENTIFIER\n";}

    curFunction = idName = iter->content(), curScope;

    if (checkReserveWord(idName, FUNCTION_SPECIFIC_RESERVES))
    {
        outFS << "Syntax error on line " << lineNum << ": " << idName << " can not be used as the name of a procedure\n";
        errorDetected = true;
    }

    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);

    iter = iter->next();

    auto * procedure = new stNode(idName, idType, datatype, curScope);
    ST->insert(procedure);


    if (!match(iter, "("))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED (\n";}
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);

    iter = iter->next();

    auto * paramList = parseParameterList(iter, outFS);
    if (!paramList && !match(iter, "void") )
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED void\n";}

    symbolTable * PL = nullptr;
    if (paramList) {
        child = paramList;
        PL = createPL(paramList, outFS);
        ST->insertST(PL);
    }
    else
        child = new node(iter->content(), curScope);

    insert(parent, _node, child, getLastNode(child), origLineNum);
    if (!paramList)
        iter = iter->next();

    if (!match(iter, ")"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED )\n";}

    child = new node(")", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    origLineNum = lineNum;
    parseEndlToken(iter);
    child = parseBlockStatement(iter, outFS);
    insert(parent, _node, child, getLastNode(child), origLineNum);

    return parent;

}


node* CST::parseGetCharFunction( token *& iter, ofstream& outFS)
{
    int origLineNum = lineNum;
    if (!match(iter, "getchar"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected 'getchar'\n";}

    node* parent = new node(iter->content(), curScope);
    node* _node = getLastNode(parent);
    node* child;
    iter = iter->next();

    if (!match(iter, "("))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected '(' after 'getchar'\n";}
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();


    if (!matchType(iter, IDENTIFIER))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected identifier inside 'getchar(...)'\n";}
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();


    if (!match(iter, ")"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ')' after 'getchar(...)'\n";}

    child = new node(")", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    return parent;
}


bool CST::isBooleanOperator(token *&iter) {

    return (iter && (iter->content() == "<=" || iter->content() == ">=" ||
                    iter->content() == "==" || iter->content() == "<" || iter->content()== ">" ||
                    iter->content() == "&&" || iter->content() == "!=" || iter->content()== "||"));

}



node * CST::parseParameterList(token *& iter, ofstream& outFS)
{
    int origLineNum = lineNum;
    if (!isDatatype(iter, outFS)) {
        return nullptr;
    }

    auto * parent = new node(iter->content(), curScope);
    auto * _node = parent;

    iter = iter->next();

    if (!matchType(iter, IDENTIFIER))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED IDENTIFIER\n";}

    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    if (match(iter, "[")) {


        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        if (!matchType(iter, INTEGER))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected array size as integer\n";}

        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        if (!match(iter, "]"))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected closing ']' for array declaration\n";}

        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();
    }

    if (match(iter, ","))
    {
        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();
        child = parseParameterList(iter, outFS);
        insert(parent, _node, child, getLastNode(child), origLineNum);


    }
    return parent;

}


bool CST::isDatatype(token *& iter, ofstream& outFS)
{
    if (!match(iter, "char") && !match(iter, "bool")
        && !match(iter, "int"))
        return false;
    return true;
}


node* CST::parseBooleanExpression( token *& iter, ofstream& outFS) {

    node* parent = nullptr;
    node * _node = nullptr, * child = nullptr;
    int origLineNum = lineNum;


    if (match(iter, "(")) {

        parent = new node("(", curScope);
        _node = parent;
        iter = iter->next();

        child = parseBooleanExpression(iter, outFS);
        insert(parent, _node, child, getLastNode(child), origLineNum);

        if (iter && !iter->isEndlToken() && !match(iter, ";") && !match(iter, ")"))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": MISSING CLOSING PARENTHESES\n";}

        if (!match(iter, ";")) {
            child = new node(")", curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();
        }


    }
    else if (match(iter, "TRUE") || match(iter, "FALSE")) {
        parent = new node( iter->content(), curScope);
        iter = iter->next();
    }
    else if (match(iter, "'"))
    {
        parent = parseSingleQuoteString(iter, outFS);

    }
    else if (match(iter, "\""))
    {
        parent = parseDoubleQuoteString(iter, outFS);
    }
    else {
        parent = parseNumericalExpression(iter, outFS);
    }
    _node = getLastNode(parent);


    while (isBooleanOperator(iter)) {

        node* op = new node(iter->content(), curScope);
        insert(parent,_node, op, getLastNode(op),origLineNum);
        iter = iter->next();

        node* right = parseBooleanExpression(iter, outFS);
        insert(parent,_node, right, getLastNode(right),origLineNum);

//        token * peek = iter->next();
//        if (isBooleanOperator(peek))
//            iter = iter->next();

    }

    return parent;
}

symbolTable * CST::createPL(node * iter, ofstream& outFS)
{
    auto * PL = new symbolTable(curFunction);
    string idName, idType = "datatype", datatype;
    bool isArr = false;
    int size = 0;

    while (iter)
    {
        // get the datatype
        datatype = iter->content(), curScope;
        iter = iter->sibling();

        // get the name
        idName = iter->content(), curScope;
        iter = iter->sibling();

        // check if array
        if (iter && iter->content() == "[")
        {
            isArr = true;
            iter = iter->sibling();

            // check if a size is given
            if (iter->content() != "]")
            {
                size = stoi(iter->content());
                iter = iter->sibling();
            }
            // iterate past the end bracket
            iter = iter->sibling();

        }
        // check if not end of list
        if (iter && iter->content() == ",")
            iter = iter->sibling();

        auto * variable = new stNode(idName, idType, datatype, isArr, size, curScope);
        int foundScope;
        if (ST->inUse(variable, foundScope))
        {
            outFS << "Error on line " << lineNum << ": variable \"" << idName << "\" is already defined ";
            if (foundScope == 0)
                outFS << "globally\n";
            else
                outFS << "locally\n";
            errorDetected = true;
            delete variable;
        }
        else if (checkReserveWord(variable->idName(), VAR_RESERVED_WORDS))
        {
            outFS << "Syntax Error on line " << lineNum << ": the name " << idName << " cannot be used as it\n";
            outFS << " is a reserve word\n";
            errorDetected = true;
            delete variable;
        }
        else
            PL->insert(variable);
        outFS.flush();

        datatype.clear();
        idName.clear();
        size = 0;
        isArr = false;
    }
    return PL;

}


node * CST::parseFunction(token *& iter, ofstream& outFS)
{
    // symbol table parameters
    string idName, idType = "function", datatype;
    prevScope++;
    curScope = prevScope;

    int origLineNum = lineNum;
    if (!match(iter, "function"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED FUNCTION\n";}


    auto * parent = new node(iter->content(), curScope);
    node * _node = getLastNode(parent);

    iter = iter->next();

    if (!isDatatype(iter, outFS))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED DATATYPE\n";}


    datatype = iter->content(), curScope;
    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);


    iter = iter->next();

    if (!matchType(iter, IDENTIFIER))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED IDENTIFIER\n";}


    curFunction = idName = iter->content(), curScope;

    if (checkReserveWord(idName, FUNCTION_SPECIFIC_RESERVES))
    {
        outFS << "Syntax error on line " << lineNum << ": " << idName << " can not be used as the name of a function\n";
        errorDetected = true;
    }

    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);

    auto * function = new stNode(idName, idType, datatype, curScope);
    ST->insert(function);

    if (iter) iter = iter->next();
    else { iter = parseUntilEndL(iter); deleteNodes(parent); return nullptr;}

    if (!match(iter, "("))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED (\n";}
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);

    iter = iter->next();

    auto * paramList = parseParameterList(iter, outFS);
    if (!paramList && !match(iter, "void") )
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED void\n";}

    symbolTable * PL = nullptr;
    if (paramList) {
        child = paramList;
        PL = createPL(child, outFS);
        ST->insertST(PL);
    }
    else
        child = new node(iter->content(), curScope);

    insert(parent, _node, child, getLastNode(child), origLineNum);

    if (!match(iter, ")"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED )\n";}
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    parseEndlToken(iter);

    child = parseBlockStatement(iter, outFS);
    insert(parent, _node, child, getLastNode(child), origLineNum);


    return parent;

}


node * CST::getLastSibling(node * _node) {

    node * iter = _node;
    while (iter && iter->sibling())
        iter = iter->sibling();
    return iter;

}



bool CST::match(token *& iter, const string& expected)
{
    if (!iter)
        return false;
    if (iter->isScope()) {
        parseEndlToken(iter);
    }
    if (iter && iter->content() == expected)
    {
        return true;
    }
    return false;
}



node* CST::parseFactor( token *& iter, ofstream& outFS)
{
    int origLineNum = lineNum;
    if (!iter)
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Unexpected end of input in factor\n";}

    node* parent = nullptr, *child = nullptr, *_node = nullptr;

    if (match(iter, "(")) {

        parent = new node("(", curScope);
        _node = parent;
        iter = iter->next();

        child = parseNumericalExpression(iter, outFS);
        insert(parent, _node, child, getLastNode(child), origLineNum);

        if (!match(iter, ")"))
        {
            errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ')'\n";
        }

        child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();
    }
    else if (matchType(iter, INTEGER) || matchType(iter, IDENTIFIER))
    {
        parent = new node(iter->content(), curScope);
        _node = parent;
        iter = iter->next();

        if (match(iter, "[")) {
            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            if (!matchType(iter, INTEGER) && !matchType(iter, IDENTIFIER))
            { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected array size as integer\n";}

            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            if (!match(iter, "]"))
            { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected closing ']' for array declaration\n";}

            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();
        }
    }
    else if (match(iter, "'"))
    {
        parent = parseSingleQuoteString(iter, outFS);
    }
    else if (match(iter, "\""))
    {
        parent = parseDoubleQuoteString(iter, outFS);
    }
    else if (match(iter, "!"))
    {
        parent = new node("!", curScope);
        iter = iter->next();
        if (!matchType(iter, IDENTIFIER))
        { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": EXPECTED IDENTIFIER AFTER !\n";}

        token * peek = iter->next();
        if (match(peek, "("))
            parent->sibling(parseUserDefinedFunction(iter, outFS));
        else {
            child = new node(iter->content(), curScope);
            parent->sibling(child);
            iter = iter->next();
        }

    }
    else
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected identifier, integer, or '('\n";
    }

    return parent;
}


node* CST::parseUserDefinedFunction( token *& iter, ofstream& outFS)
{
    int origLineNum = lineNum;
    if (!matchType(iter, IDENTIFIER))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected function name\n";}

    node* parent = new node(iter->content(), curScope);
    node* _node = getLastNode(parent);
    node* child;
    iter = iter->next();


    if (!match(iter, "("))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected '(' after function name\n";}
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();


    if (matchType(iter, IDENTIFIER))
    {

        token* lookahead = iter->next();
        if (lookahead && (lookahead->content() == "," || lookahead->content() == "["))
        {
            node* args = parseIdentifierAndIdentifierArrayList(iter, outFS);
            insert(parent,_node, args, getLastNode(args),origLineNum);
        }
        else
        {

            node* expr = parseExpression(iter, outFS);
            insert(parent,_node, expr, getLastNode(expr),origLineNum);
        }
    }
    else
    {

        node* expr = parseExpression(iter, outFS);
        insert(parent,_node, expr, getLastNode(expr),origLineNum);
    }


    if (!match(iter, ")"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ')' after function arguments\n";}

    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    if (match(iter, ";"))
    {     child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();
    }
    else if (!match(iter, ")"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected SEMICOLON/PARENTHESES AFTER USER FUNCTION\n";

    }


    return parent;
}


node* CST::parseExpression( token *& iter, ofstream& outFS) {

    token* lookahead = iter;

    while (lookahead && !lookahead->isEndlToken() && !lookahead->isScope()) {
        std::string val = lookahead->content();
        if (val == "==" || val == "!=" || val == "<" || val == "<=" ||
            val == ">"  || val == ">=" || val == "&&" || val == "||" || val == "!") {
            return parseBooleanExpression(iter, outFS);
        }

        if (val == ";" || val == ")" || val == ",")
            break;

        lookahead = lookahead->next();
    }

    return parseNumericalExpression(iter, outFS);
}


node* CST::parseNumericalExpression( token *& iter, ofstream& outFS)
{
    int origLineNum = lineNum;
    node* parent = parseTerm(iter, outFS);
    node* _node = getLastNode(parent);

    while (match(iter, "+") || match(iter, "-")) {

        node* op = new node(iter->content(), curScope);
        insert(parent,_node, op, getLastNode(op), origLineNum);
        iter = iter->next();

        node* right = parseTerm(iter, outFS);
        insert(parent,_node, right, getLastNode(right), origLineNum);

    }

    return parent;
}


node* CST::parseTerm( token *& iter, ofstream& outFS)
{
    int origLineNum = lineNum;
    node* parent = parseFactor(iter, outFS);
    node * _node = getLastNode(parent);

    while (match(iter, "*") || match(iter, "/") || match(iter, "%"))
    {

        node* op = new node(iter->content(), curScope);
        insert(parent,_node, op, getLastNode(op), origLineNum);
        iter = iter->next();


        node* right = parseFactor(iter, outFS);
        insert(parent,_node, right, getLastNode(right), origLineNum);
    }

    return parent;
}


bool CST::matchType(token *& iter, const string& expected)
{
    if (!iter)
        return false;
    if (iter->isScope()) {
        parseEndlToken(iter);
    }
    if (iter && iter->_type() == expected)
    {
        return true;
    }
    return false;
}


node * CST::parseRelationalStatement( token *& iter, ofstream& outFS)
{

    int origLineNum = lineNum;
    if (!matchType(iter, IDENTIFIER)) {

        errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected identifier FOR CONDITION\n";
    }

    auto* parent = new node(iter->content(), curScope);
    auto* _node = parent;
    iter = iter->next();


    if (isBooleanOperator(iter))
    {
        auto* opNode = new node(iter->content(), curScope);
        insert(parent,_node, opNode, getLastNode(opNode), origLineNum);
        iter = iter->next();

        if (matchType(iter, INTEGER) || matchType(iter, IDENTIFIER))
        {
            auto* operand = new node(iter->content(), curScope);
            insert(parent,_node, operand, getLastNode(operand), origLineNum);
            iter = iter->next();
        }
        else
        {
            errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected OPERAND (IDENTIFIER or INTEGER)\n";
        }
    }
    else
    {
        errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected BOOLEAN OPERATOR\n";
    }

    return parent;
}



/***
 * This outputs the contents of a CST in a human-readable way
 * @param outputFile file to hold the contents of the CST
 */
void CST::outputTraversal(const string& outputFile)
{
    ofstream outFS;
    outFS.open(outputFile);

    if (!outFS.is_open())
    {
        cout << "ERROR OPENING " << outputFile << endl;
        exit(3);
    }

    traverseCST(head,outFS);
    if (head)
        outFS << "null";

    outFS.close();

}

#include <iomanip>
#define ORIGINAL_LENGTH 1
int lineWidth = ORIGINAL_LENGTH;


/***
 * This traverse the CST and outputs the tokens in their tree
 * structure
 * @param head root node of the tree
 * @param outFS output file stream
 */
void traverseCST(node * head, ofstream& outFS)
{
    if (!head)
        return;
//    outFS << "[" << head->content() << "]->";
//    if (head->sibling())
//        lineWidth += head->content().size() + 4;
    outFS << head->content() << " ";
//    if (head->sibling())
//        lineWidth += head->content().size();

    traverseCST(head->sibling(), outFS);

    if (head->child())
    {
        outFS << "\n\n" << setw(lineWidth);
        traverseCST(head->child(), outFS);


        lineWidth = ORIGINAL_LENGTH;

    }

}

                                                                                                                   
