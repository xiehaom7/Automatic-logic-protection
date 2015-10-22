#include "module.h"

module::module(cell_library* cl) {
	pCellLib = cl;
	iInputNum = 0;
	iOutputNum = 0;
	iNodeNum = 0;
	iNetNum = 0;
}

void module::destroy() {
	vector<node*>::iterator ite_node;
	for (ite_node = vNodeLst.begin(); ite_node != vNodeLst.end(); ite_node++) {
		delete (*ite_node);
		(*ite_node) = NULL;
	}
	vector<net*>::iterator ite_net;
	for (ite_net = vNetLst.begin(); ite_net != vNetLst.end(); ite_net++) {
		delete (*ite_net);
		(*ite_net) = NULL;
	}
	list<node*>::iterator ite_uc_node;
	for (ite_uc_node = lNodeLstUncommitted.begin(); ite_uc_node != lNodeLstUncommitted.end(); ite_uc_node++) {
		delete (*ite_uc_node);
		(*ite_uc_node) = NULL;
	}
	list<net*>::iterator ite_uc_net;
	for (ite_uc_net = lNetLstUncommitted.begin(); ite_uc_net != lNetLstUncommitted.end(); ite_uc_net++) {
		delete (*ite_uc_net);
		(*ite_uc_net) = NULL;
	}
}

module::~module() {
	destroy();
}

net*	module::get_net(string net_name) const {
	map<string, net*>::const_iterator ite = mapNet.find(net_name);
	return (ite == mapNet.end()) ? NULL : ite->second;
}

node*	module::get_node(string node_name) const {
	map<string, node*>::const_iterator ite = mapNode.find(node_name);
	return (ite == mapNode.end()) ? NULL : ite->second;
}

bool	module::_remove_node(node* tar_node) {
	map<string, node*>::iterator ite_node;
	if ((ite_node = mapNode.find(tar_node->get_node_name())) != mapNode.end()) {
		mapNode.erase(ite_node);
		iNodeNum--;
		return true;
	}
	return false;
}

module&	module::remove_node(node* tar_node) {
	int i;
	int input_num = tar_node->get_input_num();
	int	output_num = tar_node->get_output_num();
	string node_name;

	if (tar_node == NULL)
		throw exception("NULL node specified. (remove_node)");

	node_name = tar_node->get_node_name();
	for (i = 0; i<input_num; i++) {
		if (tar_node->get_input_net(i) != NULL)
			throw exception((node_name + 
				" node to remove with input net connected. (remove_node)").c_str());
	}
	for (i = 0; i<output_num; i++) {
		if (tar_node->get_output_net(i) != NULL)
			throw exception((node_name + 
				"node to remove with output net connected. (remove_node)").c_str());
	}

	if (!_remove_node(tar_node))
		throw exception((node_name + 
			"node to remove not founded. (remove_node)").c_str());

	Record new_record;
	new_record.op_type = RG;
	new_record.op_info.node_ref = tar_node;
	lRecordLst.push_back(new_record);
	return *this;
}

module&	module::remove_node_commit(Record_info &op_info) {
	node* old_node = op_info.node_ref;
	vector<node*>::iterator ite;
	int index = 0;

	for (ite = vNodeLst.begin(); ite != vNodeLst.end(); ite++) {
		if ((*ite) == old_node) {
			break;
		}
		index++;
	}

	if (ite == vNodeLst.end())
		throw exception("node to remove commit not found. (remove_node_commit)");
	
	(*ite) = NULL;
	qRecycleBin.push(index);

	delete old_node;

	return *this;
}

module&	module::remove_node_rollback(Record_info &op_info) {
	node* tar_node = op_info.node_ref;
	mapNode[tar_node->get_node_name()] = tar_node;
	iNodeNum++;

	return *this;
}

node* module::_create_node(const string &node_name, Node_type node_type, cell* cell_ptr,
	module* module_ptr) {
	int cell_input_num = (node_type == TYPE_CELL) ? 
		cell_ptr->input_num : module_ptr->get_input_num();
	int	cell_output_num = (node_type == TYPE_CELL) ?
		1 : module_ptr->get_output_num();
	vector<net*> vi(cell_input_num, NULL);
	vector<net*> vo(cell_output_num, NULL);
	return new node(node_name, this, node_type, cell_ptr, module_ptr, vi, vo);
}

