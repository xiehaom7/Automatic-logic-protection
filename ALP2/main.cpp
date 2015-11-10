#include "cell.h"
#include "design.h"
#include "simulation_evaluation.h"
#include "signature.h"
#include "redundant_wire.h"
#include <fstream>
#include <map>


using namespace std;

int main() {
//	cell_library cl("test_lib");
//	cl.read_cc_file("cell_info_cc.txt");
//	cl.read_co_file("cell_info_co.txt");
//	cl.read_rw_operation("cell_rw_op_new.txt");
//
//	clock_t start, finish;
//	double totaltime;
//
//	/*string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
//		"input i_0, i_1, i_2;\n"
//		"output o_0, o_1;\n"
//		"wire w_0, w_1;\n\n"
//		"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
//		"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
//		"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
//		"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
//		"endmodule\n";
//	string  s_top_module = "module top_test (i_0, i_1, i_2, o_0, o_1);\n"
//		"input i_0, i_1, i_2;\n"
//		"output o_0, o_1;\n"
//		"wire w_0;\n\n"
//		"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(o_1) );\n"
//		"AND2_X1 U2 (.A1(w_0), .A2(o_1), .ZN(o_0) );\n"
//		"endmodule\n";
//	string s = s_module + "\n" + s_top_module + "\n";
//	stringstream ss(s);*/
//
//	ifstream pFile("D:\\Dropbox\\Public\\Programs\\C\\ALP\\ALP\\s386.v");
//	stringstream ss;
//	assert(pFile.is_open());
//	ss << pFile.rdbuf();
//	design d(&cl);
//	d.parse_design_file(ss);
//	/*simulation sim;
//	sim.construct(d.get_top_module());*/
//	
//	try {
//		start = clock();
//		/*simulation_evaluation sv;
//		sv.construct(&sim);
//		sv.run_exhaustive_fault_injection_simulation();
//		sv.evaluate_fault_injection_results();
//
//		signature sig;
//		sig.construct(&sim);
//		sig.generate_signature(false);
//		sig.count_controllability();
//		sig.analyse_observability();
//
//		for (size_t i = 0; i < sv.node_num; i++) {
//			cout << sv.vStatNodeList[i]->sName << "\t";
//			cout << (float)sv.vStatNodeList[i]->uLogicOne / (sv.vStatNodeList[i]->uLogicOne + sv.vStatNodeList[i]->uLogicZero) << "\t";
//			cout << (float)sig.vSigNodeList[i]->uTest1 / (sig.vSigNodeList[i]->uTest1 + sig.vSigNodeList[i]->uTest0) << endl;
//		}
//*/
//		redundant_wire rw;
//		rw.construct(d.get_top_module());
//
//		finish = clock();
//		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
//		cout << "\nRuntime : " << totaltime << "seconds" << endl;
//	}
//	catch (exception e)
//	{ cout << e.what(); }
//	cout << endl << "program completed! press any key to quit ..." << endl;
//	getchar();

	stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
		"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
		"#NAND2_X1\nA1 A2\nZN\n0010 0001\n1100\n"
		"#NOR2_X1\nA1 A2\nZN\n0011\n1000 0100\n"
		"#NOR3_X1\nA1 A2 A3\nZN\n000111\n100000 010000 001000\n"
		"#INV_X1\nA\nZN\n01\n10\n");
	cell_library*	cl;
	cl = new cell_library("test_lib");
	cl->parse_cc_file(ss);
	string  s_top_module = "module top_test (a, b, c, d, e, f, h, o);\n"
		"input a, b, c, d, e, f, h;\n"
		"output o;\n"
		"wire w1, w2, w3, w4, w5, w6, w7, w8, w9;\n\n"
		"OR2_X1 U1 (.A1(a), .A2(b), .ZN(w1));\n"
		"OR2_X1 U2 (.A1(b), .A2(c), .ZN(w2));\n"
		"OR2_X1 U3 (.A1(d), .A2(e), .ZN(w3));\n"
		"NAND2_X1 U4 (.A1(e), .A2(f), .ZN(w4));\n"
		"INV_X1 U5(.A(h), .ZN(w5));\n"
		"AND2_X1 U6 (.A1(w1), .A2(w2), .ZN(w6));\n"
		"AND2_X1 U7 (.A1(w4), .A2(w5), .ZN(w7));\n"
		"NOR3_X1 U8 (.A1(w6), .A2(w3), .A3(w7) .ZN(w8));\n"
		"OR2_X1 U9 (.A1(w7), .A2(h), .ZN(w9));\n"
		"AND2_X1 U10 (.A1(w8), .A2(w9), .ZN(o));\n"
		"endmodule\n";
	string s = s_top_module;
	design d(cl);
	stringstream ss_module(s);
	d.parse_design_file(ss_module);

	simulation sim;
	sim.construct(d.get_top_module());
	signature sig;
	sig.construct(&sim);
	sig.generate_signature(false);

	redundant_wire rw;
	rw.construct(d.get_top_module());
	rw.implication_matrix_generator();
	vector<list<Implication_comb>> vector_implication = rw.get_implication_list();

	return 0;
}