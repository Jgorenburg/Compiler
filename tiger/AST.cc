#include "AST.h"
#include "errormsg.h"
#include <logic.h>


// Abstract Syntax for Tiger
//
// January 2002, davew@cs.haverford.edu
//
// See AST-print.cc for printing, various other files for attributes other than pos


// First, an example to help illustrate the creation of AST nodes;
// After that come the class declarations

// To run the examples, use the command
//	g++ -std=c++0x -I/home/courses/include -D AST_EXAMPLES_IS_MAIN=1 -D ERRORMSG_SKIP_LEX=1 AST.cc AST-print.cc AST-print-attributes.cc HERA_code.cc result_reg.cc util.cc errormsg.cc -L/home/courses/lib -lcourses -o AST_examples && ./AST_examples


void AST_examples()
{
	// Use the constructors of AST_appel.h, i.e. Appel's Figure 4.7, to create some example AST's
	// 14+6
	A_exp fourteen	= A_IntExp(Position::undefined(), 14);  // 14
	A_exp six	= A_IntExp(Position::undefined(), 6);  // 6
	A_exp twenty	= A_OpExp(Position::undefined(), A_plusOp, fourteen, six);  // 14+6

	// "Some AST examples"
	A_exp title	= A_StringExp(Position::undefined(), "Some AST\texamples\n");

	// print("Some AST examples")
	A_exp call1	= A_CallExp(Position::undefined(), to_Symbol("print"),
				    A_ExpList(title, 0));  // 0 indicates end of list

	// 36*2+14+6
	Position undef	= Position::undefined();  // an abbreviation; I'm getting tired
	A_exp exp_92	= A_OpExp(undef, A_plusOp,
				  A_OpExp(undef, A_timesOp, A_IntExp(undef, 36), A_IntExp(undef, 2)),
				  twenty);

	// print("Some AST examples"); printint(mod(32*2+14+6, 50))
	A_exp all	= A_SeqExp(undef,
				   A_ExpList(call1,
					     A_ExpList(A_CallExp(undef, to_Symbol("printint"),
								 A_ExpList(A_CallExp(undef, to_Symbol("mod"),
										     A_ExpList(exp_92,
											       A_ExpList(A_IntExp(undef, 50),
													 0))  // end of mod parameters
										     ), // end of call to mod
									   0) // end of printint parameters
								 ), // end of call to printint
						       0) // end of sublist with call to printint
					     ) // end of main list of expressions with two calls
				   ); // end of top A_SeqExp

	EM_debug(repr(all), all->pos());
}

