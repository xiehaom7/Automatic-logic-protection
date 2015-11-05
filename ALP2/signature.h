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
private:
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
	void					generate_signature();
};

