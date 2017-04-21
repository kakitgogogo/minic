#ifndef MINIC_TOKEN_H
#define MINIC_TOKEN_H

#include <string>

enum Tag
{
	ERR,										// error
	END,										// end

	IDENT,										// identifier

	KW_INT, KW_FLOAT, KW_DOUBLE,				// keyword: data type
	KW_CHAR, KW_VOID,							// keyword: data type
	KW_EXTERN,									// keyword: extern
	KW_IF, KW_ELSE,								// keyword: if-else
	KW_SWITCH, KW_CASE, KW_DEFAULT,				// keyword: swicth-case-deault
	KW_WHILE, KW_DO, KW_FOR,					// keyword: loop
	KW_BREAK, KW_CONTINUE, KW_RETURN,			// keyword: break, continue, return
	KW_STRUCT, KW_ENUM, KW_UNION,				// keyword: struct, enum, union

	CONST_INT, CONST_FLOAT, 					// constant: number
	CONST_CHAR, CONST_STR,						// constant: character, std::string

	NOT, 										// delimiter: !
	ADD, SUB, MUL, DIV, MOD,					// delimiter: +, -, *, /, %
	INC, DEC,									// delimiter: ++, --
	GT, GE, LT, LE, EQ, NE,						// delimiter: >, >=, <, <=, ==, !=
	LAND, LOR,									// delimiter: &&, ||
	AND, OR,									// delimiter: &, |
	SHL, SHR,									// delimiter: <<, >>
	LPAREN, RPAREN,								// delimiter: (, )
	LBRACK, RBRACK,								// delimiter: [, ]
	LBRACE, RBRACE,								// delimiter: {, }
	COMMA, COLON, SEMICOLON,					// delimiter: , : ;
	ASSIGN,										// delimiter: =
	MACRO,										// delimiter: #
};

extern const char * tokenName[];

class Token
{
public:
	Token(Tag t): tag(t) {}
	virtual ~Token() {}
	virtual std::string to_string();

	Tag tag;
};

class Ident: public Token
{
public:
	Ident(std::string s): Token(IDENT), name(s) {}
	virtual std::string to_string();

	std::string name;
};

class Int: public Token
{
public:
	Int(int n): Token(CONST_INT), val(n) {}
	virtual std::string to_string();

	int val;
};

class Float: public Token
{
public:
	Float(double n): Token(CONST_FLOAT), val(n) {}
	virtual std::string to_string();

	float val;
};

class Char: public Token
{
public:
	Char(char n): Token(CONST_CHAR), val(n) {}
	virtual std::string to_string();

	char val;
};

class Str: public Token
{
public:
	Str(std::string n): Token(CONST_STR), val(n) {}
	virtual std::string to_string();

	std::string val;
};

#endif