node*	module::_add_node(const string &node_name, const string &node_type, map<string, module*> &map_module, map<string, cell*> &map_cell) {
	node* new_node;
	map<string, module*>::iterator ite_m;
	map<string, cell*>::iterator ite_c;
	if ((ite_c = map_cell.find(node_type)) != map_cell.end()) {
		new_node = _create_node(node_name, TYPE_CELL, ite_c->second, NULL);
	}
	else if ((ite_m = map_module.find(node_type)) != map_module.end()) {
		new_node = _create_node(node_name, TYPE_MODULE, NULL, ite_m->second);
	}
	else {
		new_node = NULL;
	}
	return new_node;
}

module&	module::add_node(const string &node_name, const string &node_type) {
	map<string, module*> map_module;
	return add_node(node_name, node_type, map_module, pCellLib->map_cell);
}

module&	module::add_node(const string &node_name, const string &node_type, map<string, module*> &map_module, map<string, cell*> &map_cell) {
	node* new_node = _add_node(node_name, node_type, map_module, map_cell);
	if (new_node == NULL)
		throw exception(("adding node " + node_name +
			" with type " + node_type + " failed. (add_node)").c_str());

	lNodeLstUncommitted.push_back(new_node);
	iNodeNum++;
	mapNode[node_name] = new_node;

	Record new_record;
	new_record.op_type = AG;
	lRecordLst.push_back(new_record);

	return *this;
}

module&	module::add_node_commit(node* tar_node) {
	if (!qRecycleBin.empty()) {
		int index = qRecycleBin.front();
		qRecycleBin.pop();
		vNodeLst[index] = tar_node;
	}
	else {
		vNodeLst.push_back(tar_node);
	}
	return *this;
}

module&	module::add_node_rollback(node* tar_node) {
	mapNode.erase(tar_node->get_node_name());
	iNodeNum--;
	delete tar_node;
	return *this;
}

net*	module::_add_net(const string &net_name) {
	net* new_net = new net(net_name, this, false, false);
	return new_net;
}

module&	module::add_net(const string &net_name) {
	net* new_net = _add_net(net_name);
	if (new_net == NULL)
		throw exception(("adding net " + net_name + " failed. (add_net)").c_str());

	lNetLstUncommitted.push_back(new_net);
	iNetNum++;
	mapNet[net_name] = new_net;

	Record new_record;
	new_record.op_type = AW;
	lRecordLst.push_back(new_record);

	return *this;
}

module&	module::add_net_commit(net* tar_net) {
	vNetLst.push_back(tar_net);
	return *this;
}

module&	module::add_net_rollback(net* tar_net) {
	mapNet.erase(tar_net->get_net_name());
	iNetNum--;
	delete tar_net;
	return *this;
}

bool	module::_connect_wire(net* tar_net, node* tar_gate, const string pin_name) {
	if (tar_gate->get_node_type() == TYPE_CELL) {
		cell* cell_ref = tar_gate->get_cell_ref();
		if (cell_ref->output_pin == pin_name) {
			tar_gate->set_output_net(tar_net, 0);
			tar_net->set_fanin_node(tar_gate);
		}
		else {
			int pos = cell_ref->get_input_pos(pin_name);
			if (pos == -1)
				return false;
			tar_gate->set_input_net(tar_net, pos);
			tar_net->add_fanout_node(tar_gate);
		}
	}
	else if (tar_gate->get_node_type() == TYPE_MODULE) {
		module* module_ref = tar_gate->get_module_ref();
		int pos = module_ref->get_output_pos(pin_name);
		if (pos != -1) {
			tar_gate->set_output_net(tar_net, pos);
			tar_net->set_fanin_node(tar_gate);
		}
		else {
			pos = module_ref->get_input_pos(pin_name);
			if (pos == -1)
				return false;
			tar_gate->set_input_net(tar_net, pos);
			tar_net->add_fanout_node(tar_gate);
		}
	}
	return true;
}

