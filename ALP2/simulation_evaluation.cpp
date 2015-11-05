#include "simulation_evaluation.h"

void simulation_evaluation::destroy() {
	vStatNodeList.clear();
	mStatNode.clear();
	sim = NULL;
	input_num = 0;
	node_num = 0;
	return;
}

simulation_evaluation::simulation_evaluation() {
	sim = NULL;
	input_num = 0;
	node_num = 0;
}

simulation_evaluation::~simulation_evaluation() {
	destroy();
}

StatNode simulation_evaluation::_init_stat_node(string name) {
	StatNode sn;
	sn.sName = name;
	sn.strSig.uTest0 = 0;
	sn.strSig.uTest1 = 0;
	sn.uInjection = 0;
	sn.uPropagation = 0;
	sn.uLogicOne = 0;
	sn.uLogicZero = 0;
	sn.uAffection = 0;
	sn.uSimulation = 0;
	return sn;
}

bool simulation_evaluation::construct(simulation* tar_sim) {
	destroy();
	if (tar_sim == NULL)
		return false;

	size_t i;
	string node_name;
	sim = tar_sim;
	node_num = sim->vSimNodeLst.size();
	input_num = sim->get_primary_inputs_num();	

	vStatNodeList.resize(node_num);
	for (i = 0; i < node_num; i++) {
		node_name = sim->vSimNodeLst[i]->sPrefix + "." + sim->vSimNodeLst[i]->sName;
		vStatNodeList[i] = _init_stat_node(node_name);
		mStatNode[node_name] = i;
	}
	
	return true;
}

simulation_evaluation&	simulation_evaluation::run_golden_simulation(
	int sim_num, bool random = true, vector<bool> *start_inputs = NULL) {
	vector<bool> input_vector(input_num, false);
	vector<bool> empty_vector(input_num, false);
	int i;
	if (random || start_inputs == NULL)
		start_inputs = &empty_vector;
	sim->set_input_vector(input_vector);
	sim->inject_faults(0, RESET);
	for (i = 0; i < sim_num; i++) {
		sim->set_parallel_input_vector(*start_inputs, input_vector, i);
		sim->generate_input_vector(*start_inputs, random ? RANDOM : SEQUENCE);
	}
	sim->simulate_module(FLIP);
	return *this;
}

simulation_evaluation& simulation_evaluation::run_fault_injection_simulation(
	int fault_num, vector<bool> *start_inputs = NULL, bool random = true, Fault_mode fm = FLIP) {
	if (fault_num > MAX_PARALLEL_NUM)
		throw exception(("specified fault number " + std::to_string(fault_num) + " exceeds max parallel number " 
			+ std::to_string(MAX_PARALLEL_NUM) + ".(run_fault_injection_simulation)").c_str());
	vector<bool> empty_vector(input_num, false);
	if (start_inputs == NULL)
		start_inputs = &empty_vector;

	sim->set_input_vector(*start_inputs);
	sim->inject_faults(fault_num, random ? RANDOM : SEQUENCE);
	sim->simulate_module(fm);
	return *this;
}

void simulation_evaluation::generate_signature() {
	Wire_value* value = NULL;
	bitset<MAX_PARALLEL_NUM>* res = NULL;
	size_t index;
	int i;
	
	run_golden_simulation(SIGNATURE_SIZE);

	for (index = 0; index < node_num; index++) {
		get_node_value(index, &value, &res);
		if (*value == ONE)
			res->flip();
		for (i = 0; i < SIGNATURE_SIZE; i++)
			vStatNodeList[index].strSig.vSig[i] = (*res)[i];
		if (*value == ONE)
			res->flip();
	}
}

void simulation_evaluation::run_exhaustive_fault_injection_simulation() {
	long sim_num = (long)pow((double)2, (double)input_num);
	vector<bool> input_vector(input_num, false);
	int target_num;
	int fault_num;
	sim->generate_fault_list();
	sim->generate_input_vector(input_vector, RESET);
	sim->inject_faults(0, RESET);
	while (sim_num > 0) {
		target_num = sim->get_fault_candidate_size();
		while (target_num > 0) {
			fault_num = (target_num > MAX_PARALLEL_NUM) ? MAX_PARALLEL_NUM : target_num;
			run_fault_injection_simulation(fault_num, &input_vector, false);
			summarize_fault_injection_results(fault_num);
			target_num -= MAX_PARALLEL_NUM;
		}
		sim->generate_input_vector(input_vector, SEQUENCE);
		sim_num--;
	}
	return;
}

