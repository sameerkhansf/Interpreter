ST.x: CST.o tokenize.o main.o removeComments.o tokenList.o ST.o
	g++ -std=c++20 CST.o tokenize.o tokenList.o main.o removeComments.o ST.o -o ST.x

ST.o: symbolTable/symbolTable.h symbolTable/stNode.h
	g++ -std=c++20 -c symbolTable/symbolTable.cpp -o ST.o

CST.o: CST/CST.h CST/node.h
	g++ -std=c++20 -c CST/CST.cpp -o CST.o

tokenize.o: tokenize/tokenize.h tokenize/token.h tokenize/tokenList.h
	g++ -std=c++20 -c tokenize/tokenize.cpp -o tokenize.o

tokenList.o: tokenize/tokenList.h tokenize/token.h
	g++ -std=c++20 -c tokenize/tokenList.cpp -o tokenList.o

main.o: main.cpp removeComments/removeComments.h
	g++ -std=c++20 -c main.cpp -o main.o

removeComments.o: removeComments/removeComments.cpp removeComments/removeComments.h
	g++ -std=c++20 -c removeComments/removeComments.cpp -o removeComments.o


clean:
	rm -f *.o *.x
