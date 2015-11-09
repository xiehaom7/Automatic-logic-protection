#include "cell.h"

void cell::generate_forward_ref_table() {
	unsigned long max_input = (unsigned int)pow((double)3, (double)input_num) - 1;
	unsigned long input_vector;
	unsigned long index;
	bitset<MAX_CELL_INPUTS_X2> curr;
	vector<bitset<MAX_CELL_INPUTS_X2>>::const_iterator c_ite;

	curr.reset();

	for (input_vector = 0; input_vector <= max_input; input_vector++){
		index = curr.to_ulong();

		if (cell_utility::forward_ref(curr, cc1_array))
			forward_ref_table[index] = true;
		else if (cell_utility::forward_ref(curr, cc0_array))
			forward_ref_table[index] = false;
		
		cell_utility::next_value(curr, input_num);
	}
}

void cell::generate_backward_ref_table() {
	unsigned long max_input = (unsigned int)pow((double)3, (double)input_num) - 1;
	unsigned long input_vector;
	unsigned long index;
	bitset<MAX_CELL_INPUTS_X2> mask, curr, res;
	vector<bitset<MAX_CELL_INPUTS_X2>>::const_iterator c_ite;

	curr.reset();

	for (input_vector = 0; input_vector <= max_input; input_vector++) {
		res.set();
		cell_utility::create_mask(curr, mask, input_num);
		index = curr.to_ulong();
		
		if (!cell_utility::check_all_known(curr, input_num)) {
			res = cell_utility::backward_ref(curr, mask, cc1_array);
			if (!res.none())
				one_backward_ref_table[index] = res;

			res = cell_utility::backward_ref(curr, mask, cc0_array);
			if (!res.none())
				zero_backward_ref_table[index] = res;
		}
		cell_utility::next_value(curr, input_num);
	}
}

void cell::generate_truth_table() {
	unsigned long max_input = (unsigned int)pow((double)2, (double)input_num) - 1;
	unsigned long input_vector, current_result;
	bitset<MAX_CELL_INPUTS_X2> input_vector_b, mask;
	mask.reset();

	for (input_vector = 0; input_vector <= max_input; input_vector++) {
		current_result = 0;
		input_vector_b.reset();

		cell_utility::int_to_vector(input_vector, input_vector_b, input_num);
		
		if (cell_utility::forward_ref(input_vector_b, cc1_array))
			truth_table.push_back(true);
		else
			truth_table.push_back(false);
	}
}

cell::cell() {
	input_num = 0;
}

cell::~cell() {
	for (map<string, Pin_implication_item*>::iterator ite_rw_op = rw_op_collection.begin(); ite_rw_op != rw_op_collection.end(); ite_rw_op++) {
		for (int implication_item_index = 0; implication_item_index < 4; implication_item_index++) {
			if (ite_rw_op->second->implication_item_array[implication_item_index] != NULL) {
				delete ite_rw_op->second->implication_item_array[implication_item_index];
				ite_rw_op->second->implication_item_array[implication_item_index] = NULL;
			}
			delete ite_rw_op->second->implication_item_array[implication_item_index];
			ite_rw_op->second->implication_item_array[implication_item_index] = NULL;
		}
		delete ite_rw_op->second;
		ite_rw_op->second = NULL;
	}
}

int cell::get_input_pos(const string pin_name) {
	map<string, int>::iterator ite = map_input_pin.find(pin_name);
	return (ite != map_input_pin.end()) ? ite->second : -1;
}

string cell::get_inpin_name(int input_pos) {
	assert((input_pos >= 0) && input_pos <input_num);
	return input_pinlist[input_pos];
}

cell_library::cell_library(const string l_name) {
	library_name = l_name;
}

cell_library::~cell_library() {
	for (deque<cell*>::iterator ite_cell_list = cell_list.begin(); ite_cell_list != cell_list.end(); ite_cell_list++) {
		delete (*ite_cell_list);
	}
}

cell* cell_library::find_cell(const string cell_name) {
	map<string, cell*>::iterator ite = map_cell.find(cell_name);
	return (ite != map_cell.end()) ? ite->second : NULL;
}

bool cell_library::_parse_inputs(string &s, vector<string> *v, map<string, int> *m, int *n) {
	std::regex e;
	std::regex_iterator<std::string::iterator> rit;
	std::regex_iterator<std::string::iterator> rend;
	unsigned int num_input = 0;

	e = ("\\w+");
	rit = std::regex_iterator<std::string::iterator>(s.begin(), s.end(), e);
	while (rit != rend) {
		if (v != NULL)
			v->push_back(rit->str());
		if (m != NULL)
			(*m)[rit->str()] = num_input;
		num_input++;
		rit++;
	}
	if (n != NULL)
		*n = num_input;
	assert(num_input * 2 <= MAX_CELL_INPUTS_X2);
	return true;
}

