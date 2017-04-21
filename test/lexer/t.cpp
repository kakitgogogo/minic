#include <iostream>
#include "token.h"
#include "lexer.h"
using namespace std;

int main(int argc, char* argv[])
{
	if(argc != 2) 
	{
		return 1;
	}
	Lexer lexer(argv[1]);

	auto token = lexer.tokenize();
	while(token->tag != END)
	{
		cout << token->to_string() << endl;
		token = lexer.tokenize();
	}

}