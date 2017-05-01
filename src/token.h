#ifndef MINIC_TOKEN_H
#define MINIC_TOKEN_H

#include <string>

enum Tag
{
	ERR,										// error
	END,										// end

	IDENT,										// identifier

	KW_INT, KW_FLOAT, KW_DOUBLE,				// keyword: int, float, double
	KW_CHAR, KW_VOID,							// keyword: char, void
	KW_EXTERN, KW_STATIC,						// keyword: extern, static
	KW_IF, KW_ELSE,								// keyword: if-else
	KW_SWITCH, KW_CASE, KW_DEFAULT,				// keyword: swicth-case-deault
	KW_WHILE, KW_DO, KW_FOR,					// keyword: loop
	KW_BREAK, KW_CONTINUE, KW_RETURN,			// keyword: break, continue, return
	KW_STRUCT, KW_ENUM, KW_UNION,				// keyword: struct, enum, union
	KW_SIZEOF,									// keyword: sizeof
	KW_VOLATILE, KW_CONST,						// keyword: volatile, const
	KW_INLINE,									// keyword: inline
	KW_GOTO,									// keyword: goto

	CONST_INT, CONST_FLOAT, 					// constant: number
	CONST_CHAR, CONST_STR,						// constant: character, std::string

	NOT,										// delimiter: !
	ADD, SUB, MUL, DIV, MOD,					// delimiter: + - * / %
	INC, DEC,									// delimiter: ++ --
	GT, GE, LT, LE, EQ, NE,						// delimiter: > >= < <= == !=
	LAND, LOR,									// delimiter: && ||
	AND, OR, XOR, BNOT,							// delimiter: & | ^ ~
	SHL, SHR,									// delimiter: << >>
	LPAREN, RPAREN,								// delimiter: ( )
	LBRACK, RBRACK,								// delimiter: [ ]
	LBRACE, RBRACE,								// delimiter: { }
	COMMA, COLON, SEMICOLON,					// delimiter: , : ;
	ASSIGN,										// delimiter: =
	MACRO,										// delimiter: #
	DOT, PTR,									// delimiter: . ->
	QUES,										// delimiter: ?
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
	Int(long long n): Token(CONST_INT), val(n) {}
	virtual std::string to_string();

	long long val;
};

class Float: public Token
{
public:
	Float(double n): Token(CONST_FLOAT), val(n) {}
	virtual std::string to_string();

	double val;
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