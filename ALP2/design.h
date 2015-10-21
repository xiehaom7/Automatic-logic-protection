#pragma once
#include <vector>
#include <sstream>
#include "module.h"

class design
{
private:
	module*						pTopModule;
	cell_library*				pCellLib;
	vector<module*>				vModuleLst;
	map<string, module*>		mapModuleLst;

public:
	design();
	design(cell_library* cl);
	~design();

public:
	bool						read_design_file(const string file_name);
	string						output_design_file();
	bool						parse_design_file(stringstream &ss);
	void						destroy();

public:
	module*						get_module(const string &module_name) const;
	module*						get_top_module() const;
	design&						set_top_module(const string &module_name);
};

inline	module*				design::get_top_module() const { return pTopModule; }
