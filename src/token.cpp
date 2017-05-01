#include "token.h"
#include <string>
using namespace std;

const char* tokenName[] = {
	"error",
	"end",

	"ident",

	"int", "float", "double", 
	"char", "void",
	"extern", "static", 
	"if", "else",
	"switch", "case", "default",
	"while", "do", "for",
	"break", "continue", "return",
	"struct", "enum", "union",
	"sizeof",
	"volatile", "const",
	"inline",
	"goto",

	"INT", "FLOAT", 
	"CHAR", "STR",

	/* old version
	"not",
	"add", "sub", "mul", "div", "mod",
	"inc", "dec",
	"gt", "ge", "lt", "le", "eq", "ne",
	"land","lor",
	"and", "or", "xor", "bnot",
	"shl", "shr",
	"lparen", "rparen",
	"lbrack", "rbrack",
	"lbrace", "rblace",
	"comma", "colon", "semicolon",
	"assign",
	"macro",
	"dot", "ptr",
	"ques",
	*/

	/* new version */
	"!",
	"+", "-", "*", "/", "%",
	"++", "--",
	"gt", "ge", "lt", "le", "==", "!=",
	"&&", "||",
	"&", "|", "^", "~",
	"shl", "shr",
	"(", ")",
	"[", "]",
	"{", "}",
	",", ":", ";",
	"=",
	"#",
	".", "ptr",
	"?",
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
	return string("<")+::to_string(val)+">";
}

string Float::to_string()
{
	return string("<")+::to_string(val)+">";
}

string Char::to_string()
{
	return string("<'")+val+"'>";
}

string Str::to_string()
{
	return string("<\"")+val+"\">";
}