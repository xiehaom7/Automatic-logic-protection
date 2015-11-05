#include "simulation.h"

simulation::simulation() {
	uFaultIndex = 0;
}

void simulation::destroy() {
	for (vector<SimNode*>::iterator ite = vSimNodeLst.begin(); ite != vSimNodeLst.end(); ite++) {
		delete (*ite);
	}
	vSimNodeLst.clear();
	vPrimaryInputLst.clear();
	vPrimaryOutputLst.clear();
	vFaultCandidateLst.clear();
	vFaultInjectionLst.clear();
	mapPrefix.clear();
	mapSimNodeLst.clear();
	uFaultIndex = 0;
}

simulation::~simulation() {
	destroy();
}

int simulation::construct_module(const vector<int> &input_list, 
	vector<int> &output_list, int start_pos, module* tar_module, string prefix) {
	if (tar_module == NULL)
		throw exception("NULL module specified. (module_sim_lst_gen)");
	if (tar_module->get_input_num() != input_list.size())
		throw exception(("input numbers from target module " + tar_module->get_module_name() + 
			" and input list not match. (module_sim_lst_gen)").c_str());

	vSimNodeLst.resize(vSimNodeLst.size() + tar_module->get_net_num() - 
		tar_module->get_input_num() - tar_module->get_output_num());
	mapPrefix[prefix].reserve(tar_module->get_net_num() - tar_module->get_input_num());

	set<string> visited_module_instances;
	vector<string> topological_node_list;
	vector<string>::const_iterator ite;
	vector<int> input_index_list;
	vector<int> output_index_list;
	int input_num = tar_module->get_input_num();
	int output_num = tar_module->get_output_num();
	int index = start_pos;
	net* current_net;
	node* driving_node;
	string driving_node_name;
	int i;
	if (!tar_module->get_topological_sequence(topological_node_list))
		throw exception(("module " + tar_module->get_module_name() + 
			" has loop. (module_sim_lst_gen)").c_str());
	for (i = 0; i < input_num; i++) {
		current_net = tar_module->get_input_net(i);
		mapSimNodeLst[prefix + "." + current_net->get_net_name()] = input_list[i];
	}
	for (ite = topological_node_list.cbegin(); ite != topological_node_list.cend(); ite++) {
		input_index_list.clear();
		current_net = tar_module->get_net(*ite);
		if (current_net->get_is_input())
			continue;

		driving_node = current_net->get_fanin_node();
		driving_node_name = driving_node->get_node_name();
		for (i = 0; i < driving_node->get_input_num(); i++) {
			input_index_list.push_back(mapSimNodeLst[prefix + 
				"." + driving_node->get_input_net(i)->get_net_name()]);
		}

		if (driving_node->get_node_type() == TYPE_MODULE) {
			if (visited_module_instances.find(driving_node_name)
				!= visited_module_instances.end())
				continue;
			index = construct_module(input_index_list, output_index_list, index,
				driving_node->get_module_ref(), prefix + "." + driving_node_name);
			for (i = 0; i < driving_node->get_output_num(); i++)
				mapSimNodeLst[prefix + "." + driving_node->get_output_net(i)->get_net_name()] = 
				output_index_list[i];
			visited_module_instances.insert(driving_node_name);
		}
		if (driving_node->get_node_type() == TYPE_CELL) {
			SimNode* new_sim_node = _creat_sim_node(current_net, prefix, input_index_list);

			vSimNodeLst[index] = new_sim_node;
			mapPrefix[prefix].push_back(index);
			mapSimNodeLst[prefix + "." + new_sim_node->sName] = index++;
		}
	}
	output_list.resize(output_num, -1);
	for (i = 0; i < output_num; i++) {
		output_list[i] = mapSimNodeLst[prefix + "." + tar_module->get_output_net(i)->get_net_name()];
	}
	return index;
}

SimNode* simulation::_creat_sim_node(net* current_net, string &prefix, vector<int> &input_index_list) {
	SimNode* new_sim_node = new SimNode;
	new_sim_node->sName = current_net->get_net_name();
	new_sim_node->sPrefix = prefix;
	new_sim_node->eValue = X;
	new_sim_node->vFanin = input_index_list;
	if (current_net->get_fanin_node() != NULL)
		new_sim_node->pCell = current_net->get_fanin_node()->get_cell_ref();
	return new_sim_node;
}

