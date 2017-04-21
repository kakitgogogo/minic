#ifndef MINIC_LEXER_H
#define MINIC_LEXER_H

#include "token.h"
#include <fstream>
#include <string>
#include <memory>
#include <unordered_map>

enum LexError
{
	LE_STR_NO_RQUOTE,
	LE_CHAR_NO_RQUOTE,
	LE_CHAR_NO_DATA,
	LE_BIN_INVALID,
	LE_HEX_INVALID,
	LE_FLOAT_INVALID,
	LE_COMMENT_NO_END,
	LE_TOKEN_NOT_EXIST,
};

extern std::unordered_map<std::string, Tag> keywords;

class Lexer
{
public:
	Lexer(std::string f);

	std::string getfile() { return fileName; }
	int getrow() { return row; }
	int getcol() { return col; }

	std::shared_ptr<Token> tokenize();
private:
	std::string fileName;
	int row;
	int col;

	std::ifstream fin;
	std::string line;
	char ch;

	char nextc();
	bool scan(char need = 0);

	void error(LexError code);
};

#endif