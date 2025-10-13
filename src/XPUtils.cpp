//geode header
#include <Geode/Geode.hpp>
#include <cmath>

#include "XPUtils.hpp"

//geode namespace
using namespace geode::prelude;

const std::vector<std::string> XPUtils::skillIDs = { "chokepoints", "duals", "fastPaced", "cps", "memory", "nerve", "ship", "swing", "timings", "wave" };

const float PI = M_PI;
const std::vector<int> FIBONACCI = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181};
const float scaling = (1.f / 500.f);

void XPUtils::getMaxLevels() {
	/*
	scaling = 1/500
	MAX_LEVEL(skill) = Math.ceil(TWS(skill) * scaling)
	*/

	log::info("Getting Max Levels...");

	/*std::vector<int> skills = {};
	for (int i = 0; i < skillIDs.size(); i++) { skills.push_back(0); }*/
	std::vector<int> skills(skillIDs.size(), 0);
	for (int i = 0; i < skillIDs.size(); i++) {
		skills[i] = ceil(getTotalWeightedSum(skillIDs[i]) * scaling);
		//log::info("{} max level: {}", skillIDs[i], skills[i]);
	}

	Mod::get()->setSavedValue<std::vector<int>>("max-levels", skills);
	
	return;
}

void XPUtils::getXP() {
	log::info("Getting XP...");

	/*std::vector<float> skills = {};
	for (int i = 0; i < skillIDs.size(); i++) { skills.push_back(0.f); }*/
	std::vector<float> skills(skillIDs.size(), 0.f);

	for (int i = 0; i < skillIDs.size(); i++) {
		skills[i] = getCompletedWeightedSum(skillIDs[i]) / getTotalWeightedSum(skillIDs[i]);
		//log::info("{}: {}", skillIDs[i], skills[i]);
	}

	Mod::get()->setSavedValue<std::vector<float>>("xp", skills);
	
	return;
}

void XPUtils::getLevels() {

	getMaxLevels();
	getXP();

	log::info("Getting Levels...");

	auto xp = Mod::get()->getSavedValue<std::vector<float>>("xp");
	auto maxLvl = Mod::get()->getSavedValue<std::vector<int>>("max-levels");
	/*std::vector<int> skills = {};
	for (int i = 0; i < skillIDs.size(); i++) { skills.push_back(0); }
	std::vector<float> percent = {};
	for (int i = 0; i < skillIDs.size(); i++) { percent.push_back(0.f); }*/
	std::vector<int> skills(skillIDs.size(), 0);
    std::vector<float> percent(skillIDs.size(), 0.f);
	
	for (int i = 0; i < skillIDs.size(); i++) {
		// Pre-calculate exponents for powf
		std::vector<float> exponents(maxLvl[i] + 1, 0.f);
		for (int lvl = 1; lvl <= maxLvl[i]; lvl++) {
			exponents[lvl] = static_cast<float>(lvl) / static_cast<float>(maxLvl[i]);
		}
		
		//Get XP Requirements
		std::vector<float> xpRequirements(maxLvl[i] + 1, 0.f); // Pre-allocate xpRequirements
        for (int lvl = 1; lvl <= maxLvl[i]; lvl++) {
            xpRequirements[lvl] = powf(exponents[lvl], PI); // Use pre-calculated exponent
        }
		
		/*std::vector<float> xpRequirements = { 0.f };

		for (int lvl = 1; lvl <= maxLvl[i]; lvl++) {
			xpRequirements.push_back(powf(static_cast<float>(lvl) / static_cast<float>(maxLvl[i]), PI));
		}*/

		//log::info("{} level requirements: {}", skillIDs[i], xpRequirements);

		//Get Level
		auto level = -1;
        auto next = 0.f;
        auto start = 1;
        auto end = xpRequirements.size() - 1;
        while (start <= end) {
            auto mid = (start + end) / 2;
            if (xp[i] >= xpRequirements[mid]) {
                level = mid;
                start = mid + 1;
            } else {
                end = mid - 1;
            }
        }
		
		if (level != maxLvl[i] && level + 1 < xpRequirements.size()) {
            next = (xpRequirements[level + 1] - xp[i]) / (xpRequirements[level + 1] - xpRequirements[level]);
        }

        skills[i] = std::max(level, 1);
        percent[i] = next;
		
		/*auto level = -1;
		auto next = 0.f;
		for (int req = 0; req < xpRequirements.size(); req++) {
			if (xp[i] >= xpRequirements[req]) {
				level = req;

				//Percent to Next Level
				//(req - xp) / (req - lvl)
				if (level != maxLvl[i] && req + 1 < xpRequirements.size()) {
					next = (xpRequirements[req + 1] - xp[i]) / (xpRequirements[req + 1] - xpRequirements[req]);
				}
			}
		}

		skills[i] = level;
		percent[i] = next;*/

		//log::info("{}: {}, next: {}", skillIDs[i], skills[i], percent[i]);
	}

	Mod::get()->setSavedValue<std::vector<int>>("level", skills);
	Mod::get()->setSavedValue<std::vector<float>>("percent-to-level", percent);

	return;
}

float XPUtils::getTotalWeightedSum(std::string skillID) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	auto sum = 0.f;
	
	auto numOfPacks = data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size();
	auto lvlData = data["level-data"];

	for (int i = 0; i < numOfPacks; i++) {
		auto levelIDs = data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
		auto multiplier = FIBONACCI[i + 1];

		auto tierSum = 0.f;

		for (int j = 0; j < levelIDs.size(); j++) {
			auto levelID = std::to_string(levelIDs[j]);

			if (lvlData[levelID]["xp"][skillID].isNumber()) {
				auto xp = lvlData[levelID]["xp"][skillID].asInt().unwrapOr(0);
				tierSum += xp;
			}
		}

		sum += (tierSum * multiplier);
	}

	//log::info("total weighted sum: {}", sum);
	
	return sum;
}

float XPUtils::getCompletedWeightedSum(std::string skillID) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
	
	if (completedLvls.empty()) {
		return 0.f;
	}

	auto sum = 0.f;
	
	auto numOfPacks = data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size();
	auto lvlData = data["level-data"];

	for (int i = 0; i < numOfPacks; i++) {
		auto levelIDs = data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
		auto multiplier = FIBONACCI[i + 1];

		auto tierSum = 0.f;

		for (int j = 0; j < levelIDs.size(); j++) {
			auto levelID = std::to_string(levelIDs[j]);

			if (lvlData[levelID]["xp"][skillID].isNumber()) {
				if (std::find(completedLvls.begin(), completedLvls.end(), levelIDs[j]) != completedLvls.end()) {
					auto xp = lvlData[levelID]["xp"][skillID].asInt().unwrapOr(0);
					tierSum += xp;
				}
			}
		}

		sum += (tierSum * multiplier);
	}

	//log::info("completed weighted sum: {}", sum);

	return sum;
}