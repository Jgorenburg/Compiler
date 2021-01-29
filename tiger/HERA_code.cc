#include "AST.h"
#include "types.h"
/*
 * HERA_code methods
 */

const string indent_math = "    ";  // might want to use something different for, e.g., branches
const string indent_call = "\n";    // makes an empty line before a function call


string AST_node_::HERA_code()  // Default used during development; could be removed in final version
{
	string message = "HERA_code() requested for AST node type not yet having a HERA_code() method";
	EM_error(message);
	return "#error " + message;  //if somehow we try to HERA-C-Run this, it will fail
}


string A_root_::HERA_code()
{
	EM_debug("Got HERA_code root");
	return "CBON()\n\n" + main_expr->HERA_code();  // was SETCB for HERA 2.3
}



string A_intExp_::HERA_code()
{
	return indent_math + "SET(" + result_reg_s() + ", " + str(value) +")\n";
}

string A_stringExp_::HERA_code()
{
	return indent_math + "SET(" + result_reg_s() + ", " + this->cur_DLABEL_s() + ")\n"; // code for putting string in a register
}

// For me, true and false just indicate whether the 0 register is on or off
string A_boolExp_::HERA_code()
{
	// Sets register to 1 or 0
	if (value) { return indent_math + "SET(" + this->result_reg_s() + ", 1)\n"; }
	else { return indent_math + "SET(" + this->result_reg_s() + ", 0)\n"; }

}

string A_varExp_::HERA_code()
{
	return _var->HERA_code();
}


static string HERA_math_op(Position p, A_oper op) // needed for opExp
{
	switch (op) {
	case A_plusOp:
		return "ADD";
	case A_timesOp:
		return "MUL";
	case A_minusOp:
		return "SUB";
	default:
		EM_error("Unhandled case in HERA_math_op", false, p);
		return "Oops_unhandled_hera_math_op";
	}
}

static string HERA_cmp_op(Position p, A_oper op) // needed for opExp
{
	switch (op) {
	case A_eqOp:
		return "BZ";
	case A_neqOp:
		return "BNZ";
	case A_ltOp:
		return "BL";
	case A_leOp:
		return "BLE";
	case A_gtOp:
		return "BG";
	case A_geOp:
		return "BGE";
	default:
		EM_error("Unhandled case in HERA_math_op", false, p);
		return "Oops_unhandled_hera_math_op";
	}
}

// Op expressions require two ints for children and return an int or bool
string A_opExp_::HERA_code()
{

	this->type_check(this->get_type() == Ty_Bool());  	// type is a bool if it's a comp

	string leftSide = _left->HERA_code();				// defining these here so string comp can modify them
	string rightSide = _right->HERA_code();

	int leftSize = _left->result_reg();
	int rightSize = _right->result_reg();
	string leftReg;

	if (leftSize == rightSize) { leftReg = this->result_reg_s(); } 	// if L and R have the same SU number, we will move L to the register
	else { leftReg = _left->result_reg_s(); }												// of the op, so this makes sure the op call uses the right register

	string the_op;
	if (this->get_type() == Ty_Int()) {															// type is an int if its +/*/-
		the_op = indent_math + HERA_math_op(pos(), _oper) + "(" +
						this->result_reg_s() + ", " +
						leftReg + ", " +
						_right->result_reg_s() + ")\n";
	}
	else if (_right->get_type() == Ty_Int()){												// type is bool if its a comparison
																																	// below is code for int comparisons
		string label = gen_LABEL();
		the_op = indent_math + "CMP(" + leftReg + ", " + _right->result_reg_s() + ")\n" 	// The CMP expression
				   + indent_math + "SET(" + this->result_reg_s() + ", 1)\n"										// Default sets true
				   +	HERA_cmp_op(pos(), _oper) + "(Op_" + label + ")\n"											// If CMP true, skips to end
				   + indent_math + "SET(" + this->result_reg_s() + ", 0)\n"										// Otherwise sets false
			     + "LABEL(Op_" + label + ")\n";																							// End label
	}

	else {																													// code for string comparison
																																	// similar to funct call
		string label = gen_LABEL();

		// setting up call to tstrcmp()
		string FP_set = "MOVE(FP_alt, SP)\n";
		string SP_inc = "INC(SP, 5)\n";																								// always takes two args
		string store_right = "STORE(" + _right->result_reg_s() + ", 4, FP_alt)\n";			// storing commands for the two params
		string store_left = "STORE(" + _left->result_reg_s() + ", 3, FP_alt)\n";				// don't use leftReg bcz no move happens here
		string call = "CALL(FP_alt, tstrcmp)\n";
		string retrieve = "LOAD(" + this->result_reg_s() + ", 3, FP_alt)\n";
		string SP_dec = "DEC(SP, 5)\n";

		// using storage - want to store immidietly after getting
		leftSide = leftSide + store_left;
		rightSide = rightSide + store_right;

		// the actual op
		the_op = indent_math + "SET(" + this->result_reg_s() + ", 1)\n"										// Default sets true
							+	HERA_cmp_op(pos(), _oper) + "(Op_" + label + ")\n"										// If CMP true, skips to end
							+ indent_math + "SET(" + this->result_reg_s() + ", 0)\n"								// Otherwise sets false
							+ "LABEL(Op_" + label + ")\n";																					// End label

		return  FP_set + SP_inc + leftSide + rightSide + call + retrieve + the_op + SP_dec;

	}

	if (leftSize > rightSize) {
		return leftSide + rightSide + the_op;
	}
	else if (leftSize < rightSize) {
		return rightSide + leftSide + the_op;
	}
	else {     														// if they are equal
		return leftSide +
				indent_math + "MOVE(" + this->result_reg_s() + ", " + _left->result_reg_s() + ")\n" + // moves register some the second one doesn't overwrite it
				rightSide + the_op;
	}
}

