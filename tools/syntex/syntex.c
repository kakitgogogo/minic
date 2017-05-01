#----------------------------- Expressions -----------------------------
<prim-expr>	->	ident | constant | ( <expr> )

<post-expr>	->	<prim-expr> <post-expr-tail> 
<post-expr-tail> ->	[ <expr> ] <post-expr-tail> 
				|	( <args-expr> ) <post-expr-tail>
				|	. ident <post-expr-tail>
				|	-> ident <post-expr-tail>
				|	++ <post-expr-tail>
				|	-- <post-expr-tail>
				|	empty

<args-expr> ->	<args-list-expr> | empty
<args-list-expr> ->	<assign-expr> <args-list-expr-tail>
<args-list-expr-tail> ->	, <assign-expr> <args-list-expr-tail> | empty

<unary-expr> ->	<post-expr>
			|	++ <unary-expr>
			|	-- <unary-expr>
			|	<unary-oper> <cast-expr>
			|	sizeof ( <type-name> )

<unary-oper> ->	& | * | + | - | ~ | !

<cast-expr> ->	<unary-expr> | < <type-name> > ( <unary-expr> )

<mul-expr> ->	<cast-expr> <mul-expr-tail> 
<mul-expr-tail> ->	* <cast-expr> <mul-expr-tail>
				|	/ <cast-expr> <mul-expr-tail>
				|	% <cast-expr> <mul-expr-tail>
				|	empty
 
<add-expr> ->	<mul-expr> <add-expr-tail> 
<add-expr-tail> ->	+ <mul-expr> <add-expr-tail>
				|	- <mul-expr> <add-expr-tail>
				|	empty

<shift-expr> ->	<add-expr> <shift-expr-tail> 
<shift-expr-tail> ->	<< <add-expr> <shift-expr-tail>
					|	>> <add-expr> <shift-expr-tail>
					|	empty

<rela-expr> ->	<shift-expr> <rela-expr-tail> 
<rela-expr-tail> ->	< <shift-expr> <rela-expr-tail>
				|	> <shift-expr> <rela-expr-tail>
				|	<= <shift-expr> <rela-expr-tail>
				|	>= <shift-expr> <rela-expr-tail>
				|	empty

<equal-expr> ->	<rela-expr> <equal-expr-tail> 
<equal-expr-tail> ->	== <rela-expr> <equal-expr-tail>
					|	!= <rela-expr> <equal-expr-tail>
					|	empty

<and-expr> ->	<equal-expr> <and-expr-tail> 
<and-expr-tail> ->	& <equal-expr> <and-expr-tail>
				|	empty

<xor-expr> ->	<and-expr> <xor-expr-tail> 
<xor-expr-tail> ->	^ <and-expr> <xor-expr-tail>
				|	empty

<or-expr> ->	<xor-expr> <or-expr-tail> 
<or-expr-tail> ->	or <xor-expr> <or-expr-tail>
				|	empty

<land-expr> ->	<or-expr> <land-expr-tail> 
<land-expr-tail> ->	&& <or-expr> <land-expr-tail>
				|	empty

<lor-expr> ->	<land-expr> <lor-expr-tail> 
<lor-expr-tail> ->	|| <land-expr> <lor-expr-tail>
				|	empty

<cond-expr> ->	<lor-expr> <cond-expr-tail>
<cond-expr-tail> ->	? <expr> : <cond-expr> | empty

<assign-expr> ->	<cond-expr> <assign-expr-tail> 
<assign-expr-tail> ->	<assign-oper> <assign-expr> | empty

<assign-oper> ->	=

<expr> ->	<assign-expr> <expr-tail> 
<expr-tail> ->	, <assign-expr> <expr-tail>
			|	empty

<const-expr> ->	<cond-expr>

#----------------------------- Declarations -----------------------------
<decl> ->	<decl-spec> <init-decl-list> ;

<decl-spec> ->	<storage-spec> <decl-spec-tail>
			|	<type-spec> <decl-spec-tail>
			|	<type-qual> <decl-spec-tail>
			|	<func-spec> <decl-spec-tail>
<decl-spec-tail> ->	<decl-spec> | empty

