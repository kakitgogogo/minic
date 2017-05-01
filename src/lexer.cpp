#include "lexer.h"
#include <iostream>
#include <assert.h>
using namespace std;

const int KW_IDX = 3;
const int KW_NUM = 26;

unordered_map<std::string, Tag> keywords;

Lexer::Lexer(string f): fileName(f), row(0), col(0), fin(f)
{
	if(keywords.empty())
	{
		for(int i = KW_IDX; i < KW_IDX+KW_NUM; ++i)
		{
			keywords[tokenName[i]] = Tag(i);
		}
	}
	ch = ' ';
}

char Lexer::nextc()
{
	if(line.size() <= col)
	{
		if(!getline(fin, line)) 
		{
			fin.close();
			return -1;
		}
		line += '\n';
		row++;
		col = 0;
	}
	return line[col++];
}

bool Lexer::scan(char need)
{
	ch = nextc();
	if(need)
	{
		if(ch != need) return false;
		ch = nextc();
		return true;
	}
	return true;
}

static const char* LexErrorStr[] = {
	"missing terminating \" character",
	"missing terminating \' character",
	"empty character constant",
	"invalid binary constant",
	"invalid hex constant",
	"invalid float constant",
	"unterminated comment",
	"token not exist",
};

void Lexer::error(LexError code)
{
	cout << fileName << ":" << row << ":" << col << ": " << LexErrorStr[code] << endl;
}