void AST_example_let()
{
/*
  Build (using the functions from AST_appel.h and Figure 4.7) and print
  the AST corresponding to the following tiger program:

    let
	var wombat : int := 14+6
	var arthropod : int := 2
    in
	(let
	    var wombat : int := 35
	 in
	    arthropod := wombat+arthropod
	 end)
       +
	(let
	    var arthropod : int := 4
	 in
	    wombat/arthropod
	 end)
    end

    Since this isn't coming from the lexical scanner
      (the example is run before the actual compiler starts),
      the "adjust" function hasn't had a chance to record the lengths of lines,
      and we can't refer to "positions" in any useful way.

    So ... in this example, the "pos" parameter (for position)
      is just left at the initial EM_currentPos(), which will show up as line 1.
    In the compiler, you could just use EM_currentPos() for all "pos" fields as well,
      but this would end up identifying the position at the _end_ of each complete
      expression, (i.e., line 15 would be associated with the "+" on line 10 of the example).
    Better would be to associate a "pos" attribute with the PLUS token,
      and use that as the "pos" for the A_opExp_.

    The function names below are the same as those used in Appel's book.
    I usually use the most general appropriate type of pointer for the AST node produced,
      e.g. A_exp for both integers and operations, but A_dec for a vardec or declist.

    This gets taught before types, so for the moment all types are just given as "0" (null pointer).
*/

	A_exp fourteen = A_IntExp(Position::undefined(), 14);  // 14
	A_exp six      = A_IntExp(Position::undefined(), 6);  // 6
	A_exp twenty   = A_OpExp(Position::undefined(), A_plusOp, fourteen, six);  // 14+6

	A_dec wombat1  = A_VarDec(Position::undefined(), to_Symbol("wombat"), 0, twenty); // no type info given
	A_dec arth1    = A_VarDec(Position::undefined(), to_Symbol("arthropod"), 0, A_IntExp(Position::undefined(), 2));
	A_decList let1_decs = A_DecList(wombat1, A_DecList(arth1, 0));  // that's the declarations for let #1


	// *** Now the stuff for the 2nd let (the first one inside the outer let) ***
	A_dec wombat2  = A_VarDec(Position::undefined(), to_Symbol("wombat"), 0, A_IntExp(Position::undefined(), 35));
	A_decList let2_decs = A_DecList(wombat2, 0);

	// wombat + arthropod
	A_var w_var2   = A_SimpleVar(Position::undefined(), to_Symbol("wombat"));
	A_exp w_use2   = A_VarExp(Position::undefined(), w_var2);  // for the use of "wombat" in "wombat+arthropod"
	A_exp a_use2   = A_VarExp(Position::undefined(), A_SimpleVar(Position::undefined(), to_Symbol("arthropod")));
	A_exp sum2     = A_OpExp(Position::undefined(), A_plusOp, w_use2, a_use2);

	// arthropod :=
	A_var a_var2   = A_SimpleVar(Position::undefined(), to_Symbol("arthropod"));

	// arthropod := wombat + arthropod
	A_exp assign_a = A_AssignExp(Position::undefined(), a_var2, sum2);

	// now, build the node for "let2" from everything from "*** Now the stuff for the 2nd let" to here
	A_exp let2     = A_LetExp(Position::undefined(), let2_decs, assign_a);


	// *** Now the stuff for the 3rd let (the 2nd one inside the outer let)
	A_exp let3     = A_LetExp(Position::undefined(),
				  A_DecList(A_VarDec(Position::undefined(), to_Symbol("arthropod"), 0, A_IntExp(Position::undefined(), 4)),
					    0),
				  A_OpExp(Position::undefined(), A_divideOp,
					  A_VarExp(Position::undefined(), A_SimpleVar(Position::undefined(), to_Symbol("wombat"))),
					  A_VarExp(Position::undefined(), A_SimpleVar(Position::undefined(), to_Symbol("arthropod")))));

	// *** At long last, we can build that "+" that sums the two inner lets, and the main let itself:
	A_exp let1      = A_LetExp(Position::undefined(),
				   let1_decs,
				   A_OpExp(Position::undefined(), A_plusOp, let2, let3));

	A_root_ *local_AST_root = new A_root_(let1);

	// Phew. Done at last. Let's print it.

	EM_debug("Here's a simple AST for 14+6, printed with to_String");
	EM_debug(str(twenty));

	EM_debug("Now,  here's the HERA code we get at the moment for that:");
	EM_debug(twenty->HERA_code());

	EM_debug("Here's the full example AST, printed with to_String");
	EM_debug(str(local_AST_root));
}