bool cell_library::_parse_output(string &s, string &o) {
	std::regex e;
	std::smatch sm;

	e = ("\\w+");
	std::regex_match(s, sm, e);
	o = sm[0];
	return true;
}

bool cell_library::_parse_01_vector(string &s, vector<bitset<MAX_CELL_INPUTS_X2>> &v) {
	std::regex e;
	std::regex_iterator<std::string::iterator> rit;
	std::regex_iterator<std::string::iterator> rend;
	string match;

	v.clear();
	e = ("[01]+");
	rit = std::regex_iterator<std::string::iterator>(s.begin(), s.end(), e);
	while (rit != rend) {
		match = rit->str();
		std::reverse(match.begin(), match.end());
		bitset<MAX_CELL_INPUTS_X2> cc1_vector(match);
		v.push_back(cc1_vector);
		rit++;
	}
	return true;
}

bool cell_library::parse_cc_file(stringstream &ss) {
	string buff;
	cell* tar_cell;
	bool new_flag = false;
	std::regex e;
	std::smatch sm;

	while (getline(ss, buff)) {
		e = ("#(\\w+)");
		if (!std::regex_match(buff, sm, e))
			continue;

		tar_cell = find_cell(sm[1]);
		if (tar_cell == NULL) {
			new_flag = true;
			tar_cell = new cell;
		}

		if (new_flag)
			tar_cell->cell_name = sm[1];

		getline(ss, buff);
		if (new_flag) {
			_parse_inputs(buff, &tar_cell->input_pinlist, &tar_cell->map_input_pin, &tar_cell->input_num);
		}

		getline(ss, buff);
		if (new_flag) {
			_parse_output(buff, tar_cell->output_pin);
		}

		getline(ss, buff);
		_parse_01_vector(buff, tar_cell->cc1_array);

		getline(ss, buff);
		_parse_01_vector(buff, tar_cell->cc0_array);

		tar_cell->generate_truth_table();
		tar_cell->generate_backward_ref_table();
		tar_cell->generate_forward_ref_table();

		if (new_flag) {
			cell_list.push_back(tar_cell);
			map_cell[tar_cell->cell_name] = tar_cell;
		}
	}
	return true;
}

bool cell_library::read_cc_file(const string file_path) {
	ifstream pFile(file_path.c_str());
	stringstream buffer;
	if (!pFile.is_open()) {
		cerr << "ERROR: failed to open file " + file_path + "\n";
		return false;
	}

	cout << "INFO: successed to open file " + file_path + "\n";
	buffer << pFile.rdbuf();
	pFile.close();
	return parse_cc_file(buffer);
}

bool cell_library::parse_co_file(stringstream &ss) {
	string buff;
	int j;
	cell* tar_cell;
	bool new_flag = false;
	std::regex e;
	std::smatch sm;
	bitset<MAX_CELL_INPUTS_X2> co_vector(0);

	while (getline(ss, buff)) {
		e = ("#(\\w+)");
		if (!std::regex_match(buff, sm, e))
			continue;
		tar_cell = find_cell(sm[1]);
		if (tar_cell == NULL) {
			new_flag = true;
			tar_cell = new cell;
		}

		if (new_flag)
			tar_cell->cell_name = sm[1];

		getline(ss, buff);
		if (new_flag) {
			_parse_inputs(buff, &tar_cell->input_pinlist, &tar_cell->map_input_pin, &tar_cell->input_num);
		}

		tar_cell->co_array.resize(tar_cell->input_num);

		getline(ss, buff);
		if (new_flag) {
			_parse_output(buff, tar_cell->output_pin);
		}

		for (j = 0; j < tar_cell->input_num; j++) {
			getline(ss, buff);
			_parse_01_vector(buff, tar_cell->co_array[j]);
		}

		if (new_flag) {
			cell_list.push_back(tar_cell);
			map_cell[tar_cell->cell_name] = tar_cell;
		}
	}
	return true;
}

bool cell_library::read_co_file(const string file_path) {
	ifstream pFile(file_path.c_str());
	stringstream buffer;
	std::regex e;
	std::smatch sm;
	std::regex_iterator<std::string::iterator> rit;
	std::regex_iterator<std::string::iterator> rend;
	bitset<MAX_CELL_INPUTS_X2> co_vector(0);

	if (!pFile.is_open()) {
		cerr << "ERROR: failed to open file " + file_path + "\n";
		return false;
	}
		
	cout << "INFO: successed to open file " + file_path + "\n";
	buffer << pFile.rdbuf();	
	pFile.close();
	return parse_co_file(buffer);
}

