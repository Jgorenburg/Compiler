// Like HERA_code, but for data

#include "AST.h"

// works for any node that does not generate DLABELS 
string AST_node_::HERA_data()
{

	std::list<AST_node_*> children = this->getChildren();
	string data = "";

	while (!children.empty()) {
		data = data + children.front()->HERA_data();
		children.pop_front();
	}
	return data;

}

string A_root_::HERA_data()
{
	return main_expr->HERA_data() + "\n"; //no data needed from a root
}

/*string A_leafExp_::HERA_data() //overwritten for string
{
	return ""; //defaults to nothing
}*/

string A_stringExp_::HERA_data()
{
	return "DLABEL(" + this->cur_DLABEL_s() + ")\t" +
			"LP_STRING(" + value + ")\n";
}

/*string A_opExp_::HERA_data()
{
	return _left->HERA_data() + _right->HERA_data(); //so far op expressions have way to add data themselves
}

string A_callExp_::HERA_data()
{
	if (_args == 0) { return ""; }
	else { return _args->HERA_data(); }
}

string A_expList_::HERA_data(string soFar)
{
	if (_tail == 0) {
		return _head->HERA_data() + soFar;
	}
	else {
		return _tail->HERA_data(_head->HERA_data() + soFar);
	}
}

string A_ifExp_::HERA_data()
{
	if ( _else_or_null != 0 ) {
		return _test->HERA_data() + _then->HERA_data() + _else_or_null->HERA_data();
	}
	else {
		return _test->HERA_data() + _then->HERA_data();
	}
}

string A_whileExp_::HERA_data()
{
	return _test->HERA_data() + _body->HERA_data();
}

string A_breakExp_::HERA_data()
{
	return "";
}

string A_seqExp_::HERA_data()
{
	if (_seq == 0) { return ""; }
	else { return _seq->HERA_data(); }
}
*/
