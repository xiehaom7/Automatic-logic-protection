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

void signature::generate_signature(bool random) {
	Wire_value* value = NULL;
	bitset<MAX_PARALLEL_NUM>* res = NULL;
	size_t index;
	int i;

	sim->run_golden_simulation(SIGNATURE_SIZE, random);

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

void signature::count_controllability() {
	size_t index;
	for (index = 0; index < node_num; index++) {
		vSigNodeList[index]->uTest1 = vSigNodeList[index]->vSig.count();
		vSigNodeList[index]->uTest0 = SIGNATURE_SIZE - vSigNodeList[index]->uTest1;
	}
	return;
}

void signature::mark_fanin_nodes(const vector<int> &tar_node_list, vector<int> &fanin_node_list) {
	stack<int> node_stack;
	int current;
	int current_fanin;
	size_t i;
	vector<int>::const_iterator c_ite;
	vector<bool> fanin_flag_list(node_num, false);

	for (c_ite = tar_node_list.cbegin(); c_ite != tar_node_list.cend(); c_ite++) {
		node_stack.push(*c_ite);
		fanin_flag_list[*c_ite] = true;
	}

	while (!node_stack.empty()) {
		current = node_stack.top();
		node_stack.pop();
		fanin_node_list.push_back(current);

		for (i = 0; i < sim->vSimNodeLst[current]->vFanin.size(); i++) {
			current_fanin = sim->vSimNodeLst[current]->vFanin[i];
			if (fanin_flag_list[current_fanin])
				continue;
			node_stack.push(current_fanin);
			fanin_flag_list[current_fanin] = true;
		}
	}
	return;
}

void signature::analyse_observability(const vector<int> &tar_node_list, const vector<int> &exclude_node_list) {
	if (tar_node_list.empty())
		throw exception("target node list empty. (analyse_observability)\n");
	vector<int>::const_iterator c_ite;
	vector<int> fanin_node_list;
	vector<bool> exclude_flag_list(node_num, false);
	stack<int> node_stack;
	int current;
	SimNode* current_node;
	vector<bitset<MAX_CELL_INPUTS_X2>>::const_iterator ob_ite;
	bitset<SIGNATURE_SIZE> observability_main;
	bitset<SIGNATURE_SIZE> observability_branch;
	int current_fanin;
	int fanin_size;
	size_t i, j;

	mark_fanin_nodes(tar_node_list, fanin_node_list);

	for (c_ite = tar_node_list.cbegin(); c_ite != tar_node_list.cend(); c_ite++) {
		vSigNodeList[*c_ite]->vODCmask.set();
	}
	for (c_ite = exclude_node_list.cbegin(); c_ite != exclude_node_list.cend(); c_ite++) {
		vSigNodeList[*c_ite]->vODCmask.reset();
		exclude_flag_list[*c_ite] = true;
	}
	for (c_ite = fanin_node_list.cbegin(); c_ite != fanin_node_list.cend(); c_ite++) {
		current = *c_ite;
		current_node = sim->vSimNodeLst[current];
		fanin_size = current_node->vFanin.size();
		for (i = 0; i < fanin_size; i++) {
			current_fanin = current_node->vFanin[i];
			if (exclude_flag_list[current_fanin])
				continue;
			observability_main.reset();
			for (ob_ite = current_node->pCell->co_array[i].cbegin();
			ob_ite != current_node->pCell->co_array[i].cend(); ob_ite++) {
				observability_branch.set();
				for (j = 0; j < fanin_size; j++) {
					if ((*ob_ite).test(j) && !(*ob_ite).test(j + fanin_size))
						observability_branch &= vSigNodeList[current_node->vFanin[j]]->vSig;
					else if (!(*ob_ite).test(j) && (*ob_ite).test(j + fanin_size))
						observability_branch &= 
						(bitset<SIGNATURE_SIZE>(vSigNodeList[current_node->vFanin[j]]->vSig)).flip();
				}
				observability_main |= observability_branch;
			}
			vSigNodeList[current_fanin]->vODCmask |= 
				observability_main & vSigNodeList[current]->vODCmask;
		}
	}
	return;
}

void signature::analyse_observability() {
	vector<int> exclude_node_list;

	analyse_observability(sim->vPrimaryOutputLst, exclude_node_list);
	return;
}

SignatureNode* signature::get_signature_node(string& node_name) {
	map<string, int>::iterator ite;
	if ((ite = mSigNode.find(node_name)) == mSigNode.end())
		return NULL;
	return vSigNodeList[(*ite).second];
}