#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class XPUtils {
public:
	static const matjson::Value skills;
	/*static const std::vector<std::string> skillIDs;
	static const std::map<std::string, std::string> skillNames;
	static const std::map<std::string, CCPoint> skillPositions;
	static const std::map<std::string, ccColor3B> skillColors;*/

	static void getMaxLevels();
	static void getXP();
	static void getLevels();

	static matjson::Value getProjectedXP(int);
	static matjson::Value getProjectedLevels(int);

	static float getProjectedWeightedSum(std::string, int);

	static float getTotalWeightedSum(std::string);
	static float getCompletedWeightedSum(std::string);
};