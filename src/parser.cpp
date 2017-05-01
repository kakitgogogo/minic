#include "token.h"
#include "lexer.h"
#include "parser.h"
#include <memory>
#include <iostream>
using namespace std;

string Node::to_string()
{
	if(token)
	{
		return token->to_string();
	}
	switch(kind)
	{
		case NK_NULL:
			return "<null>";
		case NK_IDX:
			return "<idx>";
		case NK_CALL:
			return "<call>";
		case NK_DECL:
			return "<decl>";
		case NK_FUNC_DECL:
			return "<funcdecl>";
		case NK_PTR:
			return "<pointer>";
		case NK_ARRAY:
			return "<array>";
		case NK_CAST:
			return "<cast>";
		case NK_LIST:
			return "<list>";
	}
}

Parser::Parser(Lexer& l): lexer(l) {}

void Parser::lookahead()
{
	look = lexer.tokenize();
}

bool Parser::match(Tag tag)
{
	if(look->tag == tag) 
	{
		lookahead();
		return true;
	}
	else
	{
		return false;
	}
}

Node* Parser::parse()
{
	lookahead();
	return program();
}


void Parser::error(string expect)
{
	string errMsg = "expected \""+expect+"\" before \"" + look->to_string() + "\"";
	cout << lexer.getfile() << ":" << lexer.getrow() << ":" << lexer.getcol() << ": " << errMsg << endl;
}

#define RECOVER(e) while(!(e)) lookahead();

#define IS(t) (look->tag == t)
#define _(t) || (look->tag == t)

#define IS_CONST() IS(CONST_INT)_(CONST_FLOAT)_(CONST_CHAR)_(CONST_STR)
#define IS_TYPE() IS(KW_INT)_(KW_FLOAT)_(KW_DOUBLE)_(KW_CHAR)_(KW_VOID)_(KW_EXTERN)_(KW_STATIC)_(KW_CONST)_(KW_VOLATILE)_(KW_INLINE)_(KW_STRUCT)_(KW_UNION)_(KW_ENUM)
#define IS_BINARY_OPER() IS(NE)_(EQ)_(LT)_(LE)_(GT)_(GE)_(ADD)_(SUB)_(MUL)_(DIV)_(MOD)_(SHL)_(SHR)_(AND)_(OR)_(XOR)_(LAND)_(XOR)_(ASSIGN)
#define IS_UNARY_OPER() IS(AND)_(MUL)_(ADD)_(SUB)_(BNOT)_(NOT)_(INC)_(DEC)_(KW_SIZEOF)
#define IS_CTRL_OPER() IS(KW_IF)_(KW_DO)_(KW_WHILE)_(KW_BREAK)_(KW_CONTINUE)_(KW_RETURN)_(KW_SWITCH)_(KW_CASE)_(KW_DEFAULT)_(KW_GOTO)
#define IS_BOUND() IS(END)_(SEMICOLON)_(COMMA)_(RPAREN)_(RBRACK)_(RBRACE)
#define IS_NULL(n) (n->kind == NK_NULL)

#define NULL_NODE new Node(NK_NULL)
#define MAKE_NODE(kind) new Node(kind, look)



Node* Parser::program()
{
	if(IS(END))
	{
		return NULL_NODE;
	}
	else
	{
		return translation_unit();
	}
}

/* Expressions */

/* 
<prim-expr>	->	ident | const | ( <expr> )
*/
Node* Parser::prim_expr()
{
	if(IS(IDENT))
	{
		Node* node = MAKE_NODE(NK_IDENT);
		lookahead();
		return node;
	}
	else if(IS_CONST())
	{
		Node* node = MAKE_NODE(NK_CONST);
		lookahead();
		return node;
	}
	else if(match(LPAREN))
	{
		Node* node = expr();
		if(!match(RPAREN))
		{
			error(")");
			RECOVER(IS_BINARY_OPER() || IS_BOUND() || IS(QUES)_(COLON)_(PTR)_(DOT)_(INC)_(DEC)_(LPAREN)_(LBRACK)_(LBRACE));
		}
		return node;
	}
	return NULL_NODE;
}

