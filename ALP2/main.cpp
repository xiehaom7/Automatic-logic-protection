#include "cell.h"
#include "design.h"
#include "simulation.h"
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
	assert(pFile.is_open());
	ss << pFile.rdbuf();
	design d(&cl);
	d.parse_design_file(ss);
	simulation sim;
	sim.construct(d.get_top_module());

	return 0;
}