bool simulation::construct(module* top_module) {
	try {
		if (top_module == NULL)
			throw exception("NULL top module specified. (construct)");

		vSimNodeLst.clear();
		mapSimNodeLst.clear();
		vPrimaryInputLst.clear();
		vPrimaryOutputLst.clear();

		string top_prefix = top_module->get_module_name();
		net* temp_net;
		int input_num = top_module->get_input_num();
		int output_num = top_module->get_output_num();
		int index = 0;
		int i;
		vector<int> empty_input_index_list;

		vSimNodeLst.resize(input_num + output_num);
		vPrimaryInputLst.resize(input_num);
		vPrimaryOutputLst.resize(output_num);

		for (i = 0; i < input_num; i++) {
			temp_net = top_module->get_input_net(i);

			SimNode* new_sim_node = _creat_sim_node(temp_net, top_prefix, empty_input_index_list);

			vSimNodeLst[index] = new_sim_node;
			vPrimaryInputLst[i] = index++;
		}

		construct_module(vPrimaryInputLst, vPrimaryOutputLst, index, top_module, top_prefix);

		int cap = 0;
		for (vector<SimNode*>::iterator ite = vSimNodeLst.begin(); ite != vSimNodeLst.end(); ite++)
			if (*ite != NULL)
				cap++;
		vSimNodeLst.resize(cap);
	}
	catch (exception& e) {
		cerr << "constructing simulation faild. (construct)\n";
		cerr << "description: " << e.what();
		destroy();
		return false;
	}
	return true;
}

void simulation::generate_fault_list(vector<string> &prefix_list) {
	vFaultCandidateLst.clear();

	for (vector<string>::const_iterator ite_prefix = prefix_list.cbegin(); 
		ite_prefix != prefix_list.cend(); ite_prefix++) {
		if (mapPrefix.find(*ite_prefix) == mapPrefix.end())
			throw exception(("prefix " + *ite_prefix + " not found. (generate_fault_list)").c_str());
		vector<int> &tar_list = mapPrefix[*ite_prefix];
		vFaultCandidateLst.reserve(vFaultCandidateLst.size() + tar_list.size());
		vFaultCandidateLst.insert(vFaultCandidateLst.end(), tar_list.begin(), tar_list.end());
	}
	return;
}

void simulation::generate_fault_list() {
	map<string, vector<int>>::const_iterator ite_prefix;
	vector<string> prefix_list;
	for (ite_prefix = mapPrefix.cbegin(); ite_prefix != mapPrefix.cend(); ite_prefix++)
		prefix_list.push_back(ite_prefix->first);
	generate_fault_list(prefix_list);
	return;
}

simulation& simulation::_parallel_simulate_node(SimNode* tar_node, Fault_mode fm) {
	if (tar_node == NULL)
		throw exception("NULL node specified. (_parallel_simulation)");
	if (tar_node->pCell == NULL)
		throw exception(("net " + tar_node->sName +
			" specified without driving node. (_parallel_simulation)").c_str());

	int j;
	int mask;
	int fanin_num = tar_node->vFanin.size();
	int truth_table_index = 0;
	int parallel_index = 0;
	size_t i;
	bitset<MAX_PARALLEL_NUM> parallel_vector(0);

	for (j = fanin_num - 1; j >= 0; j--) {
		if (vSimNodeLst[tar_node->vFanin[j]]->eValue != ZERO &&
			vSimNodeLst[tar_node->vFanin[j]]->eValue != ONE)
			throw exception(("unknown value on " + vSimNodeLst[tar_node->vFanin[j]]->sPrefix + "." +
				vSimNodeLst[tar_node->vFanin[j]]->sName + " during simulation. (_parallel_simulate_node)").c_str());
		truth_table_index = (vSimNodeLst[tar_node->vFanin[j]]->eValue == ZERO) ? (truth_table_index << 1) : ((truth_table_index << 1) + 1);
		parallel_vector |= vSimNodeLst[tar_node->vFanin[j]]->bsParallelVector;
	}

	tar_node->eValue = (tar_node->pCell->truth_table[truth_table_index]) ? ONE : ZERO;

	for (i = 0; i < parallel_vector.size() && parallel_vector.any(); i++) {
		if (!parallel_vector.test(i))
			continue;
		mask = 0;
		for (j = fanin_num - 1; j >= 0; j--) {
			mask = (!vSimNodeLst[tar_node->vFanin[j]]->bsParallelVector.test(i)) 
				? (mask << 1) : ((mask << 1) + 1);
		}
		switch (fm)
		{
		case SA1:
			parallel_index = truth_table_index | mask;
			break;
		case SA0:
			parallel_index = truth_table_index & !mask;
			break;
		case FLIP:
			parallel_index = truth_table_index ^ mask;
			break;
		default:
		{}
		}
		if (tar_node->pCell->truth_table[truth_table_index] != tar_node->pCell->truth_table[parallel_index]) {
			tar_node->bsParallelVector.set(i);
		}
		parallel_vector.reset(i);
	}
	return *this;
}

