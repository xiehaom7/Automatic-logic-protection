#include "redundant_wire.h"



redundant_wire::redundant_wire() {
	pTopModule = NULL;
	rw_wire_added_counter = 0;
	rw_gate_added_counter = 0;
}

void redundant_wire::destroy() {
	vector<RWNode*>::iterator rw_ite;

	for (rw_ite = vRWNodeList.begin(); rw_ite != vRWNodeList.end(); rw_ite++) {
		delete *rw_ite;
	}
	pTopModule = NULL;
	vRWNodeList.clear();
	mapRWNodeList.clear();
	vPrimaryInputList.clear();
	vPrimaryOutputList.clear();
	matrixImplication.resize(0);
	matrixImplicationScreen.resize(0);
	vSimpleIndirectImplication.clear();
	vIndirectImplication.clear();
	vNodeFaninSet.clear();
	vNodeFanoutSet.clear();

	rw_wire_added_counter = 0;
	rw_gate_added_counter = 0;
	return;
}

redundant_wire::~redundant_wire() {
	destroy();
	return;
}

void redundant_wire::set_value(int index, Wire_value value) {
	vRWNodeList[index]->eValue = value;
	lModificationIndexLst.push_back(index);
}

void redundant_wire::set_X() {
	list<int>::iterator ite;
	for (ite = lModificationIndexLst.begin(); ite != lModificationIndexLst.end(); ite++) {
		vRWNodeList[(*ite)]->eValue = X;
	}
	lModificationIndexLst.clear();
	return;
}

int redundant_wire::_construct_module(const vector<int> &input_list,
	vector<int> &output_list, int start_pos, module* tar_module, string& prefix) {
	if (tar_module == NULL)
		throw exception("NULL module specified. (redundant_wire::construct_module)");
	if (tar_module->get_input_num() != input_list.size())
		throw exception(("input numbers from target module " + tar_module->get_module_name() +
			" and input list not match. (redundant_wire::construct_module)").c_str());

	vRWNodeList.resize(vRWNodeList.size() + tar_module->get_net_num() -
		tar_module->get_input_num() - tar_module->get_output_num());

	set<string> visited_module_instances;
	vector<string> topological_node_list;
	vector<string>::const_iterator ite;
	vector<int> input_index_list;
	vector<int> output_index_list;
	int input_num = tar_module->get_input_num();
	int output_num = tar_module->get_output_num();
	int index = start_pos;
	int fanin_index;
	net* current_net;
	node* driving_node;
	string driving_node_name;
	int i;
	if (!tar_module->get_topological_sequence(topological_node_list))
		throw exception(("module " + tar_module->get_module_name() +
			" has loop. (redundant_wire::construct_module)").c_str());
	for (i = 0; i < input_num; i++) {
		current_net = tar_module->get_input_net(i);
		mapRWNodeList[prefix + "." + current_net->get_net_name()] = input_list[i];
	}
	for (ite = topological_node_list.cbegin(); ite != topological_node_list.cend(); ite++) {
		input_index_list.clear();
		output_index_list.clear();
		current_net = tar_module->get_net(*ite);
		if (current_net->get_is_input())
			continue;

		driving_node = current_net->get_fanin_node();
		driving_node_name = driving_node->get_node_name();

		for (i = 0; i < driving_node->get_input_num(); i++) {
			fanin_index = mapRWNodeList[prefix +
				"." + driving_node->get_input_net(i)->get_net_name()];
			input_index_list.push_back(fanin_index);

			if (driving_node->get_node_type() == TYPE_MODULE)
				continue;

			vRWNodeList[fanin_index]->vFanout.push_back(index);
		}

		if (driving_node->get_node_type() == TYPE_MODULE) {
			if (visited_module_instances.find(driving_node_name)
				!= visited_module_instances.end())
				continue;
			index = _construct_module(input_index_list, output_index_list, index,
				driving_node->get_module_ref(), prefix + "." + driving_node_name);
			for (i = 0; i < driving_node->get_output_num(); i++)
				mapRWNodeList[prefix + "." + driving_node->get_output_net(i)->get_net_name()] =
				output_index_list[i];
			visited_module_instances.insert(driving_node_name);
		}
		if (driving_node->get_node_type() == TYPE_CELL) {
			RWNode* new_RW_node = _create_RW_node(current_net, prefix, input_index_list);
			vRWNodeList[index] = new_RW_node;
			mapRWNodeList[new_RW_node->sName] = index++;
		}
	}
	output_list.resize(output_num, -1);
	for (i = 0; i < output_num; i++) {
		output_list[i] = mapRWNodeList[prefix + "." + tar_module->get_output_net(i)->get_net_name()];
	}
	return index;
}

