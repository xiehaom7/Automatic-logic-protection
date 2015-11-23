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
	sim.destroy();
	sig.destroy();
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

		sim.construct(pTopModule);
		sig.construct(&sim);
		sig.generate_signature();

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

int redundant_wire::_generate_implication_vector() {
	size_t size = vRWNodeList.size();
	unsigned i, j;
	Implication_comb new_comb;
	int index;
	int total = 0;

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			if (_count_distance(j, i) <= 1)
				continue;
			index = i * 2;
			if (matrixImplication[index * size * 2 + j * 2]) {
				new_comb.index = j;
				new_comb.val = ZERO;
				vImplication[index].push_back(new_comb);
				total++;
			}
			else if (matrixImplication[index * size * 2 + j * 2 + 1]) {
				new_comb.index = j;
				new_comb.val = ONE;
				vImplication[index].push_back(new_comb);
				total++;
			}
			index = i * 2 + 1;
			if (matrixImplication[index * size * 2 + j * 2]) {
				new_comb.index = j;
				new_comb.val = ZERO;
				vImplication[index].push_back(new_comb);
				total++;
			}
			else if (matrixImplication[index * size * 2 + j * 2 + 1]) {
				new_comb.index = j;
				new_comb.val = ONE;
				vImplication[index].push_back(new_comb);
				total++;
			}
		}
	}
#ifdef DISPLAY
	cout << "Total implication candidates: " << total << endl;
#endif
	return total;
}

void redundant_wire::setup_observability(vector<bitset<SIGNATURE_SIZE>> &cache_node_ob) {
	SignatureNode* sig_node;
	int i;

	sig.analyse_observability(0);
	cache_node_ob.resize(vRWNodeList.size());
	for (i = 0; i < vRWNodeList.size(); i++) {
		sig_node = sig.get_signature_node(vRWNodeList[i]->sName);
		cache_node_ob[i] = sig_node->vODCmask[0];
	}
	return;
}

void redundant_wire::setup_to_source_observability(int target,
	map<int, vector<bitset<SIGNATURE_SIZE>>> &cache_to_source_ob) {
	cache_to_source_ob[target].resize(vRWNodeList.size());
	vector<int> exclude_node_list;
	vector<int> tar_node_list;
	SignatureNode* sig_node;
	int i;

	tar_node_list.push_back(sig.get_node_index(vRWNodeList[target]->sName));
	sig.analyse_observability(tar_node_list, exclude_node_list, 1);
	for (i = 0; i < vRWNodeList.size(); i++) {
		sig_node = sig.get_signature_node(vRWNodeList[i]->sName);
		cache_to_source_ob[target][i] = sig_node->vODCmask[1];
	}
	return;
}

void redundant_wire::setup_not_to_dest_observability(int target,
	map<int, vector<bitset<SIGNATURE_SIZE>>> &cache_not_to_dest_ob) {
	cache_not_to_dest_ob[target].resize(vRWNodeList.size());
	vector<int> exclude_node_list;
	SignatureNode* sig_node;
	int i;

	exclude_node_list.push_back(sig.get_node_index(vRWNodeList[target]->sName));
	sig.analyse_observability(exclude_node_list, 2);
	for (i = 0; i < vRWNodeList.size(); i++) {
		sig_node = sig.get_signature_node(vRWNodeList[i]->sName);
		cache_not_to_dest_ob[target][i] = sig_node->vODCmask[2];
	}
	return;
}