Node* Parser::post_expr()
{
	Node* lvar = prim_expr();
	return post_expr_tail(lvar);
}

Node* Parser::post_expr_tail(Node* lvar)
{
	if(match(LBRACK))
	{
		Node* index = expr();
		Node* node = new Node(NK_IDX, NULL, lvar, index);
		if(!match(RBRACK))
		{
			error("]");
			RECOVER(IS_BINARY_OPER() || IS_BOUND() || IS(LBRACE));
		}
		return post_expr_tail(node);
	}
	else if(match(LPAREN))
	{
		Node* args = args_expr();
		Node* node = new Node(NK_CALL, NULL, lvar, args);
		if(!match(RPAREN))
		{
			error(")");
			RECOVER(IS_BINARY_OPER() || IS_BOUND() || IS(LBRACE));
		}
		return post_expr_tail(node);
	}
	else if(IS(DOT)_(PTR))
	{
		Node* node = MAKE_NODE(NK_MEMBER);
		lookahead();
		if(IS(IDENT))
		{
			Node* rvar = MAKE_NODE(NK_IDENT);
			lookahead();
			node->argv[0] = lvar;
			node->argv[1] = rvar;
			return post_expr_tail(node);
		}
		else
		{
			error("identifier");
			RECOVER(IS_BINARY_OPER() || IS_BOUND() || IS(LBRACE));
			return NULL_NODE;
		}
	}
	else if(IS(INC)_(DEC))
	{
		Node* node = MAKE_NODE(NK_UNARY_POST_OPER);
		lookahead();
		node->argv[0] = lvar;
		return post_expr_tail(node);
	}
	return lvar;
}

Node* Parser::args_expr()
{
	if(IS(RPAREN))
	{
		return NULL_NODE;
	}
	else 
	{
		return args_list_expr();
	}
}

Node* Parser::args_list_expr()
{
	Node* lvar = assign_expr();
	return args_list_expr_tail(lvar);
}

Node* Parser::args_list_expr_tail(Node* lvar)
{
	if(IS(COMMA))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = assign_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return args_list_expr_tail(node);
	}
	return lvar;
}

Node* Parser::unary_expr()
{
	if(IS(INC)_(DEC))
	{
		Node* node = MAKE_NODE(NK_UNARY_OPER);
		lookahead();
		node->argv[0] = unary_expr();
		return node;
	}
	else if(IS(AND)_(MUL)_(ADD)_(SUB)_(BNOT)_(NOT)_(END))
	{
		Node* node = MAKE_NODE(NK_UNARY_OPER);
		lookahead();
		node->argv[0] = cast_expr();
		return node;
	}
	else if(IS(KW_SIZEOF))
	{
		Node* node = MAKE_NODE(NK_UNARY_OPER);
		lookahead();
		if(match(LPAREN))
		{
			node->argv[0] = type_name();
			if(!match(RPAREN))
			{
				error(")");
				RECOVER(IS_BINARY_OPER() || IS_BOUND() || IS(LBRACE));
			}
			return node;
		}
		return NULL_NODE;
	}
	else
	{
		return post_expr();
	}
}

Node* Parser::cast_expr()
{
	if(match(LT))
	{
		Node* lvar = type_name();
		if(!match(GT))
		{
			error(">");
			RECOVER(IS_BINARY_OPER() || IS_BOUND() || IS(LBRACE));
			return NULL_NODE;
		}

		Node* node = new Node(NK_CAST, NULL, lvar);
		if(match(LPAREN))
		{
			node->argv[1] = unary_expr();
			if(!match(RPAREN))
			{
				error(")");
				RECOVER(IS_BINARY_OPER() || IS_BOUND() || IS(LBRACE));
			}
			return node;
		}
		else
		{
			node->argv[1] = unary_expr();
			return node;
		}
	}
	else
	{
		return unary_expr();
	}
}

Node* Parser::mul_expr()
{
	Node* lvar = cast_expr();
	return mul_expr_tail(lvar);
}

