#pragma once
#include <set>
#include "module.h"
#include "config.h"

typedef enum { RANDOM, SEQUENCE, RESET } Gen_mode;
typedef enum { SA1, SA0, FLIP, NONE } Fault_mode;

typedef struct {
public:
	string						sName;
	string						sPrefix;
	Wire_value					eValue;
	vector<int>					vFanin;
	cell*						pCell;
	bitset<MAX_PARALLEL_NUM>	bsParallelVector;
}SimNode;

class simulation {
private:
	vector<SimNode*>			vSimNodeLst;
	map<string, int>			mapSimNodeLst;
	map<string, vector<int>>	mapPrefix;
	vector<int>					vPrimaryInputLst;
	vector<int>					vPrimaryOutputLst;
	vector<int>					vFaultCandidateLst;
	vector<int>					vFaultInjectionLst;
	unsigned					uFaultIndex;

private:
	unsigned	_count_fault_res();
	SimNode*	_creat_sim_node(net* current_net, string &prefix, vector<int> &input_index_list);

public:
	simulation();
	~simulation();

private:
	simulation& _parallel_simulate_node(SimNode* tar_node, Fault_mode fm);

public:
	void		destroy();
	int			construct_module(const vector<int> &input_list, vector<int> &output_list, int start_pos, module* tar_module, string prefix);
	bool		construct(module* top_module);
	void		generate_fault_list(vector<string> &prefix_list);
	void		generate_fault_list();
	simulation& simulate_module(Fault_mode fm);
	//Wire_value	simulate_single_node(SimNode* tar_node);
	//Wire_value	simulate_single_node_w_fault(SimNode* tar_node, Fault_mode fm, int fault_num);
	//void		simulate();
	//void		simulate_w_fault(Fault_mode fm, int fault_num);
	//void		simulate_w_fault_dump(Fault_mode fm, int fault_num, const string dump_file_name);
	//simulation&	run_fault_injection_simulation(Sim_mode sm, Fault_mode fm, int sim_num, int fault_num, bool random = true);
	//simulation&	run_exhaustive_FI_simulation(Fault_mode fm);
	//simulation& run_random_golden_simulation(int sim_num);
	//simulation& run_random_FI_simulation(Fault_mode fm, int sim_num, int fault_num);
	simulation&	inject_faults(int fault_num, Gen_mode mode);
	void		display(const string& display_file);
	string		save_info();
	bool		return_outputs(vector<Wire_value> &outputs);
	bool		return_inputs(vector<Wire_value> &inputs);
	simulation&	set_input_vector(vector<bool> &inputs);
	simulation& set_parallel_input_vector(vector<bool> &parallel, vector<bool> &main, unsigned round);
	simulation& get_node_value(string name, Wire_value &value, bitset<MAX_PARALLEL_NUM> &vector);
	simulation&	get_node_name_list(vector<string> &name_list);
	simulation& get_primary_outputs_list(set<string> &output_list);
	int			get_primary_inputs_num();
	int			get_primary_outputs_num();
	int			get_fault_candidate_size();
	vector<int>*	get_fault_injection_list();

public:
	static void generate_input_vector(vector<bool> &input_vector, Gen_mode mode);
};

inline int simulation::get_primary_inputs_num() {
	return vPrimaryInputLst.size();
}

inline int simulation::get_primary_outputs_num() {
	return vPrimaryOutputLst.size();
}

inline vector<int>*	simulation::get_fault_injection_list() {
	return &vFaultInjectionLst;
}

inline int simulation::get_fault_candidate_size() {
	return vFaultCandidateLst.size();
}