void simulation_evaluation::run_exhaustive_golden_simulation() {
	long sim_num = (long)pow((double)2, (double)input_num);
	long round_num;
	vector<StatNode>::iterator ite;
	vector<bool>::const_iterator v_ite;
	vector<bool> input_vector(input_num, false);
	bitset<MAX_PARALLEL_NUM> res;

	while (sim_num > 0) {
		round_num = (sim_num > MAX_PARALLEL_NUM) ? MAX_PARALLEL_NUM : sim_num;
		sim_num -= MAX_PARALLEL_NUM;
		run_golden_simulation(round_num, false, &input_vector);
		summarize_golden_results(round_num);		
	}
	return;
}

void simulation_evaluation::summarize_fault_injection_results(int fault_num) {
	Wire_value* value = NULL;
	bitset<MAX_PARALLEL_NUM>* res = NULL;
	bitset<MAX_PARALLEL_NUM> output_res;
	size_t count;
	size_t index;
	set<string> primary_output_list;
	vector<int>* fault_injeciton_list = sim->get_fault_injection_list();
	output_res.reset();
	sim->get_primary_outputs_list(primary_output_list);
	for (index = 0; index < node_num; index++) {
		get_node_value(index, &value, &res);
		count = res->count();
		vStatNodeList[index].uSimulation += fault_num;
		vStatNodeList[index].uAffection += count;
		if (primary_output_list.find(vStatNodeList[index].sName) != primary_output_list.end())
			output_res |= *res;
		(*value == ONE) 
			? vStatNodeList[index].uLogicOne += fault_num 
			: vStatNodeList[index].uLogicZero += fault_num;
	}
	
	for (index = 0; index < (*fault_injeciton_list).size(); index++) {
		if (output_res.test(index)) {
			vStatNodeList[(*fault_injeciton_list)[index]].uPropagation++;
		}
		vStatNodeList[(*fault_injeciton_list)[index]].uInjection++;
	}
	return;
}

void simulation_evaluation::summarize_golden_results(long parallel_num) {
	Wire_value* value = NULL;
	bitset<MAX_PARALLEL_NUM>* res = NULL;
	size_t count;
	size_t index;
	for (index = 0; index < node_num; index++) {
		get_node_value(index, &value, &res);
		count = res->count();
		
		if (*value == ONE) {
			vStatNodeList[index].uLogicZero += count;
			vStatNodeList[index].uLogicOne += parallel_num - count;
		}
		else {
			vStatNodeList[index].uLogicOne += count;
			vStatNodeList[index].uLogicZero += parallel_num - count;
		}
	}
	return;
}

StatNode* simulation_evaluation::get_stat_node(string& node_name) {
	map<string, int>::iterator ite;
	if ((ite = mStatNode.find(node_name)) == mStatNode.end())
		return NULL;
	return &vStatNodeList[(*ite).second];
}

void	simulation_evaluation::get_node_value(string name, Wire_value **value, bitset<MAX_PARALLEL_NUM> **vector) {
	std::map<string, int>::const_iterator ite = sim->mapSimNodeLst.find(name);
	if (ite == sim->mapSimNodeLst.end())
		throw exception(("node " + name + " not found. (get_node_value)").c_str());
	*value = &sim->vSimNodeLst[ite->second]->eValue;
	*vector = &sim->vSimNodeLst[ite->second]->bsParallelVector;
	return;
}

void	simulation_evaluation::get_node_value(size_t index, Wire_value **value, bitset<MAX_PARALLEL_NUM> **vector) {
	if (index >= sim->vSimNodeLst.size())
		throw exception(("index " + std::to_string(index) + " exceeds range. (get_node_value)").c_str());
	*value = &sim->vSimNodeLst[index]->eValue;
	*vector = &sim->vSimNodeLst[index]->bsParallelVector;
	return;
}