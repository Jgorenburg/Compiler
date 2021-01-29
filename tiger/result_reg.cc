#include "AST.h"
#include <list>
#include <vector>


/*
 * methods for working with "result_reg" attribute
 */


int AST_node_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
	int current_largest = -1;
	int to_add = 0;

	std::list<AST_node_*> children = this->getChildren();
	std::list<int> SUlist;
	std::transform(children.begin(), children.end(), std::back_inserter(SUlist), [](AST_node_* child) { return child->result_reg();});
	if (SUlist.empty()) { // if it's a root, assign it 1
		return 1;
	}
	else {
		while (!SUlist.empty()) {
			if (SUlist.front() > current_largest) { // if this child takes more registers than previous ones, make it the new largest
				current_largest = SUlist.front();   // makes new largest
				to_add = 0; 						// resets the counter
			}
			else if (SUlist.front() == current_largest){  // if it is the same size, add one to the counter
				to_add++;						 // this will be added at the end if this is still the largest SU number
			}
			SUlist.pop_front();
		}
		return current_largest + to_add;
	}

}