bool redundant_wire::construct(module* tar_module) {
	try {
		if (tar_module == NULL)
			throw exception("NULL top module specified. (redundant_wire::construct)\n");
		
		string top_prefix = tar_module->get_module_name();
		net* temp_net;
		int input_num = tar_module->get_input_num();
		int output_num = tar_module->get_output_num();
		int index = 0;
		int i;
		vector<int> empty_input_index_list;

		pTopModule = tar_module;
		vRWNodeList.resize(input_num + output_num);
		vPrimaryInputList.resize(input_num);
		vPrimaryOutputList.resize(output_num);

		for (i = 0; i < input_num; i++) {
			temp_net = tar_module->get_input_net(i);

			RWNode* new_RW_node = _create_RW_node(temp_net, top_prefix, empty_input_index_list);

			vRWNodeList[index] = new_RW_node;
			vPrimaryInputList[i] = index;
			mapRWNodeList[new_RW_node->sName] = index;
			index++;
		}
		
		_construct_module(vPrimaryInputList, vPrimaryOutputList, index, tar_module, top_prefix);

		int cap = 0;
		for (vector<RWNode*>::iterator ite = vRWNodeList.begin(); ite != vRWNodeList.end(); ite++)
			if (*ite != NULL)
				cap++;
		vRWNodeList.resize(cap);

		_setup_simple_indirect_implication_vector();
		_setup_indirect_implication_vector();
		_setup_implication_matrix();
		_setup_implication_screen_matrix();
		_setup_fanin_fanout_set();
	}
	catch (exception e) {
		cerr << "constructing redundant wire faild. (redundant_wire::construct)\n";
		cerr << "description: " << e.what();
		destroy();
		return false;
	}
	return true;
}

RWNode* redundant_wire::_create_RW_node(net* current_net, string &prefix,
	vector<int> &i_list) {
	RWNode* new_rw_node = new RWNode;
	new_rw_node->sName = prefix + "." + current_net->get_net_name();
	new_rw_node->pCell = (current_net->get_fanin_node() == NULL)
		? NULL
		: current_net->get_fanin_node()->get_cell_ref();
	new_rw_node->eValue = X;
	new_rw_node->vFanin = i_list;
	return new_rw_node;
}

int redundant_wire::get_node_index(string node_name) {
	map<string, int>::iterator ite = mapRWNodeList.find(node_name);
	if (ite == mapRWNodeList.end())
		return -1;
	return ite->second;
}

string redundant_wire::get_node_name(int index) {
	if (index < 0 || index >= vRWNodeList.size())
		throw exception(("index " + std::to_string(index) + " exceeds range (0 : " + 
			std::to_string(vRWNodeList.size()) + "). (redundant_wire::get_node_name)").c_str());
	return vRWNodeList[index]->sName;
}

set<string> redundant_wire::get_node_fanin(string node_name) {
	int index = get_node_index(node_name);
	size_t i;
	set<string> res;
	if (index == -1)
		throw exception(("node " + node_name + 
			" not found. (redundant_wire::get_node_fanin)\n").c_str());
	for (i = 0; i < vRWNodeList[index]->vFanin.size(); i++)
		res.insert(vRWNodeList[vRWNodeList[index]->vFanin[i]]->sName);
	return res;
}

set<string> redundant_wire::get_node_fanout(string node_name) {
	int index = get_node_index(node_name);
	size_t i;
	set<string> res;
	if (index == -1)
		throw exception(("node " + node_name +
			" not found. (redundant_wire::get_node_fanout)\n").c_str());
	for (i = 0; i < vRWNodeList[index]->vFanout.size(); i++)
		res.insert(vRWNodeList[vRWNodeList[index]->vFanout[i]]->sName);
	return res;
}