float redundant_wire::implication_evaluator(Implication_comb left_imp, Implication_comb right_imp, 
	vector<bitset<SIGNATURE_SIZE>> &unprotected_sig, vector<Gate_record_item> &vector_gate_record, 
	vector<bitset<SIGNATURE_SIZE>> &cache_node_ob, 
	map<int, vector<bitset<SIGNATURE_SIZE>>> &cache_to_source_ob,
	map<int, vector<bitset<SIGNATURE_SIZE>>> &cache_not_to_dest_ob) {
	float res = 0.0;
	float single_node_res;
	bitset<SIGNATURE_SIZE> v;
	SignatureNode* sig_left_node;
	set<int>::const_iterator fanin_ite;
	vector<Gate_record_item>::const_iterator gate_record_ite;
	int count;

#ifdef DISPLAY
	cout << "---\t" << vRWNodeList[left_imp.index]->sName << "\t" << left_imp.val << 
		"\t" << vRWNodeList[right_imp.index]->sName << "\t" << right_imp.val << "\t" << "---" << endl;
#endif
	if (vector_gate_record.empty()) {
		if (cache_node_ob.empty()) 
			setup_observability(cache_node_ob);

		if (cache_to_source_ob.find(left_imp.index) == cache_to_source_ob.end())
			setup_to_source_observability(left_imp.index, cache_to_source_ob);
		
		if (cache_not_to_dest_ob.find(right_imp.index) == cache_not_to_dest_ob.end())
			setup_not_to_dest_observability(right_imp.index, cache_not_to_dest_ob);

		for (fanin_ite = vNodeFaninSet[right_imp.index].cbegin();
		fanin_ite != vNodeFaninSet[right_imp.index].cend(); fanin_ite++) {
			if (vRWNodeList[*fanin_ite]->vFanin.empty())
				continue;
			Gate_record_item new_item;
			sig_left_node = sig.get_signature_node(vRWNodeList[left_imp.index]->sName);
			new_item.gate_index = *fanin_ite;
			new_item.ODCmask[0] = cache_node_ob[*fanin_ite];
			new_item.ODCmask[1] = cache_to_source_ob[left_imp.index][*fanin_ite];
			new_item.ODCmask[2] = cache_not_to_dest_ob[right_imp.index][*fanin_ite];
			calculate_ODCres((left_imp.val == ONE) ? sig_left_node->vSig : ~(sig_left_node->vSig),
				new_item.ODCmask[0],
				new_item.ODCmask[1],
				new_item.ODCmask[2],
				new_item.ODCres);
			if (new_item.ODCres.count() != 0)
				vector_gate_record.push_back(new_item);
		}
	}
	for (gate_record_ite = vector_gate_record.cbegin(); gate_record_ite != vector_gate_record.cend(); gate_record_ite++) {
		calculate_protect((*gate_record_ite).ODCres,
			unprotected_sig[(*gate_record_ite).gate_index],
			v);

		count = v.count();
		if (count != 0) {
			single_node_res = (float)count / SIGNATURE_SIZE;
			res += single_node_res;
		}

#ifdef DISPLAY
		if (count != 0) {
			cout << vRWNodeList[(*gate_record_ite).gate_index]->sName << "\t";
			cout << single_node_res << endl;
		}
#endif
	}
#ifdef DISPLAY
	cout << res << endl;
#endif
	return res;
}

void redundant_wire::calculate_ODCres(const bitset<SIGNATURE_SIZE> &source,
	const bitset<SIGNATURE_SIZE> &observability_to_all,
	const bitset<SIGNATURE_SIZE> &observability_to_source,
	const bitset<SIGNATURE_SIZE> &observability_exclude_target,
	bitset<SIGNATURE_SIZE> &res) {
	res.set();
	res &= source;
	res &= observability_to_all;
	res &= ~(observability_to_source);
	res &= ~(observability_exclude_target);
	return;
}

void redundant_wire::calculate_protect(const bitset<SIGNATURE_SIZE> &ODCres, 
	const bitset<SIGNATURE_SIZE> &unprotect, 
	bitset<SIGNATURE_SIZE> &res) {
	res.set();
	res &= ODCres;
	res &= unprotect;
	return;
}

