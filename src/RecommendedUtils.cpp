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
#include <winspool.h>

#include "./menus/DPLayer.hpp"
#include "./popups/StatsPopup.hpp"
#include "XPUtils.hpp"
#include "RecommendedUtils.hpp"
#include "DPUtils.hpp"

//geode namespace
using namespace geode::prelude;

void RecommendedUtils::validateLevels() {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto recommendations = Mod::get()->getSavedValue<matjson::Value>("recommended-levels");
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

	std::vector<int> levels = {};

	for (auto [key, value] : XPUtils::skills) {
		for (auto l : recommendations[key].as<std::vector<int>>().unwrapOrDefault()) {
			if (!DPUtils::containsInt(levels, l)) levels.push_back(l);
		}
	}

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return;
	}

	if (levels.empty() && completedLvls.size() > 0) generateRecommendations();
	else {

		//check if you completed a recommended level
		for (auto lvl : levels) {
			if (DPUtils::containsInt(completedLvls, lvl)) {
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

				if (DPUtils::containsInt(mainList, lvl)) {
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

std::vector<std::string> RecommendedUtils::sortSkills(matjson::Value xp) {
	
	//assign skills to key
	std::vector<SkillSort> out;
	for (auto [key, value] : XPUtils::skills) out.push_back(SkillSort(key, xp[key].as<float>().unwrapOr(0.f)));

	//sort
	std::sort(out.begin(), out.end(), std::greater<SkillSort>());

	//convert to int vector and return
	std::vector<std::string> result;
	for (SkillSort s : out) result.push_back(s.key);

	return result;
}

void RecommendedUtils::generateRecommendations() {
	log::info("Generating Recommendations...");

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

	XPUtils::getXP();

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return;
	}

	//Sort Skills
	auto xp = Mod::get()->getSavedValue<matjson::Value>("xp");
	std::vector<std::string> skills = sortSkills(xp); //Highest -> Lowest
	//log::info("sorted skills: (highest) {} (lowest)", skills);

	//Get Highest Rank
	auto highest = 0; //Defaults to Beginner
	for (int i = 0; i < data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
		//Check For Non-Plus Rank
		if (StatsPopup::getPercentToRank(i, false) >= 1.f) highest = i;

		//Check For Plus Rank
		if (StatsPopup::getPercentToRank(i, true) >= 1.f) highest = std::min(i + 1, static_cast<int>(data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size() - 1));
	}
	//log::info("highest rank: {}", highest);

	//Get Highest Partial Rank
	auto highestPartial = -1;
	auto partialDelta = 0;
	for (int i = 0; i < data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
		auto saveID = data["main"][i]["saveID"].asString().unwrapOr("null");
		auto reqLevels = data["main"][i]["reqLevels"].as<int>().unwrapOr(-1);

		if (saveID == "null") continue;

		auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);
		auto delta = listSave.progress - (reqLevels + 3);

		if (listSave.progress > 0 && i > highestPartial && delta > 0 && delta <= 2) {
			highestPartial = i;
			partialDelta = delta;
		}
	}
	//log::info("highest partial rank: {}", highestPartial);

	// get hardest demons by skill by tier
	matjson::Value hardestSkills = {};
	for (int i = 0; i < data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
		auto saveID = data["main"][i]["saveID"].asString().unwrapOr("null");
		
		matjson::Value res = {};
		for (auto s : skills) res.set(s, -1);
		hardestSkills.set(saveID, res);
		
		for (auto lvl : data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault()) {
			if (!DPUtils::containsInt(completedLvls, lvl)) continue;

			for (auto s : skills) {
				auto val = data["level-data"][std::to_string(lvl)]["xp"][s].as<int>().unwrapOr(0);
				auto hVal = hardestSkills[saveID][s].as<int>().unwrapOr(-1);
				if (val >= hVal) hardestSkills[saveID].set(s, val);
			}
		}
	}
	//log::info("hardest skills by tier: {}", hardestSkills.dump());

	// if partial < highest rank, skip partial recommendations
	if (highestPartial < highest) highestPartial = -1;
	bool hasJump = (highestPartial != -1);
	int jumpThreshold = skills.size() - partialDelta;
	//log::info("jump threshold: {}", jumpThreshold);
	
	matjson::Value recommendations = {};
	std::vector<int> used = {};

	const int numOfPacks = data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size();

	//reverse list to go weakest -> strongest
	std::reverse(skills.begin(), skills.end());

	for (int i = 0; i < skills.size(); i++) {
		auto skillID = skills[i];
		
		int start = highest;
		int end = numOfPacks - 1;

		bool isJump = (hasJump && (i >= jumpThreshold));
		bool isStrong = ((jumpThreshold - 4 <= i) && (i < jumpThreshold));
		bool isWeakest = (i == 0);
		bool isUnique = (hasJump ? isJump : isStrong);

		if (skills[0] != skills[skills.size() - 1]) {
			if (isStrong) start++;
			if (isWeakest) start--;
			if (isJump) start = highestPartial;
		}

		//log::info("strong: {}, weakest: {}, jump: {}", isStrong, isWeakest, isJump);

		// Generate Recommendation
		for (int j = std::min(start, end); j <= end; j++) {
			auto lvls = data["main"][j]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
			auto saveID = data["main"][j]["saveID"].as<std::string>().unwrapOrDefault();

			auto hardestSkillRating = std::max(hardestSkills[saveID][skills[i]].as<int>().unwrapOr(-1), 2);

			bool isTierAbove = (j == highest + 1);
			bool isTierBelow = (j == highest - 1);
			bool isTierCurrent = (j == highest);

			//log::info("above: {}, below: {}, current: {}", isTierAbove, isTierBelow, isTierCurrent);

			int ratingStart = isTierBelow ? 3 : (isTierCurrent ? hardestSkillRating : (isTierAbove ? 3 : 2));
			int step = isTierBelow ? -1 : 1;

			std::vector<int> ratingSearch = {ratingStart, cycleRating(ratingStart, step)};

			for (auto k : ratingSearch) {
				int res = 0;

				//log::info("searching for rating: {}", k);

				int lStart = (step > 0 ? 0 : lvls.size() - 1);
				int lEnd = (step > 0 ? lvls.size() - 1 : 0);

				//log::info("start: {}, end: {}", lStart, lEnd);

				for (auto l = lStart; step > 0 ? l <= lEnd : l >= lEnd; l += step) {
					auto d = lvls[l];
					if (DPUtils::containsInt(completedLvls, d)) continue; // if the level is completed, skip
							
					auto val = data["level-data"][std::to_string(d)]["xp"][skillID].as<int>().unwrapOr(0);
					if (val != k) continue; // if values don't match, skip
					if (isUnique && DPUtils::containsInt(used, d)) continue; // marked as "unique" and therefore can't qualify for another skill
					
					res = d;
					//log::info("found {} recommendation: {}", skillID, res);
					break;
				}

				if (res == 0) continue;

				auto s = recommendations[skillID].as<std::vector<int>>().unwrapOrDefault();
				s.push_back(res);
				used.push_back(res);
				recommendations.set(skillID, s);
				break;
			}

			if (recommendations[skillID].as<std::vector<int>>().unwrapOrDefault().size() > 0) break;
		}

	}

	log::info("Recommendations: {}", recommendations.dump());
	Mod::get()->setSavedValue<matjson::Value>("recommended-levels", recommendations);
	return;
}

int RecommendedUtils::cycleRating(int n, int step) {
	const int min = 2;
	const int max = 3;

	n += step;
	if (n > max) return min;
	if (n < min) return max;

	return n;
}

void RecommendedUtils::getSkillsForRecommendation(int levelID) {
	auto recommendations = Mod::get()->getSavedValue<matjson::Value>("recommended-levels");
	std::vector<std::string> skills;

	//log::info("recommended results for {}", levelID);

	for (auto [key, value] : XPUtils::skills) {
		if (DPUtils::containsInt(recommendations[key].as<std::vector<int>>().unwrapOrDefault(), levelID)) skills.push_back(key);
	}

	std::string str = "Recommended for:\n";
	for (int s = 0; s < skills.size(); s++) {
		str += fmt::format(
			"{}<cy>{}</c>{}",
			(s == skills.size() - 1 && skills.size() > 1) ? "& " : "",
			XPUtils::skills[skills[s]]["name"].asString().unwrapOr("???"),
			(s < skills.size() && skills.size() > 2) ? ", " : " "
		);
	}

	str.pop_back();

	FLAlertLayer::create(
		"Recommended",
		str.c_str(),
		"OK"
	)->show();

	return;
}