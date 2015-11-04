#include "simulation_evaluation.h"

void simulation_evaluation::destroy() {
	vStatNodeList.clear();
	mStatNode.clear();
	sim.destroy();
	input_num = 0;
	return;
}

simulation_evaluation::simulation_evaluation() {
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

bool simulation_evaluation::construct(module* top_module) {
	vector<string> name_list;
	vector<string>::const_iterator ite;
	destroy();
	if (!sim.construct(top_module))
		return false;

	sim.get_node_name_list(name_list);
	for (ite = name_list.cbegin(); ite != name_list.cend(); ite++) {
		vStatNodeList.push_back(_init_stat_node((*ite)));
		mStatNode[*ite] = vStatNodeList.size() - 1;
	}

	input_num = sim.get_primary_inputs_num();
	return true;
}

simulation_evaluation&	simulation_evaluation::run_golden_simulation(
	int sim_num, bool random = true, vector<bool> *start_inputs = NULL) {
	vector<bool> input_vector(input_num, false);
	vector<bool> empty_vector(input_num, false);
	int i;
	if (random || start_inputs == NULL)
		start_inputs = &empty_vector;
	sim.set_input_vector(input_vector);
	sim.inject_faults(0, RESET);
	for (i = 0; i < sim_num; i++) {
		sim.set_parallel_input_vector(*start_inputs, input_vector, i);
		sim.generate_input_vector(*start_inputs, random ? RANDOM : SEQUENCE);
	}
	sim.simulate_module(FLIP);
	return *this;
}

simulation_evaluation& simulation_evaluation::run_fault_injection_simulation(
	int fault_num, vector<bool> *start_inputs = NULL, bool random = true, Fault_mode fm = FLIP) {
	if (fault_num >= MAX_PARALLEL_NUM)
		throw exception(("specified fault number " + std::to_string(fault_num) + " exceeds max parallel number " 
			+ std::to_string(MAX_PARALLEL_NUM) + ".(run_fault_injection_simulation)").c_str());
	int i;
	vector<bool> empty_vector(input_num, false);
	if (start_inputs == NULL)
		start_inputs = &empty_vector;

	sim.set_input_vector(*start_inputs);
	sim.inject_faults(fault_num, random ? RANDOM : SEQUENCE);
	sim.simulate_module(fm);
	return *this;
}

void simulation_evaluation::generate_signature() {
	vector<StatNode>::iterator ite;
	Wire_value value;
	bitset<MAX_PARALLEL_NUM> res;
	int i;
	
	run_golden_simulation(SIGNATURE_SIZE);

	for (ite = vStatNodeList.begin(); ite != vStatNodeList.end(); ite++) {
		sim.get_node_value((*ite).sName, value, res);
		if (value == ONE)
			res = res.flip();
		for (i = 0; i < SIGNATURE_SIZE; i++)
			(*ite).strSig.vSig[i] = res[i];	
	}
}

void simulation_evaluation::run_exhaustive_fault_injection_simulation() {
	long sim_num = (long)pow((double)2, (double)input_num);
	vector<bool> input_vector(input_num, false);
	int target_num;
	int fault_num;
	sim.generate_fault_list();
	sim.generate_input_vector(input_vector, RESET);
	sim.inject_faults(0, RESET);
	while (sim_num > 0) {
		target_num = sim.get_fault_candidate_size();
		while (target_num > 0) {
			fault_num = (target_num > MAX_PARALLEL_NUM) ? MAX_PARALLEL_NUM : target_num;
			run_fault_injection_simulation(fault_num, &input_vector, false);
			summarize_fault_injection_results(fault_num);
			target_num -= MAX_PARALLEL_NUM;
		}
		sim.generate_input_vector(input_vector, SEQUENCE);
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
	vector<StatNode>::iterator ite;
	Wire_value value;
	bitset<MAX_PARALLEL_NUM> res;
	bitset<MAX_PARALLEL_NUM> output_res;
	size_t count;
	int i;
	set<string> primary_output_list;
	vector<int>* fault_injeciton_list = sim.get_fault_injection_list();
	output_res.reset();
	sim.get_primary_outputs_list(primary_output_list);
	for (ite = vStatNodeList.begin(); ite != vStatNodeList.end(); ite++) {
		sim.get_node_value((*ite).sName, value, res);
		count = res.count();
		ite->uSimulation += fault_num;
		ite->uAffection += count;
		if (primary_output_list.find((*ite).sName) != primary_output_list.end())
			output_res |= res;
		(value == ONE) ? ite->uLogicOne++ : ite->uLogicZero++;
	}
	
	for (i = 0; i < (*fault_injeciton_list).size(); i++) {
		if (output_res.test(i)) {
			vStatNodeList[(*fault_injeciton_list)[i]].uPropagation++;
		}
		vStatNodeList[(*fault_injeciton_list)[i]].uInjection++;
	}
	return;
}

void simulation_evaluation::summarize_golden_results(long parallel_num) {
	vector<StatNode>::iterator ite;
	Wire_value value;
	bitset<MAX_PARALLEL_NUM> res;
	size_t count;
	for (ite = vStatNodeList.begin(); ite != vStatNodeList.end(); ite++) {
		sim.get_node_value((*ite).sName, value, res);
		count = res.count();
		if (value == ONE) {
			ite->uLogicZero += count;
			ite->uLogicOne += parallel_num - count;
		}
		else {
			ite->uLogicOne += count;
			ite->uLogicZero += parallel_num - count;
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