void	redundant_wire::_execute_op(const Op_item &op, map<string, node*> &mapNode, map<string, net*> &mapNet) {
	switch (op.op) {
	case AW:
	{
		net* new_net = pTopModule->add_net(WIRE_ADDED_NAME + std::to_string((_ULonglong)rw_wire_added_counter++));
		mapNet[op.new_name] = new_net;
		break;
	}
	case AG:
	{
		node* new_node = pTopModule->add_node(GATE_ADDED_NAME + std::to_string((_ULonglong)rw_gate_added_counter++), op.op1);
		mapNode[op.new_name] = new_node;
		break;
	}
	case CW:
	{
		net* tar_net;
		node* tar_node;
		map<string, net*>::iterator ite_net;
		map<string, node*>::iterator ite_node;
		if (op.op1_pin.empty()) {
			ite_net = mapNet.find(op.op1);
			assert(ite_net != mapNet.end());
			tar_net = ite_net->second;
		}
		else {
			ite_net = mapNet.find(op.op1 + "." + op.op1_pin);
			assert(ite_net != mapNet.end());
			tar_net = ite_net->second;
		}
		ite_node = mapNode.find(op.op2);
		assert(ite_node != mapNode.end());
		tar_node = ite_node->second;
		pTopModule->connect_wire(tar_net, tar_node, op.op2_pin);
		break;
	}
	case DW:
	{
		net* tar_net;
		node* tar_node;
		map<string, net*>::iterator ite_net;
		map<string, node*>::iterator ite_node;
		assert(!op.op1_pin.empty());

		ite_node = mapNode.find(op.op1);
		assert(ite_node != mapNode.end());
		node* temp_node = ite_node->second;
		if (temp_node->get_node_type() == TYPE_CELL) {
			if (temp_node->get_cell_ref()->output_pin == op.op1_pin) {
				tar_net = temp_node->get_output_net(0);
			}
			else {
				tar_net = temp_node->get_input_net(temp_node->get_cell_ref()->get_input_pos(op.op1_pin));
			}
		}
		else {
			int index = temp_node->get_module_ref()->get_output_pos(op.op1_pin);
			if (index != -1) {
				tar_net = temp_node->get_output_net(index);
			}
			else {
				tar_net = temp_node->get_input_net(temp_node->get_module_ref()->get_input_pos(op.op1_pin));
			}
		}

		mapNet[op.op1 + "." + op.op1_pin] = tar_net;

		tar_node = temp_node;
		pTopModule->disconnect_wire(tar_net, tar_node, op.op1_pin);
		break;
	}
	case RG:
	{
		node* tar_node;
		map<string, node*>::iterator ite_node;

		ite_node = mapNode.find(op.op1);
		assert(ite_node != mapNode.end());
		tar_node = ite_node->second;

		pTopModule->remove_node(tar_node);
		break;
	}
	}
}

