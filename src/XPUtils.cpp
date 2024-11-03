//geode header
#include <Geode/Geode.hpp>

#include "XPUtils.hpp"

//geode namespace
using namespace geode::prelude;

const std::vector<std::string> XPUtils::skillIDs = { "chokepoints", "duals", "fastPaced", "cps", "memory", "nerve", "ship", "swing", "timings", "wave" };

const float PI = 3.1415926535897932384626433f;
const std::vector<int> FIBONACCI = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181};

void XPUtils::getMaxLevels() {
	/*
	scaling = 1/500
	MAX_LEVEL(skill) = Math.ceil(TWS(skill) * scaling)
	*/

	log::info("Getting Max Levels...");

	matjson::Array skills = {};
	for (int i = 0; i < skillIDs.size(); i++) { skills.push_back(0); }
	const float scaling = (1.f / 500.f);

	for (int i = 0; i < skillIDs.size(); i++) {
		skills[i] = ceil(getTotalWeightedSum(skillIDs[i]) * scaling);
		//log::info("{} max level: {}", skillIDs[i], skills[i]);
	}

	Mod::get()->setSavedValue<matjson::Array>("max-levels", skills);
	
	return;
}

void XPUtils::getXP() {
	log::info("Getting XP...");

	matjson::Array skills = {};
	for (int i = 0; i < skillIDs.size(); i++) { skills.push_back(0.f); }

	for (int i = 0; i < skillIDs.size(); i++) {
		skills[i] = getCompletedWeightedSum(skillIDs[i]) / getTotalWeightedSum(skillIDs[i]);
		//log::info("{}: {}", skillIDs[i], skills[i]);
	}

	Mod::get()->setSavedValue<matjson::Array>("xp", skills);
	
	return;
}

void XPUtils::getLevels() {

	getMaxLevels();
	getXP();

	log::info("Getting Levels...");

	auto xp = Mod::get()->getSavedValue<matjson::Array>("xp");
	matjson::Array skills = {};
	for (int i = 0; i < skillIDs.size(); i++) { skills.push_back(0); }
	matjson::Array percent = {};
	for (int i = 0; i < skillIDs.size(); i++) { percent.push_back(0.f); }
	
	for (int i = 0; i < skillIDs.size(); i++) {
		//Get XP Requirements
		std::vector<float> xpRequirements = { 0.f };
		auto maxLvl = Mod::get()->getSavedValue<matjson::Array>("max-levels");

		for (int lvl = 1; lvl <= maxLvl[i].as_int(); lvl++) {
			xpRequirements.push_back(powf(static_cast<float>(lvl) / static_cast<float>(maxLvl[i].as_double()), PI));
		}

		//log::info("{} level requirements: {}", skillIDs[i], xpRequirements);

		//Get Level
		auto level = -1;
		auto next = 0.f;
		for (int req = 0; req < xpRequirements.size(); req++) {
			if (xp[i].as_double() >= xpRequirements[req]) {
				level = req;

				//Percent to Next Level
				//(req - xp) / (req - lvl)
				if (level != maxLvl[i].as_int() && req + 1 < xpRequirements.size()) {
					next = (xpRequirements[req + 1] - static_cast<float>(xp[i].as_double())) / (xpRequirements[req + 1] - xpRequirements[req]);
				}
			}
		}

		skills[i] = level;
		percent[i] = next;

		//log::info("{}: {}, next: {}", skillIDs[i], skills[i], percent[i]);
	}

	Mod::get()->setSavedValue<matjson::Array>("level", skills);
	Mod::get()->setSavedValue<matjson::Array>("percent-to-level", percent);

	return;
}

float XPUtils::getTotalWeightedSum(std::string skillID) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	auto sum = 0.f;

	for (int i = 0; i < data["main"].as_array().size(); i++) {
		auto levelIDs = data["main"][i]["levelIDs"].as_array();
		auto multiplier = FIBONACCI[i + 1];

		auto tierSum = 0.f;

		for (int j = 0; j < levelIDs.size(); j++) {
			auto levelID = std::to_string(levelIDs[j].as_int());

			if (data["level-data"][levelID]["xp"][skillID].is_number()) {
				auto xp = data["level-data"][levelID]["xp"][skillID].as_int();
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
	auto completedLvls = Mod::get()->getSavedValue<matjson::Array>("completed-levels");

	auto sum = 0.f;

	for (int i = 0; i < data["main"].as_array().size(); i++) {
		auto levelIDs = data["main"][i]["levelIDs"].as_array();
		auto multiplier = FIBONACCI[i + 1];

		auto tierSum = 0.f;

		for (int j = 0; j < levelIDs.size(); j++) {
			auto levelID = std::to_string(levelIDs[j].as_int());

			if (data["level-data"][levelID]["xp"][skillID].is_number()) {
				if (std::find(completedLvls.begin(), completedLvls.end(), levelIDs[j].as_int()) != completedLvls.end()) {
					auto xp = data["level-data"][levelID]["xp"][skillID].as_int();
					tierSum += xp;
				}
			}
		}

		sum += (tierSum * multiplier);
	}

	//log::info("completed weighted sum: {}", sum);

	return sum;
}