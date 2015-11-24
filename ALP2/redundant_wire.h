#pragma once
#include "module.h"
#include "simulation_evaluation.h"
#include "signature.h"
#include <set>
#include <algorithm>
#include <valarray>

const string WIRE_ADDED_NAME = "new_wire_";
const string GATE_ADDED_NAME = "new_gate_";

const string INVERTER_TYPE = "INV_X1";
const string INVERTER_INPUT_PIN = "A";
const string INVERTER_OUTPUT_PIN = "ZN";

const string AND_TYPE = "AND2_X1";
const string AND_INPUT_PIN_1 = "A1";
const string AND_INPUT_PIN_2 = "A2";
const string AND_OUTPUT_PIN = "ZN";

const string OR_TYPE = "OR2_X1";
const string OR_INPUT_PIN_1 = "A1";
const string OR_INPUT_PIN_2 = "A2";
const string OR_OUTPUT_PIN = "ZN";

typedef enum {
	BACKWARD = 0, DIRECT, INDIRECT_LOW, INDIRECT_HIGH
}Implicaton_method;

typedef enum {
	NO_INVERTED_OR_ADDED = 0, INVERTED, ADDED, INVERTED_AND_ADDED
}Redundant_wire_type;

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

typedef enum {
	PROTECT = 0, SOURCE_NEG, ADDED_NEG
}Gate_record_type;

typedef struct {
	int							gate_index;
	Gate_record_type			type;
	bitset<SIGNATURE_SIZE>		ODCmask[4];
	bitset<SIGNATURE_SIZE>		ODCres;
}Gate_record_item;

typedef struct {
	Implication_pair			imp_pair;
	bitset<SIGNATURE_SIZE>		source_sig;
	bitset<SIGNATURE_SIZE>		dest_ob_sig;
	float						evaluation_res;
	vector<Gate_record_item>	gate_record_vector;
	bool						flag;
}Rank_item;

typedef struct {
	string						sName;
	Wire_value					eValue;
	vector<int>					vFanin;
	vector<int>					vFanout;
	cell*						pCell;
}RWNode;

class redundant_wire
{
private:
	module*						pTopModule;
	simulation					sim;
	signature					sig;
	vector<RWNode*>				vRWNodeList;
	map<string, int>			mapRWNodeList;
	vector<int>					vPrimaryInputList;
	vector<int>					vPrimaryOutputList;
	valarray<bool>				matrixImplication;
	valarray<bool>				matrixImplicationScreen;
	vector<list<Implication_comb>>	vImplication;
	vector<set<int>>			vNodeFaninSet;
	vector<set<int>>			vNodeFanoutSet;
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
	int							_generate_implication_vector();
	void						_execute_op(const Op_item &op, map<string, node*> &mapNode, map<string, net*> &mapNet);
	void						_parse_op(const Op_item &op, int &inverted, int &added);
	int							_count_distance(int dest_node_index, int source_node_index);

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
	
	void						calculate_ODCres(const bitset<SIGNATURE_SIZE> &source,
		const bitset<SIGNATURE_SIZE> &observability_to_all,
		const bitset<SIGNATURE_SIZE> &observability_to_source,
		const bitset<SIGNATURE_SIZE> &observability_exclude_target,
		bitset<SIGNATURE_SIZE> &res);
	void						calculate_NEG_ODCres(const bitset<SIGNATURE_SIZE> &dest,
		const bitset<SIGNATURE_SIZE> &observability_to_all,
		const bitset<SIGNATURE_SIZE> &observability_to_source,
		const bitset<SIGNATURE_SIZE> &dest_observability_to_all,
		bitset<SIGNATURE_SIZE> &res);
	void						calculate_protect(const bitset<SIGNATURE_SIZE> &ODCres,
		const bitset<SIGNATURE_SIZE> &unprotect,
		bitset<SIGNATURE_SIZE> &res);
	void						update_unprotect(Rank_item& tar_item, vector<bitset<SIGNATURE_SIZE>>& unprotect);
	void						setup_observability(vector<bitset<SIGNATURE_SIZE>> &cache_node_ob);
	void						setup_to_source_observability(int target,
		map<int, vector<bitset<SIGNATURE_SIZE>>> &cache_to_source_ob);
	void						setup_not_to_dest_observability(int target,
		map<int, vector<bitset<SIGNATURE_SIZE>>> &cache_not_to_dest_ob);
	float						implication_evaluator(Implication_comb left_imp, 
		Implication_comb right_imp,
		vector<bitset<SIGNATURE_SIZE>> &unprotected_sig, 
		vector<Gate_record_item> &vector_gate_record,
		vector<bitset<SIGNATURE_SIZE>> &cache_node_ob,
		map<int, vector<bitset<SIGNATURE_SIZE>>> &cache_to_source_ob,
		map<int, vector<bitset<SIGNATURE_SIZE>>> &cache_not_to_dest_ob);
	bool						redundant_wire_adder(Implication_comb &left_comb, Implication_comb &right_comb);
	Redundant_wire_type			redundant_wire_type_predict(Implication_comb &left_comb, Implication_comb &right_comb);
	void						redundant_wire_selector(const string module_name, int rw_num, 
		int sample_freq, int sim_num, int fault_num);
	int							implication_counter(Implicaton_method method);
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