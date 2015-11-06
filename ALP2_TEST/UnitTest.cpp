#include "stdafx.h"
#include "CppUnitTest.h"
#include "cell_utility.h"
#include "cell_utility.cpp"
#include "string_utility.h"
#include "string_utility.cpp"
#include "cell.h"
#include "cell.cpp"
#include "module.h"
#include "module.cpp"
#include "design.h"
#include "design.cpp"
#include "net.h"
#include "net.cpp"
#include "node.h"
#include "node.cpp"
#include "simulation.h"
#include "simulation.cpp"
#include "simulation_evaluation.h"
#include "simulation_evaluation.cpp"
#include "signature.h"
#include "signature.cpp"
#include <map>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ALP2_TEST
{		
	TEST_CLASS(cell_test)
	{
	public:
		TEST_METHOD(test_generate_truth_table) {
			cell c;
			bool res;
			c.input_num = 3;
			c.cc1_array.push_back(bitset<MAX_CELL_INPUTS_X2>("0000000000000011"));
			c.cc1_array.push_back(bitset<MAX_CELL_INPUTS_X2>("0000000000000101"));
			c.generate_truth_table();

			res = c.truth_table[0];
			Assert::AreEqual(false, res, L"0");
			res = c.truth_table[1];
			Assert::AreEqual(false, res, L"1");
			res = c.truth_table[2];
			Assert::AreEqual(false, res, L"2");
			res = c.truth_table[3];
			Assert::AreEqual(true, res, L"3");
			res = c.truth_table[4];
			Assert::AreEqual(false, res, L"4");
			res = c.truth_table[5];
			Assert::AreEqual(true, res, L"5");
			res = c.truth_table[6];
			Assert::AreEqual(false, res, L"6");
			res = c.truth_table[7];
			Assert::AreEqual(true, res, L"7");
		}
		//TO DO: test forward and backward ref generation functions
	};
	TEST_CLASS(cell_utility_test)
	{
	public:
		
		TEST_METHOD(test_next_value)
		{
			bitset<MAX_CELL_INPUTS_X2> vector;
			int bit_num = 2;
			vector.reset();
			
			Assert::AreEqual((unsigned long)0, vector.to_ulong());	
			cell_utility::next_value(vector, bit_num);
			Assert::AreEqual((unsigned long)4, vector.to_ulong());
			cell_utility::next_value(vector, bit_num);
			Assert::AreEqual((unsigned long)1, vector.to_ulong());
			cell_utility::next_value(vector, bit_num);
			Assert::AreEqual((unsigned long)8, vector.to_ulong());
			cell_utility::next_value(vector, bit_num);
			Assert::AreEqual((unsigned long)12, vector.to_ulong());
			cell_utility::next_value(vector, bit_num);
			Assert::AreEqual((unsigned long)9, vector.to_ulong());
			cell_utility::next_value(vector, bit_num);
			Assert::AreEqual((unsigned long)2, vector.to_ulong());
			cell_utility::next_value(vector, bit_num);
			Assert::AreEqual((unsigned long)6, vector.to_ulong());
			cell_utility::next_value(vector, bit_num);
			Assert::AreEqual((unsigned long)3, vector.to_ulong());
			cell_utility::next_value(vector, bit_num);
			Assert::AreEqual((unsigned long)0, vector.to_ulong());
		}

		TEST_METHOD(test_implication_verify)
		{
			bitset<MAX_CELL_INPUTS_X2> mask("1111111111111111"), 
				curr("0000000000001001"), req("0000000000000001");
			Assert::AreEqual(false, cell_utility::implication_verify(mask, curr, req));
			mask[0] = 0, mask[2] = 0;
			Assert::AreEqual(true, cell_utility::implication_verify(mask, curr, req));
			mask[1] = 0, mask[3] = 0;
			Assert::AreEqual(true, cell_utility::implication_verify(mask, curr, req));
			req[0] = 0, req[2] = 1;
			Assert::AreEqual(false, cell_utility::implication_verify(mask, curr, req));
		}

		TEST_METHOD(test_create_mask)
		{
			bitset<MAX_CELL_INPUTS_X2> mask, curr("0000000000000000");
			int bit_num = 2;

			cell_utility::create_mask(curr, mask, bit_num);
			Assert::AreEqual(string("1111111111111111"), mask.to_string());
			curr[0] = 1;
			cell_utility::create_mask(curr, mask, bit_num);
			Assert::AreEqual(string("1111111111111010"), mask.to_string());
			curr[3] = 1;
			cell_utility::create_mask(curr, mask, bit_num);
			Assert::AreEqual(string("1111111111110000"), mask.to_string());
			curr[4] = 1;
			cell_utility::create_mask(curr, mask, bit_num);
			Assert::AreEqual(string("1111111111110000"), mask.to_string());
		}

		TEST_METHOD(test_check_all_known) {
			bitset<MAX_CELL_INPUTS_X2> curr("0000000000000000");
			int bit_num = 2;

			Assert::AreEqual(false, cell_utility::check_all_known(curr, bit_num));
			curr[0] = 1, curr[3] = 1;
			Assert::AreEqual(true, cell_utility::check_all_known(curr, bit_num));
			curr[0] = 0;
			Assert::AreEqual(false, cell_utility::check_all_known(curr, bit_num));
		}

		TEST_METHOD(test_forward_ref) {
			bitset<MAX_CELL_INPUTS_X2> curr("0000000000000000");
			vector<bitset<MAX_CELL_INPUTS_X2>> cc_array;
			cc_array.push_back(bitset<MAX_CELL_INPUTS_X2>("0000000000000001"));
			cc_array.push_back(bitset<MAX_CELL_INPUTS_X2>("0000000000000010"));

			Assert::AreEqual(false, cell_utility::forward_ref(curr, cc_array), L"0000000000000000");
			curr[0] = 1;
			Assert::AreEqual(true, cell_utility::forward_ref(curr, cc_array), L"0000000000000001");
			curr[1] = 1;
			Assert::AreEqual(true, cell_utility::forward_ref(curr, cc_array), L"0000000000000011");
			curr[0] = 0, curr[1] = 0, curr[3] = 1;
			Assert::AreEqual(false, cell_utility::forward_ref(curr, cc_array), L"0000000000001000");
		}

		TEST_METHOD(test_backward_ref) {
			bitset<MAX_CELL_INPUTS_X2> curr("0000000000000000"),
				mask, res;
			int bit_num = 4;
			vector<bitset<MAX_CELL_INPUTS_X2>> cc_array;
			cc_array.push_back(bitset<MAX_CELL_INPUTS_X2>("0000000000000111"));
			cc_array.push_back(bitset<MAX_CELL_INPUTS_X2>("0000000000001011"));

			cell_utility::create_mask(curr, mask, bit_num);
			res = cell_utility::backward_ref(curr, mask, cc_array);
			Assert::AreEqual(string("0000000000000000"), res.to_string());
			curr[0] = 1;
			cell_utility::create_mask(curr, mask, bit_num);
			res = cell_utility::backward_ref(curr, mask, cc_array);
			Assert::AreEqual(string("0000000000000010"), res.to_string());
			curr[1] = 1, curr[0] = 0;
			cell_utility::create_mask(curr, mask, bit_num);
			res = cell_utility::backward_ref(curr, mask, cc_array);
			Assert::AreEqual(string("0000000000000001"), res.to_string());
			curr[1] = 0, curr[2] = 1;
			cell_utility::create_mask(curr, mask, bit_num);
			res = cell_utility::backward_ref(curr, mask, cc_array);
			Assert::AreEqual(string("0000000000000011"), res.to_string());
			curr[2] = 0, curr[4] = 1;
			cell_utility::create_mask(curr, mask, bit_num);
			res = cell_utility::backward_ref(curr, mask, cc_array);
			Assert::AreEqual(string("0000000000000000"), res.to_string());
			curr[4] = 0, curr[6] = 1;
			cell_utility::create_mask(curr, mask, bit_num);
			res = cell_utility::backward_ref(curr, mask, cc_array);
			Assert::AreEqual(string("0000000000001011"), res.to_string());
		}

		TEST_METHOD(test_vector_to_int) {
			bitset<MAX_CELL_INPUTS_X2> v("0000000000001100");
			int bit_num = 2;

			Assert::AreEqual(0, cell_utility::vector_to_int(v, bit_num));
			v[0] = 1, v[2] = 0;
			Assert::AreEqual(1, cell_utility::vector_to_int(v, bit_num));
			v[1] = 1, v[3] = 0;
			Assert::AreEqual(3, cell_utility::vector_to_int(v, bit_num));
			v[0] = 0, v[2] = 1;
			Assert::AreEqual(2, cell_utility::vector_to_int(v, bit_num));
		}

		TEST_METHOD(test_int_to_vector) {
			bitset<MAX_CELL_INPUTS_X2> v;
			int bit_num = 2;

			cell_utility::int_to_vector(0, v, bit_num);
			Assert::AreEqual((unsigned long)12, v.to_ulong());
			cell_utility::int_to_vector(1, v, bit_num);
			Assert::AreEqual((unsigned long)9, v.to_ulong());
			cell_utility::int_to_vector(2, v, bit_num);
			Assert::AreEqual((unsigned long)6, v.to_ulong());
			cell_utility::int_to_vector(3, v, bit_num);
			Assert::AreEqual((unsigned long)3, v.to_ulong());
		}
	};
	TEST_CLASS(string_utility_test) {
		TEST_METHOD(test_read_space) {
			string test = "";
			size_t pos = 0;

			string_utility::read_space(test, pos);
			Assert::AreEqual(string::npos, pos, L"test ''");

			test = " \r\n";
			pos = 0;
			string_utility::read_space(test, pos);
			Assert::AreEqual(string::npos, pos, L"test ' \\r\\n'");

			test = "A \r\n";
			pos = 0;
			string_utility::read_space(test, pos);
			Assert::AreEqual((size_t)0, pos, L"test 'A \\r\\n'");

			test = " \r\nA";
			pos = 0;
			string_utility::read_space(test, pos);
			Assert::AreEqual((size_t)3, pos, L"test ' \\r\\nA'");

			test = "A \r\nA";
			pos = 1;
			string_utility::read_space(test, pos);
			Assert::AreEqual((size_t)4, pos, L"test 'A \\r\\n'A");
		}

		TEST_METHOD(test_read_next_word) {
			string test = "", res;
			size_t pos = 0;
			res = string_utility::read_next_word(test, pos);
			Assert::AreEqual(string::npos, pos, L"test ''");
			Assert::AreEqual(string(""), res, L"test ''");

			test = " \r\n";
			pos = 0;
			res = string_utility::read_next_word(test, pos);
			Assert::AreEqual(string::npos, pos, L"test ' \\r\\n'");
			Assert::AreEqual(string(""), res, L"test ' \\r\\n'");

			test = "A_1 \r\n";
			pos = 0;
			res = string_utility::read_next_word(test, pos);
			Assert::AreEqual(string::npos, pos, L"test 'A_1 \\r\\n'");
			Assert::AreEqual(string("A_1"), res, L"test 'A_1 \\r\\n'");

			test = " \r\nA_1";
			pos = 0;
			res = string_utility::read_next_word(test, pos);
			Assert::AreEqual(string::npos, pos, L"test ' \\r\\nA_1'");
			Assert::AreEqual(string("A_1"), res, L"test ' \\r\\nA_1'");

			test = "A_1 \r\nA";
			pos = 0;
			res = string_utility::read_next_word(test, pos);
			Assert::AreEqual((size_t)6, pos, L"test 'A_1 \\r\\n'A");
			Assert::AreEqual(string("A_1"), res, L"test 'A_1 \\r\\n'A");

			test = "A_1@ \r\nA";
			pos = 0;
			res = string_utility::read_next_word(test, pos);
			Assert::AreEqual((size_t)3, pos, L"test 'A_1@ \\r\\n'A");
			Assert::AreEqual(string("A_1"), res, L"test 'A_1@ \\r\\n'A");
		}

		TEST_METHOD(test_test_within) {
			string s = "", test = "";
			size_t pos = 0;

			Assert::AreEqual(false, string_utility::test_within(s, pos, test), 
				L"test '' 0 ''");

			s = "a", pos = 0, test = "a_1";
			Assert::AreEqual(true, string_utility::test_within(s, pos, test), 
				L"test 'a' 0 'a_1'");

			s = "a", pos = 1, test = "a_1";
			Assert::AreEqual(false, string_utility::test_within(s, pos, test),
				L"test 'a' 1 'a_1'");

			s = "a", pos = 0, test = "A_1";
			Assert::AreEqual(false, string_utility::test_within(s, pos, test),
				L"test 'a' 0 'A_1'");
		}
	};
	TEST_CLASS(cell_library_test) {
		TEST_METHOD(test_parse_cc_file) {
			cell_library cl("test_lib");
			stringstream ss("#AOI211_X1\nA B C1 C2\nZN\n00001110 00001101\n10000000 01000000 00110000\n#AND2_X1\nA1 A2\nZN\n1100\n0010 0001");
			cell* p_cell;

			cl.parse_cc_file(ss);

			p_cell = cl.find_cell("AOI211_X1");
			Assert::IsNotNull(p_cell, L"AOI211_X1 not found.");
			Assert::AreEqual((size_t)2, p_cell->cc1_array.size(), L"AOI211_X1 cc1 array size not match.");
			Assert::AreEqual(string("0000000001110000"), p_cell->cc1_array[0].to_string(), L"AOI211_X1 cc1 array[0] not match.");
			Assert::AreEqual(string("0000000010110000"), p_cell->cc1_array[1].to_string(), L"AOI211_X1 cc1 array[1] not match.");
			Assert::AreEqual((size_t)3, p_cell->cc0_array.size(), L"AOI211_X1 cc0 array size not match.");
			Assert::AreEqual(string("0000000000000001"), p_cell->cc0_array[0].to_string(), L"AOI211_X1 cc0 array[0] not match.");
			Assert::AreEqual(string("0000000000000010"), p_cell->cc0_array[1].to_string(), L"AOI211_X1 cc0 array[1] not match.");
			Assert::AreEqual(string("0000000000001100"), p_cell->cc0_array[2].to_string(), L"AOI211_X1 cc0 array[2] not match.");
			p_cell = cl.find_cell("AND2_X1");
			Assert::IsNotNull(p_cell, L"AND2_X1 not found.");
			Assert::AreEqual((size_t)1, p_cell->cc1_array.size(), L"AND2_X1 cc1 array size not match.");
			Assert::AreEqual(string("0000000000000011"), p_cell->cc1_array[0].to_string(), L"AND2_X1 cc1 array[0] not match.");
			Assert::AreEqual((size_t)2, p_cell->cc0_array.size(), L"AND2_X1 cc0 array size not match.");
			Assert::AreEqual(string("0000000000000100"), p_cell->cc0_array[0].to_string(), L"AND2_X1 cc0 array[0] not match.");
			Assert::AreEqual(string("0000000000001000"), p_cell->cc0_array[1].to_string(), L"AND2_X1 cc0 array[1] not match.");
		}
		TEST_METHOD(test_parse_co_file) {
			cell_library cl("test_lib");
			stringstream ss("#AND2_X1\nA1 A2\nZN\n0100\n1000\n#AOI211_X1\nA B C1 C2\nZN\n00000110 00000101\n00001010 00001001\n00011100\n00101100");
			cell* p_cell;

			cl.parse_co_file(ss);

			p_cell = cl.find_cell("AND2_X1");
			Assert::IsNotNull(p_cell, L"AND2_X1 not found.");
			Assert::AreEqual((size_t)2, p_cell->co_array.size(), L"AND2_X1 co array size not match.");
			Assert::AreEqual((size_t)1, p_cell->co_array[0].size(), L"AND2_X1 co array[0] size not match.");
			Assert::AreEqual((size_t)1, p_cell->co_array[1].size(), L"AND2_X1 co array[1] size not match.");
			Assert::AreEqual(string("0000000000000010"), p_cell->co_array[0][0].to_string(), L"AND2_X1 co array[0][0] not match.");
			Assert::AreEqual(string("0000000000000001"), p_cell->co_array[1][0].to_string(), L"AND2_X1 co array[1][0] not match.");
			p_cell = cl.find_cell("AOI211_X1");
			Assert::IsNotNull(p_cell, L"AOI211_X1 not found.");
			Assert::AreEqual((size_t)4, p_cell->co_array.size(), L"AOI211_X1 co array size not match.");
			Assert::AreEqual((size_t)2, p_cell->co_array[0].size(), L"AOI211_X1 co array[0] size not match.");
			Assert::AreEqual((size_t)2, p_cell->co_array[1].size(), L"AOI211_X1 co array[1] size not match.");
			Assert::AreEqual((size_t)1, p_cell->co_array[2].size(), L"AOI211_X1 co array[2] size not match.");
			Assert::AreEqual((size_t)1, p_cell->co_array[3].size(), L"AOI211_X1 co array[3] size not match.");
			Assert::AreEqual(string("0000000001100000"), p_cell->co_array[0][0].to_string(), L"AOI211_X1 co array[0][0] not match.");
			Assert::AreEqual(string("0000000010010000"), p_cell->co_array[1][1].to_string(), L"AOI211_X1 co array[1][1] not match.");
			Assert::AreEqual(string("0000000000111000"), p_cell->co_array[2][0].to_string(), L"AOI211_X1 co array[2][0] not match.");
			Assert::AreEqual(string("0000000000110100"), p_cell->co_array[3][0].to_string(), L"AOI211_X1 co array[3][0] not match.");
		}

		TEST_METHOD(test_parse_rw_op) {
			cell_library cl("test_lib");
			stringstream ss_setup("#AND2_X1\nA1 A2\nZN\n0100\n1000\n#BUF_X1\nA\nZ\n00\n00");
			stringstream ss("#AND2_X1\n@A1\n$0 -> 0\n&AW() : nw_1\n&DW(old.A1)\n&CW(old.A1, ng_1.A1)\n$1 -> 0\n&AG(INV_X1) : ng_1\n@A2\n$0 -> 1\n&RG(old)\n#BUF_X1");
			cell* p_cell;
			vector<Op_item>* p_op_list;


			//setup
			cl.parse_co_file(ss_setup);
			cl.parse_rw_op(ss);

			p_cell = cl.find_cell("AND2_X1");
			Assert::IsNotNull(p_cell, L"AND2_X1 not found.");
			Assert::AreEqual((size_t)2, p_cell->rw_op_collection.size(), L"AND2_X1 rw_op_collection size not match");

			Assert::IsNotNull(p_cell->rw_op_collection["A1"]->implication_item_array[0], L"AND2_X1 rw_op_collection: A1 0 -> 0 not found");
			Assert::IsNotNull(p_cell->rw_op_collection["A1"]->implication_item_array[2], L"AND2_X1 rw_op_collection: A1 1 -> 0 not found");
			Assert::AreEqual(true, p_cell->rw_op_collection["A1"]->implication_item_array[0]->x_value == ZERO, L"AND2_X1 rw_op_collection: A1 0 -> 0 x_value not match");
			Assert::AreEqual(true, p_cell->rw_op_collection["A1"]->implication_item_array[0]->pin_value == ZERO, L"AND2_X1 rw_op_collection: A1 0 -> 0 pin_value not match");
			
			p_op_list = &p_cell->rw_op_collection["A1"]->implication_item_array[0]->op_list;
			Assert::AreEqual((size_t)3, p_op_list->size(), L"rw_op_collection: A1 0 -> 0 op_list size not match");
			Assert::AreEqual(true, (*p_op_list)[0].op == AW, L"op AW not found");
			Assert::AreEqual(string("nw_1"), (*p_op_list)[0].new_name, L"op AW new_name not found");
			Assert::AreEqual(true, (*p_op_list)[1].op == DW, L"op DW not found");
			Assert::AreEqual(string("old"), (*p_op_list)[1].op1, L"op DW op1 not found");
			Assert::AreEqual(string("A1"), (*p_op_list)[1].op1_pin, L"op DW op1_pin not found");
			Assert::AreEqual(true, (*p_op_list)[2].op == CW, L"op CW not found");
			Assert::AreEqual(string("old"), (*p_op_list)[2].op1, L"op CW op1 not found");
			Assert::AreEqual(string("A1"), (*p_op_list)[2].op1_pin, L"op CW op1_pin not found");
			Assert::AreEqual(string("ng_1"), (*p_op_list)[2].op2, L"op CW op2 not found");
			Assert::AreEqual(string("A1"), (*p_op_list)[2].op2_pin, L"op CW op2_pin not found");
			
			p_op_list = &p_cell->rw_op_collection["A1"]->implication_item_array[2]->op_list;
			Assert::AreEqual((size_t)1, p_op_list->size(), L"rw_op_collection: A1 1 -> 0 op_list size not match");
			Assert::AreEqual(true, (*p_op_list)[0].op == AG, L"op AG not found");
			Assert::AreEqual(string("ng_1"), (*p_op_list)[0].new_name, L"op AG new_name not found");
			Assert::AreEqual(string("INV_X1"), (*p_op_list)[0].op1, L"op AG op1 not found");

			Assert::IsNotNull(p_cell->rw_op_collection["A2"]->implication_item_array[1], L"rw_op_collection: A2 0 -> 1 not found");
			
			p_op_list = &p_cell->rw_op_collection["A2"]->implication_item_array[1]->op_list;
			Assert::AreEqual((size_t)1, p_op_list->size(), L"rw_op_collection: A1 0 -> 0 op_list size not match");
			Assert::AreEqual(true, (*p_op_list)[0].op == RG, L"op RG not found");
			Assert::AreEqual(string("old"), (*p_op_list)[0].op1, L"op RG op1 not found");

			p_cell = cl.find_cell("BUF_X1");
			Assert::IsNotNull(p_cell, L"BUF_X1 not found.");
			Assert::AreEqual((size_t)0, p_cell->rw_op_collection.size(), L"BUF_X1 rw_op_collection size not match");
		}
	};
	TEST_CLASS(module_test) {
	public:
		TEST_METHOD(test_module) {
			stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n#OR2_X1\nA1 A2\nZN\n1000 0100\n0011");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss);
			string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			module m(cl);
			map<string, module*> mm;
			m.read_module(s_module, mm, cl->map_cell);

			Assert::AreEqual(string("test"), m.get_module_name(), L"module name not match");
			Assert::AreEqual(7, m.get_net_num(), L"net number not match");
			Assert::AreEqual(4, m.get_node_num(), L"node number not match");
			Assert::AreEqual(3, m.get_input_num(), L"input number not match");
			Assert::AreEqual(2, m.get_output_num(), L"output number not match");
			Assert::AreEqual(1, m.get_input_pos("i_1"), L"get_input_pos not match");
			Assert::AreEqual(1, m.get_output_pos("o_1"), L"get_output_pos not match");
			Assert::AreEqual(s_module, m.write_module(), L"write_module not match");
			Assert::AreEqual(true, m.check_module(), L"check_module not match");
			delete cl;
		}

		TEST_METHOD(test_check_module) {
			stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
				"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
				"#AND3_X1\nA1 A2 A3\nZN");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss);
			string s_incorrect_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND3_X1 U1 (.A1(i_0), .A2(i_1), .A3(o_0), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0;\n"
				"wire w_0, w_1;\n\n"
				"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
				"endmodule\n";
			module m1(cl);
			module m2(cl);
			map<string, module*> mm;
			m1.read_module(s_incorrect_module, mm, cl->map_cell);
			mm[m1.get_module_name()] = &m1;
			m2.read_module(s_top_module, mm, cl->map_cell);

			Assert::IsFalse(m1.check_module());
			Assert::IsFalse(!m2.check_module());
		}

		TEST_METHOD(test_get_topological_sequence) {
			stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
				"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
				"#AND3_X1\nA1 A2 A3\nZN");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss);
			string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string s_incorrect_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND3_X1 U1 (.A1(i_0), .A2(i_1), .A3(o_0), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0;\n"
				"wire w_0, w_1;\n\n"
				"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
				"endmodule\n";
			module m1(cl);
			module m2(cl);
			map<string, module*> mm;
			int i;
			m1.read_module(s_module, mm, cl->map_cell);
			mm[m1.get_module_name()] = &m1;
			m2.read_module(s_top_module, mm, cl->map_cell);
			
			string a2[] = { "i_0", "i_1", "i_2", "w_0", "w_1", "o_0" };
			vector<string> v2;
			Assert::AreEqual(true, m2.get_topological_sequence(v2));
			Assert::AreEqual((size_t)6, v2.size());
			for (i = 0; i < 6; i++)
				Assert::AreEqual(a2[i], v2[i]);

			string a1[] = { "i_0", "i_1", "i_2", "w_0", "w_1", "o_0", "o_1" };
			vector<string> v1;
			Assert::AreEqual(true, m1.get_topological_sequence(v1));
			Assert::AreEqual((size_t)7, v1.size());
			for (i = 0; i < 7; i++)
				Assert::AreEqual(a1[i], v1[i]);
		}
	};
	TEST_CLASS(design_test) {
		TEST_METHOD(test_design) {
			stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n#OR2_X1\nA1 A2\nZN\n1000 0100\n0011");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss);
			string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string  s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0;\n"
				"wire w_0, w_1;\n\n"
				"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
				"endmodule\n";
			string s = s_module + "\n" + s_top_module + "\n";
			design d(cl);
			stringstream ss_module(s);
			d.parse_design_file(ss_module);

			Assert::AreEqual(string("top_test"), d.get_top_module()->get_module_name());
			d.set_top_module("test");
			Assert::AreEqual(string("test"), d.get_top_module()->get_module_name());

			Assert::AreEqual(s, d.output_design_file());
		}
	};
	TEST_CLASS(simulation_test) {
		TEST_METHOD(test_generate_input_vector) {
			vector<bool> vi;
			vector<bool> res(3, false);
			vi.resize(3);

			simulation::generate_input_vector(vi, RESET);
			Assert::AreEqual(true, vi == res);

			simulation::generate_input_vector(vi, SEQUENCE);
			res[0] = true;
			Assert::AreEqual(true, vi == res);

			simulation::generate_input_vector(vi, SEQUENCE);
			res[0] = false, res[1] = true;
			Assert::AreEqual(true, vi == res);

			simulation::generate_input_vector(vi, RESET);
			res[1] = false;
			Assert::AreEqual(true, vi == res);
		}
		TEST_METHOD(test_simulation_module) {
			stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
				"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
				"#AND3_X1\nA1 A2 A3\nZN");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss);
			string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string  s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0;\n"
				"wire w_0, w_1;\n\n"
				"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
				"endmodule\n";
			string s = s_module + "\n" + s_top_module + "\n";
			design d(cl);
			stringstream ss_module(s);
			d.parse_design_file(ss_module);
			
			if (MAX_PARALLEL_NUM < 8) {
				Logger::WriteMessage(
					"test_simulation_module skipped due to small MAX_PARALLEL_NUM.");
				return;
			}

			simulation sim;
			vector<bool> vi(3, false);
			vector<bool> parallel(3, false);
			sim.construct(d.get_top_module());

			sim.set_input_vector(vi);
			parallel[0] = true;  //[0] 001
			sim.set_parallel_input_vector(parallel, vi, 0);
			parallel[1] = true;  //[1] 011
			sim.set_parallel_input_vector(parallel, vi, 1);
			parallel[0] = false; //[2] 010
			sim.set_parallel_input_vector(parallel, vi, 2);
			parallel[2] = true;  //[3] 110
			sim.set_parallel_input_vector(parallel, vi, 3);
			parallel[0] = true;  //[4] 111
			sim.set_parallel_input_vector(parallel, vi, 4);
			parallel[1] = false; //[5] 101
			sim.set_parallel_input_vector(parallel, vi, 5);
			parallel[0] = false; //[6] 100
			sim.set_parallel_input_vector(parallel, vi, 6);

			sim.simulate_module(FLIP);

			Wire_value* value = NULL;
			bitset<MAX_PARALLEL_NUM>* vector = NULL;
			sim.get_node_value("top_test.o_0", &value, &vector);
			Assert::AreEqual(true, *value == ZERO);
			Assert::AreEqual((unsigned long)16, vector->to_ulong());

			sim.get_node_value("top_test.U1.o_1", &value, &vector);
			Assert::AreEqual(true, *value == ZERO);
			Assert::AreEqual((unsigned long)127, vector->to_ulong());
		}
		TEST_METHOD(test_simulation_module_2) {
			stringstream ss("#BIAS\nA1 A2\nZN\n1001\n1100 0110 0011\n");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss);
			string  s_top_module = "module top_test (i_0, i_1, o_0);\n"
				"input i_0, i_1;\n"
				"output o_0;\n"
				"BIAS U1 (.A1(i_0), .A2(i_1), .ZN(o_0) );\n"
				"endmodule\n";
			string s = s_top_module;
			design d(cl);
			stringstream ss_module(s);
			d.parse_design_file(ss_module);

			simulation sim;
			vector<bool> vi(2, false);
			vector<bool> parallel(2, false);
			sim.construct(d.get_top_module());

			if (MAX_PARALLEL_NUM < 3) {
				Logger::WriteMessage(
					"test_simulation_module_2 skipped due to small MAX_PARALLEL_NUM.");
				return;
			}

			sim.set_input_vector(vi);
			parallel[0] = true;  //[0] 01
			sim.set_parallel_input_vector(parallel, vi, 0);
			parallel[1] = true;  //[1] 11
			sim.set_parallel_input_vector(parallel, vi, 1);
			parallel[0] = false; //[2] 10
			sim.set_parallel_input_vector(parallel, vi, 2);

			sim.simulate_module(FLIP);

			Wire_value* value = NULL;
			bitset<MAX_PARALLEL_NUM>* vector = NULL;
			sim.get_node_value("top_test.o_0", &value, &vector);
			Assert::AreEqual(true, *value == ZERO);
			Assert::AreEqual((unsigned long)1, vector->to_ulong());
		}
	};
	TEST_CLASS(simulation_evaluation_test) {
		TEST_METHOD(test_run_exhaustive_golden_simulation) {
			stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
				"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
				"#AND3_X1\nA1 A2 A3\nZN");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss);
			string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string  s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0;\n"
				"wire w_0, w_1;\n\n"
				"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
				"endmodule\n";
			string s = s_module + "\n" + s_top_module + "\n";
			design d(cl);
			stringstream ss_module(s);
			d.parse_design_file(ss_module);
			simulation sim;
			sim.construct(d.get_top_module());

			simulation_evaluation sv;
			sv.construct(&sim);
			sv.run_exhaustive_golden_simulation();
			StatNode* tar_node;

			tar_node = sv.get_stat_node(string("top_test.i_0"));
			Assert::AreEqual((unsigned)4, tar_node->uLogicOne);
			Assert::AreEqual((unsigned)4, tar_node->uLogicZero);
			tar_node = sv.get_stat_node(string("top_test.i_1"));
			Assert::AreEqual((unsigned)4, tar_node->uLogicOne);
			Assert::AreEqual((unsigned)4, tar_node->uLogicZero);
			tar_node = sv.get_stat_node(string("top_test.i_2"));
			Assert::AreEqual((unsigned)4, tar_node->uLogicOne);
			Assert::AreEqual((unsigned)4, tar_node->uLogicZero);
			tar_node = sv.get_stat_node(string("top_test.U1.w_0"));
			Assert::AreEqual((unsigned)2, tar_node->uLogicOne);
			Assert::AreEqual((unsigned)6, tar_node->uLogicZero);
			tar_node = sv.get_stat_node(string("top_test.U1.w_1"));
			Assert::AreEqual((unsigned)6, tar_node->uLogicOne);
			Assert::AreEqual((unsigned)2, tar_node->uLogicZero);
			tar_node = sv.get_stat_node(string("top_test.U1.o_0"));
			Assert::AreEqual((unsigned)1, tar_node->uLogicOne);
			Assert::AreEqual((unsigned)7, tar_node->uLogicZero);
			tar_node = sv.get_stat_node(string("top_test.U1.o_1"));
			Assert::AreEqual((unsigned)7, tar_node->uLogicOne);
			Assert::AreEqual((unsigned)1, tar_node->uLogicZero);
			tar_node = sv.get_stat_node(string("top_test.o_0"));
			Assert::AreEqual((unsigned)1, tar_node->uLogicOne);
			Assert::AreEqual((unsigned)7, tar_node->uLogicZero);
		}
		TEST_METHOD(test_run_exhaustive_fault_injection_simulation) {
			try {
				stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
					"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
					"#AND3_X1\nA1 A2 A3\nZN");
				cell_library*	cl;
				cl = new cell_library("test_lib");
				cl->parse_cc_file(ss);
				string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
					"input i_0, i_1, i_2;\n"
					"output o_0, o_1;\n"
					"wire w_0, w_1;\n\n"
					"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
					"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
					"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
					"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
					"endmodule\n";
				string  s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
					"input i_0, i_1, i_2;\n"
					"output o_0;\n"
					"wire w_0, w_1;\n\n"
					"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
					"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
					"endmodule\n";
				string s = s_module + "\n" + s_top_module + "\n";
				design d(cl);
				stringstream ss_module(s);
				d.parse_design_file(ss_module);
				simulation sim;
				sim.construct(d.get_top_module());

				simulation_evaluation sv;
				sv.construct(&sim);
				sv.run_exhaustive_fault_injection_simulation();
				StatNode* tar_node;

				tar_node = sv.get_stat_node(string("top_test.i_0"));
				Assert::AreEqual((unsigned)4, tar_node->uLogicOne);
				Assert::AreEqual((unsigned)4, tar_node->uLogicZero);
				Assert::AreEqual((unsigned)0, tar_node->uPropagation);
				Assert::AreEqual((unsigned)40, tar_node->uSimulation);
				Assert::AreEqual((unsigned)0, tar_node->uInjection);
				Assert::AreEqual((unsigned)0, tar_node->uAffection);
				tar_node = sv.get_stat_node(string("top_test.i_1"));
				Assert::AreEqual((unsigned)4, tar_node->uLogicOne);
				Assert::AreEqual((unsigned)4, tar_node->uLogicZero);
				Assert::AreEqual((unsigned)0, tar_node->uPropagation);
				Assert::AreEqual((unsigned)40, tar_node->uSimulation);
				Assert::AreEqual((unsigned)0, tar_node->uInjection);
				Assert::AreEqual((unsigned)0, tar_node->uAffection);
				tar_node = sv.get_stat_node(string("top_test.i_2"));
				Assert::AreEqual((unsigned)4, tar_node->uLogicOne);
				Assert::AreEqual((unsigned)4, tar_node->uLogicZero);
				Assert::AreEqual((unsigned)0, tar_node->uPropagation);
				Assert::AreEqual((unsigned)40, tar_node->uSimulation);
				Assert::AreEqual((unsigned)0, tar_node->uInjection);
				Assert::AreEqual((unsigned)0, tar_node->uAffection);
				tar_node = sv.get_stat_node(string("top_test.U1.w_0"));
				Assert::AreEqual((unsigned)2, tar_node->uLogicOne);
				Assert::AreEqual((unsigned)6, tar_node->uLogicZero);
				Assert::AreEqual((unsigned)4, tar_node->uPropagation);
				Assert::AreEqual((unsigned)40, tar_node->uSimulation);
				Assert::AreEqual((unsigned)8, tar_node->uInjection);
				Assert::AreEqual((unsigned)8, tar_node->uAffection);
				tar_node = sv.get_stat_node(string("top_test.U1.w_1"));
				Assert::AreEqual((unsigned)6, tar_node->uLogicOne);
				Assert::AreEqual((unsigned)2, tar_node->uLogicZero);
				Assert::AreEqual((unsigned)0, tar_node->uPropagation);
				Assert::AreEqual((unsigned)40, tar_node->uSimulation);
				Assert::AreEqual((unsigned)8, tar_node->uInjection);
				Assert::AreEqual((unsigned)8, tar_node->uAffection);
				tar_node = sv.get_stat_node(string("top_test.U1.o_0"));
				Assert::AreEqual((unsigned)1, tar_node->uLogicOne);
				Assert::AreEqual((unsigned)7, tar_node->uLogicZero);
				Assert::AreEqual((unsigned)7, tar_node->uPropagation);
				Assert::AreEqual((unsigned)40, tar_node->uSimulation);
				Assert::AreEqual((unsigned)8, tar_node->uInjection);
				Assert::AreEqual((unsigned)12, tar_node->uAffection);
				tar_node = sv.get_stat_node(string("top_test.U1.o_1"));
				Assert::AreEqual((unsigned)7, tar_node->uLogicOne);
				Assert::AreEqual((unsigned)1, tar_node->uLogicZero);
				Assert::AreEqual((unsigned)1, tar_node->uPropagation);
				Assert::AreEqual((unsigned)40, tar_node->uSimulation);
				Assert::AreEqual((unsigned)8, tar_node->uInjection);
				Assert::AreEqual((unsigned)12, tar_node->uAffection);
				tar_node = sv.get_stat_node(string("top_test.o_0"));
				Assert::AreEqual((unsigned)1, tar_node->uLogicOne);
				Assert::AreEqual((unsigned)7, tar_node->uLogicZero);
				Assert::AreEqual((unsigned)8, tar_node->uPropagation);
				Assert::AreEqual((unsigned)40, tar_node->uSimulation);
				Assert::AreEqual((unsigned)8, tar_node->uInjection);
				Assert::AreEqual((unsigned)20, tar_node->uAffection);
			}
			catch (exception e) {
				Logger::WriteMessage(e.what());
			}
		}
	};
	TEST_CLASS(signature_test) {
		TEST_METHOD(test_generate_signature) {
			stringstream ss("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
				"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
				"#AND3_X1\nA1 A2 A3\nZN");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss);
			string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string  s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0;\n"
				"wire w_0, w_1;\n\n"
				"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
				"endmodule\n";
			string s = s_module + "\n" + s_top_module + "\n";
			design d(cl);
			stringstream ss_module(s);
			d.parse_design_file(ss_module);
			simulation sim;
			sim.construct(d.get_top_module());

			signature sig;
			sig.construct(&sim);
			sig.generate_signature(false);
			
			if (SIGNATURE_SIZE < 8) {
				Logger::WriteMessage(
					"test_generate_signature skipped due to small SIGNATURE_SIZE.");
				return;
			}

			bitset<SIGNATURE_SIZE> mask(false);
			size_t i;
			for (i = 0; i < 8; i++)
				mask.set(i);
			SignatureNode* tar_node;

			tar_node = sig.get_signature_node(string("top_test.i_0"));
			Assert::AreEqual((unsigned long)170, (tar_node->vSig & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.i_1"));
			Assert::AreEqual((unsigned long)204, (tar_node->vSig & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.i_2"));
			Assert::AreEqual((unsigned long)240, (tar_node->vSig & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.w_0"));
			Assert::AreEqual((unsigned long)136, (tar_node->vSig & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.w_1"));
			Assert::AreEqual((unsigned long)238, (tar_node->vSig & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.o_0"));
			Assert::AreEqual((unsigned long)128, (tar_node->vSig & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.o_1"));
			Assert::AreEqual((unsigned long)254, (tar_node->vSig & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.o_0"));
			Assert::AreEqual((unsigned long)128, (tar_node->vSig & mask).to_ulong());
		}
		TEST_METHOD(test_analyse_observability_1) {
			stringstream ss_cc("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
				"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
				"#AND3_X1\nA1 A2 A3\nZN");
			stringstream ss_co("#AND2_X1\nA1 A2\nZN\n0100\n1000\n"
				"#AND3_X1\nA1 A2 A3\nZN\n011000\n101000\n110000\n"
				"#OR2_X1\nA1 A2\nZN\n0001\n0010");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss_cc);
			cl->parse_co_file(ss_co);
			string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string  s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0;\n"
				"wire w_0, w_1;\n\n"
				"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
				"endmodule\n";
			string s = s_module + "\n" + s_top_module + "\n";
			design d(cl);
			stringstream ss_module(s);
			d.parse_design_file(ss_module);
			simulation sim;
			sim.construct(d.get_top_module());

			signature sig;
			sig.construct(&sim);
			sig.generate_signature(false);
			sig.analyse_observability();

			if (SIGNATURE_SIZE < 8) {
				Logger::WriteMessage(
					"test_generate_signature skipped due to small SIGNATURE_SIZE.");
				return;
			}

			bitset<SIGNATURE_SIZE> mask(false);
			size_t i;
			for (i = 0; i < 8; i++)
				mask.set(i);
			SignatureNode* tar_node;

			tar_node = sig.get_signature_node(string("top_test.i_0"));
			Assert::AreEqual((unsigned long)192, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.i_1"));
			Assert::AreEqual((unsigned long)160, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.i_2"));
			Assert::AreEqual((unsigned long)136, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.w_0"));
			Assert::AreEqual((unsigned long)240, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.w_1"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.o_0"));
			Assert::AreEqual((unsigned long)254, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.o_1"));
			Assert::AreEqual((unsigned long)128, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.o_0"));
			Assert::AreEqual((unsigned long)255, (tar_node->vODCmask & mask).to_ulong());
		}
		TEST_METHOD(test_analyse_observability_2) {
			stringstream ss_cc("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
				"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
				"#AND3_X1\nA1 A2 A3\nZN");
			stringstream ss_co("#AND2_X1\nA1 A2\nZN\n0100\n1000\n"
				"#AND3_X1\nA1 A2 A3\nZN\n011000\n101000\n110000\n"
				"#OR2_X1\nA1 A2\nZN\n0001\n0010");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss_cc);
			cl->parse_co_file(ss_co);
			string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string  s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0;\n"
				"wire w_0, w_1;\n\n"
				"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
				"endmodule\n";
			string s = s_module + "\n" + s_top_module + "\n";
			design d(cl);
			stringstream ss_module(s);
			d.parse_design_file(ss_module);
			simulation sim;
			sim.construct(d.get_top_module());

			signature sig;
			vector<int> tar_node_list;
			vector<int> exclude_node_list;
			sig.construct(&sim);
			sig.generate_signature(false);
			tar_node_list.push_back(sim.get_node_index("top_test.U1.o_1"));
			sig.analyse_observability(tar_node_list, exclude_node_list);

			if (SIGNATURE_SIZE < 8) {
				Logger::WriteMessage(
					"test_generate_signature skipped due to small SIGNATURE_SIZE.");
				return;
			}

			bitset<SIGNATURE_SIZE> mask(false);
			size_t i;
			for (i = 0; i < 8; i++)
				mask.set(i);
			SignatureNode* tar_node;

			tar_node = sig.get_signature_node(string("top_test.i_0"));
			Assert::AreEqual((unsigned long)3, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.i_1"));
			Assert::AreEqual((unsigned long)5, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.i_2"));
			Assert::AreEqual((unsigned long)17, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.w_0"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.w_1"));
			Assert::AreEqual((unsigned long)15, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.o_0"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.o_1"));
			Assert::AreEqual((unsigned long)255, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.o_0"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
		}
		TEST_METHOD(test_analyse_observability_3) {
			stringstream ss_cc("#AND2_X1\nA1 A2\nZN\n1100\n0010 0001\n"
				"#OR2_X1\nA1 A2\nZN\n1000 0100\n0011\n"
				"#AND3_X1\nA1 A2 A3\nZN");
			stringstream ss_co("#AND2_X1\nA1 A2\nZN\n0100\n1000\n"
				"#AND3_X1\nA1 A2 A3\nZN\n011000\n101000\n110000\n"
				"#OR2_X1\nA1 A2\nZN\n0001\n0010");
			cell_library*	cl;
			cl = new cell_library("test_lib");
			cl->parse_cc_file(ss_cc);
			cl->parse_co_file(ss_co);
			string s_module = "module test (i_0, i_1, i_2, o_0, o_1);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0, o_1;\n"
				"wire w_0, w_1;\n\n"
				"AND2_X1 U1 (.A1(i_0), .A2(i_1), .ZN(w_0) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(i_2), .ZN(o_0) );\n"
				"OR2_X1 U3 (.A1(i_0), .A2(i_1), .ZN(w_1) );\n"
				"OR2_X1 U4 (.A1(w_1), .A2(i_2), .ZN(o_1) );\n"
				"endmodule\n";
			string  s_top_module = "module top_test (i_0, i_1, i_2, o_0);\n"
				"input i_0, i_1, i_2;\n"
				"output o_0;\n"
				"wire w_0, w_1;\n\n"
				"test U1 (.i_0(i_0), .i_1(i_1), .i_2(i_2), .o_0(w_0), .o_1(w_1) );\n"
				"AND2_X1 U2 (.A1(w_0), .A2(w_1), .ZN(o_0) );\n"
				"endmodule\n";
			string s = s_module + "\n" + s_top_module + "\n";
			design d(cl);
			stringstream ss_module(s);
			d.parse_design_file(ss_module);
			simulation sim;
			sim.construct(d.get_top_module());

			signature sig;
			vector<int> tar_node_list;
			vector<int> exclude_node_list;
			sig.construct(&sim);
			sig.generate_signature(false);
			tar_node_list.push_back(sim.get_node_index("top_test.o_0"));
			exclude_node_list.push_back(sim.get_node_index("top_test.U1.o_0"));
			sig.analyse_observability(tar_node_list, exclude_node_list);

			if (SIGNATURE_SIZE < 8) {
				Logger::WriteMessage(
					"test_generate_signature skipped due to small SIGNATURE_SIZE.");
				return;
			}

			bitset<SIGNATURE_SIZE> mask(false);
			size_t i;
			for (i = 0; i < 8; i++)
				mask.set(i);
			SignatureNode* tar_node;

			tar_node = sig.get_signature_node(string("top_test.i_0"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.i_1"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.i_2"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.w_0"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.w_1"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.o_0"));
			Assert::AreEqual((unsigned long)0, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.U1.o_1"));
			Assert::AreEqual((unsigned long)128, (tar_node->vODCmask & mask).to_ulong());
			tar_node = sig.get_signature_node(string("top_test.o_0"));
			Assert::AreEqual((unsigned long)255, (tar_node->vODCmask & mask).to_ulong());
		}
	};
}