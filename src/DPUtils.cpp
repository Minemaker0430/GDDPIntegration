//geode header
#include <Geode/Geode.hpp>

#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>

#include "DPUtils.hpp"

//geode namespace
using namespace geode::prelude;

std::vector<std::string> DPUtils::substring(std::string s, std::string d) {
	std::vector<std::string> res;
	std::string delim = d;
	std::string token = "";
	for (int i = 0; i < s.size(); i++) {
		bool flag = true;
		for (int j = 0; j < delim.size(); j++) {
			if (s[i + j] != delim[j]) flag = false;
		}
		if (flag) {
			if (token.size() > 0) {
				res.push_back(token);
				token = "";
				i += delim.size() - 1;
			}
		}
		else {
			token += s[i];
		}
	}
	res.push_back(token);
	return res;
};

CCSprite* DPUtils::safeSprite(std::string name, CCSprite* fallback) {
	if (CCSprite::create(name.c_str()) == nullptr) return fallback;

	return CCSprite::create(name.c_str());
};

CCSprite* DPUtils::safeSpriteWithFrameName(std::string name, CCSprite* fallback) {
	if (CCSprite::createWithSpriteFrameName(name.c_str()) == nullptr) return fallback;

	return CCSprite::createWithSpriteFrameName(name.c_str());
};

std::vector<int>::iterator DPUtils::findInt(std::vector<int> v, int t) {
	return std::find(v.begin(), v.end(), t);
};

std::vector<std::string>::iterator DPUtils::findString(std::vector<std::string> v, std::string t) {
	return std::find(v.begin(), v.end(), t);
};

std::vector<matjson::Value>::iterator DPUtils::findJson(std::vector<matjson::Value> v, matjson::Value t) {
	return std::find(v.begin(), v.end(), t);
};

bool DPUtils::containsInt(std::vector<int> v, int t) {
	return (std::find(v.begin(), v.end(), t) != v.end());
};

bool DPUtils::containsString(std::vector<std::string> v, std::string t) {
	return (std::find(v.begin(), v.end(), t) != v.end());
};

bool DPUtils::containsJson(std::vector<matjson::Value> v, matjson::Value t) {
	return (std::find(v.begin(), v.end(), t) != v.end());
};

void DPUtils::verifyCompletedLevels() {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	std::vector<int> completedLvls;

	// check completed levels in player's save file
	auto glm = GameLevelManager::sharedState();
	auto glmCompletedLvls = glm->getCompletedLevels(false)->asExt();

	// make sure any level that should be there is there
	if (glmCompletedLvls.size() > 0) {
		for (auto i : glmCompletedLvls) {
			auto lvl = static_cast<GJGameLevel*>(i);
			auto lvlID = lvl->m_levelID.value();

			if (
				data["level-data"].contains(std::to_string(lvlID)) && 
				lvl->getNormalPercent() >= 100 && 
				!DPUtils::containsInt(completedLvls, lvlID)
			) {
				completedLvls.push_back(lvlID);
			}
		}
	}

	Mod::get()->setSavedValue<std::vector<int>>("completed-levels", completedLvls);
};