void AST_example_functions()
{
/*
  Build (using the functions from AST_appel.h and Figure 4.7) and print
  the AST corresponding to the following tiger program:

    let
        var two : int := 2
	function half_answer(): int = 21
	function get_two(): int = two
	function answer() : int = half_answer() * get_two()
	var it : int := answer()
    in
	printint(it)
    end

    Note that tiger "sees" this as a three-element let, with a variable,
         a collection of potentially-recursive functions, and another variable.
	 The FunctionDec holds the list of functions.
  */


	Position u = Position::undefined();
	Symbol tig_int = to_Symbol("int");
	A_root_ *r = A_RootExp(A_LetExp(u, A_DecList(A_VarDec(u, to_Symbol("two"), tig_int, A_IntExp(u, 2)),
					       A_DecList(A_FunctionDec(u, A_FundecList(A_Fundec(u, to_Symbol("half_answer"), 0, tig_int, A_IntExp(u, 21)),
										       A_FundecList(A_Fundec(u, to_Symbol("get_two"),     0, tig_int, A_VarExp(u, A_SimpleVar(u, to_Symbol("two")))),
												 A_FundecList(A_Fundec(u, to_Symbol("answer"),      0, tig_int, A_OpExp(u, A_timesOp,
																    A_CallExp(u, to_Symbol("half_answer"), 0),
																    A_CallExp(u, to_Symbol("get_two"), 0))),
							     0)))),
				     A_DecList(A_VarDec(u, to_Symbol("it"), tig_int, A_CallExp(u, to_Symbol("answer"), 0)),
				     0))),

				     A_CallExp(u, to_Symbol("printint"),
					       A_ExpList(A_VarExp(u, A_SimpleVar(u, to_Symbol("it"))), 0))));

	EM_debug("Here's the example from AST_example_functions:");
	EM_debug(str(r));
}

#if defined(AST_EXAMPLES_IS_MAIN) && AST_EXAMPLES_IS_MAIN
int main()
{
	EM_reset("Examples in AST_examples() in AST.cc", -1, true);
	AST_examples();
	AST_example_functions();
	return 0;
}
#endif




// Now, the functions for the actual AST classes...

AST_node_::AST_node_(A_pos pos) : stored_pos(pos)  // concise initialization of "pos" data field
{
}

AST_node_::~AST_node_()
{
}

A_root_::A_root_(A_exp main_exp) : AST_node_(main_exp->pos()), main_expr(main_exp) {
	// We'd *like* to call
	//     this->set_parent_pointers_for_me_and_my_decendents(0);
	// HOWEVER, the type of "this" is still AST_Node_, until the end of the constructor when it's a full-formed A_root_.
	//          Thus, we'll write the code that would have been in that function:
	this->stored_parent = 0;
	main_exp->set_parent_pointers_for_me_and_my_decendents(this);
}



String to_String(AST_node_ *n)
{
	return n->print_rep(0, have_AST_attrs);
}

A_exp_::A_exp_(A_pos p) : AST_node_(p)
{
}

A_literalExp_::A_literalExp_(A_pos p) : A_exp_(p)
{
}

A_leafExp_::A_leafExp_(A_pos p) : A_literalExp_(p)
{
}

A_nilExp_::A_nilExp_(A_pos pos) :  A_leafExp_(pos)
{
}

A_boolExp_::A_boolExp_(A_pos pos, bool init) :  A_leafExp_(pos), value(init)
{
}
Ty_ty A_boolExp_::get_type() {	return Ty_Bool();	}

A_intExp_::A_intExp_(A_pos pos, int i) :  A_leafExp_(pos), value(i)
{
}
Ty_ty A_intExp_::get_type() {	return Ty_Int();	}

A_stringExp_::A_stringExp_(A_pos pos, String s) : A_leafExp_(pos), value(s)
{
}
Ty_ty A_stringExp_::get_type() {	return Ty_String();	}


A_recordExp_::A_recordExp_(A_pos pos, Symbol typ, A_efieldList fields) :  A_literalExp_(pos), _typ(typ), _fields(fields)
{
	precondition(typ != 0);
}

A_arrayExp_::A_arrayExp_(A_pos pos, Symbol typ, A_exp size, A_exp init) :  A_literalExp_(pos), _typ(typ), _size(size), _init(init)
{
	precondition(typ!=0 && size!=0 && init!=0);
}


A_varExp_::A_varExp_(A_pos pos, A_var var) :  A_exp_(pos), _var(var)
{
	precondition(var != 0);
}
Ty_ty A_varExp_::get_type()
{
	return _var->get_type();
}
bool A_varExp_::valid_var()
{
	return _var->valid_var();
}