bool redundant_wire::redundant_wire_adder(Implication_comb &left_comb, Implication_comb &right_comb) {
	map<string, node*> mapNode;
	map<string, net*> mapNet;

	int source_net_index = left_comb.index;
	int dest_net_index = right_comb.index;
	net* source_net = pTopModule->get_net(
		vRWNodeList[source_net_index]->sName.substr(vRWNodeList[source_net_index]->sName.find_first_of(".")+1));
	net* dest_net = pTopModule->get_net(
		vRWNodeList[dest_net_index]->sName.substr(vRWNodeList[dest_net_index]->sName.find_first_of(".")+1));
	map<string, Pin_implication_item*>::const_iterator ite_pin;
	int implication_index = 0;

	mapNet["X"] = source_net;

	if (left_comb.val == ONE)	implication_index += 2;
	if (right_comb.val == ONE)	implication_index += 1;

	bool execute_flag = false;

	if (dest_net->get_fanout_num() == 1) {
		node* dest_driving_node = dest_net->get_fanout_nodes(0);
		int net_index = dest_driving_node->find_input_net(dest_net);
		assert(net_index != -1);
		cell* cell_ref = dest_driving_node->get_cell_ref();
		string pin_name = cell_ref->input_pinlist[net_index];
		ite_pin = cell_ref->rw_op_collection.find(pin_name);
		if (ite_pin != cell_ref->rw_op_collection.cend()) {
			if (ite_pin->second->implication_item_array[implication_index] != NULL) {
				execute_flag = true;
				mapNode["old"] = dest_driving_node;
				for (vector<Op_item>::const_iterator ite_op = ite_pin->second->implication_item_array[implication_index]->op_list.cbegin(); ite_op != ite_pin->second->implication_item_array[implication_index]->op_list.cend(); ite_op++) {
					_execute_op((*ite_op), mapNode, mapNet);
				}
			}
		}
	}
	if (!execute_flag) {
		node* dest_driven_node = dest_net->get_fanin_node();
		cell* cell_ref = dest_driven_node->get_cell_ref();
		string pin_name = cell_ref->output_pin;
		ite_pin = cell_ref->rw_op_collection.find(pin_name);
		if (ite_pin != cell_ref->rw_op_collection.cend()) {
			if (ite_pin->second->implication_item_array[implication_index] != NULL) {
				execute_flag = true;
				mapNode["old"] = dest_driven_node;
				for (vector<Op_item>::const_iterator ite_op = ite_pin->second->implication_item_array[implication_index]->op_list.cbegin(); ite_op != ite_pin->second->implication_item_array[implication_index]->op_list.cend(); ite_op++) {
					_execute_op((*ite_op), mapNode, mapNet);
				}
			}
		}
	}
	if (!execute_flag) {
		net* source_net_inverted = NULL;
		net* dest_net_unchecked = NULL;

		node* dest_driven_node = dest_net->get_fanin_node();
		node* inverter_node = NULL;
		node* checker_node = NULL;

		pTopModule->disconnect_wire(dest_net, dest_driven_node, dest_driven_node->get_cell_ref()->output_pin);


		if (left_comb.val != right_comb.val) {
			inverter_node = pTopModule->add_node(GATE_ADDED_NAME + std::to_string((_ULonglong)rw_gate_added_counter++), INVERTER_TYPE);
			pTopModule->connect_wire(source_net, inverter_node, INVERTER_INPUT_PIN);
			source_net_inverted = pTopModule->add_net(WIRE_ADDED_NAME + std::to_string((_ULonglong)rw_wire_added_counter++));
			pTopModule->connect_wire(source_net_inverted, inverter_node, INVERTER_OUTPUT_PIN);
			source_net = source_net_inverted;
		}

		if (right_comb.val == ONE) {
			checker_node = pTopModule->add_node(GATE_ADDED_NAME + std::to_string((_ULonglong)rw_gate_added_counter++), OR_TYPE);
			pTopModule->connect_wire(source_net, checker_node, OR_INPUT_PIN_1);
			dest_net_unchecked = pTopModule->add_net(WIRE_ADDED_NAME + std::to_string((_ULonglong)rw_wire_added_counter++));
			pTopModule->connect_wire(dest_net_unchecked, dest_driven_node, dest_driven_node->get_cell_ref()->output_pin);
			pTopModule->connect_wire(dest_net_unchecked, checker_node, OR_INPUT_PIN_2);
			pTopModule->connect_wire(dest_net, checker_node, OR_OUTPUT_PIN);
		}
		else {
			checker_node = pTopModule->add_node(GATE_ADDED_NAME + std::to_string((_ULonglong)rw_gate_added_counter++), AND_TYPE);
			pTopModule->connect_wire(source_net, checker_node, AND_INPUT_PIN_1);
			dest_net_unchecked = pTopModule->add_net(WIRE_ADDED_NAME + std::to_string((_ULonglong)rw_wire_added_counter++));
			pTopModule->connect_wire(dest_net_unchecked, dest_driven_node, dest_driven_node->get_cell_ref()->output_pin);
			pTopModule->connect_wire(dest_net_unchecked, checker_node, AND_INPUT_PIN_2);
			pTopModule->connect_wire(dest_net, checker_node, AND_OUTPUT_PIN);
		}
	}
	return true;
}