Node* Parser::mul_expr_tail(Node* lvar)
{
	if(IS(MUL)_(DIV)_(MOD))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = cast_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return mul_expr_tail(node);
	}
	return lvar;
}

Node* Parser::add_expr()
{
	Node* lvar = mul_expr();
	return add_expr_tail(lvar);
}

Node* Parser::add_expr_tail(Node* lvar)
{
	if(IS(ADD)_(SUB))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = mul_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return add_expr_tail(node);
	}
	return lvar;
}

Node* Parser::shift_expr()
{
	Node* lvar = add_expr();
	return shift_expr_tail(lvar);
}

Node* Parser::shift_expr_tail(Node* lvar)
{
	if(IS(SHL)_(SHR))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = add_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return shift_expr_tail(node);
	}
	return lvar;
}

Node* Parser::rela_expr()
{
	Node* lvar = shift_expr();
	return rela_expr_tail(lvar);
}

Node* Parser::rela_expr_tail(Node* lvar)
{
	if(IS(LT)_(LE)_(GT)_(GE))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = shift_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return rela_expr_tail(node);
	}
	return lvar;
}

Node* Parser::equal_expr()
{
	Node* lvar = rela_expr();
	return equal_expr_tail(lvar);
}

Node* Parser::equal_expr_tail(Node* lvar)
{
	if(IS(EQ)_(NE))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = rela_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return equal_expr_tail(node);
	}
	return lvar;
}

Node* Parser::and_expr()
{
	Node* lvar = equal_expr();
	return and_expr_tail(lvar);
}

Node* Parser::and_expr_tail(Node* lvar)
{
	if(IS(AND))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = equal_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return and_expr_tail(node);
	}
	return lvar;
}

Node* Parser::xor_expr()
{
	Node* lvar = and_expr();
	return xor_expr_tail(lvar);
}

Node* Parser::xor_expr_tail(Node* lvar)
{
	if(IS(XOR))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = and_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return xor_expr_tail(node);
	}
	return lvar;
}

Node* Parser::or_expr()
{
	Node* lvar = xor_expr();
	return or_expr_tail(lvar);
}

Node* Parser::or_expr_tail(Node* lvar)
{
	if(IS(OR))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = xor_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return or_expr_tail(node);
	}
	return lvar;
}

Node* Parser::land_expr()
{
	Node* lvar = or_expr();
	return land_expr_tail(lvar);
}

Node* Parser::land_expr_tail(Node* lvar)
{
	if(IS(LAND))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = or_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return land_expr_tail(node);
	}
	return lvar;
}

Node* Parser::lor_expr()
{
	Node* lvar = land_expr();
	return lor_expr_tail(lvar);
}

Node* Parser::lor_expr_tail(Node* lvar)
{
	if(IS(LOR))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = land_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return lor_expr_tail(node);
	}
	return lvar;
}

Node* Parser::cond_expr()
{
	Node* lvar = lor_expr();
	return cond_expr_tail(lvar);
}

Node* Parser::cond_expr_tail(Node* lvar)
{
	if(IS(QUES))
	{
		Node* node = MAKE_NODE(NK_TERNARY_OPER);
		lookahead();
		Node* mid = expr();
		if(match(COLON))
		{
			Node* rval = cond_expr();
			node->argv[0] = lvar;
			node->argv[1] = mid;
			node->argv[2] = rval;
			return node;
		}
		else
		{
			error(":");
			RECOVER(IS_BOUND() || IS(LBRACE)_(ASSIGN));
			return NULL_NODE;
		}
	}
	return lvar;
}

Node* Parser::assign_expr()
{
	Node* lvar = cond_expr();
	return assign_expr_tail(lvar);
}

Node* Parser::assign_expr_tail(Node* lvar)
{
	if(IS(ASSIGN))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = assign_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return node;
	}
	return lvar;
}

Node* Parser::expr()
{
	Node* lvar = assign_expr();
	return expr_tail(lvar);
}

Node* Parser::expr_tail(Node* lvar)
{
	if(IS(COMMA))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = assign_expr();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return expr_tail(node);
	}
	return lvar;
}

