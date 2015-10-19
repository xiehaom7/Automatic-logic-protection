#pragma once
#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <math.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <bitset>
#include <map>
#include "config.h"
#include "cell_utility.h"
#include "string_utility.h"
#include "net.h"

using namespace std;

typedef enum { RG, AG, AW, CW, DW, SI, SO } Module_op;
typedef struct {
	Module_op					op;
	string						op1;
	string						op1_pin;
	string						op2;
	string						op2_pin;
	string						new_name;
} Op_item;
typedef struct {
	Wire_value					x_value;
	Wire_value					pin_value;
	vector<Op_item>				op_list;
} Implication_item;
typedef struct {
	string						pin_name;
	Implication_item*			implication_item_array[4];
} Pin_implication_item;

class cell {
public:
	string										cell_name;
	vector<string>								input_pinlist;
	int											input_num;
	string										output_pin;
	vector<bitset<MAX_CELL_INPUTS_X2>>			cc1_array;
	vector<bitset<MAX_CELL_INPUTS_X2>>			cc0_array;
	vector<vector<bitset<MAX_CELL_INPUTS_X2>>>	co_array;
	vector<bool>								truth_table;
	map<string, Pin_implication_item*>			rw_op_collection;
	map<string, int>							map_input_pin;
	map<unsigned, bitset<MAX_CELL_INPUTS_X2>>	one_backward_ref_table;
	map<unsigned, bitset<MAX_CELL_INPUTS_X2>>	zero_backward_ref_table;
	map<unsigned, bool>							forward_ref_table;
	 
public:
	cell();
	~cell();

public:
	void										generate_forward_ref_table();
	void										generate_truth_table();
	void										generate_backward_ref_table();
	int											get_input_pos(const string pin_name);
	string										get_inpin_name(int input_pos);
};

class cell_library {
private:
	string							library_name;
	deque<cell*>					cell_list;
public:
	map<string, cell*>				map_cell;

public:
	cell_library(const string l_name = "Unknown Library");
	cell* find_cell(const string cell_name);
	~cell_library();

public:
	bool	read_cc_file(const string file_path);
	bool	read_co_file(const string file_path);
	bool	read_rw_operation(const string file_path);
	bool	parse_cc_file(stringstream &ss);
	bool	parse_co_file(stringstream &ss);
	bool	parse_rw_op(stringstream &ss);

private:
	bool	_parse_inputs(string &s, vector<string> *v = NULL, 
		map<string, int> *m = NULL, int *n = NULL);
	bool	_parse_output(string &s, string &o);
	bool	_parse_01_vector(string &s, vector<bitset<MAX_CELL_INPUTS_X2>> &v);
	bool	_parse_operation(string &s, size_t &pos, vector<Op_item> &v);
	bool	_parse_implication_item(string &s, size_t &pos, Implication_item** i);
	bool	_parse_pin_item(string &s, size_t &pos, map<string, Pin_implication_item*> &m);
	bool	_parse_cell_item(string &s, size_t &pos);
};