void redundant_wire::redundant_wire_selector(const string module_name, int rw_num, 
	int sample_freq, int sim_num, int fault_num) {
#ifdef RECORD
	ofstream pFile((module_name + "_selection.log").c_str());
	ofstream pFile_ie((module_name + "_implicaiton_evaluation.log").c_str());
#endif
	int node_size = vRWNodeList.size();
	int sample_counter;
	int i, index;
	bitset<SIGNATURE_SIZE> temp;
	temp.set();
	vector<bitset<SIGNATURE_SIZE>> unprotect(node_size, temp);
	vector<bitset<SIGNATURE_SIZE>> cache_node_ob;
	map<int, vector<bitset<SIGNATURE_SIZE>>> cache_to_source_ob;
	map<int, vector<bitset<SIGNATURE_SIZE>>> cache_not_to_dest_ob;

	vector<Implication_pair> history_rw;
	list<Rank_item*> rank_rw;
	list<Rank_item*>::iterator rank_ite;

#ifdef RECORD
	if (!pFile.is_open())
		throw exception("failed to open redundant wire selection file. "
			"(redundant_wire::redundant_wire_selector)\n");
	if (!pFile_ie.is_open())
		throw exception("failed to open redundant wire implication evaluation file. "
			"(redundant_wire::redundant_wire_selector)\n");
#endif

	int counter = 0;

	list<Implication_comb>::const_iterator list_ite;
	for (i = 0; i < node_size*2; i++) {
		index = (i%2 == 0)? i/2 : (i-1)/2;
		for (list_ite = vImplication[i].cbegin(); list_ite != vImplication[i].cend(); list_ite++) {
			Rank_item* new_item = new Rank_item;
			new_item->imp_pair.left_item.index = index;
			new_item->imp_pair.left_item.val = (i%2 == 0)? ZERO : ONE;
			new_item->imp_pair.right_item = *list_ite;
			new_item->source_sig = sig.get_signature_node(vRWNodeList[index]->sName)->vSig;
			new_item->evaluation_res = implication_evaluator(new_item->imp_pair.left_item, 
				new_item->imp_pair.right_item, unprotect, new_item->gate_record_vector,
				cache_node_ob, cache_to_source_ob, cache_not_to_dest_ob);
			for (rank_ite = rank_rw.begin(); rank_ite != rank_rw.end(); rank_ite++) {
				if ((*rank_ite)->evaluation_res <= new_item->evaluation_res) {
					rank_rw.insert(rank_ite, new_item);
					break;
				}
			}
			if (rank_ite == rank_rw.end()) {
				rank_rw.insert(rank_ite, new_item);
			}
#ifdef RECORD
			pFile_ie << endl << vRWNodeList[new_item->imp_pair.left_item.index]->sName << "\t" 
				<< ((new_item->imp_pair.left_item.val == ONE) ? "1" : "0") << "\t" 
				<< vRWNodeList[new_item->imp_pair.right_item.index]->sName << "\t"
				<< ((new_item->imp_pair.right_item.val == ONE) ? "1" : "0") << "\t";
			pFile_ie << (float)((new_item->imp_pair.left_item.val == ONE) 
				? new_item->source_sig.count() 
				: (~new_item->source_sig).count()) / SIGNATURE_SIZE << "\t";
			pFile_ie << new_item->evaluation_res << endl;
			for (vector<Gate_record_item>::const_iterator gate_ite = new_item->gate_record_vector.cbegin();
			gate_ite != new_item->gate_record_vector.cend(); gate_ite++) {
				pFile_ie << vRWNodeList[(*gate_ite).gate_index]->sName << "\t"
					<< (*gate_ite).ODCmask[0].count() << "\t" 
					<< (*gate_ite).ODCmask[1].count() << "\t" 
					<< (*gate_ite).ODCmask[2].count() << "\t" 
					<< (float)(*gate_ite).ODCres.count()/SIGNATURE_SIZE << endl;
			}
#endif
		}
	}
#ifdef RECORD
	pFile_ie.close();
#endif
	bool select_flag;
	float new_evaluation;
	double sim_res;
	Rank_item* temp_ptr;
	list<Rank_item*>::iterator head_ite;

#ifdef RECORD
	pFile << "NULL - NULL - ";
	simulation module_sim;
	module_sim.construct(pTopModule);
	simulation_evaluation sim_evaluate;
	sim_evaluate.construct(&module_sim);
	if (sim_num == 0 && fault_num == 0)
		sim_evaluate.run_exhaustive_fault_injection_simulation();
	else
		sim_evaluate.run_random_fault_injection_simulation(sim_num, fault_num);
	sim_res = sim_evaluate.evaluate_fault_injection_results();
	pFile << " " << "0" << " " << sim_res << endl;
#endif

	sample_counter = 0;
	while (rw_num >  0 && !rank_rw.empty()) {
		select_flag = false;
		while (!select_flag) {
			head_ite = rank_rw.begin();
			temp_ptr = *head_ite;
			rank_rw.erase(head_ite);
			new_evaluation = implication_evaluator(temp_ptr->imp_pair.left_item, 
				temp_ptr->imp_pair.right_item, 
				unprotect, temp_ptr->gate_record_vector,
				cache_node_ob, cache_to_source_ob, cache_not_to_dest_ob);
			if (abs(new_evaluation - temp_ptr->evaluation_res) < 0.0000001) {
				redundant_wire_adder(temp_ptr->imp_pair.left_item, temp_ptr->imp_pair.right_item);
				if (pTopModule->check_module()) {
					update_unprotect(*temp_ptr, unprotect);
					history_rw.push_back(temp_ptr->imp_pair);
#ifdef RECORD
					pFile << vRWNodeList[temp_ptr->imp_pair.left_item.index]->sName + "\t" + ((temp_ptr->imp_pair.left_item.val == ONE) ? "1" : "0");
					pFile << "\t" + vRWNodeList[temp_ptr->imp_pair.right_item.index]->sName + "\t" + ((temp_ptr->imp_pair.right_item.val == ONE) ? "1" : "0");
					
					if (sample_counter % sample_freq == 0) {
						simulation module_sim;
						module_sim.construct(pTopModule);
						simulation_evaluation sim_evaluate;
						sim_evaluate.construct(&module_sim);
						if (sim_num == 0 && fault_num == 0)
							sim_evaluate.run_exhaustive_fault_injection_simulation();
						else
							sim_evaluate.run_random_fault_injection_simulation(sim_num, fault_num);
						sim_res = sim_evaluate.evaluate_fault_injection_results();
						pFile << " " << "0" << " " << sim_res << endl;
						pFile << "\t" << temp_ptr->evaluation_res << "\t" << sim_res << endl;
					}
					else {
						pFile << "\t" << temp_ptr->evaluation_res << "\t" << "-" << endl;
					}
#endif
					delete temp_ptr;
					select_flag = true;
				}
				else {
					pTopModule->rollback();
				}
			}
			else {
				temp_ptr->evaluation_res = new_evaluation;
				for (rank_ite = rank_rw.begin(); rank_ite != rank_rw.end(); rank_ite++) {
					if ((*rank_ite)->evaluation_res <= temp_ptr->evaluation_res) {
						rank_rw.insert(rank_ite, temp_ptr);
						break;
					}
				}
				if (rank_ite == rank_rw.end()) {
					rank_rw.insert(rank_ite, temp_ptr);
				}
			}
		}
		pTopModule->commit();
#ifdef RECORD
		ofstream pFile_mo(module_name + "_rw_" + std::to_string((long double)sample_counter) + ".v");
		pFile_mo << pTopModule->write_module();
		pFile_mo.close();
#endif
		rw_num--;
		sample_counter++;
	}
#ifdef RECORD
	pFile.close();
#endif
	return;
}

