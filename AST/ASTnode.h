//
// Created by natem on 4/4/2025.
//

#ifndef ASSIGNMENT4_ASTNODE_H
#define ASSIGNMENT4_ASTNODE_H
#include <string>
#include <utility>
#include "../symbolTable/stNode.h"
using namespace std;


class ASTnode {

public:

    ASTnode(string title, stNode * _node): _sibling{nullptr}, _child{nullptr},
            _title{std::move(title)}, _node{_node} {}

    ASTnode * sibling() { return _sibling; }
    ASTnode * child() { return _child; }
    stNode * node() { return _node; }
    string content() { return _title; }

    void sibling(ASTnode * astNode) { _sibling = astNode; }
    void child(ASTnode * astNode) { _child = astNode; }


private:
    string _title;
    ASTnode * _sibling, * _child;
    stNode * _node;
};


#endif //ASSIGNMENT4_ASTNODE_H
