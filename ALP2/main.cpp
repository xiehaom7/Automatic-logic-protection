#include "cell.h"
#include "design.h"
#include "simulation_evaluation.h"
#include "signature.h"
#include "redundant_wire.h"
#include <fstream>
#include <map>


using namespace std;

int main() {
	cell_library cl("test_lib");
	cl.read_cc_file("cell_info_cc.txt");
	cl.read_co_file("cell_info_co.txt");
	cl.read_rw_operation("cell_rw_op_new.txt");

	clock_t start, finish;
	double totaltime;

	string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
		"input i_0, i_1, i_2;\n"
		"output o_0, o_1;\n"
		"wire w_0, w_1;\n\n"
		"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
		"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
		"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
		"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
		"endmodule\n";
	string  s_top_module = "module top_test (i_0, i_1, i_2, o_0, o_1);\n"
		"input i_0, i_1, i_2;\n"
		"output o_0, o_1;\n"
		"wire w_0;\n\n"
		"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(o_1) );\n"
		"AND2_X1 U2 (.A1(w_0), .A2(o_1), .ZN(o_0) );\n"
		"endmodule\n";
	string s = s_module + "\n" + s_top_module + "\n";
	stringstream ss(s);

	///*ifstream pFile("D:\\Dropbox\\Public\\Programs\\C\\ALP\\ALP\\s386.v");
	//stringstream ss;
	//assert(pFile.is_open());
	//ss << pFile.rdbuf();*/
	design d(&cl);
	d.parse_design_file(ss);
	simulation sim;
	sim.construct(d.get_top_module());
	
	try {
		start = clock();
		/*simulation_evaluation sv;
		sv.construct(&sim);
		sv.run_exhaustive_fault_injection_simulation();
		sv.evaluate_fault_injection_results();

		signature sig;
		sig.construct(&sim);
		sig.generate_signature(false);
		sig.count_controllability();
		sig.analyse_observability();

		for (size_t i = 0; i < sv.node_num; i++) {
			cout << sv.vStatNodeList[i]->sName << "\t";
			cout << (float)sv.vStatNodeList[i]->uLogicOne / (sv.vStatNodeList[i]->uLogicOne + sv.vStatNodeList[i]->uLogicZero) << "\t";
			cout << (float)sig.vSigNodeList[i]->uTest1 / (sig.vSigNodeList[i]->uTest1 + sig.vSigNodeList[i]->uTest0) << endl;
		}
*/
		redundant_wire rw;
		rw.construct(d.get_top_module());

		finish = clock();
		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "\nRuntime : " << totaltime << "seconds" << endl;
	}
	catch (exception e)
	{ cout << e.what(); }
	cout << endl << "program completed! press any key to quit ..." << endl;
	getchar();
	return 0;
}