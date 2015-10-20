#pragma once
#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include "net.h"
#include "cell.h"

using namespace std;

class module;

typedef enum { TYPE_CELL = 0, TYPE_MODULE } Node_type;
typedef union {
	cell			*pCell;
	module			*pModule;
} Type_ptr;

class node {
private:
	string			sNodeName;
	module*			pUpperModule;
	Node_type		eNodeType;
	Type_ptr		pType;
	vector<net*>	vInputs;
	vector<net*>	vOutputs;
	int				iInputNum;
	int				iOutputNum;
	int				iFunc;

public:
	node(const string node_name, module* upper_module_ptr, Node_type node_type, 
		cell* cell_ptr, module* module_ptr);
	node(const string node_name, module* upper_module_ptr, Node_type node_type, 
		cell* cell_ptr, module* module_ptr, vector<net*> &vi, vector<net*> &vo);
	~node();

public:
	string			get_node_name() const;
	module*			get_upper_module() const;
	Node_type		get_node_type() const;
	cell*			get_cell_ref() const;
	module*			get_module_ref() const;
	int				get_input_num() const;
	int				get_output_num() const;
	net*			get_input_net(int pos) const;
	net*			get_output_net(int pos) const;
	int				get_ifunc() const;

	node&			set_node_name(const string &node_name);
	node&			set_upper_module(module* const p_module);
	node&			set_node_type(Node_type node_type);
	node&			set_cell_ref(cell* cell_ptr);
	node&			set_module_ref(module* module_ptr);
	//node&			set_input_list(const vector<net*> &ip);
	//node&			set_output_list(const vector<net*> &op);
	node&			set_output_net(net* tar_net, int pos);
	node&			set_input_net(net* tar_net, int pos);
	node&			set_ifunc(int new_value);

	int				find_input_net(net* tar_net);
};

inline	string			node::get_node_name() const { return sNodeName; }
inline	module*			node::get_upper_module() const { return pUpperModule; }
inline	Node_type		node::get_node_type() const { return eNodeType; }
inline	cell*			node::get_cell_ref() const { assert(eNodeType == TYPE_CELL); return pType.pCell; }
inline	module*			node::get_module_ref() const { assert(eNodeType == TYPE_MODULE); return pType.pModule; }
inline	int				node::get_input_num() const { return iInputNum; }
inline	int				node::get_output_num() const { return iOutputNum; }
inline	net*			node::get_input_net(int pos) const { assert(pos < iInputNum); return vInputs[pos]; }
inline	net*			node::get_output_net(int pos) const { assert(pos < iOutputNum); return vOutputs[pos]; }
inline	int				node::get_ifunc() const { return iFunc; }

inline	node&			node::set_node_name(const string &node_name) { sNodeName = node_name; return *this; }
inline	node&			node::set_upper_module(module* const p_module) { pUpperModule = p_module; return *this; }
inline	node&			node::set_node_type(Node_type node_type) {
	if (eNodeType == TYPE_CELL) {
		pType.pCell = NULL;
	}
	else {
		pType.pModule = NULL;
	}
	eNodeType = node_type;
	return *this;
}
inline	node&			node::set_cell_ref(cell* cell_ptr) { assert(eNodeType == TYPE_CELL); pType.pCell = cell_ptr; return *this; }
inline	node&			node::set_module_ref(module* module_ptr) { assert(eNodeType == TYPE_MODULE); pType.pModule = module_ptr; return *this; }
inline	node&			node::set_output_net(net* tar_net, int pos) {
	if (pos >= iOutputNum)
		throw exception("position out of output range. (set_output_net)");
	if (vOutputs[pos] != NULL && tar_net != NULL)
		throw exception("overlapping existed connection. (set_output_net)");
	vOutputs[pos] = tar_net;
	return *this;
}
inline	node&			node::set_input_net(net* tar_net, int pos) {
	if (pos >= iInputNum)
		throw exception("position out of input range. (set_input_net)");
	if (vInputs[pos] != NULL && tar_net != NULL)
		throw exception("overlapping existed connection. (set_input_net)");
	vInputs[pos] = tar_net;
	return *this;
}

inline	node&			node::set_ifunc(int new_value) { iFunc = new_value; return *this; }

