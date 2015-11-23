#include "simulation_evaluation.h"

void simulation_evaluation::destroy() {
	vector<StatNode*>::iterator ite;
	for (ite = vStatNodeList.begin(); ite != vStatNodeList.end(); ite++)
		delete *ite;
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

StatNode* simulation_evaluation::_create_stat_node(string name) {
	StatNode* sn = new StatNode;
	sn->sName = name;
	sn->uInjection = 0;
	sn->uPropagation = 0;
	sn->uLogicOne = 0;
	sn->uLogicZero = 0;
	sn->uAffection = 0;
	sn->uSimulation = 0;
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
		vStatNodeList[i] = _create_stat_node(node_name);
		mStatNode[node_name] = i;
	}	
	return true;
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
			sim->run_fault_injection_simulation(fault_num, &input_vector, false);
			summarize_fault_injection_results(fault_num);
			target_num -= MAX_PARALLEL_NUM;
		}
		sim->generate_input_vector(input_vector, SEQUENCE);
		sim_num--;
	}
	return;
}

void simulation_evaluation::run_random_fault_injection_simulation(int sim_num, int fault_num) {
	vector<bool> input_vector(input_num, false);
	sim->generate_fault_list();
	sim->inject_faults(0, RESET);
	while (sim_num > 0) {
		sim->generate_input_vector(input_vector, RANDOM);
		sim->run_fault_injection_simulation(fault_num, &input_vector);
		summarize_fault_injection_results(fault_num);
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
		sim->run_golden_simulation(round_num, false, &input_vector);
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
		sim->get_node_value(index, &value, &res);
		count = res->count();
		vStatNodeList[index]->uSimulation += fault_num;
		vStatNodeList[index]->uAffection += count;
		if (primary_output_list.find(vStatNodeList[index]->sName) != primary_output_list.end())
			output_res |= *res;
		(*value == ONE) 
			? vStatNodeList[index]->uLogicOne += 1
			: vStatNodeList[index]->uLogicZero += 1;
	}
	
	for (index = 0; index < (*fault_injeciton_list).size(); index++) {
		if (output_res.test(index)) {
			vStatNodeList[(*fault_injeciton_list)[index]]->uPropagation++;
		}
		vStatNodeList[(*fault_injeciton_list)[index]]->uInjection++;
	}
	return;
}

void simulation_evaluation::summarize_golden_results(long parallel_num) {
	Wire_value* value = NULL;
	bitset<MAX_PARALLEL_NUM>* res = NULL;
	size_t count;
	size_t index;
	for (index = 0; index < node_num; index++) {
		sim->get_node_value(index, &value, &res);
		count = res->count();
		
		if (*value == ONE) {
			vStatNodeList[index]->uLogicZero += count;
			vStatNodeList[index]->uLogicOne += parallel_num - count;
		}
		else {
			vStatNodeList[index]->uLogicOne += count;
			vStatNodeList[index]->uLogicZero += parallel_num - count;
		}
	}
	return;
}

StatNode* simulation_evaluation::get_stat_node(string& node_name) {
	map<string, int>::iterator ite;
	if ((ite = mStatNode.find(node_name)) == mStatNode.end())
		return NULL;
	return vStatNodeList[(*ite).second];
}

double simulation_evaluation::evaluate_fault_injection_results() {
	vector<StatNode*>::const_iterator c_ite;
	double ser = 0.0;
	long error = 0;
	long injection = 0;
	for (c_ite = vStatNodeList.cbegin(); c_ite != vStatNodeList.cend(); c_ite++) {
		if ((*c_ite)->uInjection == 0)
			continue;
		error += (*c_ite)->uPropagation;
		injection += (*c_ite)->uInjection;
		ser += (double)(*c_ite)->uPropagation / (*c_ite)->uInjection;
	}
	cout << "ERROR:\t\t" << error << endl;
	cout << "injection:\t" << injection << endl;
	cout << "ER:\t\t" << (float)error / injection << endl;
	cout << "SER:\t\t" << ser << endl;
	return ser;
}
