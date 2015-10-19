#pragma once
#include <bitset>
#include <vector>
#include "config.h"

using namespace std;

class cell_utility
{
public:
	static bool									implication_verify(bitset<MAX_CELL_INPUTS_X2> mask, bitset<MAX_CELL_INPUTS_X2> curr, const bitset<MAX_CELL_INPUTS_X2> &req);
	static bool									check_all_known(const bitset<MAX_CELL_INPUTS_X2> &vector, int bit_num);
	static void									next_value(bitset<MAX_CELL_INPUTS_X2> &vector, int bit_num);
	static void									create_mask(const bitset<MAX_CELL_INPUTS_X2> &vector, bitset<MAX_CELL_INPUTS_X2> &mask, int bit_num);
	static bitset<MAX_CELL_INPUTS_X2>			backward_ref(const bitset<MAX_CELL_INPUTS_X2> &curr,
		bitset<MAX_CELL_INPUTS_X2> &mask,
		vector<bitset<MAX_CELL_INPUTS_X2>> &cc_array);
	static bool									forward_ref(const bitset<MAX_CELL_INPUTS_X2> &curr,
		vector<bitset<MAX_CELL_INPUTS_X2>> &cc_array);
	static void									int_to_vector(int i, bitset<MAX_CELL_INPUTS_X2> &v, int bit_num);
	static int									vector_to_int(bitset<MAX_CELL_INPUTS_X2> v, int bit_num);
};

