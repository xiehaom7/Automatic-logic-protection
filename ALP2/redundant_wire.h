#pragma once
#include "module.h"
#include "simulation_evaluation.h"
#include "signature.h"
#include <set>
#include <algorithm>
#include <valarray>

typedef enum {
	BACKWARD, DIRECT, INDIRECT_LOW, INDIRECT_HIGH
}Implicaton_method;

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
	Implication_comb				ori;
	map<string, Implication_comb>	imp_results;
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
	valarray<bool>				matrixImplication;
	valarray<bool>				matrixImplicationScreen;
	vector<list<Implication_comb>>	vImplication;
	vector<set<int>>			vNodeFaninSet;
	vector<set<int>>			vNodeFanoutSet;
	/*vector<map<int, set<int>>>	vNodeDominatorSet;*/
	list<int>					lModificationIndexLst;

	unsigned					rw_wire_added_counter;
	unsigned					rw_gate_added_counter;
public:
	redundant_wire();
	~redundant_wire();
public:
	void						destroy();
	bool						construct(module* tar_module);
	int							get_node_index(string node_name);
	string						get_node_name(int index);
	set<string>					get_node_fanin(string node_name);
	set<string>					get_node_fanout(string node_name);
	string						get_cell_type(string node_name);
	int							get_implication_num();
	vector<list<Implication_comb>>	get_implication_list();
	void						set_value(int index, Wire_value value);
	void						set_X();
private:
	RWNode*						_create_RW_node(net* current_net, string &prefix, 
		vector<int> &i_list);
	int							_construct_module(const vector<int> &input_list,
		vector<int> &output_list, int start_pos, module* tar_module, string& prefix);

//methods for initialization
	void						_setup_implication_vector();
	void						_setup_implication_matrix();
	void						_setup_implication_screen_matrix();
	void						_setup_fanin_fanout_set();
	/*void						_setup_dominator_set();*/

//methods for implication matrix generation
	bool						_implication_verify(bitset<MAX_CELL_INPUTS_X2> mask, 
		bitset<MAX_CELL_INPUTS_X2> curr, const bitset<MAX_CELL_INPUTS_X2> &req) const;
	void						_backward_justify(int tar_index, stack<int> &stack_index);
	void						_lookup_backward_table(unsigned index, int tar_index, map<unsigned,
		bitset < MAX_CELL_INPUTS_X2 >> &table, stack<int> &stack_index);
	void redundant_wire::_lookup_forward_table(unsigned index, int tar_index, map<unsigned,
		bool> &table, stack<int> &stack_index);
	unsigned					_calculate_inputs_index(int tar_index);
	void						_forward_justify(int tar_index, stack<int> &stack_index);
	void						_matrix_generator(Implicaton_method method);
	void						_generate_implication_vector();

public:
	void						justification(Implication_list &imp_list);
	void						justification_full(Implication_list &imp_list, 
		set<Implication_comb, Implication_comb_compare> &update_set);
	void						direct_justification(Implication_list &imp_list);
	void						backward_justification(Implication_list &imp_list);
	void						implication_matrix_generator_backward();
	void						implication_matrix_generator_direct();
	void						implication_matrix_generator();
	void						implication_matrix_generator_full();
	
};

inline	bool	redundant_wire::_implication_verify(bitset<MAX_CELL_INPUTS_X2> mask, 
	bitset<MAX_CELL_INPUTS_X2> curr, const bitset<MAX_CELL_INPUTS_X2> &req) const {
	//mask bits with value 1 : bits already known
	curr.flip();
	return (mask & curr & req).none();
}

inline vector<list<Implication_comb>>	redundant_wire::get_implication_list() {
	return vImplication;
}