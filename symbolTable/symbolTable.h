//
// Created by natem on 3/10/2025.
//

#ifndef ASSIGNMENT4_SYMBOLTABLE_H
#define ASSIGNMENT4_SYMBOLTABLE_H
#include "stNode.h"
#include "../tokenize/tokenList.h"
#include "../tokenize/tokenize.h"


class symbolTable {

public:
    symbolTable(): _head{nullptr}, _next{nullptr}, _tail{nullptr} {}
    symbolTable(string name): _name{name}, _head{nullptr}, _next{nullptr},
    _tail{nullptr} {}
    ~symbolTable();

    void insert( stNode *& _token);
    void insertST(symbolTable * ST);
    stNode * head() { return _head; }
    bool inUse(stNode * node, int& foundScope);
    bool programHasMain();
    bool checkVariableName(stNode * node);
    void output(const string& outputFile);
    void deleteNodes();
    symbolTable * next() { return _next; }
    void next(symbolTable * ST) { _next = ST; }
    symbolTable * tail() { return _tail; }
    bool hasErrors() { return has_errors; }
    stNode * retrieveNode(const string& name, const int& scope);

private:
    stNode * _head, * curNode;
    string _name;
    symbolTable * _next, * _tail;
    bool has_errors = false;


};


#endif //ASSIGNMENT4_SYMBOLTABLE_H