// this function recursively creates the store calls for the parameters
string call_store_helper(A_expList args, int count = 1, string sofar = "")
{
		if (args->_tail == 0) {
			return sofar + "STORE(" + args->_head->result_reg_s() + ", " + std::to_string(2 + count) + ", FP_alt)\n";
		}
		else {
			return call_store_helper(args->_tail, count + 1,
				sofar + "STORE(" + args->_head->result_reg_s() + ", " + std::to_string(2 + count) + ", FP_alt)\n");
		}
}

string A_callExp_::HERA_code()
{
	EM_debug("in call exp");
	this->type_check();

	int SP_num = 3;																									// if no params, inc by 3
	string inside = "";
	if (_args != 0) {      																					// only runs if there are params
		inside = _args->HERA_code(true);
 		SP_num = SP_num + _args->length();														// SP includes params
	}
	// the steps of a funct call
	string FP_set = "MOVE(FP_alt, SP)\n";
	string SP_inc = "INC(SP, " + std::to_string(SP_num) + ")\n";
	string call = "CALL(FP_alt, " + Symbol_to_string(_func) + ")\n";
	string retrieve = "LOAD(" + this->result_reg_s() + ", 3, FP_alt)\n";
	string SP_dec = "DEC(SP, " + std::to_string(SP_num) + ")\n";

	return FP_set + SP_inc + inside + call + retrieve + SP_dec;
}


string A_seqExp_::HERA_code()
{
	if (_seq == 0) {
	EM_debug("right way");
	return indent_math + ""; }			// if it's a null expression
	else { return _seq->HERA_code() + "MOVE(" + this->result_reg_s() + ", " + _seq->get_last()->result_reg_s() + ")\n"; }

}


// recursively builds the children and adds move instructions after them to prep for the function call
string A_expList_::HERA_code(string soFar, int count, bool store)
{
	// code for storing param
	string to_store = "";
	if (store) { to_store = "STORE(" + _head->result_reg_s() + ", " + std::to_string(2 + count) + ", FP_alt)";	}

	if (_tail == 0) {
		return soFar 																																													// previous params
						+ _head->HERA_code()		   																																		// code for this params
 						+ to_store + "\n";

	}
	else {
		return _tail->HERA_code(soFar
							+ _head->HERA_code()
							+ to_store + "\n",
						count + 1, store);
	}

}

string A_ifExp_::HERA_code()
{
	this->type_check();

	//need one label (two if has else statement)
	string label1 = this->gen_LABEL();

	//builds the code for all three subparts
	string test = _test->HERA_code();
	string then = _then->HERA_code();


	//code is different if there is an else statement
	if (_else_or_null != 0)	{
		string label2 = this->gen_LABEL();
		string else_or_null = _else_or_null->HERA_code();
		return test + "\nADD(" + this->result_reg_s() + ", R0, " + _test->result_reg_s() + ")\nBZ(If_" + label1 + ")\n" + then + "MOVE(" + this->result_reg_s() + ", " + _then->result_reg_s() + ")\n" + "\n BR(If_" + label2 + ")\nLABEL(If_" + label1 +")\n" +  else_or_null + "MOVE(" + this->result_reg_s() + ", " + _else_or_null->result_reg_s() + ")\n" +"\nLABEL(If_" + label2 + ")\n";
	}
	else {
		return test + "\nADD(" + this->result_reg_s() + ", R0, " + _test->result_reg_s() + ")\n" + "\nBZ(If_" + label1 + ")\n" + then + "MOVE(" + this->result_reg_s() + ", " + _then->result_reg_s() + ")\n" +"\nLABEL(If_" + label1 +")\n";
	}

}


