#ifndef MINIC_PARSER_H
#define MINIC_PARSER_H

#include "token.h"
#include "lexer.h"
#include <memory>

enum NodeKind
{
	NK_NULL,
	NK_TYPE,
	NK_IDENT,
	NK_CONST,
	NK_ARRAY,
	NK_FUNC_DECL,
	NK_ENUM,
	NK_UNION,
	NK_STRUCT,
	NK_FUNC,
	NK_PTR,
	NK_IDX,
	NK_CALL,
	NK_MEMBER,
	NK_UNARY_OPER,
	NK_UNARY_POST_OPER,
	NK_BINARY_OPER,
	NK_TERNARY_OPER,
	NK_CAST,
	NK_DECL,
	NK_LIST,
	NK_IF_STAT,
	NK_SWITCH_STAT,
	NK_FOR_STAT,
	NK_WHILE_STAT,
	NK_JUMP_STAT,
	NK_EXTERNAL_DEF,
};

struct Node
{
	NodeKind kind;
	std::shared_ptr<Token> token;
	Node* argv[4];
	Node(NodeKind k = NK_NULL, std::shared_ptr<Token> t = NULL, Node* a0 = NULL, Node* a1 = NULL): kind(k), token(t) 
	{
		argv[0] = a0;
		argv[1] = a1;
		argv[2] = NULL;
		argv[3] = NULL;
	}

	std::string to_string();
};

class Parser
{
private:
	// Program
	Node* program();

	// Expreesion
	Node* prim_expr();
	Node* post_expr();
	Node* post_expr_tail(Node* lval);
	Node* args_expr();
	Node* args_list_expr();
	Node* args_list_expr_tail(Node* lval);
	Node* unary_expr();
	Node* unary_oper();
	Node* cast_expr();
	Node* mul_expr();
	Node* mul_expr_tail(Node* lval);
	Node* add_expr();
	Node* add_expr_tail(Node* lval);
	Node* shift_expr();
	Node* shift_expr_tail(Node* lval);
	Node* rela_expr();
	Node* rela_expr_tail(Node* lval);
	Node* equal_expr();
	Node* equal_expr_tail(Node* lval);
	Node* and_expr();
	Node* and_expr_tail(Node* lval);
	Node* xor_expr();
	Node* xor_expr_tail(Node* lval);
	Node* or_expr();
	Node* or_expr_tail(Node* lval);
	Node* land_expr();
	Node* land_expr_tail(Node* lval);
	Node* lor_expr();
	Node* lor_expr_tail(Node* lval);
	Node* cond_expr();
	Node* cond_expr_tail(Node* lval);
	Node* assign_expr();
	Node* assign_expr_tail(Node* lval);
	Node* expr();
	Node* expr_tail(Node* lval);
	Node* const_expr();

	// Declaration
	Node* decl();
	Node* decl_spec();
	Node* decl_spec_tail();
	Node* init_decl_list();
	Node* init_decl_list_tail(Node* lvar);
	Node* init_decl();
	Node* struct_or_union_spec();
	Node* struct_or_union_spec_tail();
	Node* struct_decl_list();
	Node* struct_decl();
	Node* spec_qual_list();
	Node* spec_qual_list_tail();
	Node* struct_declarator_list();
	Node* struct_declarator_list_tail();
	Node* struct_declarator();
	Node* struct_declarator_tail();
	Node* enum_spec();
	Node* enum_spec_tail();
	Node* enum_list();
	Node* enum_list_tail(Node* lvar);
	Node* enumerator();
	Node* declarator();
	Node* direct_declarator();
	Node* direct_declarator_tail(Node* lvar);
	Node* pointer();
	Node* array_size();
	Node* param_list();
	Node* param_list_tail(Node* lvar);
	Node* param_decl();
	Node* type_name();
	Node* initializer();
	Node* initializer_list();
	Node* initializer_list_tail(Node* lvar);

	/* Statements */
	Node* stat();
	Node* compound_stat();
	Node* stat_list();
	Node* decl_stat();
	Node* expr_stat();
	Node* if_stat();
	Node* switch_stat();
	Node* case_stat_list();
	Node* case_stat();
	Node* for_stat();
	Node* while_stat();
	Node* jump_stat();

	/* External Definitions */
	Node* translation_unit();
	Node* extern_decl();

	std::shared_ptr<Token> look;
	Lexer& lexer;

	void lookahead();
	bool match(Tag tag);

	void error(std::string expect);

public:
	Parser(Lexer& l);

	Node* parse();
};

#endif