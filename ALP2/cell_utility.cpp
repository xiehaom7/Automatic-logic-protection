#include "cell_utility.h"

bool cell_utility::implication_verify(bitset<MAX_CELL_INPUTS_X2> mask, bitset<MAX_CELL_INPUTS_X2> curr, const bitset<MAX_CELL_INPUTS_X2> &req) {
	curr.flip();
	mask.flip();
	return !mask.none() && (mask & curr & req).none();
}

void cell_utility::next_value(bitset<MAX_CELL_INPUTS_X2> &vector, int bit_num) {
	int j;
	for (j = 0; j < bit_num; j++) {
		if (!vector.test(j) && !vector.test(j + bit_num)) {
			vector.set(j + bit_num);
			break;
		}
		else if (!vector.test(j) && vector.test(j + bit_num)) {
			vector.set(j);
			vector.reset(j + bit_num);
			break;
		}
		else {
			vector.reset(j);
		}
	}
}

void cell_utility::create_mask(const bitset<MAX_CELL_INPUTS_X2> &vector,
	bitset<MAX_CELL_INPUTS_X2> &mask, int bit_num) {
	int j;
	mask.set();
	for (j = 0; j < bit_num; j++)
		if (vector[j] != vector[j + bit_num]) {
			mask.reset(j);
			mask.reset(j + bit_num);
		}
}

bool cell_utility::check_all_known(const bitset<MAX_CELL_INPUTS_X2> &vector,
	int bit_num) {
	int j;
	for (j = 0; j < bit_num; j++)
		if (vector[j] == vector[j + bit_num])
			return false;
	return true;
}

bool cell_utility::forward_ref(const bitset<MAX_CELL_INPUTS_X2> &curr,
	vector<bitset<MAX_CELL_INPUTS_X2>> &cc_array) {
	vector<bitset<MAX_CELL_INPUTS_X2>>::const_iterator c_ite;

	for (c_ite = cc_array.cbegin(); c_ite != cc_array.cend(); c_ite++)
		if ((((*c_ite)&curr) ^ (*c_ite)).none())
			return true;
	return false;
}

bitset<MAX_CELL_INPUTS_X2> cell_utility::backward_ref(const bitset<MAX_CELL_INPUTS_X2> &curr,
	bitset<MAX_CELL_INPUTS_X2> &mask, vector<bitset<MAX_CELL_INPUTS_X2>> &cc_array) {
	bool update_flag;
	bitset<MAX_CELL_INPUTS_X2> res;
	vector<bitset<MAX_CELL_INPUTS_X2>>::const_iterator c_ite;

	update_flag = false;
	res.set();
	res &= mask;

	for (c_ite = cc_array.cbegin(); c_ite != cc_array.cend(); c_ite++) {
		if (cell_utility::implication_verify(mask, curr, *c_ite) || mask.all()) {
			res &= (*c_ite);
			update_flag = true;
		}
	}

	if (update_flag)
		return res;

	return res.reset();
}

void cell_utility::int_to_vector(int i, bitset<MAX_CELL_INPUTS_X2> &v, int bit_num) {
	int j;
	v.reset();
	for (j = 0; j < bit_num; j++) {
		((i >> j) & 0x01) ? v.set(j) : v.set(j + bit_num);
	}
}

int cell_utility::vector_to_int(bitset<MAX_CELL_INPUTS_X2> v, int bit_num) {
	int j;

	for (j = bit_num; j < bit_num * 2; j++)
		v.reset(j);
	return (int)v.to_ulong();
}