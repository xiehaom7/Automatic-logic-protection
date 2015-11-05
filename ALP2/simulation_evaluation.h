#pragma once
#include "module.h"
#include "simulation.h"
#include "config.h"

typedef struct {
	bitset<SIGNATURE_SIZE>	vSig;
	bitset<SIGNATURE_SIZE>	vODCmask;
	unsigned				uTest0;
	unsigned				uTest1;
}SignatureNode;

typedef struct {
	string					sName;
	SignatureNode			strSig;
	unsigned				uLogicOne;
	unsigned				uLogicZero;
	unsigned				uInjection;
	unsigned				uPropagation;
	unsigned				uSimulation;
	unsigned				uAffection;
}StatNode;

class simulation_evaluation
{
private:
	vector<StatNode>		vStatNodeList;
	map<string, int>		mStatNode;
	simulation*				sim;
	size_t					input_num;
	size_t					node_num;
private:
	StatNode				_init_stat_node(string name);
public:
	simulation_evaluation();
	~simulation_evaluation();
public:
	void					destroy();
	bool					construct(simulation* tar_sim);
	void					generate_signature();
	void					run_exhaustive_golden_simulation();
	void					run_exhaustive_fault_injection_simulation();
	simulation_evaluation&	run_golden_simulation(int sim_num, bool random,
		vector<bool> *start_inputs);
	simulation_evaluation& run_fault_injection_simulation(
		int fault_num, vector<bool> *start_inputs, bool random, Fault_mode fm);
	void					summarize_golden_results(long parallel_num);
	void					summarize_fault_injection_results(int fault_num);
	StatNode*				get_stat_node(string& node_name);
	void					get_node_value(string name, Wire_value **value, bitset<MAX_PARALLEL_NUM> **vector);
	void					get_node_value(size_t index, Wire_value **value, bitset<MAX_PARALLEL_NUM> **vector);
};