module&	module::connect_wire(net* tar_net, node* tar_gate, const string pin_name) {
	if (tar_net == NULL || tar_gate == NULL)
		throw exception("NULL net or gate specified. (connect_wire)");
	if (!_connect_wire(tar_net, tar_gate, pin_name))
		throw exception(("connecting " + tar_net->get_net_name() + " to " +
			tar_gate->get_node_name() + " on " + pin_name + " failed. (connect_wire)").c_str());
	Record new_record;
	new_record.op_type = CW;
	new_record.op_info.net_ref = tar_net;
	new_record.op_info.node_ref = tar_gate;
	new_record.op_info.pin_name = pin_name;
	lRecordLst.push_back(new_record);
	return *this;
}

module&	module::connect_wire_commit(Record_info &op_info) {
	return *this;
}

module&	module::connect_wire_rollback(Record_info &op_info) {
	net* tar_net = op_info.net_ref;
	node* tar_gate = op_info.node_ref;
	string pin_name = op_info.pin_name;

	if (!_disconnect_wire(tar_net, tar_gate, pin_name))
		throw exception("rollback connection failed. (connect_wire_rollback)");
	return *this;
}

bool	module::_disconnect_wire(net* tar_net, node* tar_gate, const string pin_name) {
	if (tar_gate->get_node_type() == TYPE_CELL) {
		cell* cell_ref = tar_gate->get_cell_ref();
		if (cell_ref->output_pin == pin_name) {
			tar_gate->set_output_net(NULL, 0);
			tar_net->set_fanin_node(NULL);
		}
		else {
			int pos = cell_ref->get_input_pos(pin_name);
			if (pos == -1)
				return false;
			tar_gate->set_input_net(NULL, pos);
			tar_net->remove_fanout_node(tar_gate);
		}
	}
	else if (tar_gate->get_node_type() == TYPE_MODULE) {
		module* module_ref = tar_gate->get_module_ref();
		int pos = module_ref->get_output_pos(pin_name);
		if (pos != -1) {
			tar_gate->set_output_net(NULL, pos);
			tar_net->set_fanin_node(NULL);
		}
		else {
			pos = module_ref->get_input_pos(pin_name);
			if (pos == -1)
				return false;
			tar_gate->set_input_net(NULL, pos);
			tar_net->remove_fanout_node(tar_gate);
		}
	}
	return true;
}

module&	module::disconnect_wire(net* tar_net, node* tar_gate, const string pin_name) {
	if (tar_net == NULL || tar_gate == NULL)
		throw exception("NULL net or gate specified. (disconnect_wire)");
	if (!_disconnect_wire(tar_net, tar_gate, pin_name))
		throw exception(("disconnecting " + tar_net->get_net_name() + " from " +
			tar_gate->get_node_name() + " on " + pin_name + " failed. (disconnect_wire)").c_str());
	Record new_record;
	new_record.op_type = DW;
	new_record.op_info.net_ref = tar_net;
	new_record.op_info.node_ref = tar_gate;
	new_record.op_info.pin_name = pin_name;
	lRecordLst.push_back(new_record);
	return *this;
}

module&	module::disconnect_wire_commit(Record_info &op_info) {
	return *this;
}

module&	module::disconnect_wire_rollback(Record_info &op_info) {
	net* tar_net = op_info.net_ref;
	node* tar_gate = op_info.node_ref;
	string pin_name = op_info.pin_name;

	if (!_connect_wire(tar_net, tar_gate, pin_name))
		throw exception("rollback disconnection failed. (disconnect_wire_rollback)");
	return *this;
}

bool module::_set_input(net* tar_net, bool value) {
	if (value) {
		if (!tar_net->get_is_input()) {
			tar_net->set_is_input(true);
			vInputLst.push_back(tar_net);
			mapInputLst[tar_net->get_net_name()] = iInputNum++;

			return true;
		}
	}
	else {
		if (tar_net->get_is_input()) {
			tar_net->set_is_input(false);
			vector<net*>::iterator ite_target;
			bool flag = false;
			for (vector<net*>::iterator ite_net = vInputLst.begin(); ite_net != vInputLst.end(); ite_net++) {
				if ((*ite_net) == tar_net) {
					ite_target = ite_net;
					flag = true;
					break;
				}
			}
			if (flag) {
				vInputLst.erase(ite_target);

				map<string, int>::iterator ite_target_map;
				int pos = get_input_pos(tar_net->get_net_name());
				for (map<string, int>::iterator ite_map = mapInputLst.begin(); ite_map != mapInputLst.end(); ite_map++) {
					if ((*ite_map).second == pos) {
						ite_target_map = ite_map;
					}
					else if ((*ite_map).second > pos) {
						(*ite_map).second--;
					}
				}
				mapInputLst.erase(ite_target_map);
				iInputNum--;

				return true;
			}
		}
	}
	return false;
}

