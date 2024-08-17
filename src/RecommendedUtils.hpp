#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class RecommendedUtils {
public:
	static void validateLevels();
	static void generateRecommendations();

	static bool hasPartial(int);
};