#pragma once

#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class Utils {
public:
	static std::vector<std::string> substring(std::string string, std::string delim);
	static int safe_stoi(std::string input, int fallback = 0);
};