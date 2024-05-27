//geode header
#include <Geode/Geode.hpp>

#include <Geode/modify/LevelListLayer.hpp>
#include "DPLayer.hpp"

//geode namespace
using namespace geode::prelude;

//modify gddp list layout
class $modify(LevelListLayer) {
	static void onModify(auto & self) {
		static_cast<void>(self.setHookPriority("LevelListLayer::init", -42));
	}

	bool init(GJLevelList * p0) {
		if (!LevelListLayer::init(p0)) return false;

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");
		if (!inGDDP) { return true; }

		auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
		auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);
		auto reqLevels = Mod::get()->getSavedValue<int>("current-pack-requirement", 0);
		auto totalLevels = Mod::get()->getSavedValue<int>("current-pack-totalLvls", 0);

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		auto listID = data[type][id]["listID"].as_int();
		auto mainPack = 0;
		if (type == "legacy") { mainPack = data[type][id]["mainPack"].as_int(); }
		auto practiceID = 0;
		if (type == "main") { practiceID = data[type][id]["practiceID"].as_int(); }
		if (type == "legacy") { practiceID = data["main"][mainPack]["practiceID"].as_int(); }

		if (p0->m_listID == listID || p0->m_listID == practiceID) {

			log::info("{}", Mod::get()->getSavedValue<bool>("in-gddp"));

			// Gotta use getObjectAtIndex because there's no Node IDs here yet :v

			auto children = this->getChildren();

			if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
				auto bg = typeinfo_cast<CCSprite*>(children->objectAtIndex(0));
				bg->setColor({ 18, 18, 86 });
			}

			//Get Completed Levels & Store in Save Data

			//auto children = this->getChildren();
			auto menu = getChildOfType<CCMenu>(this, 0);
			auto menuChildren = menu->getChildren();

			auto commentBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(3));
			auto rateBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(4));
			auto copyBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(5));
			//auto infoBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(7));
			auto favBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(6));

			commentBtn->setVisible(true);
			rateBtn->setVisible(false);
			copyBtn->setVisible(false);
			//infoBtn->setVisible(false);
			favBtn->setVisible(false);

			auto diffIcon = getChildOfType<CCSprite>(this, 3);
			diffIcon->setVisible(false);

			//get list save
			auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(listID));

			log::info("{}", type);
			log::info("{}", id);
			log::info("{}", reqLevels);
			log::info("{}", totalLevels);

			//progress bar stuff
			if (auto progBar = getChildOfType<CCSprite>(this, 6)) {

				progBar->setID("progress-bar");

				if (auto frontSpr = typeinfo_cast<CCSprite*>(progBar->getChildren()->objectAtIndex(0))) {
					if (listSave.completed) {
						frontSpr->setColor({ 255, 255, 0 });
					}
					else if (type == "main" && !listSave.hasRank) {
						frontSpr->setColor({ 255, 84, 50 });
					}
				}
			}

			//create sprite thing
			std::string sprName = "DP_Beginner";

			if (type == "main") {
				if (listSave.hasRank) {
					sprName = fmt::format("{}.png", data[type][id]["plusSprite"].as_string());
				}
				else {
					sprName = fmt::format("{}.png", data[type][id]["sprite"].as_string());
				}
			}
			else {
				sprName = fmt::format("{}.png", data[type][id]["sprite"].as_string());
			}

			//fallbacks
			if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName.c_str())) == nullptr) {
				sprName = "DP_Invisible.png";
			}

			if (sprName != "DP_Invisible.png") {
				auto dpIcon = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName.c_str()));
				dpIcon->setPosition(diffIcon->getPosition());
				dpIcon->setZOrder(diffIcon->getZOrder());
				dpIcon->setID("gddp-difficulty");
				this->addChild(dpIcon);
			}

		}

		return true;
	}

	virtual void onBack(CCObject * sender) {
		LevelListLayer::onBack(sender);

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (inGDDP && !Mod::get()->getSavedValue<bool>("is-practice", false)) {
			auto progText = getChildOfType<CCLabelBMFont>(this, 2);
			std::string progressStr = progText->getString();

			auto packProgress = 0;

			//get number from string
			std::vector<std::string> res;
			std::string delim = "/";
			std::string token = "";
			for (int i = 0; i < progressStr.size(); i++) {
				bool flag = true;
				for (int j = 0; j < delim.size(); j++) {
					if (progressStr[i + j] != delim[j]) flag = false;
				}
				if (flag) {
					if (token.size() > 0) {
						res.push_back(token);
						token = "";
						i += delim.size() - 1;
					}
				}
				else {
					token += progressStr[i];
				}
			}
			res.push_back(token);

			packProgress = std::stoi(res[0]);

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);
			auto reqLevels = Mod::get()->getSavedValue<int>("current-pack-requirement", 0);
			auto totalLevels = Mod::get()->getSavedValue<int>("current-pack-totalLvls", 0);

			auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

			auto listID = data[type][id]["listID"].as_int();

			//get list save
			auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(listID));

			auto hasRank = false;

			auto progress = packProgress;
			if ((packProgress >= reqLevels) && (type == "main")) {
				hasRank = true;
			}

			auto completed = false;

			if ((packProgress == totalLevels) && type != "monthly") {
				completed = true;
				if (type == "main") {
					hasRank = true;
				}
			}
			else if ((type == "monthly") && (packProgress > 5)) {
				completed = true;
			}

			Mod::get()->setSavedValue<ListSaveFormat>(std::to_string(listID), ListSaveFormat { .progress = packProgress, .completed = completed, .hasRank = hasRank });
		}
	}
};