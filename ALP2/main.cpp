#include "cell.h"
#include "design.h"
#include "simulation_evaluation.h"
#include "signature.h"
#include "redundant_wire.h"
#include <fstream>
#include <map>

//"D:\\Dropbox\\Public\\Programs\\C\\results\\test\\test"
//"D:\\Dropbox\\Public\\Programs\\C\\results\\s1488_simple\\s1488_simple"
//string sao2 = "D:\\Dropbox\\Public\\Programs\\C\\results\\sao2_simple\\sao2_simple";
//string s298_original = "D:\\Dropbox\\Public\\Programs\\C\\results\\s298_original\\s298";
//"D:\\Dropbox\\Public\\Programs\\C\\results\\s1196_original\\s1196"
//"D:\\Dropbox\\Public\\Programs\\C\\results\\s641_original\\s641"
//"D:\\Dropbox\\Public\\Programs\\C\\results\\s713_original\\s713"


using namespace std;

#define GEN

#ifdef GEN
int main(int argc, char *argv[]) {
	/*string target(argv[1]);
	int num_rw = atoi(argv[2]);
	int sample_freq = atoi(argv[3]);
	int sim_num = atoi(argv[4]);
	int fault_num = atoi(argv[5]);

	srand(time(NULL));
	cell_library cl("test_lib");
	cl.read_cc_file("cell_info_cc.txt");
	cl.read_co_file("cell_info_co.txt");
	cl.read_rw_operation("cell_rw_op_new.txt");

	clock_t start, finish;
	double totaltime;

	ifstream pFile(target + ".v");
	stringstream ss;
	assert(pFile.is_open());
	ss << pFile.rdbuf();

	design d(&cl);
	d.parse_design_file(ss);

	try {
		start = clock();

		redundant_wire rw;
		rw.construct(d.get_top_module());
		rw.implication_matrix_generator_full();
		rw.redundant_wire_selector(target, num_rw, sample_freq, sim_num, fault_num);

		finish = clock();
		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "\nRuntime : " << totaltime << "seconds" << endl;
	}
	catch (exception e)
	{
		cout << e.what();
	}
	cout << endl << "program completed! press any key to quit ..." << endl;
	getchar();
	return 0;*/

	string target(argv[1]);
	/*string target = "D:\\Dropbox\\Public\\Programs\\C\\results\\s641_original\\s641";
	int method = 3;*/

	srand(time(NULL));
	cell_library cl("test_lib");
	cl.read_cc_file("cell_info_cc.txt");
	cl.read_co_file("cell_info_co.txt");
	cl.read_rw_operation("cell_rw_op_new.txt");

	clock_t start, finish;
	double totaltime;
	int total;

	ifstream pFile(target + ".v");
	stringstream ss;
	assert(pFile.is_open());
	ss << pFile.rdbuf();

	design d(&cl);
	d.parse_design_file(ss);

	try {
		redundant_wire rw_0, rw_1, rw_2, rw_3;
		rw_0.construct(d.get_top_module());
		rw_1.construct(d.get_top_module());
		rw_2.construct(d.get_top_module());
		rw_3.construct(d.get_top_module());

		start = clock();	
		total = rw_0.implication_counter(BACKWARD);
		cout << "implication number: " << total << endl;
		finish = clock();
		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "\nRuntime : " << totaltime << "seconds" << endl;

		start = clock();
		total = rw_1.implication_counter(DIRECT);
		cout << "implication number: " << total << endl;
		finish = clock();
		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "\nRuntime : " << totaltime << "seconds" << endl;

		start = clock();
		total = rw_2.implication_counter(INDIRECT_LOW);
		cout << "implication number: " << total << endl;
		finish = clock();
		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "\nRuntime : " << totaltime << "seconds" << endl;

		start = clock();
		total = rw_3.implication_counter(INDIRECT_HIGH);
		cout << "implication number: " << total << endl;
		finish = clock();
		totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "\nRuntime : " << totaltime << "seconds" << endl;
	}
	catch (exception e)
	{
		cout << e.what();
	}
	cout << endl << "program completed! press any key to quit ..." << endl;
	getchar();
	return 0;
}
#else
int main() {
	string target = "D:\\Dropbox\\Public\\Programs\\C\\results\\s1423_original\\s1423";
	srand(time(NULL));
	cell_library cl("test_lib");
	cl.read_cc_file("cell_info_cc.txt");
	cl.read_co_file("cell_info_co.txt");
	cl.read_rw_operation("cell_rw_op_new.txt");

	clock_t start, finish;
	double totaltime;

	ifstream pFile(target + ".v");
	stringstream ss;
	assert(pFile.is_open());
	ss << pFile.rdbuf();

	/*string  s_top_module = "module top_test (a, b, c, d, e, f, h, o);\n"
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
	stringstream ss(s);*/

	design d(&cl);
	d.parse_design_file(ss);
		
	try {
		start = clock();

		redundant_wire rw;
		rw.construct(d.get_top_module());
		rw.implication_matrix_generator_full();
		rw.redundant_wire_selector(target, 40, 2, 10000, 50);

		//simulation sim;
		//sim.construct(d.get_top_module());
		//simulation_evaluation sim_eva;
		//sim_eva.construct(&sim);
		//sim_eva.run_exhaustive_fault_injection_simulation();
		//sim_eva.evaluate_fault_injection_results();

		//cout << "100		10" << endl;
		//sim_eva.run_random_fault_injection_simulation(100, 10);
		//sim_eva.evaluate_fault_injection_results();

		//cout << "1000		10" << endl;
		//sim_eva.run_random_fault_injection_simulation(1000, 10);
		//sim_eva.evaluate_fault_injection_results();

		//cout << "1000		50" << endl;
		//sim_eva.run_random_fault_injection_simulation(1000, 50);
		//sim_eva.evaluate_fault_injection_results();

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
#endif