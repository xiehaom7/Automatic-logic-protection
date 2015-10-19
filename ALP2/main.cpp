#include "cell.h"

using namespace std;

int main() {
	cell_library cl("test_lib");
	cl.read_cc_file("cell_info_cc.txt");
	cl.read_co_file("cell_info_co.txt");
	cl.read_rw_operation("cell_rw_op_new.txt");
	return 0;
}