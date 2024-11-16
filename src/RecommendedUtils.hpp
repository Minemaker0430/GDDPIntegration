#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class RecommendedUtils {
public:
	static void validateLevels();
	static void generateRecommendations();

	static std::vector<int> sortSkills(std::vector<float>);

	static bool hasPartial(int);
};

struct SkillSort
{
    int key;
    float value;

    SkillSort(int k, const float& s) : key(k), value(s) {}

    bool operator > (const SkillSort& str) const
    {
        return (value > str.value);
    }
};