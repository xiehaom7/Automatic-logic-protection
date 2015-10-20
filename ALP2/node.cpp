#include "node.h"

node::node(const string node_name, module* upper_module_ptr, 
	Node_type node_type, cell* cell_ptr, module* module_ptr)
	:sNodeName(node_name)
	, pUpperModule(upper_module_ptr)
	, eNodeType(node_type)
	, iInputNum(0)
	, iOutputNum(0)
{
	if (node_type == TYPE_CELL) {
		pType.pCell = cell_ptr;
	}
	else if (node_type == TYPE_MODULE) {
		pType.pModule = module_ptr;
	}
}

node::node(const string node_name, module* upper_module_ptr, Node_type node_type, 
	cell* cell_ptr, module* module_ptr, vector<net*> &vi, vector<net*> &vo)
	:sNodeName(node_name)
	, pUpperModule(upper_module_ptr)
	, eNodeType(node_type)
{
	if (node_type == TYPE_CELL) {
		pType.pCell = cell_ptr;
	}
	else if (node_type == TYPE_MODULE) {
		pType.pModule = module_ptr;
	}
	vInputs = vi;
	vOutputs = vo;
	iInputNum = vInputs.size();
	iOutputNum = vOutputs.size();
}

node::~node() {}

int	node::find_input_net(net* tar_net) {
	int res_index = 0;
	vector<net*>::const_iterator ite;
	for (ite = vInputs.cbegin(); ite != vInputs.cend(); ite++, res_index++) {
		if (*ite == tar_net) {
			return res_index;
		}
	}
	return -1;
}


