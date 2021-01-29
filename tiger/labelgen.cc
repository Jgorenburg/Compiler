// makes sure each DLABEL is unique, based off result_reg.cc

#include "AST.h"


static int next_number = 0;

string AST_node_::gen_LABEL()  // generate unique numbers, starting from 0, each time this is called
{
	int my_number = next_number;
	next_number = next_number + 1;
	// end of atomic transaction
	return std::to_string(my_number);
}
