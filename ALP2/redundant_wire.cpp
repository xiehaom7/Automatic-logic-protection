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
	matrixImplication.clear();
	/*matrixImplicationScreen.clear();*/
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

int redundant_wire::construct_module(const vector<int> &input_list,
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
			index = construct_module(input_index_list, output_index_list, index,
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
		
		construct_module(vPrimaryInputList, vPrimaryOutputList, index, tar_module, top_prefix);

		int cap = 0;
		for (vector<RWNode*>::iterator ite = vRWNodeList.begin(); ite != vRWNodeList.end(); ite++)
			if (*ite != NULL)
				cap++;
		vRWNodeList.resize(cap);
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

int redundant_wire::get_node_index(string &node_name) {
	map<string, int>::iterator ite = mapRWNodeList.find(node_name);
	if (ite == mapRWNodeList.end())
		return -1;
	return ite->second;
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