A_opExp_::A_opExp_(A_pos pos, A_oper oper, A_exp left, A_exp right) :  A_exp_(pos), _oper(oper), _left(left), _right(right)
{
	precondition(left != 0 && right != 0);
}
Ty_ty A_opExp_::get_type()
{
	if (_oper == A_plusOp || _oper ==  A_minusOp || _oper ==  A_timesOp)	{	return Ty_Int();	}
	else if (_oper ==  A_eqOp || _oper == A_neqOp || _oper == A_ltOp ||
 					 _oper == A_leOp || _oper == A_gtOp || _oper == A_geOp) 			{	return Ty_Bool();	}
	else { EM_error("Unhandled case in HERA_math_op", true);
					return Ty_Void();		// should never happen, just to avoid compiler warning
			}
}
void A_opExp_::type_check(bool comp)
{
	if (!(_left->get_type() == Ty_Int() && _right->get_type() == Ty_Int())) {
		if (comp) {																																				// comparisons can also be strings
			if (!(_left->get_type() == Ty_String() && _right->get_type() == Ty_String())) {
				EM_error("Children are not both integers or both strings: " + to_String(_left->get_type()) + " + " + to_String(_right->get_type()), true);
			}
		}
		else { EM_error("Children are not both integers: " + to_String(_left->get_type()) + " + " + to_String(_right->get_type()), true); }
	}
}

A_assignExp_::A_assignExp_(A_pos pos, A_var var, A_exp exp) : A_exp_(pos), _var(var), _exp(exp)
{
	precondition(exp != 0 && var != 0);
}
void A_assignExp_::type_check()
{
	try {
		Ty_ty oldType = lookup(_var->get_var(), this->get_ST())._type;
		assert(oldType == _exp->get_type());
	}
	catch(ST_bi::undefined_symbol missing) {
		EM_error(Symbol_to_string(missing.name) + " is not a defined variable", true);
	}
	catch(...) {
		EM_error("The type of " + Symbol_to_string(_var->get_var()) + " is not consistent between the original definition: " +
							to_String(lookup(_var->get_var(), this->get_ST())._type) + " and the new one: " + to_String(this->get_type()));
	}
}

A_letExp_::A_letExp_(A_pos pos, A_decList decs, A_exp body) :  A_exp_(pos), _decs(decs), _body(body)
{
	// Appel says body and decs can each be null
}
Ty_ty A_letExp_:: get_type()
{
	return _body->get_type();
}

A_callExp_::A_callExp_(A_pos pos, Symbol func, A_expList args) :  A_exp_(pos), _func(func), _args(args)
{
	precondition(func != 0);
}
Ty_ty A_callExp_::get_type()
{
	if (type == 0) { type = lookup(_func, this->get_ST())._type->u.function.return_type; }
	return type;
}
void A_callExp_::type_check()
{
/*	//getting the two things we need to compare
	if (!(is_name_there(_func, this->get_ST()) && lookup(_func, this->get_ST())._type->kind == Ty_function)) {
		EM_error("Function " + Symbol_to_string(_func) + " does not have a definition", true);
	}
	Ty_fieldList arg_types = 0;
	if (_args != 0) { arg_types = _args->get_listType(); }
	Ty_fieldList param_types = lookup(_func, this->get_ST())._type->u.function.parameter_types;
	//EM_debug ("past defs");

//	if(Ty_listTypeEqual(0,0)){}
	if (Ty_listTypeEqual(arg_types, param_types)) { }
	else { EM_error("Functions parameters do not have the proper types", true); }
*/
}

A_controlExp_::A_controlExp_(A_pos p) : A_exp_(p)
{
}

A_ifExp_::A_ifExp_(A_pos pos, A_exp test, A_exp then, A_exp else_or_0_pointer_for_no_else) :  A_controlExp_(pos), _test(test), _then(then), _else_or_null(else_or_0_pointer_for_no_else)
{
	precondition(test != 0 && then != 0);
}
Ty_ty A_ifExp_::get_type()
{
	if (type == 0) { type = _then->get_type();	}		// if statements have the same type as their then and else branches
	return type;
}
void A_ifExp_::type_check()
{
	if ( _test->get_type() == Ty_Bool() ) {
		if (_else_or_null != 0 && _then->get_type() != _else_or_null->get_type()) {
			EM_error("Problem with then and else. \nTest: " + to_String(_test->get_type()) + "\nThen: " + to_String(_then->get_type()) + "\nElse: " + to_String(_else_or_null->get_type()), true);
			}
	}
	else {
		EM_error("Problem with the test. \nTest: " + to_String(_test->get_type()), true);
	}
}