module& module::set_input(net* tar_net, bool value) {
	if (tar_net == NULL)
		throw exception("NULL net specified. (set_input)");
	if (!_set_input(tar_net, value))
		throw exception(("setting " + tar_net->get_net_name() + ((value)? "" : " not") + 
			" as input failed. (set_input)").c_str());
	
	Record new_record;
	new_record.op_type = SI;
	new_record.op_info.net_ref = tar_net;
	new_record.op_info.value_set = value;
	lRecordLst.push_back(new_record);
	return *this;
}

module&	module::set_input_commit(Record_info &op_info) {
	return *this;
}

module&	module::set_input_rollback(Record_info &op_info) {
	net* tar_net = op_info.net_ref;
	bool value = !op_info.value_set;

	if (!_set_input(tar_net, value))
		throw exception("rollback set input failed. (set_input_rollback)");
	return *this;
}

bool	module::_set_output(net* tar_net, bool value) {
	if (value) {
		if (!tar_net->get_is_output()) {
			tar_net->set_is_output(true);
			vOutputLst.push_back(tar_net);
			mapOutputLst[tar_net->get_net_name()] = iOutputNum++;

			return true;
		}
	}
	else {
		if (tar_net->get_is_output()) {
			tar_net->set_is_output(false);
			vector<net*>::iterator ite_target;
			bool flag = false;
			for (vector<net*>::iterator ite_net = vOutputLst.begin(); ite_net != vOutputLst.end(); ite_net++) {
				if ((*ite_net) == tar_net) {
					ite_target = ite_net;
					flag = true;
					break;
				}
			}
			if (flag) {
				vOutputLst.erase(ite_target);

				map<string, int>::iterator ite_target_map;
				int pos = get_output_pos(tar_net->get_net_name());
				for (map<string, int>::iterator ite_map = mapOutputLst.begin(); ite_map != mapOutputLst.end(); ite_map++) {
					if ((*ite_map).second == pos) {
						ite_target_map = ite_map;
					}
					else if ((*ite_map).second > pos) {
						(*ite_map).second--;
					}
				}
				mapOutputLst.erase(ite_target_map);
				iOutputNum--;

				return true;
			}
		}
	}
	return false;
}

module& module::set_output(net* tar_net, bool value) {
	if (tar_net == NULL)
		throw exception("NULL net specified. (set_output)");
	if (!_set_output(tar_net, value))
		throw exception(("setting " + tar_net->get_net_name() + ((value) ? "" : " not") +
			" as output failed. (set_output)").c_str());

	Record new_record;
	new_record.op_type = SO;
	new_record.op_info.net_ref = tar_net;
	new_record.op_info.value_set = value;
	lRecordLst.push_back(new_record);
	return *this;
}

module&	module::set_output_commit(Record_info &op_info) {
	return *this;
}

module&	module::set_output_rollback(Record_info &op_info) {
	net* tar_net = op_info.net_ref;
	bool value = !op_info.value_set;

	if (!_set_output(tar_net, value))
		throw exception("rollback set output failed. (set_output_rollback)");
	return *this;
}

module&	module::commit(int step_num) {
	if (step_num > (int)lRecordLst.size())
		throw exception(("commit steps " + 
			to_string(step_num) + " too big. (commit)").c_str());

	Record curr_record;
	node* curr_node;
	net* curr_net;
	while (step_num != 0) {
		curr_record = lRecordLst.front();
		lRecordLst.pop_front();
		step_num--;

		switch (curr_record.op_type) {
		case RG:
		{
			remove_node_commit(curr_record.op_info);
			break;
		}
		case AG:
		{
			curr_node = lNodeLstUncommitted.front();
			lNodeLstUncommitted.pop_front();
			add_node_commit(curr_node);
			break;
		}
		case AW:
		{
			curr_net = lNetLstUncommitted.front();
			lNetLstUncommitted.pop_front();
			add_net_commit(curr_net);
			break;
		}
		case CW:
		{
			connect_wire_commit(curr_record.op_info);
			break;
		}
		case DW:
		{
			disconnect_wire_commit(curr_record.op_info);
			break;
		}
		case SI:
		{
			set_input_commit(curr_record.op_info);
			break;
		}
		case SO:
		{
			set_output_commit(curr_record.op_info);
			break;
		}
		default:
			break;
		}
	}
	return *this;
}

