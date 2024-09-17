/*
How does the recommendations system work?
The recommendations system is pretty complicated. Since most people cannot understand code, the best way to explain it is with a high-level description.
This aims to go over the specifics of how it works while glancing over minor edge cases or implementation details:

Definitions:
- highest rank: the highest tier that you have earned a non-plus rank (e.g. diamond rank = 4 completed diamond levels). If your highest rank is a plus rank,
the recommendations system considers your highest rank as the rank above (e.g. if i have diamond+, my highest rank is onyx)
- strong skill: skills whose level divided by its max level is relatively higher than other skills
- weak skill: skills whose level divided by its max level is relatively lower than other skills
- unique recommendation: the level is recommended for only one skill
- partial rank: you completed [x-2, x) demons in a tier, where x is the required number of demons for obtaining the rank of that tier
- skill difficulty: gddp levels are rated based on the difficulty (relative to other levels in the same tier) of each skill present in that level. the difficulty can be any of the following:
not present
below average
average
above average

High-level Overview:
recommendations are deterministic, this means they are not based on probability ("rng") or AI
levels that are either below average difficulty or not present for a skill will never be recommended for that skill

if you have no ranks, the system defaults you to "having" beginner rank
this means users with no ranks will get beginner/bronze demon recommendations

for each of your skills, you will receive recommendations based on these conditions:
- strong skills: you will receive up to 4 unique recommendations for 4 of your strongest skills from any tier above your highest rank (starting with the tier above your highest rank).
You will also receive up to 2 additional recommendations for your top 2 strongest skills from your highest partially completed rank. it will prioritize picking a level that is average
difficulty for that skill, and if there is none it will pick a level with above average difficulty for that skill.
- weakest skill: you will receive up to 1 unique recommendation for your weakest skill from either (a) the tier below your highest rank or (b) the tier of your highest rank. In scenario (a),
it will prioritize picking a level that is above average difficulty for that skill, and if there is none it will pick a level with average difficulty for that skill.
If you have no levels that can be chosen from scenario (a), then we pick a level from scenario (b). In scenario (b), it will prioritize picking a level that is rated equally as difficult
as the hardest level you've beaten in that tier for the same skill, and if there is none it will pick either an easier or harder level.
- other skills: you will receive non-unique recommendations for the rest of your skills that are not classified as "strong skills" or "weakest skill". these recommendations will come from
the tier of your highest rank. it will prioritize picking a level that is rated equally as difficult as the hardest level you've beaten in that tier for the same skill, and if there is
none it will pick either an easier or harder level.

Other Notes:
it is possible that if you have completed all levels with at least average difficulty for a skill within the tier of your highest rank that you will receive a recommendation from any tier
above for that skill. its very unlikely that anyone will encounter this scenario, but it (for the most part) ensures that you will receive recommendations for all of your skills
*/

//geode header
#include <Geode/Geode.hpp>
#include <Geode/utils/JsonValidation.hpp>

#include "./menus/DPLayer.hpp"
#include "./popups/StatsPopup.hpp"
#include "XPUtils.hpp"
#include "RecommendedUtils.hpp"

//geode namespace
using namespace geode::prelude;

void RecommendedUtils::validateLevels() {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto levels = Mod::get()->getSavedValue<matjson::Array>("recommended-levels");
	auto completedLvls = Mod::get()->getSavedValue<matjson::Array>("completed-levels");

	//check for errors
	auto jsonCheck = JsonChecker(data);

	if (jsonCheck.isError()) {
		log::info("Something went wrong validating the list data. ({})", jsonCheck.getError());

		return;
	}

	//check if data exists
	if (!data["main"].is_array() && !data["legacy"].is_array()) { return; }

	if (levels.empty() && completedLvls.size() > 0) {
		generateRecommendations();
	}
	else {

		//check if you completed a recommended level
		for (int i = 0; i < levels.size(); i++) {
			if (levels[i].is_number() && std::find(completedLvls.begin(), completedLvls.end(), levels[i].as_int()) != completedLvls.end()) {
				generateRecommendations();
				break;
			}
		}

		//check if a level was moved to legacy
		for (int i = 0; i < data["legacy"].as_array().size(); i++) {
			auto levelIDs = data["legacy"][i]["levelIDs"].as_array();
			auto mainList = data["main"][i]["levelIDs"].as_array();
			auto stop = false;

			for (int j = 0; j < levelIDs.size(); j++) {
				if (!levelIDs[j].is_number()) { continue; }
				auto levelID = std::to_string(levelIDs[j].as_int());

				if (std::find(mainList.begin(), mainList.end(), levelIDs[j].as_int()) != mainList.end()) {
					generateRecommendations();
					stop = true;
					break;
				}
			}

			if (stop) { break; }
		}
	}

	return;
}

