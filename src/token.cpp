#include "token.h"
#include <string>
using namespace std;

const char* tokenName[] = {
	"error",
	"end",

	"ident",

	"int", "float", "double", 
	"char", "void",
	"extern",
	"if", "else",
	"switch", "case", "default",
	"while", "do", "for",
	"break", "continue", "return",
	"struct", "enum", "union",

	"INTEGER", "FLOAT", 
	"CHARACTER", "STRING",

	"not",
	"add", "sub", "mul", "div", "mod",
	"inc", "dec",
	"gt", "ge", "lt", "le", "eq", "ne",
	"land","lor",
	"and", "or",
	"shl", "shr",
	"lparen", "rparen",
	"lbrack", "rbrack",
	"lbrace", "rblace",
	"comma", "colon", "semicolon",
	"assign",
	"macro",
};


string Token::to_string()
{
	return string("<")+tokenName[tag]+">";
}

string Ident::to_string()
{
	return string("<")+tokenName[tag]+":"+name+">";
}

string Int::to_string()
{
	return string("<")+tokenName[tag]+":"+::to_string(val)+">";
}

string Float::to_string()
{
	return string("<")+tokenName[tag]+":"+::to_string(val)+">";
}

string Char::to_string()
{
	return string("<")+tokenName[tag]+":"+val+">";
}

string Str::to_string()
{
	return string("<")+tokenName[tag]+":"+val+">";
}