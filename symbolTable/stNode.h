//
// Created by natem on 3/10/2025.
//

#ifndef ASSIGNMENT4_STNODE_H
#define ASSIGNMENT4_STNODE_H
#include <string>
using namespace std;


class stNode {

public:

    // non-arrays
    stNode(string name, string _type, string dataType, int programScope): _idName{name},
     _idType{_type}, _dataType{dataType}, _isArr{false}, _arrSize{0}, _scope{programScope},
     _next{nullptr}, _value{new string[1]} {}

    // arrays
    stNode(string name, string _type, string dataType, bool isArray, int size, int programScope):
    _idName{name},_idType{_type}, _dataType{dataType}, _isArr{isArray}, _arrSize{size},
    _scope{programScope}, _next{nullptr}, _value{new string[size]} {}

    // functions
    void next(stNode * ST) { _next = ST; }
    stNode * next() { return _next; }
    string idName() { return _idName; }
    string type() { return _idType; }
    string dataType() { return _dataType; }
    bool isArr() { return _isArr; }
    int size() { return _arrSize; }
    int scope() { return _scope; }
    string * _value;

private:

    string _idName;
    string _idType;
    string _dataType;
    bool _isArr;
    int _arrSize;
    int _scope;
    stNode * _next;

};


#endif //ASSIGNMENT4_STNODE_H