bool cell_library::read_rw_operation(const string file_path) {
	ifstream pFile(file_path.c_str());
	stringstream buffer;

	if (!pFile.is_open()) {
		cerr << "ERROR: failed to open file " + file_path + "\n";
		return false;
	}
	
	cout << "INFO: successed to open file " + file_path + "\n";
	buffer << pFile.rdbuf();
	pFile.close();
	return parse_rw_op(buffer);
}

bool cell_library::parse_rw_op(stringstream &ss) {
	string buff = ss.str();
	size_t pos;

	pos = buff.find_first_of('#');
	while (string_utility::test_within(buff, pos, "#")) {
		pos += 1;
		_parse_cell_item(buff, pos);
	}
	return true;
}

bool cell_library::_parse_cell_item(string &s, size_t &pos) {
	string cell_name = string_utility::read_next_word(s, pos);
	cell *tar_cell = find_cell(cell_name);
	if (tar_cell == NULL) {
		cerr << "WARNING: Failed to find cell " << cell_name << ". (read_rw_operation)\n";
		return false;
	}
	while (string_utility::test_within(s, pos, "@")) {
		pos += 1;
		_parse_pin_item(s, pos, tar_cell->rw_op_collection);
	}
	return true;
}

bool cell_library::_parse_pin_item(string &s, size_t &pos, map<string, Pin_implication_item*> &m) {
	Pin_implication_item* new_pii = new Pin_implication_item;
	new_pii->pin_name = string_utility::read_next_word(s, pos);

	for (int i = 0; i < 4; i++) {
		new_pii->implication_item_array[i] = NULL;
	}
	while (string_utility::test_within(s, pos, "$")) {
		pos += 1;
		_parse_implication_item(s, pos, new_pii->implication_item_array);
	}
	m[new_pii->pin_name] = new_pii;
	return true;
}

bool cell_library::_parse_implication_item(string &s, size_t &pos, Implication_item** i) {
	int implication_index = 0;
	Implication_item* new_ii;
	char left, right;
	if (string_utility::expect_within(s, pos, RW_VALID_LOGIC) == false)
		return false;
	left = s[pos - 1];
	string_utility::read_space(s, pos);
	string_utility::expect_within(s, pos, "-");
	string_utility::expect_within(s, pos, ">");
	string_utility::read_space(s, pos);
	if (string_utility::expect_within(s, pos, RW_VALID_LOGIC) == false)
		return false;
	right = s[pos - 1];
	string_utility::read_space(s, pos);
	new_ii = new Implication_item;
	if (left == '0') {
		new_ii->x_value = ZERO;
	}
	else {
		new_ii->x_value = ONE;
		implication_index += 2;
	}
	if (right == '0') {
		new_ii->pin_value = ZERO;
	}
	else {
		new_ii->pin_value = ONE;
		implication_index += 1;
	}
	while (string_utility::test_within(s, pos, "&")) {
		pos += 1;
		_parse_operation(s, pos, new_ii->op_list);
	}
	*(i + implication_index) = new_ii;
	return true;
}

bool cell_library::_parse_operation(string &s, size_t &pos, vector<Op_item> &v) {
	string op = string_utility::read_next_word(s, pos);
	Op_item new_ot;
	if (op == "RG")
		new_ot.op = RG;
	else if (op == "AG")
		new_ot.op = AG;
	else if (op == "AW")
		new_ot.op = AW;
	else if (op == "CW")
		new_ot.op = CW;
	else if (op == "DW")
		new_ot.op = DW;
	else {
		cerr << "ERROR: un-known operation " << op << ". (read_rw_operation)";
		throw string("RW_FAIL");
	}
	string_utility::expect_within(s, pos, "(");
	new_ot.op1 = string_utility::read_next_word(s, pos);
	if (string_utility::test_within(s, pos, ".")) {
		pos += 1;
		new_ot.op1_pin = string_utility::read_next_word(s, pos);
	}
	if (string_utility::test_within(s, pos, ",")) {
		pos += 1;
		string_utility::read_space(s, pos);
		new_ot.op2 = string_utility::read_next_word(s, pos);
		if (string_utility::test_within(s, pos, ".")) {
			pos += 1;
			new_ot.op2_pin = string_utility::read_next_word(s, pos);
		}
	}
	string_utility::expect_within(s, pos, ")");
	string_utility::read_space(s, pos);
	if (string_utility::test_within(s, pos, ":")) {
		pos += 1;
		string_utility::read_space(s, pos);
		new_ot.new_name = string_utility::read_next_word(s, pos);
	}
	v.push_back(new_ot);
	return true;
}