module&	module::commit() {
	commit((int)lRecordLst.size());
	return *this;
}

module&	module::rollback(int step_num) {
	if (step_num > (int)lRecordLst.size())
		throw exception(("rollback steps " +
			to_string(step_num) + " too big. (rollback)").c_str());

	Record curr_record;
	node* curr_node;
	net* curr_net;
	while (step_num != 0) {
		curr_record = lRecordLst.back();
		lRecordLst.pop_back();
		step_num--;

		switch (curr_record.op_type) {
		case RG:
		{
			remove_node_rollback(curr_record.op_info);
			break;
		}
		case AG:
		{
			curr_node = lNodeLstUncommitted.back();
			lNodeLstUncommitted.pop_back();
			add_node_rollback(curr_node);
			break;
		}
		case AW:
		{
			curr_net = lNetLstUncommitted.back();
			lNetLstUncommitted.pop_back();
			add_net_rollback(curr_net);
			break;
		}
		case CW:
		{
			connect_wire_rollback(curr_record.op_info);
			break;
		}
		case DW:
		{
			disconnect_wire_rollback(curr_record.op_info);
			break;
		}
		case SI:
		{
			set_input_rollback(curr_record.op_info);
			break;
		}
		case SO:
		{
			set_output_rollback(curr_record.op_info);
			break;
		}
		default:
			break;
		}
	}
	return *this;
}

module&	module::rollback() {
	rollback((int)lRecordLst.size());
	return *this;
}

int	module::get_input_pos(const string pin_name) const {
	map<string, int>::const_iterator ite = mapInputLst.find(pin_name);
	if (ite != mapInputLst.end()) {
		return ite->second;
	}
	return -1;
}

int module::get_output_pos(const string pin_name) const {
	map<string, int>::const_iterator ite = mapOutputLst.find(pin_name);
	if (ite != mapOutputLst.end()) {
		return ite->second;
	}
	return -1;
}

