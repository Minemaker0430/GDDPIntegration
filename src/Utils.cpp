//geode header
#include <Geode/Geode.hpp>

#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>

#include "Utils.hpp"

//geode namespace
using namespace geode::prelude;

std::vector<std::string> Utils::substring(std::string s, std::string d) {
	std::vector<std::string> res;
	std::string delim = d;
	std::string token = "";
	for (int i = 0; i < s.size(); i++) {
		bool flag = true;
		for (int j = 0; j < delim.size(); j++) {
			if (s[i + j] != delim[j]) flag = false;
		}
		if (flag) {
			if (token.size() > 0) {
				res.push_back(token);
				token = "";
				i += delim.size() - 1;
			}
		}
		else {
			token += s[i];
		}
	}
	res.push_back(token);
	return res;
};

int Utils::safe_stoi(std::string input, int fallback) {
	std::string result = "";
	std::string allowedChars = "0123456789";

	for (auto c : input) {
		for (auto ch : allowedChars) {
			if (c == ch) {
				result.push_back(c);
				break;
			}
		}
	}

	if (result.empty()) {
		return fallback;
	}

	return std::stoi(result);
};