A_whileExp_::A_whileExp_(A_pos pos,A_exp test, A_exp body) :  A_controlExp_(pos),  _test(test), _body(body)
{
	precondition(test != 0 && body != 0);
}
/*Ty_ty A_whileExp_::get_type()
{
	if (type == 0) { type = _body->get_type();	}
	return type;
}*/
void A_whileExp_::type_check()
{
	if (_test->get_type() != Ty_Bool()) { EM_error("Test in while not type bool: " + to_String(_test->get_type()), true); }
}

A_forExp_::A_forExp_(A_pos pos, Symbol var, A_exp lo, A_exp hi, A_exp body) :  A_controlExp_(pos), _var(var), _lo(lo), _hi(hi), _body(body)
{
	precondition(var != 0 && lo != 0 && hi != 0 && body != 0);
}
void A_forExp_::type_check()
{
	if (_lo->get_type() != Ty_Int() || _hi->get_type() != Ty_Int()) {
		EM_error("Lo and hi in For not both bools.\n Lo: " + to_String(_lo->get_type()) +"\n Hi: " + to_String(_hi->get_type()), true );
	}
}

A_breakExp_::A_breakExp_(A_pos pos) :  A_controlExp_(pos)
{
}

A_seqExp_::A_seqExp_(A_pos pos, A_expList seq) :  A_controlExp_(pos), _seq(seq)
{
}
Ty_ty A_seqExp_::get_type()
{
	if (type == 0) {
		if (_seq == 0) { type = Ty_Void(); }
		else { type = _seq->get_type_last(); }
	}
	return type;
}




A_var_::A_var_(A_pos p) : AST_node_(p)
{
}

A_simpleVar_::A_simpleVar_(A_pos pos, Symbol sym) :  A_var_(pos), _sym(sym)
{
	precondition(sym != 0);
}
Ty_ty A_simpleVar_::get_type()
{
	if (this->valid_var()) {
		if (lookup(_sym, this->get_ST())._reg >= 0) {
				return lookup(_sym, this->get_ST())._type;
			}
		else {
			EM_error("Variable is defined as a function", true);
			return Ty_Error();
		}
	}
	else {
		EM_error("Variable is not defined: " + Symbol_to_string(_sym), true);
		return Ty_Error();
	}
}
bool A_simpleVar_::valid_var()
{
	return is_name_there(_sym, this->get_ST());
}

A_fieldVar_::A_fieldVar_(A_pos pos, A_var var, Symbol sym) :  A_var_(pos), _var(var), _sym(sym)
{
	precondition(var != 0 && sym != 0);
}

A_subscriptVar_::A_subscriptVar_(A_pos pos, A_var var, A_exp exp) :  A_var_(pos), _var(var), _exp(exp)
{
	precondition(exp != 0 && var != 0);
}


A_expList_::A_expList_(A_exp head, A_expList tail) :  AST_node_(head->pos()), _head(head), _tail(tail)
{
	precondition(head != 0);
}
int A_expList_::length()
{
	if (_tail == 0)
		return 1;
	else
		return 1+_tail->length();
}
// gets the last element of the expList
A_exp A_expList_::get_last()
{
	if (_tail != 0)	{	return _tail->get_last();	}
	else 						{	return _head;	}
}

Ty_tyList A_expList_::init_listType()
{
	if (_tail != 0)	{	return Ty_TyList(_head->get_type(), _tail->get_listType());	}
	else 						{	return Ty_TyList(_head->get_type(), 0);									}	// if it's the end of the list
}