bool module::read_module(string buff, map<string, module*> &map_module, map<string, cell*> &map_cell) {
	try {
		string buff_s = buff;
		string input_line, output_line, wire_line, instance_line;
		std::regex e;
		std::smatch sm;
		std::regex_iterator<std::string::iterator> rit;
		std::regex_iterator<std::string::iterator> rend;

		e = "module ([\\\\\\._a-zA-Z0-9]+)";
		std::regex_search(buff, sm, e);
		sModuleName = sm[1];
		cout << "module_name: " << sModuleName << endl;

		e = "input\\s+([^;]+;)";
		string buff_input = buff;
		while (std::regex_search(buff_input, sm, e)) {
			input_line = sm[1];
			std::regex e_input_line("([\\\\\\w]+)");
			rit = std::regex_iterator<std::string::iterator>(input_line.begin(), input_line.end(), e_input_line);
			while (rit != rend) {
				add_net(rit->str());
				set_input(get_net(rit->str()), true);
				rit++;
			}
			buff_input = sm.suffix().str();
		}

		e = "output\\s+([^;]+;)";
		string buff_output = buff;
		while (std::regex_search(buff_output, sm, e)) {
			output_line = sm[1];
			std::regex e_output_line("([\\\\\\w]+)");
			rit = std::regex_iterator<std::string::iterator>(output_line.begin(), output_line.end(), e_output_line);
			while (rit != rend) {
				add_net(rit->str());
				set_output(get_net(rit->str()), true);
				rit++;
			}
			buff_output = sm.suffix().str();
		}

		e = "wire\\s+([^;]+;)";
		string buff_wire = buff;
		while (std::regex_search(buff_wire, sm, e)) {
			wire_line = sm[1];
			std::regex e_wire_line("([\\\\\\w]+)");
			rit = std::regex_iterator<std::string::iterator>(wire_line.begin(), wire_line.end(), e_wire_line);
			while (rit != rend) {
				add_net(rit->str());
				rit++;
			}
			buff_wire = sm.suffix().str();
		}

		e = "([\\\\\\.\\w]+)\\s+([\\\\\\.\\w]+)\\s*\\(.*?\\);";
		rit = std::regex_iterator<std::string::iterator>(buff_s.begin(), buff_s.end(), e);
		while (rit != rend) {
			instance_line = rit->str();
			std::regex e_instance_line("([\\\\\\.\\w]+)\\s+([\\\\\\.\\w]+)\\s*\\((.*?)\\);");
			if (std::regex_match(instance_line, sm, e_instance_line)) {
				string gate_type = sm[1];
				string instance_name = sm[2];
				string port_info = sm[3];

				if (gate_type != "module") {
					node* new_node =
						add_node(instance_name, gate_type, map_module, map_cell).get_node(instance_name);

					vector<string> pin_list;
					vector<string> wire_list;
					std::regex e_port_info("\\.([\\\\\\[\\]\\w]+)\\(\\s*([\\\\\\[\\]\\w]+)\\s*\\)");
					std::regex_iterator<std::string::iterator> it_port_info(port_info.begin(), port_info.end(), e_port_info);
					while (it_port_info != rend) {
						string single_port = it_port_info->str();
						smatch sm_single_port;
						if (std::regex_match(single_port, sm_single_port, e_port_info)) {
							connect_wire(get_net(sm_single_port[2]), new_node, sm_single_port[1]);
						}
						it_port_info++;
					}
				}
			}
			rit++;
		}
		commit();
	}
	catch (std::exception &e) {
		cerr << "reading module faild. (read_module)\n";
		cerr << "description: " << e.what();
		destroy();
		return false;
	}
	return true;
}

string	module::_generate_wire_list(vector<net*> v, bool &first_item, string &delimiter, 
	bool exclude_input = false, bool exclude_output = false) {
	string res;
	for (vector<net*>::const_iterator ite = v.cbegin(); ite != v.cend(); ite++) {
		if (exclude_input && (*ite)->get_is_input())
			continue;
		if (exclude_output && (*ite)->get_is_output())
			continue;
		if (!first_item) {
			res = res + delimiter;
		}
		else {
			first_item = false;
		}
		res += (*ite)->get_net_name();
	}
	return res;
}

string	module::_generate_instance(const node &tar_node) {
	string res;
	bool first_item;
	int index_pin;
	if (tar_node.get_node_type() == TYPE_MODULE) {
		module* module_ref = tar_node.get_module_ref();
		res = module_ref->get_module_name() + " " + tar_node.get_node_name() + " (";
		first_item = true;
		for (index_pin = 0; index_pin < tar_node.get_input_num(); index_pin++) {
			if (!first_item) {
				res = res + ", ";
			}
			else {
				first_item = false;
			}
			res = res + "." + module_ref->get_input_net(index_pin)->get_net_name() + 
				"(" + tar_node.get_input_net(index_pin)->get_net_name() + ")";
		}
		for (index_pin = 0; index_pin < tar_node.get_output_num(); index_pin++) {
			if (!first_item) {
				res = res + ", ";
			}
			else {
				first_item = false;
			}
			res = res + "." + module_ref->get_output_net(index_pin)->get_net_name() + 
				"(" + tar_node.get_output_net(index_pin)->get_net_name() + ")";
		}
	}
	else {
		cell* cell_ref = tar_node.get_cell_ref();
		res = cell_ref->cell_name + " " + tar_node.get_node_name() + " (";
		first_item = true;
		for (index_pin = 0; index_pin < tar_node.get_input_num(); index_pin++) {
			if (!first_item) {
				res = res + ", ";
			}
			else {
				first_item = false;
			}
			res = res + "." + cell_ref->input_pinlist[index_pin] + "(" + tar_node.get_input_net(index_pin)->get_net_name() + ")";
		}
		if (!first_item) {
			res = res + ", ";
		}
		else {
			first_item = false;
		}
		res = res + "." + cell_ref->output_pin + "(" + tar_node.get_output_net(0)->get_net_name() + ")";
	}
	res = res + " );\n";
	return res;
}

