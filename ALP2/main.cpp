#include "cell.h"
#include "module.h"
#include <fstream>
#include <map>


using namespace std;

int main() {
	cell_library cl("test_lib");
	cl.read_cc_file("cell_info_cc.txt");
	cl.read_co_file("cell_info_co.txt");
	cl.read_rw_operation("cell_rw_op_new.txt");

	ifstream pFile("D:\\Dropbox\\Public\\Programs\\C\\ALP\\ALP\\cmb.v");
	stringstream ss;
	map<string, module*> mm;
	assert(pFile.is_open());
	ss << pFile.rdbuf();
	module m(&cl);
	m.read_module(ss.str(), mm, cl.map_cell);
	cout << m.check_module();

	return 0;
}