// makes sure each DLABEL is unique, based off result_reg.cc

#include "AST.h"


static int next_unique_number = 0;

int AST_node_::init_DLABEL()  // generate unique numbers, starting from 0, each time this is called
{
	int my_number = next_unique_number;
	next_unique_number = next_unique_number + 1;
	// end of atomic transaction
	return my_number;
}
