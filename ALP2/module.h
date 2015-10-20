#pragma once
#include <iostream>
#include <string>
#include <queue>
#include <list>
#include <stack>
#include <time.h>
#include <random>
#include <cmath>
#include <regex>

#include "cell.h"
#include "net.h"
#include "node.h"

using namespace std;

typedef struct {
	net*					net_ref;			//restore changed net's name in CW and DW operations
	node*					node_ref;			//restore changed gate's name in CW and DW operations
												//restore removed node RG operations
	string					pin_name;			//restore changed gate's pin name in CW and DW operations
	bool					value_set;			//restore value set by SI and SO operations
}Record_info;

typedef struct {
	Module_op				op_type;
	Record_info				op_info;
}Record;

typedef struct {
	unsigned				uLogicOne;
	unsigned				uLogicZero;
	unsigned				uInjection;
	unsigned				uPropagation;
	unsigned				uSimulation;
	unsigned				uAffection;
}StatNode;

class module {
private:
	string					sModuleName;
	cell_library*			pCellLib;
	vector<node*>			vNodeLst;
	vector<net*>			vNetLst;
	vector<net*>			vInputLst;
	vector<net*>			vOutputLst;
	int						iInputNum;
	int						iOutputNum;
	int						iNetNum;
	int						iNodeNum;
	map<string, node*>		mapNode;
	map<string, net*>		mapNet;
	map<string, int>		mapInputLst;
	map<string, int>		mapOutputLst;
	int						iFunc;

	list<node*>				lNodeLstUncommitted;
	list<net*>				lNetLstUncommitted;
	list<Record>			lRecordLst;

	queue<int>				qRecycleBin;


public:
	module(cell_library*	cl);
	~module();
	void		destroy();

private:
	bool		_remove_node(node* tar_node);
	node*		_add_node(const string &node_name, const string &node_type, 
		map<string, module*> &map_module, map<string, cell*> &map_cell);
	node*		_create_node(const string &node_name, Node_type node_type, cell* cell_ptr,
		module* module_ptr);
	net*		_add_net(const string &net_name);
	bool		_set_input(net* tar_net, bool value);
	bool		_set_output(net* tar_net, bool value);
	bool		_connect_wire(net* tar_net, node* tar_gate, const string pin_name);
	bool		_disconnect_wire(net* tar_net, node* tar_gate, const string pin_name);

public:
	bool		read_module(string buff, map<string, module*> &map_module, map<string, cell*> &map_cell);
	string		write_module();

	module&		remove_node(node* tar_node);
	module&		add_node(const string &node_name, const string &node_type, map<string, module*> &map_module, map<string, cell*> &map_cell);
	module&		add_node(const string &node_name, const string &node_type);
	module&		add_net(const string &net_name);
	module&		connect_wire(net* tar_net, node* tar_gate, const string pin_name);
	module&		disconnect_wire(net* tar_net, node* tar_gate, const string pin_name);
	module&		set_input(net* tar_net, bool value);
	module&		set_output(net* tar_wire, bool value);

	module&		remove_node_commit(Record_info &op_info);
	module&		remove_node_rollback(Record_info &op_info);
	module&		add_node_commit(node* tar_node);
	module&		add_node_rollback(node* tar_node);
	module&		add_net_commit(net* tar_net);
	module&		add_net_rollback(net* tar_net);
	module&		connect_wire_commit(Record_info &op_info);
	module&		connect_wire_rollback(Record_info &op_info);
	module&		disconnect_wire_commit(Record_info &op_info);
	module&		disconnect_wire_rollback(Record_info &op_info);
	module&		set_input_commit(Record_info &op_info);
	module&		set_input_rollback(Record_info &op_info);
	module&		set_output_commit(Record_info &op_info);
	module&		set_output_rollback(Record_info &op_info);

	module&		set_ifunc(int new_value);
	module&		reset_node_ifunc();
	module&		reset_net_fmark();
	bool		check_module();

	module&		commit();
	module&		commit(int step_num);
	module&		rollback();
	module&		rollback(int step_num);

public:
	string		get_module_name() const;
	net*		get_net(string net_name) const;
	net*		get_input_net(int pos) const;
	net*		get_output_net(int pos) const;
	node*		get_node(string node_name) const;
	int			get_net_num() const;
	int			get_node_num() const;
	int			get_input_num() const;
	int			get_output_num() const;
	int			get_input_pos(string pin_name) const;
	int			get_output_pos(string pin_name) const;
	int			get_ifunc() const;

private:
	string		_generate_wire_list(vector<net*> v, bool &first_item, string &delimiter, 
		bool exclude_input, bool exclude_output);
	string		_generate_instance(const node &tar_node);
};

inline	string	module::get_module_name() const { return sModuleName; }
inline	net*	module::get_input_net(int pos) const { assert((pos >= 0) && (pos < iInputNum)); return vInputLst[pos]; }
inline	net*	module::get_output_net(int pos) const { assert((pos >= 0) && (pos < iOutputNum)); return vOutputLst[pos]; }
inline	int		module::get_net_num() const { return iNetNum; }
inline	int		module::get_node_num() const { return iNodeNum; }
inline	int		module::get_input_num() const { return iInputNum; }
inline	int		module::get_output_num() const { return iOutputNum; }
inline	int		module::get_ifunc() const { return iFunc; }

inline	module&	module::set_ifunc(int new_value) { iFunc = new_value; return *this; }
