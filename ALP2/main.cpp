#include "cell.h"
#include "design.h"
#include "simulation_evaluation.h"
#include <fstream>
#include <map>


using namespace std;

int main() {
	//cell_library cl("test_lib");
	//cl.read_cc_file("cell_info_cc.txt");
	//cl.read_co_file("cell_info_co.txt");
	//cl.read_rw_operation("cell_rw_op_new.txt");

	//ifstream pFile("D:\\Dropbox\\Public\\Programs\\C\\ALP\\ALP\\cmb.v");
	//stringstream ss;
	//assert(pFile.is_open());
	//ss << pFile.rdbuf();
	//design d(&cl);
	//d.parse_design_file(ss);
	//simulation_evaluation sv;
	//sv.construct(d.get_top_module());
	//try {
	//	sv.run_exhaustive_golden_simulation();
	//}		
	//catch (exception e)
	//{ cout << e.what(); }


	//stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
	//	"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
	//	"#AND3_X1\nA1 A2 A3\nZN");
	//cell_library*	cl;
	//cl = new cell_library("test_lib");
	//cl->parse_cc_file(ss);
	//string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
	//	"input i_0, i_1, i_2;\n"
	//	"output o_0, o_1;\n"
	//	"wire w_0, w_1;\n\n"
	//	"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
	//	"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
	//	"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
	//	"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
	//	"endmodule\n";
	//string  s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
	//	"input i_0, i_1, i_2;\n"
	//	"output o_0;\n"
	//	"wire w_0, w_1;\n\n"
	//	"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
	//	"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
	//	"endmodule\n";
	//string s = s_module + "\n" + s_top_module + "\n";
	//design d(cl);
	//stringstream ss_module(s);
	//d.parse_design_file(ss_module);
	//simulation_evaluation sv;
	//sv.construct(d.get_top_module());
	////sv.run_exhaustive_golden_simulation();
	//
	//sv.run_exhaustive_fault_injection_simulation();

	return 0;
}