Node* Parser::const_expr()
{
	return cond_expr();
}

/* Declarations */

Node* Parser::decl()
{
	Node* lvar = decl_spec();
	Node* rvar = init_decl_list();
	Node* node = new Node(NK_DECL, NULL, lvar, rvar);
	if(!match(SEMICOLON))
	{
		error(";");
		RECOVER(IS_UNARY_OPER() || IS_TYPE() || IS_CTRL_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
	}
	return node;
}

Node* Parser::decl_spec()
{
	if(IS_TYPE())
	{
		Node* node = MAKE_NODE(NK_TYPE);
		if(IS(KW_STRUCT)_(KW_UNION))
		{
			node->argv[1] = struct_or_union_spec();
		}
		else if(IS(KW_ENUM))
		{
			node->argv[1] = enum_spec();
		}
		else
		{
			lookahead();
		}
		node->argv[0] = decl_spec_tail();
		return node;
	}
}

Node* Parser::decl_spec_tail()
{
	if(IS_TYPE())
	{
		return decl_spec();
	}
	return NULL_NODE;
}

Node* Parser::init_decl_list()
{
	if(IS(COMMA)_(SEMICOLON)_(LBRACE))
	{
		return NULL_NODE;
	}
	else
	{
		Node* lvar = init_decl();
		return init_decl_list_tail(lvar);
	}
}

Node* Parser::init_decl_list_tail(Node* lvar)
{
	if(IS(COMMA))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = init_decl();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return init_decl_list_tail(node);
	}
	return lvar;
}

Node* Parser::init_decl()
{
	Node* lvar = declarator();
	if(IS(ASSIGN))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = initializer();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return node;
	}
	return lvar;
}

Node* Parser::struct_or_union_spec()
{
	Node* node;
	if(IS(KW_STRUCT))
	{
		lookahead();
		node = MAKE_NODE(NK_STRUCT);
	}
	else 
	{
		lookahead();
		node = MAKE_NODE(NK_UNION);
	}

	lookahead();
	if(match(LBRACE))
	{
		node->argv[0] = struct_decl_list();
		if(!match(RBRACE))
		{
			error("}");
			RECOVER(IS_TYPE() || IS(LPAREN)_(RPAREN)_(MUL)_(COMMA)_(GT)_(END));
		}
	}
	return node;
}

Node* Parser::struct_decl_list()
{
	if(IS(RBRACE))
	{
		return NULL_NODE;
	}
	else
	{
		Node* lvar = decl();
		Node* rvar = struct_decl_list();
		Node* node = new Node(NK_LIST, NULL, lvar, rvar);
		return node;
	}
}

Node* Parser::enum_spec()
{
	lookahead();
	Node* node = MAKE_NODE(NK_ENUM);
	lookahead();
	if(match(LBRACE))
	{
		node->argv[0] = enum_list();
		if(!match(RBRACE))
		{
			error("}");
			RECOVER(IS_TYPE() || IS(LPAREN)_(RPAREN)_(MUL)_(COMMA)_(GT)_(END));
		}
	}
	return node;
}

Node* Parser::enum_list()
{
	Node* lvar = enumerator();
	return enum_list_tail(lvar);
}

Node* Parser::enum_list_tail(Node* lvar)
{
	if(IS(COMMA))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = enumerator();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return enum_list_tail(node);
	}
	return lvar;
}

Node* Parser::enumerator()
{
	if(IS(IDENT))
	{
		Node* lvar = MAKE_NODE(NK_IDENT);
		lookahead();
		if(IS(ASSIGN))
		{
			Node* node = MAKE_NODE(NK_BINARY_OPER);
			lookahead();
			Node* rvar = const_expr();
			node->argv[0] = lvar;
			node->argv[1] = rvar;
			return node;
		}
		return lvar;
	}
	else
	{
		error("identifier");
		RECOVER(IS(COMMA)_(RBRACE)_(END));
		return NULL_NODE;
	}
}

