#include "net.h"

int net::iNetCounter = 0;

net::net(const string new_sNetName, module* upper_module_ptr, const bool new_fInput, const bool new_fOutput, const bool new_fMark, const int new_iFunc)
	:sNetName(new_sNetName)
	, pUpperModule(upper_module_ptr)
	, fInput(new_fInput)
	, fOutput(new_fOutput)
	, fMark(new_fMark)
	, iFunc(new_iFunc)
{
	pFanin = NULL;
	iFanoutNum = 0;
	id = iNetCounter++;
}

net::net(const net &copyNet) {
	sNetName = copyNet.sNetName;
	pUpperModule = NULL;
	pFanin = NULL;
	vFanouts.clear();
	iFanoutNum = 0;
	fInput = copyNet.fInput;
	fOutput = copyNet.fOutput;
	fMark = copyNet.fMark;
	iFunc = copyNet.iFunc;
	id = iNetCounter++;
}

net& net::operator=(const net &copyNet) {
	if (this == &copyNet) {
		return *this;
	}

	vFanouts.clear();
	pFanin = NULL;
	iFanoutNum = 0;
	pUpperModule = NULL;

	sNetName = copyNet.sNetName;
	fInput = copyNet.fInput;
	fOutput = copyNet.fOutput;
	fMark = copyNet.fMark;
	iFunc = copyNet.iFunc;

	return *this;
}

net::~net() {

}

net& net::remove_fanout_node(node* const p_node) {
	assert(p_node != NULL);
	for (vector<node*>::iterator ite = vFanouts.begin(); ite != vFanouts.end(); ite++) {
		if ((*ite) == p_node) {
			vFanouts.erase(ite);
			iFanoutNum--;
			return *this;
		}
	}
	return *this;
}
