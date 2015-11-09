#pragma once
#include "module.h"
#include "simulation_evaluation.h"
#include "signature.h"
#include <set>
#include <algorithm>

typedef struct {
	int							index;
	Wire_value					val;
}Implication_comb;

struct Implication_comb_compare {
	bool operator()(const Implication_comb &a, const Implication_comb &b) const {
		if (a.index < b.index) {
			return true;
		}
		else if (a.index == b.index && a.val == ZERO && b.val == ONE) {
			return true;
		}
		else {
			return false;
		}
	}
};

typedef struct {
	Implication_comb			left_item;
	Implication_comb			right_item;
}Implication_pair;

typedef struct {
	Implication_comb			ori;
	vector<Implication_comb>	imp_results;
}Implication_list;

typedef struct {
	string						sName;
	Wire_value					eValue;
	vector<int>					vFanin;
	vector<int>					vFanout;
	cell*						pCell;
	/*StatNode					strStat;*/
}RWNode;



class redundant_wire
{
private:
	module*						pTopModule;
	vector<RWNode*>				vRWNodeList;
	map<string, int>			mapRWNodeList;
	vector<int>					vPrimaryInputList;
	vector<int>					vPrimaryOutputList;
	vector<vector<bool>>		matrixImplication;
	/*vector<vector<bool>>		matrixImplicationScreen;*/
	vector<list<Implication_comb>>	vSimpleIndirectImplication;
	vector<list<Implication_comb>>	vIndirectImplication;
	vector<set<int>>			vNodeFaninSet;
	vector<set<int>>			vNodeFanoutSet;
	/*vector<map<int, set<int>>>	vNodeDominatorSet;*/
	/*list<int>					lModificationIndexLst;*/

	unsigned					rw_wire_added_counter;
	unsigned					rw_gate_added_counter;
public:
	redundant_wire();
	~redundant_wire();
public:
	void						destroy();
	bool						construct(module* tar_module);
	int							get_node_index(string &node_name);
	set<string>				get_node_fanin(string node_name);
	set<string>				get_node_fanout(string node_name);
	string						get_cell_type(string node_name);
private:
	RWNode*						_create_RW_node(net* current_net, string &prefix, 
		vector<int> &i_list);
	int							construct_module(const vector<int> &input_list,
		vector<int> &output_list, int start_pos, module* tar_module, string& prefix);
};

