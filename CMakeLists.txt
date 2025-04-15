cmake_minimum_required(VERSION 3.27)
project(Assignment5)

set(CMAKE_CXX_STANDARD 17)

add_executable(Assignment5 main.cpp
        removeComments/removeComments.cpp
        tokenize/tokenize.cpp
        tokenize/tokenList.cpp
        CST/CST.cpp
        symbolTable/symbolTable.cpp
        CST/CST.cpp
        AST/AST.cpp
)