simulation& simulation::simulate_module(Fault_mode fm) {
	for (vector<SimNode*>::iterator ite = vSimNodeLst.begin(); ite != vSimNodeLst.end(); ite++) {
		if (!(*ite)->vFanin.empty())
			_parallel_simulate_node(*ite, fm);
	}
	return *this;
}

void simulation::generate_input_vector(vector<bool> &input_vector, Gen_mode mode) {
	int input_num = input_vector.size();
	vector<bool>::iterator ite = input_vector.begin();
	int i;

	switch (mode) {
	case RANDOM:
	{		
		unsigned int current_rand = rand() % MAX_RAND_NUM;
		while (input_num > MAX_RAND_BIT) {
			for (i = 0; i < MAX_RAND_BIT; i++) {
				*ite = ((current_rand & (unsigned)1) == 1) ? true : false;
				ite++;
				current_rand = current_rand >> 1;
			}
			input_num -= MAX_RAND_BIT;
			current_rand = rand() % MAX_RAND_NUM;
		}
		for (i = 0; i < input_num; i++) {
			*ite = ((current_rand & (unsigned)1) == 1) ? true : false;
			ite++;
			current_rand = current_rand >> 1;
		}
		break;
	}
	case SEQUENCE:
	{
		bool flag = false;
		while ((ite != input_vector.end()) && (!flag)) {
			if (*ite == false)
				flag = true;
			*ite = !(*ite);
			ite++;
		}
		break;
	}
	case RESET:
	{
		for (; ite != input_vector.end(); ite++)
			*ite = ZERO;
		break;
	}
	}
	return;
}

simulation& simulation::inject_faults(int fault_num, Gen_mode mode) {
	int target_size = vFaultCandidateLst.size();
	int i;

	for (vector<SimNode*>::iterator ite = vSimNodeLst.begin(); ite != vSimNodeLst.end(); ite++) {
		(*ite)->bsParallelVector.reset();
	}
	vFaultInjectionLst.clear();

	if (target_size == 0)
		return *this;

	switch (mode) {
	case RESET:
	{
		uFaultIndex = 0;
		break;
	}
	case SEQUENCE:
	{
		for (i = 0; i < fault_num; i++) {
			vSimNodeLst[vFaultCandidateLst[uFaultIndex]]->bsParallelVector.set(i);
			vFaultInjectionLst.push_back(vFaultCandidateLst[uFaultIndex]);
			uFaultIndex = (uFaultIndex + 1) % target_size;
		}
		break;
	}
	case RANDOM:
	{
		int target_index;
		for (i = 0; i < fault_num; i++) {
			target_index = rand() % target_size;
			vSimNodeLst[vFaultCandidateLst[target_index]]->bsParallelVector.set(i);
			vFaultInjectionLst.push_back(vFaultCandidateLst[target_index]);
		}
		break;
	}
	default:
	{}
	}
	return *this;
}

//void simulation::display(const string& display_file) {
//	ofstream pFile(display_file.c_str());
//	pFile << save_info();
//	pFile.close();
//	return;
//}

//string simulation::save_info() {
//	stringstream ss;
//	for (vector<SimNode*>::iterator ite = vSimNodeLst.begin(); ite != vSimNodeLst.end(); ite++) {
//		ss << (*ite)->sName << "\t" << (*ite)->strStat.uLogicOne << "\t" << (*ite)->strStat.uLogicZero <<
//			"\t" << (*ite)->strStat.uInjection << "\t" << (*ite)->strStat.uPropagation << "\t" <<
//			(*ite)->strStat.uSimulation << "\t" << (*ite)->strStat.uAffection << endl;
//	}
//	return ss.str();
//}

