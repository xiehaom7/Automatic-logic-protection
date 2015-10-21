#include "design.h"

design::design() {
	pCellLib = NULL;
	pTopModule = NULL;
}

design::design(cell_library* cl) {
	pCellLib = cl;
	pTopModule = NULL;
}

void design::destroy() {
	vector<module*>::iterator ite_module;
	for (ite_module = vModuleLst.begin(); ite_module != vModuleLst.end(); ite_module++) {
		mapModuleLst.erase((*ite_module)->get_module_name());
		delete (*ite_module);
		(*ite_module) = NULL;
	}
}

design::~design() {
	destroy();
}

bool design::parse_design_file(stringstream &ss) {
	string buff = ss.str(), module_buff;
	std::regex e;
	std::smatch sm;

	e = "(module\\s+([\\w\\s\\.\\\\(),;\\[\\]]*?)endmodule)";
	while (std::regex_search(buff, sm, e)) {
		module_buff = sm[1];
		module* new_module = new module(pCellLib);
		if (!new_module->read_module(module_buff, mapModuleLst, pCellLib->map_cell)) {
			delete new_module;
			destroy();
			cerr << "\nreading module faild. abort reading design (parse_design_file)\n";
			return false;
		}
		vModuleLst.push_back(new_module);
		mapModuleLst[new_module->get_module_name()] = new_module;
		pTopModule = new_module;
		buff = sm.suffix().str();
	}
	return true;
}

string	design::output_design_file() {
	string res;
	vector<module*>::iterator ite_module;
	for (ite_module = vModuleLst.begin(); ite_module != vModuleLst.end(); ite_module++) {
		res += (*ite_module)->write_module();
		res += "\n";
	}
	return res;
}

bool design::read_design_file(const string file_name) {
	ifstream pFile(file_name.c_str());
	stringstream ss;

	if (!pFile.is_open()) {
		cerr << "ERROR: failed to open file " + file_name + "\n";
		return false;
	}

	ss << pFile.rdbuf();
	pFile.close();
	return parse_design_file(ss);
}

module*	design::get_module(const string &module_name) const {
	map<string, module*>::const_iterator ite;
	ite = mapModuleLst.find(module_name);
	return (ite == mapModuleLst.cend()) ? NULL : ite->second;
}

design&	design::set_top_module(const string &module_name) {
	module* tar_module = get_module(module_name);
	if (tar_module == NULL)
		throw exception((module_name + " not found. (set_top_module)").c_str());
	pTopModule = tar_module;
	return *this;
}