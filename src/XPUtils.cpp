//geode header
#include <Geode/Geode.hpp>
#include <cmath>

#include "XPUtils.hpp"
#include "DPUtils.hpp"

//geode namespace
using namespace geode::prelude;

const matjson::Value XPUtils::skills = matjson::makeObject({
	{"chokepoints", matjson::makeObject({
		{"name", "Chokepoints"},
		{"position", std::vector<float>{-105.f, 15.f}},
		{"color", ccColor3B{255, 0, 0}}
	})},
	{"duals", matjson::makeObject({
		{"name", "Duals"},
		{"position", std::vector<float>{-105.f, -15.f}},
		{"color", ccColor3B{255, 128, 0}}
	})},
	{"fastPaced", matjson::makeObject({
		{"name", "Fast Paced"},
		{"position", std::vector<float>{-105.f, -45.f}},
		{"color", ccColor3B{255, 255, 0}}
	})},
	{"cps", matjson::makeObject({
		{"name", "High CPS"},
		{"position", std::vector<float>{-105.f, -75.f}},
		{"color", ccColor3B{128, 255, 0}}
	})},
	{"memory", matjson::makeObject({
		{"name", "Memory/Learny"},
		{"position", std::vector<float>{-105.f, -105.f}},
		{"color", ccColor3B{0, 255, 0}}
	})},
	{"nerve", matjson::makeObject({
		{"name", "Nerve Control"},
		{"position", std::vector<float>{105.f, 15.f}},
		{"color", ccColor3B{0, 255, 128}}
	})},
	{"ship", matjson::makeObject({
		{"name", "Ship"},
		{"position", std::vector<float>{105.f, -15.f}},
		{"color", ccColor3B{0, 255, 255}}
	})},
	{"swing", matjson::makeObject({
		{"name", "Swing"},
		{"position", std::vector<float>{105.f, -45.f}},
		{"color", ccColor3B{0, 128, 255}}
	})},
	{"timings", matjson::makeObject({
		{"name", "Timings"},
		{"position", std::vector<float>{105.f, -75.f}},
		{"color", ccColor3B{0, 0, 255}}
	})},
	{"wave", matjson::makeObject({
		{"name", "Wave"},
		{"position", std::vector<float>{105.f, -105.f}},
		{"color", ccColor3B{128, 0, 255}}
	})}
});

const float scaling = (1.f / 500.f);

void XPUtils::getMaxLevels() {
	/*
	scaling = 1/500
	MAX_LEVEL(skill) = Math.ceil(TWS(skill) * scaling)
	*/

	//log::info("Getting Max Levels...");

	matjson::Value res;
	for (auto [key, value] : skills) res.set(key, ceil(getTotalWeightedSum(key) * scaling));

	Mod::get()->setSavedValue<matjson::Value>("max-levels", res);
	
	return;
}

void XPUtils::getXP() {
	//log::info("Getting XP...");

	matjson::Value res;
	for (auto [key, value] : skills) res.set(key, getCompletedWeightedSum(key) / getTotalWeightedSum(key));

	Mod::get()->setSavedValue<matjson::Value>("xp", res);
	
	return;
}

void XPUtils::getLevels() {

	getMaxLevels();
	getXP();

	//log::info("Getting Levels...");

	matjson::Value skillVal;
    matjson::Value percent;
	
	for (auto [key, value] : skills) {
		auto xp = Mod::get()->getSavedValue<matjson::Value>("xp")[key].as<float>().unwrapOr(0.f);
		auto maxLvl = Mod::get()->getSavedValue<matjson::Value>("max-levels")[key].as<int>().unwrapOr(-1);

		// Pre-calculate exponents for powf
		std::vector<float> exponents(maxLvl + 1, 0.f);
		for (int lvl = 1; lvl <= maxLvl; lvl++) exponents[lvl] = static_cast<float>(lvl) / static_cast<float>(maxLvl);
		
		//Get XP Requirements
		std::vector<float> xpRequirements(maxLvl + 1, 0.f);
        for (int lvl = 1; lvl <= maxLvl; lvl++) xpRequirements[lvl] = powf(exponents[lvl], M_PI);

		//Get Level
		auto level = -1;
        auto next = 0.f;
        auto start = 0;
        auto end = xpRequirements.size() - 1;
        while (start <= end) {
            auto mid = (start + end) / 2;
            if (xp >= xpRequirements[mid]) {
                level = mid;
                start = mid + 1;
            }
			else end = mid - 1;
        }
		
		if (level != maxLvl && level + 1 < xpRequirements.size()) next = 1.f - ((xpRequirements[level + 1] - xp) / (xpRequirements[level + 1] - xpRequirements[level]));

        skillVal.set(key, std::max(level, 1));
        percent.set(key, next);
	}

	Mod::get()->setSavedValue<matjson::Value>("level", skillVal);
	Mod::get()->setSavedValue<matjson::Value>("percent-to-level", percent);

	return;
}

