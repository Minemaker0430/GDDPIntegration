#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class RecommendedUtils {
public:
	static void validateLevels();
	static void generateRecommendations();
    static void getSkillsForRecommendation(int);

	static std::vector<std::string> sortSkills(matjson::Value);

    static int cycleRating(int, int);
};

struct SkillSort
{
    std::string key;
    float value;

    SkillSort(std::string k, const float& v) : key(k), value(v) {}

    bool operator > (const SkillSort& str) const
    {
        return (value > str.value);
    }
};