shared_ptr<Token> Lexer::tokenize()
{
	Token* token = NULL;
	while(ch != -1)
	{
		assert(!token);
		// whitespace
		while(ch == ' ' || ch == '\n' || ch == '\t')
			scan();

		if(ch == -1) break;

		// identifier or keyword
		if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_'))
		{
			string tmp;
			do{
				tmp += ch;
				scan();
			}while((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_') || (ch >= '0' && ch <= '9'));

			if(keywords.find(tmp) == keywords.end()) // identifier
			{
				token = new Ident(tmp);
			}
			else
			{
				token = new Token(keywords[tmp]);
			}
		}

		// number
		else if(ch >= '0' && ch <= '9')
		{
			long long val = 0;
			int isfloat = 0;
			double dval = 0;
			if(ch != '0')
			{
				do{
					val = val*10 + (ch-'0');
					scan();
				}while(ch >= '0' && ch <= '9');
				if(ch == '.' || ch == 'E' || ch == 'e')
				{
					isfloat = 1;
				}
			}
			else
			{
				scan();
				if(ch == 'x' || ch == 'X')
				{
					scan();
					if((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))
					{
						do{
							val = val*16 + ch;
							if(ch >= '0' && ch <= '9') val -= '0';
							else if(ch >= 'A' && ch <= 'F') val += 10 - 'A';
							else if(ch >= 'a'&& ch <= 'f') val += 10 - 'a';							
							scan();
						}while((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'));
					}
					else
					{
						error(LE_HEX_INVALID);
						token = new Token(ERR);
					}
				}
				else if(ch == 'b' || ch == 'B')
				{
					scan();
					if(ch == '0' || ch == '1')
					{
						do{
							val = val*2 + (ch-'0');
							scan();
						}while(ch == '0' || ch == '1');
					}
					else
					{
						error(LE_BIN_INVALID);
						token = new Token(ERR);
					}
				}
				else if(ch >= '0' && ch <= '7')
				{
					do{
						val = val*8 + (ch-'0');
						scan();
					}while(ch >= '0' && ch <= '7');
				}
				else if(ch == '.' || ch == 'E' || ch == 'e') 
				{
					isfloat = 1;
				}
			}

			// float number
			if(isfloat)
			{
				dval = val;
				if(ch == '.')
				{
					double factor = 1.0;
					scan();
					while(ch >= '0' && ch <= '9')
					{
						factor *= 0.1;
						dval += (ch-'0') * factor;
						scan();
					}
				}
				if(ch == 'E' || ch == 'e')
				{
					double factor = 10.0;
					int ex = 0;
					scan();
					if(ch == '-')
					{
						factor = 0.1;
						scan();
					}
					else if(ch == '+')
					{
						scan();
					}

					if(ch >= '0' && ch <= '9')
					{
						while(ch >= '0' && ch <= '9')
						{
							ex = ex*10 + (ch-'0');
							scan();
						}
					}
					else
					{
						error(LE_FLOAT_INVALID);
						token = new Token(ERR);
					}

					while(true)
					{
						if(ex & 1)
						{
							dval *= factor;
						}
						if((ex >>= 1) == 0)
							break;
						factor *= factor;
					}
				}

				if(!token) token = new Float(dval);
			}
			else
			{
				if(!token) token = new Int(val);
			}
		}


		// character
		else if(ch == '\'')
		{
			char tmp;
			scan();
			if(ch == '\\')
			{
				scan();
				switch(ch)
				{
					case '\'': tmp = '\''; break;
					case '\\': tmp = '\\'; break;
					case 'b':  tmp = '\b'; break;
					case 'f':  tmp = '\f'; break;
					case 'n':  tmp = '\n'; break;
					case 'r':  tmp = '\r'; break;
					case 't':  tmp = '\t'; break;
					case '0':  tmp = '\0'; break;
					case '\n':
					case -1:  
						error(LE_CHAR_NO_RQUOTE);
						token = new Token(ERR);
						break;
					default:   tmp = ch;   break;
				}
			}
			else if(ch == '\'')
			{
				error(LE_CHAR_NO_DATA);
				token = new Token(ERR);
			}
			else if(ch == '\n' || ch == -1)
			{
				error(LE_CHAR_NO_RQUOTE);
				token = new Token(ERR);
			}
			else tmp = ch;

			if(!token)
			{
				if(scan('\''))
				{
					token = new Char(tmp);
				}
				else 
				{
					error(LE_CHAR_NO_RQUOTE);
					token = new Token(ERR);
				}
			}
		}

		// string
		else if(ch == '"')
		{
			string tmp;
			while(!scan('"'))
			{
				if(ch == '\\')
				{
					scan();
					switch(ch)
					{
						case '\\': tmp += '\\'; break;
						case 'b':  tmp += '\b'; break;
						case 'f':  tmp += '\f'; break;
						case 'n':  tmp += '\n'; break;
						case 'r':  tmp += '\r'; break;
						case 't':  tmp += '\t'; break;
						case '0':  tmp += '\0'; break;
						case '\n': break;
						case -1:  
							error(LE_STR_NO_RQUOTE);
							token = new Token(ERR);
							break;
						default:   tmp += ch;   break;
					}
				}
				else if(ch == '\n' || ch == -1)
				{
					error(LE_STR_NO_RQUOTE);
					token = new Token(ERR);
					break;
				}
				else 
				{
					tmp += ch;
				}
			}

			if(!token)
			{
				token = new Str(tmp);
			}
		}

		// delimiter
		else
		{
			switch(ch)
			{
				case '#':
					while(ch != '\n' && ch != -1)
						scan();
					token = new Token(MACRO);
					break;
				case '+':
					token = new Token(scan('+') ? INC : ADD); break;
				case '-':
					scan();
					if(ch == '-')
					{
						token = new Token(DEC); scan();
					}
					else if(ch == '>')
					{
						token = new Token(PTR); scan();
					}
					else
					{
						token = new Token(SUB);
					}
					break;
				case '*':
					token = new Token(MUL); scan(); break;
				case '/':
					scan();
					if(ch == '/')
					{
						while(ch != '\n' && ch != -1)
							scan();
						token = new Token(ERR);
					}
					else if(ch == '*')
					{
						while(!scan(-1))
						{
							if(ch=='*')
							{
								if(scan('/')) break;
							}
						}
						if(ch == -1)
							error(LE_COMMENT_NO_END);
						token = new Token(ERR);
					}
					else
					{
						token = new Token(DIV);
					}
					break;
				case '%':
					token = new Token(MOD); scan(); break;
				case '<':
					scan();
					if(ch == '=')
					{
						token = new Token(LE); scan();
					}
					else if(ch == '<')
					{
						token = new Token(SHL); scan();
					}
					else
					{
						token = new Token(LT);
					}
					break;
				case '>':
					scan();
					if(ch == '=')
					{
						token = new Token(GE); scan();
					}
					else if(ch == '>')
					{
						token = new Token(SHR); scan();
					}
					else
					{
						token = new Token(GT);
					}
					break;
				case '=':
					token = new Token(scan('=') ? EQ : ASSIGN); break;
				case '&':
					token = new Token(scan('&') ? LAND : AND); break;
				case '^':
					token = new Token(XOR); scan(); break;
				case '|':
					token = new Token(scan('|') ? LOR : OR); break;
				case '!':
					token = new Token(scan('=') ? NE : NOT); break;
				case '~':
					token = new Token(BNOT); scan(); break;
				case ',':
					token = new Token(COMMA); scan(); break;
				case ':':
					token = new Token(COLON); scan(); break;
				case ';':
					token = new Token(SEMICOLON); scan(); break;
				case '.':
					token = new Token(DOT); scan(); break;
				case '?':
					token = new Token(QUES); scan(); break;
				case '(':
					token = new Token(LPAREN); scan(); break;
				case ')':
					token = new Token(RPAREN); scan(); break;
				case '[':
					token = new Token(LBRACK); scan(); break;
				case ']':
					token = new Token(RBRACK); scan(); break;
				case '{':
					token = new Token(LBRACE); scan(); break;
				case '}':
					token = new Token(RBRACE); scan(); break;
				default:
					token = new Token(ERR);
					error(LE_TOKEN_NOT_EXIST);
					scan();
			}
		}

		if(token && token->tag != ERR)
		{
			return shared_ptr<Token>(token);
		}
		if(token)
		{
			delete token;
			token = NULL;
		}
	}

	return shared_ptr<Token>(new Token(END));
}