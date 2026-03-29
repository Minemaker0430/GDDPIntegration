#pragma once

#include <Geode/Geode.hpp>
#include <any>

//geode namespace
using namespace geode::prelude;

class DPUtils {
public:
	static std::vector<std::string> substring(std::string string, std::string delim);
	static CCSprite* safeSprite(std::string name, CCSprite* fallback);
	static CCSprite* safeSpriteWithFrameName(std::string name, CCSprite* fallback);

	static std::vector<int>::iterator findInt(std::vector<int>, int);
	static std::vector<std::string>::iterator findString(std::vector<std::string>, std::string);
	static std::vector<matjson::Value>::iterator findJson(std::vector<matjson::Value>, matjson::Value);

	static bool containsInt(std::vector<int>, int);
	static bool containsString(std::vector<std::string>, std::string);
	static bool containsJson(std::vector<matjson::Value>, matjson::Value);
};