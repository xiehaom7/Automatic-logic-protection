#pragma once

#include "simulation.h"
#include "config.h"

typedef struct {
	string					sName;
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
	vector<StatNode*>		vStatNodeList;
	map<string, int>		mStatNode;
	simulation*				sim;
	size_t					input_num;
	size_t					node_num;
private:
	StatNode*				_create_stat_node(string name);
public:
	simulation_evaluation();
	~simulation_evaluation();
public:
	void					destroy();
	bool					construct(simulation* tar_sim);
	void					run_exhaustive_golden_simulation();
	void					run_exhaustive_fault_injection_simulation();
	void					summarize_golden_results(long parallel_num);
	void					summarize_fault_injection_results(int fault_num);
	StatNode*				get_stat_node(string& node_name);
};