simulation& simulation::set_input_vector(vector<bool> &inputs) {
	unsigned input_size = vPrimaryInputLst.size();
	if (inputs.size() != input_size)
		throw exception("parameter vector's size not matched with input's size. (set_input_vector)");

	for (unsigned i = 0; i < input_size; i++)
		vSimNodeLst[vPrimaryInputLst[i]]->eValue = (inputs[i]) ? ONE : ZERO;
	return *this;
}

simulation& simulation::set_parallel_input_vector(vector<bool> &parallel, vector<bool> &main, unsigned round) {
	unsigned input_size = vPrimaryInputLst.size();
	if (main.size() != input_size || parallel.size() != input_size)
		throw exception("parameter vector's size not matched with input's size. (set_parallel_input_vector)");
	if (round >= MAX_PARALLEL_NUM)
		throw exception(("round " + std::to_string(round) + 
			" out of range. (set_parallel_input_vector)").c_str());

	for (unsigned i = 0; i < input_size; i++)
		vSimNodeLst[vPrimaryInputLst[i]]->bsParallelVector[round] = main[i] ^ parallel[i];
	return *this;
}

void simulation::get_node_value(string name, Wire_value **value, bitset<MAX_PARALLEL_NUM> **vector) {
	std::map<string, int>::const_iterator ite = mapSimNodeLst.find(name);
	if (ite == mapSimNodeLst.end())
		throw exception(("node " + name + " not found. (get_node_value)").c_str());
	*value = &vSimNodeLst[ite->second]->eValue;
	*vector = &vSimNodeLst[ite->second]->bsParallelVector;
	return;
}

void simulation::get_node_value(size_t index, Wire_value **value, bitset<MAX_PARALLEL_NUM> **vector) {
	if (index >= vSimNodeLst.size() || index < 0)
		throw exception(("index " + std::to_string(index) + " exceeds range. (get_node_value)").c_str());
	*value = &vSimNodeLst[index]->eValue;
	*vector = &vSimNodeLst[index]->bsParallelVector;
	return;
}

bool simulation::return_outputs(vector<Wire_value> &outputs) {
	return true;
}

bool simulation::return_inputs(vector<Wire_value> &inputs) {
	return true;
}

simulation&	simulation::get_node_name_list(vector<string> &name_list) {
	vector<SimNode*>::iterator ite;
	for (ite = vSimNodeLst.begin(); ite != vSimNodeLst.end(); ite++) {
		name_list.push_back((*ite)->sPrefix + "." + (*ite)->sName);
	}
	return *this;
}

simulation& simulation::get_primary_outputs_list(set<string> &output_list) {
	vector<int>::const_iterator c_ite;
	for (c_ite = vPrimaryOutputLst.cbegin(); c_ite != vPrimaryOutputLst.cend(); c_ite++) {
		output_list.insert(vSimNodeLst[(*c_ite)]->sPrefix + "." + vSimNodeLst[(*c_ite)]->sName);
	}
	return *this;
}

simulation&	simulation::run_golden_simulation(
	int sim_num, bool random, vector<bool> *start_inputs) {
	size_t input_num = get_primary_inputs_num();
	vector<bool> input_vector(input_num, false);
	vector<bool> empty_vector(input_num, false);
	int i;
	if (random || start_inputs == NULL)
		start_inputs = &empty_vector;
	set_input_vector(input_vector);
	inject_faults(0, RESET);
	for (i = 0; i < sim_num; i++) {
		set_parallel_input_vector(*start_inputs, input_vector, i);
		generate_input_vector(*start_inputs, random ? RANDOM : SEQUENCE);
	}
	simulate_module(FLIP);
	return *this;
}

simulation& simulation::run_fault_injection_simulation(
	int fault_num, vector<bool> *start_inputs, bool random, Fault_mode fm) {
	size_t input_num = get_primary_inputs_num();
	if (fault_num > MAX_PARALLEL_NUM)
		throw exception(("specified fault number " + std::to_string(fault_num) + " exceeds max parallel number "
			+ std::to_string(MAX_PARALLEL_NUM) + ".(run_fault_injection_simulation)").c_str());
	vector<bool> empty_vector(input_num, false);
	if (start_inputs == NULL)
		start_inputs = &empty_vector;

	set_input_vector(*start_inputs);
	inject_faults(fault_num, random ? RANDOM : SEQUENCE);
	simulate_module(fm);
	return *this;
}