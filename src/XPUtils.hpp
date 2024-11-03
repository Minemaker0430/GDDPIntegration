#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class XPUtils {
public:
	static const std::vector<std::string> skillIDs;

	static void getMaxLevels();
	static void getXP();
	static void getLevels();

	static float getTotalWeightedSum(std::string);
	static float getCompletedWeightedSum(std::string);
};