Ty_tyList A_expList_::get_listType()
{
	if (list_type == 0) { list_type = this->init_listType(); }
	return list_type;
}
Ty_ty A_expList_::get_type_last()
{
	return this->get_last()->get_type();
}

A_efield_::A_efield_(Symbol name, A_exp exp) :  AST_node_(exp->pos()), _name(name), _exp(exp)
{
	precondition(exp != 0);
}
String A_efield_::fieldname()
{
	return Symbol_to_string(_name);
}

A_efieldList_::A_efieldList_(A_efield head, A_efieldList tail) :  AST_node_(head->pos()), _head(head), _tail(tail)
{
	precondition(head != 0);
}



A_dec_::A_dec_(A_pos p) : AST_node_(p)
{
}

A_decList_::A_decList_(A_dec head, A_decList tail) :  A_dec_(head->pos()), _head(head), _tail(tail)
{
	precondition(head != 0);
}
int A_decList_::length()
{
	if (_tail == 0)
		return 1;
	else
		return 1+_tail->length();
}

A_varDec_::A_varDec_(A_pos pos, Symbol var, Symbol typ, A_exp init) :  A_dec_(pos), _var(var), _typ(typ), _init(init)
{
	precondition(var != 0 && init != 0);
}
Ty_ty A_varDec_::get_type()
{
	if (this->type == 0) {
		if (this->_typ == 0) { this->type = _init->get_type(); }
		else {
			if (Symbol_to_string(this->_typ) == "string") 	  { this->type = Ty_String(); }
			else if (Symbol_to_string(this->_typ) == "int")  { this->type = Ty_Int(); }
			else if (Symbol_to_string(this->_typ) == "bool") { this->type = Ty_Bool(); }
			else { EM_error("somehow got _typ to be a wrong thing:" + Symbol_to_string(this->_typ), true); }
		}
	}
	return type;
}
void A_varDec_::type_check()
{
	if (this->_typ != 0 && this->get_type() != _init->get_type()) {
		EM_error("Init is not the right type. Should be " + to_String(this->get_type())
								+ " but is " + to_String(_init->get_type()), true);
	}
}

A_functionDec_::A_functionDec_(A_pos pos, A_fundecList functions_that_might_call_each_other) : A_dec_(pos), theFunctions(functions_that_might_call_each_other)
{
	precondition(functions_that_might_call_each_other != 0);
}
A_fundecList_::A_fundecList_(A_fundec head, A_fundecList tail) :  AST_node_(head->pos()), _head(head), _tail(tail)
{
	precondition(head != 0);
}
A_fundec_::A_fundec_(A_pos pos, Symbol name, A_fieldList params, Symbol result,  A_exp body) :  AST_node_(pos), _name(name), _params(params), _result(result), _body(body)
{
	precondition(name != 0 && body != 0);
}



A_ty_::A_ty_(A_pos p) : AST_node_(p)
{
}
A_nametyList_::A_nametyList_(A_namety head, A_nametyList tail) :  AST_node_(head->pos()), _head(head), _tail(tail)
{
	precondition(head != 0);
}
A_namety_::A_namety_(A_pos pos, Symbol name, A_ty ty) :  AST_node_(pos), _name(name), _ty(ty)
{
	precondition(name != 0 && ty != 0);
}

A_fieldList_::A_fieldList_(A_field head, A_fieldList tail) :  AST_node_(head->pos()), _head(head), _tail(tail)
{
	precondition(head != 0);
}
A_field_::A_field_(A_pos pos, Symbol name, Symbol typ) :  AST_node_(pos), _name(name), _typ(typ)
{
	precondition(name != 0 && typ != 0);
}


A_nameTy_::A_nameTy_(A_pos pos, Symbol name) :  A_ty_(pos), _name(name)
{
	precondition(name != 0);
}

A_recordty_::A_recordty_(A_pos pos, A_fieldList record) :  A_ty_(pos), _record(record)
{
}

A_arrayty_::A_arrayty_(A_pos pos, Symbol array) :  A_ty_(pos), _array(array)
{
	precondition(array != 0);
}

bool have_AST_attrs = false;
