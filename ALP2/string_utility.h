#pragma once

#include <string>
#include <iostream>
#include "config.h"

using namespace std;

class string_utility {
public:
	static string		read_next_word(const string &s, size_t &pos);
	static void			read_space(const string &s, size_t &pos);
	static bool			expect_within(const string &s, size_t &pos, const string &expect);
	static bool			test_within(const string &s, size_t &pos, const string &t);
};