string redundant_wire::get_cell_type(string node_name) {
	int index = get_node_index(node_name);
	vector<string> res;
	if (index == -1)
		throw exception(("node " + node_name +
			" not found. (redundant_wire::get_cell_type)\n").c_str());
	if (vRWNodeList[index]->pCell == NULL)
		return "";
	return vRWNodeList[index]->pCell->cell_name;
}

void redundant_wire::_setup_simple_indirect_implication_vector() {
	unsigned node_size_x2 = vRWNodeList.size() * 2;
	vector<list<Implication_comb>>::iterator ite;

	for (ite = vSimpleIndirectImplication.begin(); 
		ite != vSimpleIndirectImplication.end(); ite++)
		(*ite).clear();

	vSimpleIndirectImplication.resize(node_size_x2);
	return;
}

void redundant_wire::_setup_indirect_implication_vector() {
	unsigned node_size_x2 = vRWNodeList.size() * 2;
	vector<list<Implication_comb>>::iterator ite;

	for (ite = vIndirectImplication.begin(); ite != vIndirectImplication.end(); ite++)
		(*ite).clear();

	vIndirectImplication.resize(node_size_x2);
	return;
}

void redundant_wire::_setup_implication_matrix() {
	unsigned node_size_x2 = vRWNodeList.size() * 2;
	matrixImplication.resize(node_size_x2 * node_size_x2, false);
	return;
}

void redundant_wire::_setup_implication_screen_matrix() {
	unsigned node_size_x2 = vRWNodeList.size() * 2;
	matrixImplicationScreen.resize(node_size_x2 * node_size_x2, false);
	return;
}

void redundant_wire::_setup_fanin_fanout_set() {
	vector<int>::const_iterator fanin_ite;
	vector<int>::const_iterator fanout_ite;
	int node_size = vRWNodeList.size();
	int i;

	vNodeFaninSet.clear();
	vNodeFanoutSet.clear();
	vNodeFaninSet.resize(node_size);
	vNodeFanoutSet.resize(node_size);

	for (i = 0; i<node_size; i++) {
		for (fanin_ite = vRWNodeList[i]->vFanin.cbegin(); 
			fanin_ite != vRWNodeList[i]->vFanin.cend(); fanin_ite++) {
			vNodeFaninSet[i].insert((*fanin_ite));
			vNodeFaninSet[i].insert(
				vNodeFaninSet[*fanin_ite].begin(), vNodeFaninSet[*fanin_ite].end());
		}
	}
	for (i = node_size - 1; i >= 0; i--) {
		for (fanout_ite = vRWNodeList[i]->vFanout.cbegin(); 
			fanout_ite != vRWNodeList[i]->vFanout.cend(); fanout_ite++) {
			vNodeFanoutSet[i].insert((*fanout_ite));
			vNodeFanoutSet[i].insert(
				vNodeFanoutSet[*fanout_ite].begin(), vNodeFanoutSet[*fanout_ite].end());
		}
	}
	return;
}

void redundant_wire::_lookup_backward_table(unsigned index, int tar_index, map<unsigned, 
	bitset<MAX_CELL_INPUTS_X2>> &table, stack<int> &stack_index) {
	map<unsigned, bitset<MAX_CELL_INPUTS_X2>>::iterator m_ite;
	int input_num = vRWNodeList[tar_index]->pCell->input_num;
	int i;
	m_ite = table.find(index);
	if (m_ite == table.end())
		return;
	for (i = 0; i < input_num; i++) {
		if ((*m_ite).second.test(i)) {
			this->set_value(vRWNodeList[tar_index]->vFanin[i], ONE);
			stack_index.push(vRWNodeList[tar_index]->vFanin[i]);
		}
		else if ((*m_ite).second.test(i + input_num)) {
			this->set_value(vRWNodeList[tar_index]->vFanin[i], ZERO);
			stack_index.push(vRWNodeList[tar_index]->vFanin[i]);
		}
	}
	return;
}

