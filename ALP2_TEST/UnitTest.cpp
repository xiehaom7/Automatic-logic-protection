#include "stdafx.h"
#include "CppUnitTest.h"
#include "cell_utility.h"
#include "cell_utility.cpp"
#include "string_utility.h"
#include "string_utility.cpp"
#include "cell.h"
#include "cell.cpp"
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
}