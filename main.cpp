#include <iostream>
#include "tokenize/tokenize.h"
#include "removeComments/removeComments.h"
#include "tokenize/tokenList.h"
#include "CST/CST.h"
#include "AST/AST.h"
using namespace std;


/***
 * Driver program
 */
int main() {

    string initInput = "init input/programming_assignment_5-test_file_1.c";
    string tokenInput = "token input/input1.txt";
    string outputFile = "output/output1.txt";
    int userNum, numTests = 5;

    cout << "\nENTER A VALUE (1-5) TO TEST A SINGLE FILE OR 6 (or another larger value) TO TEST THEM ALL\n";
//    cin >> userNum;
    userNum = 11;

    if (userNum <= 5)
    {
            cout << "TEST #" << userNum << endl;
            string idx = to_string(userNum);
            if (idx.size() == 1) {
                initInput[46] = idx[0];
                tokenInput[17] = idx[0];
                outputFile[13] = idx[0];
            } else {
                adjustFiles(initInput, tokenInput, outputFile, idx);
            }
            removeComments(initInput, tokenInput);
            tokenList *TL = createTokenList(tokenInput, "output/tokenTest");
            auto *ST = new symbolTable();
            auto * cst = new CST(TL->head(), outputFile, ST);
            auto * ast = new AST(cst, ST);

            if (!cst->hasErrors()) {
                cout << "SUCCESSFUL\n";
//                ST->output(outputFile);
                ast->output(outputFile);
//                cst->outputTraversal(outputFile);
            } else {
                cout << "CHECK OUTPUT FILE FOR ERROR(S)\n";
            }

            cout << "OUTPUT CAN BE FOUND AT " << outputFile << endl << endl;

    }

    else {
        for (int i = 1; i <= numTests; i++) {
            cout << "TEST #" << i << endl;
            string idx = to_string(i);
            if (idx.size() == 1) {
                initInput[46] = idx[0];
                tokenInput[17] = idx[0];
                outputFile[13] = idx[0];
            } else {
                adjustFiles(initInput, tokenInput, outputFile, idx);
            }
            removeComments(initInput, tokenInput);
            tokenList *TL = createTokenList(tokenInput, "output/tokenTest");
            auto *ST = new symbolTable();
            auto * cst = new CST(TL->head(), outputFile, ST);
            auto * ast = new AST(cst, ST);

            if (!cst->hasErrors()) {
                cout << "SUCCESSFUL\n";
//                ST->output(outputFile);
                ast->output(outputFile);
//                cst->outputTraversal(outputFile);
            } else {
                cout << "CHECK OUTPUT FILE FOR ERROR(S)\n";
            }

            cout << "OUTPUT CAN BE FOUND AT " << outputFile << endl << endl;
        }
    }


    return 0;
}