void redundant_wire::update_unprotect(Rank_item& tar_item, vector<bitset<SIGNATURE_SIZE>>& unprotect) {
	vector<Gate_record_item>::const_iterator ite;
	for (ite = tar_item.gate_record_vector.cbegin(); ite != tar_item.gate_record_vector.cend(); ite++) {
		unprotect[(*ite).gate_index] &= ~(*ite).ODCres;
	}
	return;
}

int redundant_wire::_count_distance(int dest_node_index, int source_node_index) {
	vector<int> vDistanceLst;
	queue<int> qNodeLst;
	vector<int>::const_iterator fanin_ite;
	int current_node_index;
	int node_size = vRWNodeList.size();

	if (vNodeFanoutSet[dest_node_index].find(source_node_index) != vNodeFanoutSet[dest_node_index].end()) {
		return -1;
	}
	if (vNodeFaninSet[dest_node_index].find(source_node_index) != vNodeFaninSet[dest_node_index].end()) {
		vDistanceLst.resize(node_size, -1);

		qNodeLst.push(dest_node_index);
		vDistanceLst[dest_node_index] = 0;

		while (!qNodeLst.empty()) {
			current_node_index = qNodeLst.front();
			qNodeLst.pop();

			if (current_node_index == source_node_index) {
				return vDistanceLst[current_node_index];
			}
			else {
				for (fanin_ite = vRWNodeList[current_node_index]->vFanin.cbegin(); 
				fanin_ite != vRWNodeList[current_node_index]->vFanin.cend(); fanin_ite++) {
					if (vDistanceLst[*fanin_ite] == -1) {
						vDistanceLst[*fanin_ite] = vDistanceLst[current_node_index] + 1;
						qNodeLst.push(*fanin_ite);
					}
				}
			}
		}
	}
	else {
		vDistanceLst.resize(node_size, -1);

		set<int> set_intersect;
		set_intersection(vNodeFaninSet[dest_node_index].begin(), vNodeFaninSet[dest_node_index].end(), 
			vNodeFaninSet[source_node_index].begin(), vNodeFaninSet[source_node_index].end(), 
			std::inserter(set_intersect, set_intersect.begin()));

		if (set_intersect.empty()) {
			return -1;
		}

		qNodeLst.push(dest_node_index);
		vDistanceLst[dest_node_index] = 0;

		while (!qNodeLst.empty()) {
			current_node_index = qNodeLst.front();
			qNodeLst.pop();

			if (set_intersect.find(current_node_index) != set_intersect.end()) {
				return vDistanceLst[current_node_index];
			}
			else {
				for (fanin_ite = vRWNodeList[current_node_index]->vFanin.cbegin(); 
				fanin_ite != vRWNodeList[current_node_index]->vFanin.cend(); fanin_ite++) {
					if (vDistanceLst[*fanin_ite] == -1) {
						vDistanceLst[*fanin_ite] = vDistanceLst[current_node_index] + 1;
						qNodeLst.push(*fanin_ite);
					}
				}
			}
		}
	}
	return -1;
}

int	redundant_wire::implication_counter(Implicaton_method method) {
	switch (method) {
	case BACKWARD:
		implication_matrix_generator_backward();
		break;
	case DIRECT:
		implication_matrix_generator_direct();
		break;
	case INDIRECT_LOW:
		implication_matrix_generator();
		break;
	case INDIRECT_HIGH:
		implication_matrix_generator_full();
		break;
	default:
		throw exception("unknown implication generate method. (redundant_wire::implication_counter)\n");
	}
	int res = 0;
	for (int i = 0; i < matrixImplication.size(); i++)
		if (matrixImplication[i])
			res++;
	return res;
}