string	module::write_module() {
	string buff;
	string delimiter = ", ";
	bool first_item = true;
	string input_list = _generate_wire_list(vInputLst, first_item, delimiter);
	first_item = true;
	string output_list = _generate_wire_list(vOutputLst, first_item, delimiter);
	
	buff = buff + "module " + sModuleName + " (";
	buff = buff + input_list;
	buff = buff + ((input_list.empty()) ? "" : delimiter);
	buff = buff + output_list;
	buff = buff + ");\n";

	//write the input declaration line
	buff = buff + "input ";
	buff = buff + input_list;
	buff = buff + ";\n";

	//write the output declaration line
	buff = buff + "output ";
	buff = buff + output_list;
	buff = buff + ";\n";

	//write the wire declaration line
	buff = buff + "wire ";
	first_item = true;
	buff = buff + _generate_wire_list(vNetLst, first_item, delimiter, true, true);
	buff = buff + ";\n\n";

	for (map<string, node*>::const_iterator nit = mapNode.cbegin(); nit != mapNode.cend(); nit++) {
		buff = buff + _generate_instance(*(*nit).second);
	}
	buff = buff + "endmodule\n";
	return buff;
}

module&	module::reset_node_ifunc() {
	for (vector<node*>::iterator ite_node = vNodeLst.begin(); ite_node != vNodeLst.end(); ite_node++) {
		(*ite_node)->set_ifunc(0);
	}
	for (list<node*>::iterator ite_node_uncommitted = lNodeLstUncommitted.begin(); ite_node_uncommitted != lNodeLstUncommitted.end(); ite_node_uncommitted++) {
		(*ite_node_uncommitted)->set_ifunc(0);
	}
	return *this;
}

module&	module::reset_net_fmark() {
	for (vector<net*>::iterator ite_net = vNetLst.begin(); ite_net != vNetLst.end(); ite_net++) {
		(*ite_net)->set_mark(false);
	}
	for (list<net*>::iterator ite_net_uncommitted = lNetLstUncommitted.begin(); ite_net_uncommitted != lNetLstUncommitted.end(); ite_net_uncommitted++) {
		(*ite_net_uncommitted)->set_mark(false);
	}
	return *this;
}

bool	module::check_module() {
	vector<string> verified_net_vector;
	int iDanglingNet;

	if (!get_topological_sequence(verified_net_vector))
		return false;
	iDanglingNet = 0;
	for (vector<net*>::const_iterator ite = vNetLst.cbegin(); ite != vNetLst.cend(); ite++)
		if ((*ite)->get_fanin_node() == NULL && (*ite)->get_is_input() == false)
			iDanglingNet++;
	if (verified_net_vector.size() != iNetNum - iDanglingNet)
		return false;
	return true;
}

bool	module::get_topological_sequence(vector<string> &v) {
	int i, j;
	int current_net_fanout_num;
	int node_output_num;
	net* temp_net;
	net* current_net;
	node* current_fanout_node;
	queue<net*> net_queue;
	vector<net*>::iterator net_ite;

	reset_node_ifunc();
	reset_net_fmark();
	for (net_ite = vInputLst.begin(); net_ite != vInputLst.end(); net_ite++) {
		net_queue.push(*net_ite);
		(*net_ite)->set_mark(true);
	}
	while (!net_queue.empty()) {
		current_net = net_queue.front();
		net_queue.pop();
		v.push_back(current_net->get_net_name());

		current_net_fanout_num = current_net->get_fanout_num();
		for (i = 0; i < current_net_fanout_num; i++) {
			current_fanout_node = current_net->get_fanout_nodes(i);
			current_fanout_node->set_ifunc(current_fanout_node->get_ifunc() + 1);

			if (current_fanout_node->get_ifunc() != current_fanout_node->get_input_num())
				continue;

			node_output_num = current_fanout_node->get_output_num();
			for (j = 0; j < node_output_num; j++) {
				temp_net = current_fanout_node->get_output_net(j);
				if (temp_net->get_mark()) {
					v.clear();
					return false;
				}
				net_queue.push(temp_net);
				temp_net->set_mark(true);
			}
		}
	}
	return true;
}