void redundant_wire::_lookup_forward_table(unsigned index, int tar_index, map<unsigned,
	bool> &table, stack<int> &stack_index) {
	map<unsigned, bool>::iterator m_ite;
	m_ite = table.find(index);
	if (m_ite == table.end())
		return;
	this->set_value(tar_index, ((*m_ite).second) ? ONE : ZERO);
	stack_index.push(tar_index);	
	return;
}

unsigned redundant_wire::_calculate_inputs_index(int tar_index) {
	int input_num = vRWNodeList[tar_index]->pCell->input_num;
	unsigned index = 0;
	int i;
	for (i = 0; i < input_num; i++) {
		if (vRWNodeList[vRWNodeList[tar_index]->vFanin[i]]->eValue == ONE) {
			index |= 0x01 << i;
		}
		else if (vRWNodeList[vRWNodeList[tar_index]->vFanin[i]]->eValue == ZERO) {
			index |= 0x01 << (i + input_num);
		}
	}
	return index;
}

void redundant_wire::_backward_justify(int tar_index, stack<int> &stack_index) {
	cell* tar_cell = vRWNodeList[tar_index]->pCell;
	unsigned index = _calculate_inputs_index(tar_index);
	
	if (vRWNodeList[tar_index]->eValue == ONE) {
		_lookup_backward_table(index, tar_index, tar_cell->one_backward_ref_table, stack_index);
	}
	else if (vRWNodeList[tar_index]->eValue == ZERO) {
		_lookup_backward_table(index, tar_index, tar_cell->zero_backward_ref_table, stack_index);
	}
	return;
}

void redundant_wire::_forward_justify(int tar_index, stack<int> &stack_index) {
	cell* tar_cell = vRWNodeList[tar_index]->pCell;
	unsigned index = _calculate_inputs_index(tar_index);
	_lookup_forward_table(index, tar_index, tar_cell->forward_ref_table, stack_index);
}

void redundant_wire::backward_justification(Implication_list &imp_list) {
	int tar_index = imp_list.ori.index;
	Wire_value tar_value = imp_list.ori.val;
	int current_index;
	stack<int> stack_index;

	set_X();

	this->set_value(tar_index, tar_value);
	stack_index.push(tar_index);

	while (!stack_index.empty()) {
		current_index = stack_index.top();
		stack_index.pop();

		if (current_index != tar_index) {
			Implication_comb new_ic;
			new_ic.index = current_index;
			new_ic.val = vRWNodeList[current_index]->eValue;
			imp_list.imp_results[vRWNodeList[current_index]->sName] = new_ic;
		}

		if (!vRWNodeList[current_index]->vFanin.empty())
			_backward_justify(current_index, stack_index);
	}
	return;
}

void redundant_wire::direct_justification(Implication_list &imp_list) {
	int tar_index = imp_list.ori.index;
	Wire_value tar_value = imp_list.ori.val;
	int current_index, current_fanout_index;
	stack<int> stack_index;
	int i;
	int fanout_net_num;

	set_X();

	this->set_value(tar_index, tar_value);
	stack_index.push(tar_index);

	while (!stack_index.empty()) {
		current_index = stack_index.top();
		stack_index.pop();

		if (current_index != tar_index) {
			Implication_comb new_ic;
			new_ic.index = current_index;
			new_ic.val = vRWNodeList[current_index]->eValue;
			imp_list.imp_results[vRWNodeList[current_index]->sName] = new_ic;
		}

		if (!vRWNodeList[current_index]->vFanin.empty())
			_backward_justify(current_index, stack_index);

		if (!vRWNodeList[current_index]->vFanout.empty()) {
			fanout_net_num = vRWNodeList[current_index]->vFanout.size();
			for (i = 0; i < fanout_net_num; i++) {
				current_fanout_index = vRWNodeList[current_index]->vFanout[i];

				if (vRWNodeList[current_fanout_index]->eValue == X) {
					_forward_justify(current_fanout_index, stack_index);
				}
				else {
					_backward_justify(current_fanout_index, stack_index);
				}
			}
		}
	}
	return;
}