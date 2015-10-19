#pragma once
////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Creator: Hao Xie
/// Create Date: 2014-10-24
/// Class name: wire
/// Tool versions: Visual Studio 2010
/// Description: define the wire type and the related operations
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <assert.h>

using namespace std;

#ifndef NET_H
#define NET_H

class node;
class module;

typedef enum { ZERO = 0, ONE, X } Wire_value;

class net {
private:
	string			sNetName;				//the net name
	module*			pUpperModule;
	vector<node*>	vFanouts;				//the array of fanout gates
	node*			pFanin;					//the fanin gate
	int				id;						//net id
	int				iFanoutNum;				//number of fanouts
	bool			fInput;					//marks the net is a input
	bool			fOutput;				//marks the net is an output
	bool			fMark;					//multipurpose mark
	int				iFunc;					//multipurpose int

private:
	static int		iNetCounter;

public:
	net(const string new_sNetName, module* upper_module_ptr, const bool new_fInput, const bool new_fOutput, const bool new_fMark = false, const int new_iDebug = 0);
	net(const net &copyNet);
	net& operator=(const net &copyNet);
	~net();

public:
	string			get_net_name()		const;
	module*			get_upper_module()	const;
	node*			get_fanin_node()	const;
	node*			get_fanout_nodes(int pos);
	bool			get_is_input()		const;
	bool			get_is_output()		const;
	bool			get_mark()			const;
	int				get_id()			const;
	int				get_ifunc()			const;
	int				get_fanout_num()	const;

public:
	bool			set_net_name(string new_sNetName);
	bool			set_upper_module(module* const p_module);
	bool			set_fanin_node(node* const p_node);
	bool			add_fanout_node(node* const p_node);
	bool			remove_fanout_node(node* const p_node);
	bool			set_is_input(bool new_fInput);
	bool			set_is_output(bool new_fOutput);
	bool			set_mark(bool new_fMark);
	bool			set_ifunc(int new_value);
};

inline	string			net::get_net_name()		const { return sNetName; }
inline	module*			net::get_upper_module()	const { return pUpperModule; }
inline	node*			net::get_fanin_node()	const { return pFanin; }
inline	node*			net::get_fanout_nodes(int pos) { assert(pos < iFanoutNum); return vFanouts[pos]; }
inline	bool			net::get_is_input()		const { return fInput; }
inline	bool			net::get_is_output()	const { return fOutput; }
inline	bool			net::get_mark()			const { return fMark; }
inline	int				net::get_id()			const { return id; }
inline	int				net::get_ifunc()		const { return iFunc; }
inline	int				net::get_fanout_num()	const { return iFanoutNum; }

inline	bool			net::set_net_name(string new_sNetName) { sNetName = new_sNetName; return true; }
inline	bool			net::set_upper_module(module* const p_module) { pUpperModule = p_module; return true; }
inline	bool			net::set_fanin_node(node* const p_node) { pFanin = p_node; return true; }
inline	bool			net::add_fanout_node(node* const p_node) { assert(p_node != NULL); vFanouts.push_back(p_node); iFanoutNum++; return true; }
inline	bool			net::set_is_input(bool new_fInput) { fInput = new_fInput; return true; }
inline	bool			net::set_is_output(bool new_fOutput) { fOutput = new_fOutput; return true; }
inline	bool			net::set_mark(bool new_fMark) { fMark = new_fMark; return true; }
inline	bool			net::set_ifunc(int new_value) { iFunc = new_value; return true; }

#endif