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
	vImplication.clear();
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

		_setup_implication_vector();
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
	if (index < 0 || (unsigned)index >= vRWNodeList.size())
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

int redundant_wire::get_implication_num() {
	if (matrixImplication.size() == 0)
		return -1;
	return std::count(std::begin(matrixImplication), 
		std::end(matrixImplication), true);
}

void redundant_wire::_setup_implication_vector() {
	unsigned node_size_x2 = vRWNodeList.size() * 2;
	vector<list<Implication_comb>>::iterator ite;

	for (ite = vImplication.begin(); ite != vImplication.end(); ite++)
		(*ite).clear();

	vImplication.resize(node_size_x2);
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

void redundant_wire::justification(Implication_list &imp_list) {
	int tar_index = imp_list.ori.index;
	Wire_value tar_value = imp_list.ori.val;
	size_t size = vRWNodeList.size();
	int matrix_index;
	int current_index, current_fanout_index;
	Wire_value current_value;
	stack<int> stack_index;
	int i;
	int fanout_net_num;

	set_X();

	this->set_value(tar_index, tar_value);
	stack_index.push(tar_index);

	while (!stack_index.empty()) {
		current_index = stack_index.top();
		current_value = vRWNodeList[current_index]->eValue;
		stack_index.pop();

		if (current_index != tar_index) {
			Implication_comb new_ic;
			new_ic.index = current_index;
			new_ic.val = vRWNodeList[current_index]->eValue;
			imp_list.imp_results[vRWNodeList[current_index]->sName] = new_ic;
		}

		//check if there are any implications from fanin nodes
		matrix_index = (current_value == ZERO) ? current_index * 2 + 1 : current_index * 2;
		valarray<bool> matrix_slice = matrixImplication[std::slice(matrix_index, size * 2, size * 2)];
		for (i = 0; (unsigned)i < size; i++) {
			if (matrix_slice[i * 2] && vRWNodeList[i]->eValue == X) {
				this->set_value(i, ONE);
				stack_index.push(i);
			}
			if (matrix_slice[i * 2 + 1] && vRWNodeList[i]->eValue == X) {
				this->set_value(i, ZERO);
				stack_index.push(i);
			}
		}

		if (!vRWNodeList[current_index]->vFanin.empty()) {
			_backward_justify(current_index, stack_index);
		}

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

void redundant_wire::justification_full(Implication_list &imp_list, set<Implication_comb, Implication_comb_compare> &update_stack) {
	int tar_index = imp_list.ori.index;
	Wire_value tar_value = imp_list.ori.val;
	size_t size = vRWNodeList.size();

	int matrix_index, inversed_left, inversed_right;
	int current_index, current_fanout_index;
	Wire_value current_value;
	stack<int> stack_index;
	unsigned i;
	unsigned fanout_net_num;

	set_X();

	this->set_value(tar_index, tar_value);
	stack_index.push(tar_index);

	while (!stack_index.empty()) {
		current_index = stack_index.top();
		current_value = vRWNodeList[current_index]->eValue;
		stack_index.pop();

		if (current_index != tar_index) {
			Implication_comb new_ic;
			new_ic.index = current_index;
			new_ic.val = vRWNodeList[current_index]->eValue;
			imp_list.imp_results[vRWNodeList[current_index]->sName] = new_ic;

			inversed_left = (new_ic.val == ZERO) ? new_ic.index * 2 + 1 : new_ic.index * 2;
			inversed_right = (tar_value == ZERO) ? tar_index * 2 + 1 : tar_index * 2;
			if (!matrixImplication[inversed_left * size * 2 + inversed_right]) {
				matrixImplication[inversed_left * size * 2 + inversed_right] = true;
				Implication_comb update_comb;
				update_comb.index = new_ic.index;
				update_comb.val = (tar_value == ZERO) ? ONE : ZERO;
				update_stack.insert(update_comb);
			}
		}

		//check if there are any implications from fanin nodes
		matrix_index = (current_value == ZERO) ? current_index * 2 + 1 : current_index * 2;
		valarray<bool> matrix_slice = matrixImplication[std::slice(matrix_index, size * 2, size * 2)];
		for (i = 0; i < size; i++) {
			if (matrix_slice[i * 2] && vRWNodeList[i]->eValue == X) {
				this->set_value(i, ONE);
				stack_index.push(i);
			}
			if (matrix_slice[i * 2 + 1] && vRWNodeList[i]->eValue == X) {
				this->set_value(i, ZERO);
				stack_index.push(i);
			}
		}

		if (!vRWNodeList[current_index]->vFanin.empty()) {
			_backward_justify(current_index, stack_index);
		}

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

void redundant_wire::_matrix_generator(Implicaton_method method) {
	Implication_list new_imp_list;
	map<string, Implication_comb>::const_iterator c_ite;
	size_t size = vRWNodeList.size();
	unsigned i;

	for (i = 0; i < size; i++) {
		new_imp_list.ori.index = i;
		new_imp_list.ori.val = ZERO;
		new_imp_list.imp_results.clear();

		switch (method) {
		case BACKWARD:
			backward_justification(new_imp_list);
			break;
		case DIRECT:
			direct_justification(new_imp_list);
			break;
		case INDIRECT_LOW:
			justification(new_imp_list);
			break;
		default:
			throw exception("unknown method. (redundant_wire::_matrix_generator)\n");
		}
		
		for (c_ite = new_imp_list.imp_results.cbegin(); c_ite != new_imp_list.imp_results.cend(); c_ite++) {
			((*c_ite).second.val == ZERO)
				? matrixImplication[i * 2 * size * 2 + (*c_ite).second.index * 2] = true
				: matrixImplication[i * 2 * size * 2 + (*c_ite).second.index * 2 + 1] = true;
		}

		new_imp_list.ori.index = i;
		new_imp_list.ori.val = ONE;
		new_imp_list.imp_results.clear();

		switch (method) {
		case BACKWARD:
			backward_justification(new_imp_list);
			break;
		case DIRECT:
			direct_justification(new_imp_list);
			break;
		case INDIRECT_LOW:
			justification(new_imp_list);
			break;
		default:
			throw exception("unknown method. (redundant_wire::_matrix_generator)\n");
		}
		for (c_ite = new_imp_list.imp_results.cbegin(); c_ite != new_imp_list.imp_results.cend(); c_ite++) {
			((*c_ite).second.val == ZERO)
				? matrixImplication[(i * 2 + 1) * size * 2 + (*c_ite).second.index * 2] = true
				: matrixImplication[(i * 2 + 1) * size * 2 + (*c_ite).second.index * 2 + 1] = true;
		}
	}
	_generate_implication_vector();
	return;
}

void redundant_wire::implication_matrix_generator_backward() {
	_matrix_generator(BACKWARD);
	return;
}

void redundant_wire::implication_matrix_generator_direct() {
	_matrix_generator(DIRECT);
	return;
}

void redundant_wire::implication_matrix_generator() {
	_matrix_generator(INDIRECT_LOW);
	return;
}

void redundant_wire::implication_matrix_generator_full() {
	Implication_list new_imp_list;
	Implication_comb new_ic;
	map<string, Implication_comb>::const_iterator c_ite;
	set<Implication_comb, Implication_comb_compare> update_set;
	size_t size = vRWNodeList.size();
	unsigned i;
	int matrix_index;

	for (i = 0; i < size; i++) {
		new_ic.index = i;
		new_ic.val = ZERO;
		update_set.insert(new_ic);

		new_ic.val = ONE;
		update_set.insert(new_ic);
	}

	while (!update_set.empty()) {
		new_imp_list.ori.index = (*update_set.begin()).index;
		new_imp_list.ori.val = (*update_set.begin()).val;
		new_imp_list.imp_results.clear();
		update_set.erase(update_set.begin());
		matrix_index = (new_imp_list.ori.val == ZERO) 
			? new_imp_list.ori.index * 2 
			: new_imp_list.ori.index * 2 + 1;

		justification_full(new_imp_list, update_set);
		for (c_ite = new_imp_list.imp_results.cbegin(); c_ite != new_imp_list.imp_results.cend(); c_ite++) {
			((*c_ite).second.val == ZERO) 
				? matrixImplication[matrix_index * size * 2 + (*c_ite).second.index * 2] = true 
				: matrixImplication[matrix_index * size * 2 + (*c_ite).second.index * 2 + 1] = true;
		}
	}
	_generate_implication_vector();
	return;
}

void redundant_wire::_generate_implication_vector() {
	size_t size = vRWNodeList.size();
	unsigned i, j;
	Implication_comb new_comb;
	int index;

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			index = i * 2;
			if (matrixImplication[index * size * 2 + j * 2]) {
				new_comb.index = j;
				new_comb.val = ZERO;
				vImplication[index].push_back(new_comb);
			}
			else if (matrixImplication[index * size * 2 + j * 2 + 1]) {
				new_comb.index = j;
				new_comb.val = ONE;
				vImplication[index].push_back(new_comb);
			}
			index = i * 2 + 1;
			if (matrixImplication[index * size * 2 + j * 2]) {
				new_comb.index = j;
				new_comb.val = ZERO;
				vImplication[index].push_back(new_comb);
			}
			else if (matrixImplication[index * size * 2 + j * 2 + 1]) {
				new_comb.index = j;
				new_comb.val = ONE;
				vImplication[index].push_back(new_comb);
			}
		}
	}
	return;
}