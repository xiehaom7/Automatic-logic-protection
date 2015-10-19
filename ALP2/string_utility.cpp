#include "string_utility.h"

string string_utility::read_next_word(const string &s, size_t &pos) {
	size_t s_pos = pos, e_pos;
	string_utility::read_space(s, s_pos);
	if (s_pos == string::npos) {
		pos = s_pos;
		return string("");
	}
	e_pos = pos = s.find_first_not_of(RW_VALID_CHAR, s_pos);
	string_utility::read_space(s, pos);
	return s.substr(s_pos, (e_pos == string::npos) ? e_pos : e_pos - s_pos);
}

void string_utility::read_space(const string &s, size_t &pos) {
	if (pos != string::npos)
		pos = s.find_first_not_of(RW_VALID_SPACE, pos);
	return;
}

bool string_utility::expect_within(const string &s, size_t &pos, const string &expect) {
	if (!string_utility::test_within(s, pos, expect)) {
		cerr << "ERROR: Expected within '" + expect + "' but got '" + s[pos] + "'. (expect_within)\n";
		return false;
	}
	pos += 1;
	return true;
}

bool string_utility::test_within(const string &s, size_t &pos, const string &t) {
	if (pos == string::npos || pos >= s.size())
		return false;
	return t.find(s[pos]) != string::npos;
}