<init-decl-list> ->	<init-decl> <init-decl-list-tail> | empty
<init-decl-list-tail> ->	, <init-decl> <init-decl-list-tail> | empty

<init-decl> ->	<declarator> <init-decl-tail>
<init-decl-tail> ->	= <initializer> | empty

<storage-spec> ->	extern | static

<type-spec> ->	int | float | double | char | void | <struct-or-union-spec> | <enum-spec>

# struct or union
<struct-or-union-spec> ->	struct ident <struct-or-union-spec-tail> | union ident <struct-or-union-spec-tail>
<struct-or-union-spec-tail> ->	{ <struct-decl-list> } | empty

<struct-decl-list> ->	<struct-decl> <struct-decl-list> | empty

<struct-decl> ->	<spec-qual-list> <struct-declarator-list> ;

<spec-qual-list> ->	<type-spec> <spec-qual-list-tail>
				|	<type-qual> <spec-qual-list-tail>
<spec-qual-list-tail> -> 	<spec-qual-list> | empty

<struct-declarator-list> ->	<struct-declarator> <struct-declarator-list-tail>
<struct-declarator-list-tail> ->	, <struct-declarator> <struct-declarator-list-tail> | empty

<struct-declarator> ->	<declarator> <struct-declarator-tail>
<struct-declarator-tail> ->	: <const-expr> | empty
#

# enum
<enum-spec> ->	enum ident <enum-spec-tail>
<enum-spec-tail> ->	{ <enum-list> } | empty

<enum-list> ->	<enumerator> <enum-list-tail>
<enum-list-tail> ->	, <enumerator> <enum-list-tail> | empty

<enumerator> ->	ident <enumerator-tail>
<enumerator-tail> ->	= <const-expr> | empty
#

<type-qual>	->	const | volatile

<func-spec> -> inline

<declarator> ->	<pointer> <direct-declarator> | <direct-declarator>

<direct-declarator> ->	ident <direct-declarator-tail>
					|	( <declarator> ) <direct-declarator-tail>

<direct-declarator-tail> ->	[ <array-size> ] <direct-declarator-tail>
						|	( <param-list> ) <direct-declarator-tail>
						|	empty

<pointer> ->	* <pointer-tail>
<pointer-tail> ->	<type-qual-list> <pointer> | empty

<type-qual-list> ->	<type-qual> <type-qual-list> | empty

<array-size> ->	<const-expr> | empty

<param-list> ->	<param-decl> <param-list-tail> | empty
<param-list-tail> ->	, <param-decl> <param-list-tail> | empty

<param-decl> ->	<decl-spec> <param-decl-tail>
<param-decl-tail> ->	<declarator> | empty

<type-name> ->	<type-spec>

<initializer> ->	<assign-expr> | { <initializer-list> }

<initializer-list> ->	<initializer> <initializer-list-tail> | empty
<initializer-list-tail> ->	, <initializer> <initializer-list-tail>

#----------------------------- Statements -----------------------------
<stat> ->	<compound-stat>
		|	<decl-stat>
		|	<expr-stat>
		|	<if-stat>
		|	<switch-stat>
		|	<for-stat>
		|	<while-stat>
		|	<jump-stat>

<compound-stat> ->	{ <stat-list> }

<stat-list> ->	<stat> <stat-list> | empty

<decl-stat> ->	<decl>

<expr-stat> ->	; | <expr> ;

<if-stat> ->	if ( <expr> ) { <stat> } <else-stat>
<else-stat> ->	else { <stat> } | empty

<switch-stat> -> switch ( <expr> ) { <case-stat-list> }
<case-stat-list> ->	<case-stat> <case-stat-list> | empty
<case-stat> ->	case <const-expr> : <stat> | default : <stat>

<for-stat> ->	for ( <expr-stat> <expr-stat> <expr> ) { <stat> }

<while-stat> ->	while ( <expr> ) { <stat> } | do { <stat> } while ( <expr> ) ;

<jump-stat> ->	goto ident ; | continue ; | break ; | return <expr-stat>

#----------------------------- External Definitions -----------------------------
<tran-unit> ->	<extern-decl> <tran-unit> | empty

<extern-decl> ->	<decl-spec> <init-decl-list> <extern-decl-tail>

<extern-decl-tail> ->	<compound-stat> | ;