Node* Parser::declarator()
{
	if(IS(MUL))
	{
		Node* lvar = pointer();
		Node* rvar = direct_declarator();
		Node* node = new Node(NK_PTR, NULL, lvar, rvar);
		return node;
	}
	else
	{
		return direct_declarator();
	}
}

Node* Parser::direct_declarator()
{
	Node* lvar;
	if(IS(IDENT))
	{
		lvar = MAKE_NODE(NK_IDENT);
		lookahead();
	}
	else if(match(LPAREN))
	{
		lvar = declarator();
		if(!match(RPAREN))
		{
			error(")");
			RECOVER(IS(RPAREN)_(COMMA)_(COLON)_(SEMICOLON)_(ASSIGN)_(LBRACE)_(END));
		}
	}
	else
	{
		error("declarator");
		RECOVER(IS(RPAREN)_(COMMA)_(COLON)_(SEMICOLON)_(ASSIGN)_(LBRACE)_(END));
		return NULL_NODE;
	}
	return direct_declarator_tail(lvar);
}

Node* Parser::direct_declarator_tail(Node* lvar)
{
	if(match(LBRACK))
	{
		Node* rvar = array_size();
		Node* node = new Node(NK_ARRAY, NULL, lvar, rvar);
		if(!match(RBRACK))
		{
			error("]");
			RECOVER(IS(RPAREN)_(COMMA)_(COLON)_(SEMICOLON)_(ASSIGN)_(LBRACE)_(END));
		}
		return direct_declarator_tail(node);
	}
	else if(match(LPAREN))
	{
		Node* rvar = param_list();
		Node* node = new Node(NK_FUNC_DECL, NULL, lvar, rvar);
		if(!match(RPAREN))
		{
			error(")");
			RECOVER(IS(RPAREN)_(COMMA)_(COLON)_(SEMICOLON)_(ASSIGN)_(LBRACE)_(END));
		}
		return direct_declarator_tail(node);
	}
	return lvar;
}

Node* Parser::pointer()
{
	Node* lvar = MAKE_NODE(NK_TYPE);
	lookahead();
	if(IS(KW_CONST)_(KW_VOLATILE))
	{
		lvar->argv[0] = MAKE_NODE(NK_TYPE);
		lookahead();
		lvar->argv[0]->argv[0] = pointer();
	}
	else if(IS(MUL))
	{
		lvar->argv[0] = pointer();
	}
	return lvar;
}

Node* Parser::array_size()
{
	if(IS(RBRACK))
	{
		return NULL_NODE;
	}
	else
	{
		return const_expr();
	}
}

Node* Parser::param_list()
{
	if(IS(RPAREN))
	{
		return NULL_NODE;
	}
	else
	{
		Node* lvar = param_decl();
		return param_list_tail(lvar);
	}
}

Node* Parser::param_list_tail(Node* lvar)
{
	if(IS(COMMA))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = param_decl();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return param_list_tail(node);
	}
	return lvar;
}

Node* Parser::param_decl()
{
	Node* lvar = decl_spec();
	if(IS(COMMA)_(RPAREN))
	{
		return lvar;
	}
	else
	{
		Node* rvar = declarator();
		Node* node = new Node(NK_DECL, NULL, lvar, rvar);
		return node;
	}
}

Node* Parser::type_name()
{
	if(IS_TYPE())
	{
		Node* node = MAKE_NODE(NK_TYPE);
		if(IS(KW_STRUCT)_(KW_UNION))
		{
			node->argv[0] = struct_or_union_spec();
		}
		else if(IS(KW_ENUM))
		{
			node->argv[0] = enum_spec();
		}
		else
		{
			lookahead();
		}
		return node;
	}
}

Node* Parser::initializer()
{
	if(match(LBRACE))
	{
		Node* node = initializer_list();
		if(!match(RBRACE))
		{
			error("}");
			RECOVER(IS(COMMA)_(SEMICOLON)_(LBRACE));
		}
		return node;
	}
	else
	{
		return assign_expr();
	}
}