string A_whileExp_::HERA_code()
{
	this->type_check();
	//code for returning to the test once the body is done
	string test_loop = "While_" + this->gen_LABEL();
	string label1 = "LABEL(" + test_loop + ")\n";
	string branch1 = "BR(" + test_loop + ")\n";

	//code for exiting loop
	exit = "While_" + this->gen_LABEL();
	string label2 = "LABEL(" + exit + ")\n";
	string branch2 = "ADD(" + this->result_reg_s() + ", R0, " + _test->result_reg_s() + ")\nBZ(" + exit + ")\n";


	string test = _test->HERA_code();
	string body = _body->HERA_code();


	return label1 + test + branch2 + body + branch1 + label2;
}


string A_forExp_::HERA_code()
{

	this->type_check();

	// dealing with init hi and lo
	string lo_code = _lo->HERA_code();
 	string hi_code = _hi->HERA_code();
	int lo_pos = this->get_SP_and_inc();		// How much off FP this should be
	int hi_pos = this->get_SP_and_inc();
	string store_lo = "STORE(" + _lo->result_reg_s() + ", " + str(lo_pos) + ", FP)\n";
	string store_hi = "STORE(" + _hi->result_reg_s() + ", " + str(hi_pos) + ", FP)\n";

	string init = lo_code + store_lo + hi_code + store_hi + indent_call;


	// add var to ST after the lo and hi code because those can't use it
	this->merge_ST(ST_bi(_var, bi_sym_table(Ty_Int(), lo_pos)));


	// control loop code
	exit = "For_" + this->gen_LABEL();
	string control_label = "For_" + this->gen_LABEL();
	string ret = "LABEL(" + control_label + ")\n";
	string load_lo = "LOAD(R1, " + str(lo_pos) + ", FP)\n";
	string load_hi = "LOAD(R2, " + str(hi_pos) + ", FP)\n";
	string comparison = indent_math + "CMP(R1, R2)\n"
			+ "BG(" + exit + ")\n";														// exiting the loop

	string control = ret + load_lo + load_hi + comparison + indent_call;

 	// body of the loop
	string return_br = "BR(" + control_label + ")\n";
	string inc_lo = indent_math + "INC(R1, 1)\n";
	string sto_lo = "STORE(R1, " + str(lo_pos) + ", FP)\n";
	string body = _body->HERA_code() + load_lo + inc_lo + sto_lo + return_br;

	string exit_label = "LABEL(" + exit + ")\n";

	return init + control + body + exit_label;

}

string A_breakExp_::HERA_code()
{
	try {
		string where_leave = this->get_exit();
		return "BR(" + where_leave + ")\n";
	}
	catch(...) {
		EM_error("Break with no loop");
	}
	return "This is a problem\n";  // just to get rid of warning in compiler
}



string A_simpleVar_::HERA_code()
{
	if (this->valid_var()){
		return "LOAD(" + this->result_reg_s() + ", " + str(lookup(_sym, this->get_ST())._reg) + ", FP)\n";
	}
	else {
		EM_error("Variable is not defined in this scope: " + Symbol_to_string(_sym), true);
		return "";
	}
}

string A_letExp_::HERA_code()
{
	string decs_code = _decs->HERA_code();
//	inc_SP(_decs->length());
	string for_SP = "INC(SP, " + str(_decs->length())  + ")\n";
	return for_SP + decs_code + indent_call + _body->HERA_code()
					+ "MOVE(" + this->result_reg_s() + ", " + _body->result_reg_s() + ")\n";
}

string A_varDec_::HERA_code()
{

	// adds this var to ST and passes to the let exp (inefficient but reliable)
	this->merge_ST(ST_bi(this->get_var(), bi_sym_table(this->get_type(), this->get_SP())));
	this->send_or_recieve_ST(this->get_ST());
	int this_SP = this->get_SP_and_inc();
	this->send_or_recieve_SP(this->get_SP());

	return _init->HERA_code() + "STORE(" + _init->result_reg_s() + ", " + str(this_SP) + ", FP)\n";
}

string A_assignExp_::HERA_code()
{
	this->type_check();

	// adds new value to pos of old def
	int def_SP = lookup(_var->get_var(), this->get_ST())._reg;
	return _exp->HERA_code() + "STORE(" + _exp->result_reg_s() + ", " + str(def_SP) + ", FP)\n";
}

// recursively builds the children and adds move instructions after them to prep definitions
string A_decList_::HERA_code(string soFar)
{
	//this->inc_SP(1);
	if (_tail == 0) {
		//this->merge_ST(ST_bi(_head->get_var(), bi_sym_table(_head->get_type(), this->get_SP_and_inc())));
		return soFar + _head->HERA_code() + "\n";
	}
	else {
		return _tail->HERA_code(soFar + _head->HERA_code() + "\n");
	}

}