void RecommendedUtils::generateRecommendations() {
	log::info("Generating Recommendations...");

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto completedLvls = Mod::get()->getSavedValue<matjson::Array>("completed-levels");
	auto skillIDs = XPUtils::skillIDs;

	XPUtils::getXP();

	//check for errors
	auto jsonCheck = JsonChecker(data);

	if (jsonCheck.isError()) {
		log::info("Something went wrong validating the list data. ({})", jsonCheck.getError());

		return;
	}

	//Get Highest Rank
	auto highest = 0; //Defaults to Beginner
	for (int i = 0; i < data["main"].as_array().size(); i++) {
		//Check For Non-Plus Rank
		if (StatsPopup::getPercentToRank(i, false) >= 1.f) {
			highest = i;
		}

		//Check For Plus Rank
		if (StatsPopup::getPercentToRank(i, true) >= 1.f) {
			highest = std::min(i + 1, static_cast<int>(data["main"].as_array().size() - 1));
		}

	}
	log::info("highest rank: {}", highest);

	//Get Highest Partial Rank
	auto highestPartial = -1;
	for (int i = 0; i < data["main"].as_array().size(); i++) {
		if (hasPartial(i)) {
			highestPartial = i;
		}
	}
	log::info("highest partial rank: {}", highestPartial);

	//Sort Skills
	std::vector<int> skills = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //Highest -> Lowest
	for (int i = 0; i < skills.size(); i++) {

		auto xp = Mod::get()->getSavedValue<matjson::Array>("xp", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
		
		for (int j = 0; j < skillIDs.size(); j++) {
			if (i == 0 && xp[j].as_double() > xp[skills[i]].as_double()) {
				skills[i] = j;
			}
			else if (xp[j].as_double() > xp[skills[i]].as_double() && xp[j].as_double() < xp[skills[i - 1]].as_double() && i > 0) {
				skills[i] = j;
			}
		}
	}
	log::info("sorted skills: (highest) {} (lowest)", skills);
	
	auto stop = false;
	std::vector<int> recommendations = {};

	//Generate Strong Skill Recommendations
	//4 unique recommendations for your top 4 strongest skills from any higher tier
	//2 additional recommendations for your top 2 strongest skills from your highest partially completed rank
	//skill difficulty order: average (2) -> above average (3)

	for (int i = 0; i < 4; i++) { // 4 strongest skills

		auto currentSkill = skillIDs[skills[i]];

		//log::info("skill {}: {}", i, currentSkill);

		stop = false;

		for (int j = std::min(highest + 1, static_cast<int>(data["main"].as_array().size() - 1)); j < data["main"].as_array().size(); j++) { //search the tier above your highest, it'll move on to the next tier if it can't find anything
			//log::info("current tier: {}", data["main"][j]["name"].as_string());
			auto levelIDs = data["main"][j]["levelIDs"].as_array();

			for (int rating = 2; rating <= 3; rating++) { //cycles through ratings. if it can't find a rating of 2 for any level with that skill, it starts searching for 3

				//log::info("looking for rating: {}", rating);

				for (int k = 0; k < levelIDs.size(); k++) { //search through all levels of that tier
					auto id = std::to_string(levelIDs[k].as_int());

					if (std::find(recommendations.begin(), recommendations.end(), levelIDs[k].as_int()) == recommendations.end()
						&& std::find(completedLvls.begin(), completedLvls.end(), levelIDs[k].as_int()) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
						if (data["level-data"][id]["xp"][currentSkill].is_number() && data["level-data"][id]["xp"][currentSkill].as_int() == rating) { // found a level that matches criteria, stop searching and go to the next skill
							//log::info("found level with id: {}", levelIDs[k].as_int());
							recommendations.push_back(levelIDs[k].as_int());
							stop = true; 
							break;
						}
					}

				}

				if (stop) { break; }
			}

			if (stop) { break; }
		}
	}

	if (highestPartial > -1) { // make sure your highest partial exists
		for (int i = 0; i < 2; i++) { //two strongest skills

			auto currentSkill = skillIDs[skills[i]];

			//log::info("skill {}: {}", i, currentSkill);

			stop = false;

			//log::info("current tier: {}", data["main"][highestPartial]["name"].as_string());
			auto levelIDs = data["main"][highestPartial]["levelIDs"].as_array();

			for (int rating = 2; rating <= 3; rating++) { //cycles through ratings. if it can't find a rating of 2 for any level with that skill, it starts searching for 3

				//log::info("looking for rating: {}", rating);

				for (int k = 0; k < levelIDs.size(); k++) { //search through all levels of that tier
					auto id = std::to_string(levelIDs[k].as_int());

					if (std::find(recommendations.begin(), recommendations.end(), levelIDs[k].as_int()) == recommendations.end()
						&& std::find(completedLvls.begin(), completedLvls.end(), levelIDs[k].as_int()) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
						if (data["level-data"][id]["xp"][currentSkill].is_number() && data["level-data"][id]["xp"][currentSkill].as_int() == rating) { // found a level that matches criteria, stop searching and go to the next skill
							//log::info("found level with id: {}", levelIDs[k].as_int());
							recommendations.push_back(levelIDs[k].as_int());
							stop = true;
							break;
						}
					}

				}

				if (stop) { break; }
			}
		}
	}

	//Generate Weakest Skill Recommendation
	// 1 unique recommendation for your weakest skill
	// a:
	// tier below highest rank
	// skill difficulty order: 3 -> 2
	// b:
	// tier of highest rank
	// reverse index order
	// skill difficulty order: 3 -> 2 -> 1

	stop = false;

	for (int i = 0; i <= 1; i++) { //0 = a, 1 = b

		auto currentSkill = skillIDs[skills[8]]; // always weakest

		//log::info("skill: {}", currentSkill);

		stop = false;

		if (i == 0 && highest > 0) { // if scenario a and the tier below is available
			//log::info("current tier: {}", data["main"][highest - 1]["name"].as_string());
			auto levelIDs = data["main"][highest - 1]["levelIDs"].as_array();

			for (int rating = 3; rating >= 2; rating--) { //cycles through ratings. if it can't find a rating of 3 for any level with that skill, it starts searching for 2

				//log::info("looking for rating: {}", rating);

				for (int k = 0; k < levelIDs.size(); k++) { //search through all levels of that tier
					auto id = std::to_string(levelIDs[k].as_int());

					if (std::find(recommendations.begin(), recommendations.end(), levelIDs[k].as_int()) == recommendations.end()
						&& std::find(completedLvls.begin(), completedLvls.end(), levelIDs[k].as_int()) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
						if (data["level-data"][id]["xp"][currentSkill].is_number() && data["level-data"][id]["xp"][currentSkill].as_int() == rating) { // found a level that matches criteria, stop searching
							//log::info("found level with id: {}", levelIDs[k].as_int());
							recommendations.push_back(levelIDs[k].as_int());
							stop = true;
							break;
						}
					}

				}

				if (stop) { break; }
			}
		}
		else { // scenario b
			//log::info("current tier: {}", data["main"][highest]["name"].as_string());
			auto levelIDs = data["main"][highest]["levelIDs"].as_array();

			for (int rating = 3; rating > 0; rating--) { //cycles through ratings. if it can't find a rating of 3 for any level with that skill, it starts searching for 2, then 1

				//log::info("looking for rating: {}", rating);

				for (int k = levelIDs.size() - 1; k >= 0; k--) { //search through all levels of that tier (reverse order)
					auto id = std::to_string(levelIDs[k].as_int());

					if (std::find(recommendations.begin(), recommendations.end(), levelIDs[k].as_int()) == recommendations.end()
						&& std::find(completedLvls.begin(), completedLvls.end(), levelIDs[k].as_int()) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
						if (data["level-data"][id]["xp"][currentSkill].is_number() && data["level-data"][id]["xp"][currentSkill].as_int() == rating) { // found a level that matches criteria, stop searching
							//log::info("found level with id: {}", levelIDs[k].as_int());
							recommendations.push_back(levelIDs[k].as_int());
							stop = true;
							break;
						}
					}

				}

				if (stop) { break; }
			}
		}

		if (stop) { break; } // if one scenario succeeds, don't run the second one
	}
	
	//Other Recommendations
	// use the rest of the skills (index 4-7, 4 skills remaining)
	// tier of highest rank
	// reverse index order
	// skill difficulty order: 3 -> 2 -> 1

	for (int i = 4; i <= 7; i++) { //all remaining skills
		auto currentSkill = skillIDs[skills[i]];

		//log::info("skill: {}", currentSkill);

		stop = false;

		//log::info("current tier: {}", data["main"][highest]["name"].as_string());
		auto levelIDs = data["main"][highest]["levelIDs"].as_array();

		for (int rating = 3; rating > 0; rating--) { //cycles through ratings. if it can't find a rating of 3 for any level with that skill, it starts searching for 2, then 1

			//log::info("looking for rating: {}", rating);

			for (int k = levelIDs.size() - 1; k >= 0; k--) { //search through all levels of that tier (reverse order)
				auto id = std::to_string(levelIDs[k].as_int());

				if (std::find(recommendations.begin(), recommendations.end(), levelIDs[k].as_int()) == recommendations.end()
					&& std::find(completedLvls.begin(), completedLvls.end(), levelIDs[k].as_int()) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
					if (data["level-data"][id]["xp"][currentSkill].is_number() && data["level-data"][id]["xp"][currentSkill].as_int() == rating) { // found a level that matches criteria, stop searching
						//log::info("found level with id: {}", levelIDs[k].as_int());
						recommendations.push_back(levelIDs[k].as_int());
						stop = true;
						break;
					}
				}

			}

			if (stop) { break; }
		}
	}

	log::info("Recommendations: {}", recommendations);
	matjson::Array r;
	for (auto level : recommendations) {
		r.push_back(level);
	}
	Mod::get()->setSavedValue<matjson::Array>("recommended-levels", r);
	return;
}

bool RecommendedUtils::hasPartial(int id) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	//check for errors
	auto jsonCheck = JsonChecker(data);

	if (jsonCheck.isError()) {
		log::info("Something went wrong validating the list data. ({})", jsonCheck.getError());

		return false;
	}

	auto saveID = data["main"][id]["saveID"].as_string();
	auto reqLevels = data["main"][id]["reqLevels"].as_int();
	auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

	if (listSave.progress >= reqLevels - 2 && listSave.progress < reqLevels && listSave.progress > 0) {
		return true;
	}

	return false;
}