Node* Parser::initializer_list()
{
	if(IS(RBRACE))
	{
		return NULL_NODE;
	}
	else
	{
		Node* lvar = initializer();
		return initializer_list_tail(lvar);
	}
}

Node* Parser::initializer_list_tail(Node* lvar)
{
	if(IS(COMMA))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* rvar = initializer();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return initializer_list_tail(node);
	}
	return lvar;
}

/* Statements */

Node* Parser::stat()
{
	if(IS(LBRACE))
	{
		return compound_stat();
	}
	else if(IS_TYPE())
	{
		return decl_stat();
	}
	else if(IS_UNARY_OPER() || IS_CONST() || IS(LPAREN)_(SEMICOLON)_(LT)_(IDENT))
	{
		return expr_stat();
	}
	else if(IS(KW_IF))
	{
		return if_stat();
	}
	else if(IS(KW_SWITCH))
	{
		return switch_stat();
	}
	else if(IS(KW_FOR))
	{
		return for_stat();
	}
	else if(IS(KW_WHILE)_(KW_DO))
	{
		return while_stat();
	}
	else if(IS(KW_GOTO)_(KW_CONTINUE)_(KW_BREAK)_(KW_RETURN))
	{
		return jump_stat();
	}
}

Node* Parser::compound_stat()
{
	if(match(LBRACE))
	{
		Node* node = stat_list();
		if(!match(RBRACE))
		{
			error("}");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}
		return node;
	}
}

Node* Parser::stat_list()
{
	if(IS(RBRACE))
	{
		return NULL_NODE;
	}
	else
	{
		Node* lvar = stat();
		Node* rvar = stat_list();
		Node* node = new Node(NK_LIST, NULL, lvar, rvar);
		return node;
	}
}

Node* Parser::decl_stat()
{
	return decl();
}

Node* Parser::expr_stat()
{
	if(match(SEMICOLON))
	{
		return NULL_NODE;
	}
	else
	{
		Node* node = expr();
		if(!match(SEMICOLON))
		{
			error(";");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}
		return node;
	}
}

