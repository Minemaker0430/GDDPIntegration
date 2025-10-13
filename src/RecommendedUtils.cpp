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
	auto levels = Mod::get()->getSavedValue<std::vector<int>>("recommended-levels");
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return;
	}

	//check if data exists
	if (!data["main"].isArray() && !data["legacy"].isArray()) { return; }

	if (levels.empty() && completedLvls.size() > 0) {
		generateRecommendations();
	}
	else {

		//check if you completed a recommended level
		for (auto lvl : levels) {
			if (std::find(completedLvls.begin(), completedLvls.end(), lvl) != completedLvls.end()) {
				generateRecommendations();
				break;
			}
		}

		//check if a level was moved to legacy
		for (int i = 0; i < data["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
			auto levelIDs = data["legacy"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
			auto mainList = data["main"][data["legacy"][i]["mainPack"].as<int>().unwrapOr(0)]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
			auto stop = false;

			for (auto lvl : levelIDs) {
				//auto levelID = std::to_string(lvl);

				if (std::find(mainList.begin(), mainList.end(), lvl) != mainList.end()) {
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

std::vector<int> RecommendedUtils::sortSkills(std::vector<float> xp) {
	
	//assign skills to key
	std::vector<SkillSort> out(XPUtils::skillIDs.size(), SkillSort(0, 0.f));

	for (int i = 0; i < XPUtils::skillIDs.size(); i++) {
		out[i] = SkillSort(i, xp[i]);
	}

	//sort
	std::sort(out.begin(), out.end(), std::greater<SkillSort>());

	//convert to int vector and return
	std::vector<int> result;
	for (SkillSort value : out) {
		result.push_back(value.key);
	}

	return result;
}

void RecommendedUtils::generateRecommendations() {
	log::info("Generating Recommendations...");

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
	auto skillIDs = XPUtils::skillIDs;

	XPUtils::getXP();

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return;
	}

	//Get Highest Rank
	auto highest = 0; //Defaults to Beginner
	for (int i = 0; i < data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
		//Check For Non-Plus Rank
		if (StatsPopup::getPercentToRank(i, false) >= 1.f) {
			highest = i;
		}

		//Check For Plus Rank
		if (StatsPopup::getPercentToRank(i, true) >= 1.f) {
			highest = std::min(i + 1, static_cast<int>(data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size() - 1));
		}

	}
	log::info("highest rank: {}", highest);

	//Get Highest Partial Rank
	auto highestPartial = -1;
	for (int i = 0; i < data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
		if (hasPartial(i)) {
			highestPartial = i;
		}
	}
	log::info("highest partial rank: {}", highestPartial);

	//Sort Skills
	auto xp = Mod::get()->getSavedValue<std::vector<float>>("xp", { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f });
	std::vector<int> skills = sortSkills(xp); //Highest -> Lowest
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

		auto numOfPacks = data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size();

		for (int j = std::min(highest + 1, static_cast<int>(numOfPacks - 1)); j < numOfPacks; j++) { //search the tier above your highest, it'll move on to the next tier if it can't find anything
			//log::info("current tier: {}", data["main"][j]["name"].as_string());
			auto levelIDs = data["main"][j]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();

			for (int rating = 2; rating <= 3; rating++) { //cycles through ratings. if it can't find a rating of 2 for any level with that skill, it starts searching for 3

				//log::info("looking for rating: {}", rating);

				for (auto lvl : levelIDs) { //search through all levels of that tier
					auto id = std::to_string(lvl);

					if (std::find(recommendations.begin(), recommendations.end(), lvl) == recommendations.end()
						&& std::find(completedLvls.begin(), completedLvls.end(), lvl) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
						if (data["level-data"][id]["xp"][currentSkill].isNumber() && data["level-data"][id]["xp"][currentSkill].as<int>().unwrapOr(0) == rating) { // found a level that matches criteria, stop searching and go to the next skill
							//log::info("found level with id: {}", levelIDs[k].as_int());
							recommendations.push_back(lvl);
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
			auto levelIDs = data["main"][highestPartial]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();

			for (int rating = 2; rating <= 3; rating++) { //cycles through ratings. if it can't find a rating of 2 for any level with that skill, it starts searching for 3

				//log::info("looking for rating: {}", rating);

				for (auto lvl : levelIDs) { //search through all levels of that tier
					auto id = std::to_string(lvl);

					if (std::find(recommendations.begin(), recommendations.end(), lvl) == recommendations.end()
						&& std::find(completedLvls.begin(), completedLvls.end(), lvl) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
						if (data["level-data"][id]["xp"][currentSkill].isNumber() && data["level-data"][id]["xp"][currentSkill].as<int>().unwrapOr(0) == rating) { // found a level that matches criteria, stop searching and go to the next skill
							//log::info("found level with id: {}", levelIDs[k].as_int());
							recommendations.push_back(lvl);
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

	for (int i = 0; i <= 1; i++) { //0 = a, 1 = b

		auto currentSkill = skillIDs[skills[skillIDs.size() - 1]]; // always weakest

		//log::info("skill: {}", currentSkill);

		stop = false;

		if (i == 0 && highest > 0) { // if scenario a and the tier below is available
			//log::info("current tier: {}", data["main"][highest - 1]["name"].as_string());
			auto levelIDs = data["main"][highest - 1]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();

			for (int rating = 3; rating >= 2; rating--) { //cycles through ratings. if it can't find a rating of 3 for any level with that skill, it starts searching for 2

				//log::info("looking for rating: {}", rating);

				for (auto lvl : levelIDs) { //search through all levels of that tier
					auto id = std::to_string(lvl);

					if (std::find(recommendations.begin(), recommendations.end(), lvl) == recommendations.end()
						&& std::find(completedLvls.begin(), completedLvls.end(), lvl) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
						if (data["level-data"][id]["xp"][currentSkill].isNumber() && data["level-data"][id]["xp"][currentSkill].as<int>().unwrapOr(0) == rating) { // found a level that matches criteria, stop searching
							//log::info("found level with id: {}", levelIDs[k].as_int());
							recommendations.push_back(lvl);
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
			auto levelIDs = data["main"][highest]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();

			for (int rating = 3; rating > 0; rating--) { //cycles through ratings. if it can't find a rating of 3 for any level with that skill, it starts searching for 2, then 1

				//log::info("looking for rating: {}", rating);
				std::vector<int> revLevelIDs = levelIDs;
				std::reverse(revLevelIDs.begin(), revLevelIDs.end());
				for (int lvl : revLevelIDs) { //search through all levels of that tier (reverse order)
					//auto lvl = levelIDs[k];
					auto id = std::to_string(lvl);

					if (std::find(recommendations.begin(), recommendations.end(), lvl) == recommendations.end()
						&& std::find(completedLvls.begin(), completedLvls.end(), lvl) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
						if (data["level-data"][id]["xp"][currentSkill].isNumber() && data["level-data"][id]["xp"][currentSkill].as<int>().unwrapOr(0) == rating) { // found a level that matches criteria, stop searching
							//log::info("found level with id: {}", levelIDs[k].as_int());
							recommendations.push_back(lvl);
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
		auto levelIDs = data["main"][highest]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();

		for (int rating = 3; rating > 0; rating--) { //cycles through ratings. if it can't find a rating of 3 for any level with that skill, it starts searching for 2, then 1

			//log::info("looking for rating: {}", rating);

			std::vector<int> revLevelIDs = levelIDs;
			std::reverse(revLevelIDs.begin(), revLevelIDs.end());
			for (int lvl : revLevelIDs) { //search through all levels of that tier (reverse order)
				auto id = std::to_string(lvl);

				if (std::find(recommendations.begin(), recommendations.end(), lvl) == recommendations.end()
					&& std::find(completedLvls.begin(), completedLvls.end(), lvl) == completedLvls.end()) { //make sure the level isn't already in recommendations or completed
					if (data["level-data"][id]["xp"][currentSkill].isNumber() && data["level-data"][id]["xp"][currentSkill].as<int>().unwrapOr(0) == rating) { // found a level that matches criteria, stop searching
						//log::info("found level with id: {}", levelIDs[k].as_int());
						recommendations.push_back(lvl);
						stop = true;
						break;
					}
				}

			}

			if (stop) { break; }
		}
	}

	log::info("Recommendations: {}", recommendations);
	Mod::get()->setSavedValue<std::vector<int>>("recommended-levels", recommendations);
	return;
}

bool RecommendedUtils::hasPartial(int id) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return false;
	}

	auto saveID = data["main"][id]["saveID"].asString().unwrapOr("null");
	auto reqLevels = data["main"][id]["reqLevels"].asInt().unwrapOr(999);
	auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

	if (listSave.progress >= reqLevels - 2 && listSave.progress < reqLevels && listSave.progress > 0) {
		return true;
	}

	return false;
}