matjson::Value XPUtils::getProjectedXP(int lvlID) {
	
	getLevels();

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto numOfPacks = data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size();
	
	matjson::Value res;

	matjson::Value pXP;
	matjson::Value pLevel;
	matjson::Value pNext;
	
	for (auto [key, value] : skills) {
		
		//XP
		pXP.set(key, getProjectedWeightedSum(key, lvlID) / getTotalWeightedSum(key));

		//Level
		auto maxLvl = Mod::get()->getSavedValue<matjson::Value>("max-levels")[key].as<int>().unwrapOr(-1);

		std::vector<float> exponents(maxLvl + 1, 0.f);
		for (int lvl = 1; lvl <= maxLvl; lvl++) exponents[lvl] = static_cast<float>(lvl) / static_cast<float>(maxLvl);
	
		std::vector<float> xpRequirements(maxLvl + 1, 0.f);
        for (int lvl = 1; lvl <= maxLvl; lvl++) xpRequirements[lvl] = powf(exponents[lvl], M_PI);

		auto xp = pXP[key].as<float>().unwrapOr(0.f);

		auto level = -1;
        auto next = 0.f;
        auto start = 0;
        auto end = xpRequirements.size() - 1;
        while (start <= end) {
            auto mid = (start + end) / 2;
            if (xp >= xpRequirements[mid]) {
                level = mid;
                start = mid + 1;
            }
			else end = mid - 1;
        }
		
        pLevel.set(key, std::max(level, 1));
        
		//Next
		if (level != maxLvl && level + 1 < xpRequirements.size()) next = 1.f - ((xpRequirements[level + 1] - xp) / (xpRequirements[level + 1] - xpRequirements[level]));
		pNext.set(key, next);
	}

	res.set("xp", pXP);
	res.set("level", pLevel);
	res.set("next", pNext);

	return res;
}

float XPUtils::getProjectedWeightedSum(std::string skillID, int level) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
	auto fibonacci = Mod::get()->getSavedValue<std::vector<int>>("fibonacci", {1, 1});
	
	if (completedLvls.empty()) return 0.f;

	auto sum = 0.f;
	
	auto numOfPacks = data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size();
	auto lvlData = data["level-data"];

	if (fibonacci.size() <= numOfPacks) {
		for (int i = 1; i <= numOfPacks; i++) {
			fibonacci.push_back(fibonacci[i] + fibonacci[i - 1]);
		}
		Mod::get()->setSavedValue<std::vector<int>>("fibonacci", fibonacci);
	}

	for (int i = 0; i < numOfPacks; i++) {
		auto levelIDs = data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
		auto multiplier = fibonacci[i + 1];

		auto tierSum = 0.f;

		for (auto lvlID : levelIDs) {
			if (lvlData[std::to_string(lvlID)].contains("xp")) {
				if (DPUtils::containsInt(completedLvls, lvlID) || (level == lvlID)) {
					auto xp = lvlData[std::to_string(lvlID)]["xp"][skillID].asInt().unwrapOr(0);
					tierSum += xp;
				}
			}
		}

		sum += (tierSum * multiplier);
	}

	//log::info("projected weighted sum: {}", sum);

	return sum;
}

float XPUtils::getTotalWeightedSum(std::string skillID) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto fibonacci = Mod::get()->getSavedValue<std::vector<int>>("fibonacci", {1, 1});

	auto sum = 0.f;
	
	auto numOfPacks = data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size();
	auto lvlData = data["level-data"];

	if (fibonacci.size() <= numOfPacks) {
		for (int i = 1; i <= numOfPacks; i++) {
			fibonacci.push_back(fibonacci[i] + fibonacci[i - 1]);
		}
		Mod::get()->setSavedValue<std::vector<int>>("fibonacci", fibonacci);
	}

	for (int i = 0; i < numOfPacks; i++) {
		auto levelIDs = data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
		auto multiplier = fibonacci[i + 1];

		//auto tierSum = 0.f;

		for (auto lvlID : levelIDs) {

			if (lvlData[std::to_string(lvlID)].contains("xp")) {
				auto xp = lvlData[std::to_string(lvlID)]["xp"][skillID].asInt().unwrapOr(0);
				sum += (xp * multiplier);
			}
		}

		//sum += (tierSum * multiplier);
	}

	//log::info("{} total weighted sum: {}", skillID, sum);
	
	return sum;
}

float XPUtils::getCompletedWeightedSum(std::string skillID) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
	auto fibonacci = Mod::get()->getSavedValue<std::vector<int>>("fibonacci", {1, 1});
	
	if (completedLvls.empty()) return 0.f;

	auto sum = 0.f;
	
	auto numOfPacks = data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size();
	auto lvlData = data["level-data"];

	if (fibonacci.size() <= numOfPacks) {
		for (int i = 1; i <= numOfPacks; i++) {
			fibonacci.push_back(fibonacci[i] + fibonacci[i - 1]);
		}
		Mod::get()->setSavedValue<std::vector<int>>("fibonacci", fibonacci);
	}

	for (int i = 0; i < numOfPacks; i++) {
		auto levelIDs = data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
		auto multiplier = fibonacci[i + 1];

		//auto tierSum = 0.f;

		for (auto lvlID : levelIDs) {
			if (lvlData[std::to_string(lvlID)].contains("xp")) {
				if (DPUtils::containsInt(completedLvls, lvlID)) {
					auto xp = lvlData[std::to_string(lvlID)]["xp"][skillID].asInt().unwrapOr(0);
					sum += (xp * multiplier);
				}
			}
		}

		//sum += (tierSum );
	}

	//log::info("completed weighted sum: {}", sum);

	return sum;
}