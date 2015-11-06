#pragma once
#include "simulation.h"
#include "config.h"

typedef struct {
	string					sName;
	bitset<SIGNATURE_SIZE>	vSig;
	bitset<SIGNATURE_SIZE>	vODCmask;
	unsigned				uTest0;
	unsigned				uTest1;
}SignatureNode;

class signature
{
//TODO: private
public:
	vector<SignatureNode*>	vSigNodeList;
	map<string, int>		mSigNode;
	simulation*				sim;
	size_t					input_num;
	size_t					node_num;
private:
	SignatureNode*			_create_sig_node(string name);
public:
	signature();
	~signature();
public:
	void					destroy();
	bool					construct(simulation* tar_sim);
	void					generate_signature(bool random = true);
	void					count_controllability();
	void					analyse_observability(const vector<int> &tar_node_list, const vector<int> &exclude_node_list);
	void					analyse_observability();
	void					mark_fanin_nodes(const vector<int> &tar_node_list, vector<int> &fanin_node_list);
	SignatureNode*			get_signature_node(string& node_name);
};

