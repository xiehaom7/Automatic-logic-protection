#include "signature.h"

SignatureNode* signature::_create_sig_node(string name) {
	SignatureNode* sn = new SignatureNode;
	sn->sName = name;
	sn->uTest0 = 0;
	sn->uTest1 = 0;
	return sn;
}

void signature::destroy() {
	vector<SignatureNode*>::iterator ite;
	for (ite = vSigNodeList.begin(); ite != vSigNodeList.end(); ite++)
		delete *ite;
	vSigNodeList.clear();
	mSigNode.clear();
	sim = NULL;
	input_num = 0;
	node_num = 0;
	return;
}

bool signature::construct(simulation* tar_sim) {
	destroy();
	if (tar_sim == NULL)
		return false;

	size_t i;
	string node_name;
	sim = tar_sim;
	node_num = sim->vSimNodeLst.size();
	input_num = sim->get_primary_inputs_num();

	vSigNodeList.resize(node_num);
	for (i = 0; i < node_num; i++) {
		node_name = sim->vSimNodeLst[i]->sPrefix + "." + sim->vSimNodeLst[i]->sName;
		vSigNodeList[i] = _create_sig_node(node_name);
		mSigNode[node_name] = i;
	}
	return true;
}

signature::signature() {
	sim = NULL;
	input_num = 0;
	node_num = 0;
}


signature::~signature() {
	destroy();
}

void signature::generate_signature() {
	Wire_value* value = NULL;
	bitset<MAX_PARALLEL_NUM>* res = NULL;
	size_t index;
	int i;

	sim->run_golden_simulation(SIGNATURE_SIZE);

	for (index = 0; index < node_num; index++) {
		sim->get_node_value(index, &value, &res);
		if (*value == ONE)
			res->flip();
		for (i = 0; i < SIGNATURE_SIZE; i++)
			vSigNodeList[index]->vSig[i] = (*res)[i];
		if (*value == ONE)
			res->flip();
	}
}