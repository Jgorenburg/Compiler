#include "AST.h"
#include <list>

using namespace std;

list<AST_node_*> AST_node_::getChildren()
{
	return list<AST_node_*>();
}

list<AST_node_*> A_root_::getChildren()
{
	return list<AST_node_*> {main_expr};
}

list<AST_node_*> A_leafExp_::getChildren()
{
	return list<AST_node_*>(); // a leaf should have no children
}

list<AST_node_*> A_varExp_::getChildren()
{
	return list<AST_node_*> {_var};
}

list<AST_node_*> A_opExp_::getChildren()
{
	return list<AST_node_*> {_left, _right};
}

list<AST_node_*> A_expList_::getChildren()
{
	if (_tail == 0){
		return list<AST_node_*> {_head};
	}
	else {
		list<AST_node_*> rest = _tail->getChildren();
		rest.push_front(_head);
		return rest;
	}

}

list<AST_node_*> A_callExp_::getChildren()
{
	//Russel suggested I use {} instead of ()
	if (_args != 0) { return list<AST_node_*> { _args }; }
	else { return list<AST_node_*> {}; }
}

list<AST_node_*> A_seqExp_::getChildren()
{
	if (_seq != 0) { return list<AST_node_*> {_seq}; }
	else { return list<AST_node_*> {}; }
}


list<AST_node_*> A_ifExp_::getChildren()
{
	if (_else_or_null == 0) {	return list<AST_node_*> {_test, _then};	}
	else 										{	return list<AST_node_*> {_test, _then, _else_or_null};	}
}

list<AST_node_*> A_whileExp_::getChildren()
{
	return list<AST_node_*> {_test, _body};
}

list<AST_node_*> A_forExp_::getChildren()
{
	return list<AST_node_*> {_lo, _hi, _body};
}

list<AST_node_*> A_breakExp_::getChildren()
{
	return list<AST_node_*> {};
}

list<AST_node_*> A_letExp_::getChildren()
{
	return list<AST_node_*> {_decs, _body};
}

list<AST_node_*> A_varDec_::getChildren()
{
	return list<AST_node_*> {_init};
}

list<AST_node_*> A_assignExp_::getChildren()
{
	return list<AST_node_*> {_exp};
}

list<AST_node_*> A_decList_::getChildren()
{
	if (_tail == 0){
		return list<AST_node_*> {_head};
	}
	else {
		list<AST_node_*> rest = _tail->getChildren();
		rest.push_front(_head);
		return rest;
	}

}

list<AST_node_*> A_simpleVar_::getChildren()
{
	return list<AST_node_*>();
}