Node* Parser::if_stat()
{
	if(IS(KW_IF))
	{
		Node* node = MAKE_NODE(NK_IF_STAT);
		lookahead();

		if(!match(LPAREN))
		{
			error("(");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* lvar = expr();
		if(!match(RPAREN))
		{
			error(")");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}

		if(!match(LBRACE))
		{
			error("{");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* rvar = stat();
		if(!match(RBRACE))
		{
			error("}");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}

		node->argv[0] = lvar;
		node->argv[1] = rvar;

		if(match(KW_ELSE))
		{
			if(!match(LBRACE))
			{
				error("{");
				RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
				return NULL_NODE;
			}
			Node* evar = stat();
			if(!match(RBRACE))
			{
				error("}");
				RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			}
			node->argv[2] = evar;
		}

		return node;
	}
}

Node* Parser::switch_stat()
{
	if(IS(KW_SWITCH))
	{
		Node* node = MAKE_NODE(NK_SWITCH_STAT);
		lookahead();

		if(!match(LPAREN))
		{
			error("(");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* lvar = expr();
		if(!match(RPAREN))
		{
			error(")");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}

		if(!match(LBRACE))
		{
			error("{");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* rvar = case_stat_list();
		if(!match(RBRACE))
		{
			error("}");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}

		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return node;
	}
}

Node* Parser::case_stat_list()
{
	if(IS(KW_CASE)_(KW_DEFAULT))
	{
		Node* lvar = case_stat();
		Node* rvar = case_stat_list();
		Node* node = new Node(NK_LIST, NULL, lvar, rvar);
		return node;
	}
	else
	{
		return NULL_NODE;
	}
}

Node* Parser::case_stat()
{
	if(IS(KW_CASE))
	{
		Node* node = MAKE_NODE(NK_BINARY_OPER);
		lookahead();
		Node* lvar = const_expr();
		if(!match(COLON))
		{
			error(":");
			RECOVER(IS(KW_CASE)_(KW_DEFAULT)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* rvar = stat();
		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return node;
	}
	else if(IS(KW_DEFAULT))
	{
		Node* node = MAKE_NODE(NK_UNARY_OPER);
		lookahead();
		if(!match(COLON))
		{
			error(":");
			RECOVER(IS(KW_CASE)_(KW_DEFAULT)_(RBRACE)_(END));
			return NULL_NODE;
		}
		node->argv[0] = stat();
		return node;
	}
}

Node* Parser::for_stat()
{
	if(IS(KW_FOR))
	{
		Node* node = MAKE_NODE(NK_FOR_STAT);
		lookahead();

		if(!match(LPAREN))
		{
			error("(");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* expr1 = expr_stat();
		Node* expr2 = expr_stat();
		Node* expr3 = expr();
		if(!match(RPAREN))
		{
			error(")");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}

		if(!match(LBRACE))
		{
			error("{");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* statement = stat();
		if(!match(RBRACE))
		{
			error("}");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}

		node->argv[0] = expr1;
		node->argv[1] = expr2;
		node->argv[2] = expr3;
		node->argv[3] = statement;
		return node;
	}
}

Node* Parser::while_stat()
{
	if(IS(KW_WHILE))
	{
		Node* node = MAKE_NODE(NK_WHILE_STAT);
		lookahead();

		if(!match(LPAREN))
		{
			error("(");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* lvar = expr();
		if(!match(RPAREN))
		{
			error(")");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}

		if(!match(LBRACE))
		{
			error("{");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* rvar = stat();
		if(!match(RBRACE))
		{
			error("}");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}

		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return node;
	}
	if(IS(KW_DO))
	{
		Node* node = MAKE_NODE(NK_WHILE_STAT);
		lookahead();

		if(!match(LBRACE))
		{
			error("{");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* lvar = stat();
		if(!match(RBRACE))
		{
			error("}");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}

		if(!match(KW_WHILE))
		{
			error("while");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}

		if(!match(LPAREN))
		{
			error("(");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}
		Node* rvar = expr();
		if(!match(RPAREN))
		{
			error(")");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}
		if(!match(SEMICOLON))
		{
			error(";");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}

		node->argv[0] = lvar;
		node->argv[1] = rvar;
		return node;
	}
}

Node* Parser::jump_stat()
{
	if(IS(KW_GOTO))
	{
		Node* node = MAKE_NODE(NK_JUMP_STAT);
		lookahead();

		if(!match(IDENT))
		{
			error("identifier");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
			return NULL_NODE;
		}

		node->argv[0] = MAKE_NODE(NK_IDENT);
		lookahead();

		if(!match(SEMICOLON))
		{
			error(";");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}

		return node;
	}
	else if(IS(KW_CONTINUE)_(KW_BREAK))
	{
		Node* node = MAKE_NODE(NK_JUMP_STAT);
		lookahead();

		if(!match(SEMICOLON))
		{
			error(";");
			RECOVER(IS_TYPE() || IS_CTRL_OPER() || IS_UNARY_OPER() || IS(SEMICOLON)_(LPAREN)_(LBRACE)_(RBRACE)_(END));
		}

		return node;
	}
	else
	{
		Node* node = MAKE_NODE(NK_JUMP_STAT);
		lookahead();
		node->argv[0] = expr_stat();
		return node;
	}
}

/* External Definitions */
Node* Parser::translation_unit()
{
	if(IS(END))
	{
		return NULL_NODE;
	}
	else
	{
		Node* lvar = extern_decl();
		Node* rvar = translation_unit();
		Node* node = new Node(NK_LIST, NULL, lvar, rvar);
		return node;
	}
}

Node* Parser::extern_decl()
{
	Node* lvar = decl_spec();
	Node* rvar = init_decl_list();
	Node* node = new Node(NK_EXTERNAL_DEF, NULL, lvar, rvar);
	if(IS(SEMICOLON))
	{
		return node;
	}
	else if(IS(LBRACE))
	{
		node->argv[2] = compound_stat();
		return node;
	}
	else
	{
		error("; or {");
		RECOVER(IS_TYPE());
	}
	return NULL_NODE;
}