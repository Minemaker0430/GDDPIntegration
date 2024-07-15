//geode header
#include <Geode/Geode.hpp>

#include <Geode/utils/JsonValidation.hpp>
#include <Geode/modify/LevelCell.hpp>
#include "../DPLayer.hpp"
#include "../ListManager.hpp"

//geode namespace
using namespace geode::prelude;

//modify level cells
class $modify(DemonProgression, LevelCell) {

	static void onModify(auto & self) {
		static_cast<void>(self.setHookPriority("LevelCell::loadCustomLevelCell", -42));
	}

	void skillInfoPopup(CCObject* target) {
		auto btn = static_cast<CCMenuItemSpriteExtra*>(target);
		auto skillID = btn->getID();
		
		auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("skillset-info");

		FLAlertLayer::create(
			skillsetData[skillID]["display-name"].as_string().c_str(),
			skillsetData[skillID]["description"].as_string().c_str(),
			"OK"
		)->show();

		return;
	}

	void loadCustomLevelCell() {
		LevelCell::loadCustomLevelCell();

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) {
			inGDDP = true;
		}
		
		//check for errors
		auto jsonCheck = JsonChecker(data);

		if (jsonCheck.isError()) {
			log::info("Something went wrong validating the list data. ({})", jsonCheck.getError());

			return;
		}

		//log::info("{}", inGDDP);

		if (inGDDP && (data["level-data"].contains(std::to_string(this->m_level->m_levelID.value())) || Mod::get()->getSettingValue<bool>("all-demons-rated"))) {

			//if not on the GDDP or GDDL, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars == 10 && ListManager::getSpriteName(this->m_level) == "") {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return;
				}
			}

			//if not a demon level that's registered on the gddp, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars != 10) {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return;
				}
			}

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);

			std::string saveID = "null";
			if (type == "main") {
				if (!data["main"][id]["saveID"].is_null()) { saveID = data["main"][id]["saveID"].as_string(); }
			}

			auto hasRank = Mod::get()->getSavedValue<ListSaveFormat>(saveID).hasRank;

			auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
			auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("skillset-info", matjson::parse("{\"unknown\": {\"display-name\": \"Unknown\",\"description\": \"This skill does not have a description.\",\"sprite\": \"DP_Skill_Unknown\"}}"));

			//check for errors
			auto jsonCheck2 = JsonChecker(skillsetData);

			if (jsonCheck2.isError()) {
				log::info("Something went wrong validating the skillset data. ({})", jsonCheck2.getError());

				return;
			}

			int gddpDiff = 0;
			matjson::Array skillsets = {};
			auto levelID = std::to_string(this->m_level->m_levelID.value());

			if (data["level-data"].contains(levelID)) {
				if (!data["level-data"][levelID]["difficulty"].is_null()) { gddpDiff = data["level-data"][levelID]["difficulty"].as_int(); }
				if (!data["level-data"][levelID]["skillsets"].is_null()) { skillsets = data["level-data"][levelID]["skillsets"].as_array(); }

				if (typeinfo_cast<CCSprite*>(this->m_mainLayer->getChildByID("completed-icon"))) {
					auto completedLvls = Mod::get()->getSavedValue<matjson::Array>("completed-levels");

					if (std::find(completedLvls.begin(), completedLvls.end(), this->m_level->m_levelID.value()) == completedLvls.end()) {
						completedLvls.insert(completedLvls.begin(), this->m_level->m_levelID.value());
						Mod::get()->setSavedValue<matjson::Array>("completed-levels", completedLvls);
					}
				}
			}

			//skillset badges
			if (Mod::get()->getSettingValue<bool>("skillset-badges") && skillsets.size() > 0 && Mod::get()->getSettingValue<bool>("show-skills-in-list")) {
				auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer"));

				GJDifficultySprite* diffSpr;
				if (this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer")) {
					diffSpr = typeinfo_cast<GJDifficultySprite*>(layer->getChildByID("grd-demon-icon-layer")->getChildByID("difficulty-sprite"));
				}
				else if (this->getChildByID("main-layer")->getChildByID("difficulty-container")) {
					diffSpr = typeinfo_cast<GJDifficultySprite*>(layer->getChildByID("difficulty-container")->getChildByID("difficulty-sprite"));
				}

				//create the skillset menu
				auto skillMenu = CCMenu::create();
				auto skillLayout = AxisLayout::create();
				skillLayout->setAxis(Axis::Column);
				skillMenu->setLayout(skillLayout, true, false);
				skillMenu->setID("skillset-menu"_spr);
				if (layer->getChildByID("level-place")) {
					skillMenu->setPosition({ diffSpr->getPositionX() + 45, diffSpr->getPositionY() - 17 });
					skillMenu->setScale(0.4f);
				}
				else {
					skillMenu->setPosition({ diffSpr->getPositionX() + 32, diffSpr->getPositionY() + 6 });
					skillMenu->setScale(0.5f);
				}
				skillMenu->setZOrder(42);
				skillMenu->setContentSize({ 31.5f, 90.0f });
				if (skillsets.size() >= 3) {
					skillMenu->setAnchorPoint({ 0.7f, 0.5f });
				}
				else {
					skillMenu->setAnchorPoint({ 0.5f, 0.5f });
				}
				
				//add skillset buttons
				for (int i = 0; i < skillsets.size(); i++) {

					std::string skillID = skillsets[i].as_string();

					//check data entry
					if (!skillsetData.contains(skillID)) {
						skillID = "unknown";
					}

					//get data
					auto name = skillsetData[skillID]["display-name"].as_string();
					auto desc = skillsetData[skillID]["description"].as_string();
					auto spriteName = fmt::format("{}.png", skillsetData[skillID]["sprite"].as_string());
					
					CCSprite* sprite;
					if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr) {
						spriteName = fmt::format("{}.png", skillsetData["unknown"]["sprite"].as_string());
						sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
					}
					else {
						sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
					}

					auto skillsetBtn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(DemonProgression::skillInfoPopup));
					skillsetBtn->setID(skillID);
					skillMenu->addChild(skillsetBtn);
				}

				skillMenu->updateLayout(false);
				layer->addChild(skillMenu);

			}

			//custom difficulty faces
			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) {

				//check for grandpa demon
				if (this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer") && Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
					auto diffIcon = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer")->getChildByID("difficulty-sprite"));
					auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer"));

					diffIcon->setVisible(false);

					//find and identify the grandpa demon icons
					for (int i = 0; i < layer->getChildrenCount(); i++) {
						if (getChildOfType<CCSprite>(layer, i)) {
							if (!(getChildOfType<CCSprite>(layer, i)->getID() != "") && getChildOfType<CCSprite>(layer, i)->getTag() != 69420) {
								getChildOfType<CCSprite>(layer, i)->setID("grd-difficulty-face");
							}
						}
					}

					if (layer->getChildByID("grd-infinity")) { layer->getChildByID("grd-infinity")->setVisible(false); }
					
					if (layer->getChildByTag(69420)) {
						if ((this->m_level->m_isEpic == 1) && Mod::get()->getSettingValue<bool>("replace-epic")) { layer->getChildByTag(69420)->setVisible(false); }
					}

					if (layer->getChildByID("grd-difficulty-face")) {
						layer->getChildByID("grd-difficulty-face")->removeMeAndCleanup();
					}

					std::string sprite = "DP_Beginner";
					std::string plusSprite = "DP_BeginnerPlus";

					if (Mod::get()->getSettingValue<bool>("all-demons-rated") && !data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
						sprite = ListManager::getSpriteName(this->m_level);
						plusSprite = fmt::format("{}Plus", sprite);
					}
					else {
						sprite = data["main"][gddpDiff]["sprite"].as_string();
						plusSprite = data["main"][gddpDiff]["plusSprite"].as_string();
					}
					
					//fallbacks
					if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", sprite)).data()) == nullptr) {
						sprite = "DP_Invisible";
					}

					if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", plusSprite)).data()) == nullptr) {
						plusSprite = "DP_Invisible";
					}

					std::string fullSpr = fmt::format("{}SmallText.png", sprite);
					std::string fullPlusSpr = fmt::format("{}SmallText.png", plusSprite);

					if (sprite != "DP_Invisible") {
						auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr).data());

						if (this->m_level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible") {
							typeinfo_cast<CCSprite*>(diffIcon->getChildren()->objectAtIndex(0))->setVisible(false);
							customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
						}

						if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible") {
							customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
						}

						customSpr->setID("gddp-difficulty");
						customSpr->setAnchorPoint({ 0.5f, 0.5f });
						customSpr->setPosition({ diffIcon->getPositionX() + 0.5f, diffIcon->getPositionY() });
						customSpr->setZOrder(5);

						layer->addChild(customSpr);
					}
				}
				//typical list layer
				else if (this->getChildByID("main-layer")->getChildByID("difficulty-container")) {
					auto diffIcon = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("main-layer")->getChildByID("difficulty-container")->getChildByID("difficulty-sprite"));
					auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer")->getChildByID("difficulty-container"));
					typeinfo_cast<GJDifficultySprite*>(layer->getChildByID("difficulty-sprite"))->setOpacity(0);

					std::string sprite = "DP_Beginner";
					std::string plusSprite = "DP_BeginnerPlus";

					if (Mod::get()->getSettingValue<bool>("all-demons-rated") && !data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
						sprite = ListManager::getSpriteName(this->m_level);
						plusSprite = fmt::format("{}Plus", sprite);
					}
					else {
						sprite = data["main"][gddpDiff]["sprite"].as_string();
						plusSprite = data["main"][gddpDiff]["plusSprite"].as_string();
					}

					//fallbacks
					if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", sprite)).data()) == nullptr) {
						sprite = "DP_Invisible";
					}

					if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", plusSprite)).data()) == nullptr) {
						plusSprite = "DP_Invisible";
					}

					std::string fullSpr = fmt::format("{}SmallText.png", sprite);
					std::string fullPlusSpr = fmt::format("{}SmallText.png", plusSprite);

					if (sprite != "DP_Invisible") {
						auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr).data());

						if (this->m_level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible") {
							typeinfo_cast<CCSprite*>(diffIcon->getChildren()->objectAtIndex(0))->setVisible(false);
							customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
						}

						if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible") {
							customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
						}

						customSpr->setID("gddp-difficulty");
						customSpr->setAnchorPoint({ 0.5f, 0.5f });
						customSpr->setPosition({ diffIcon->getPositionX() + 0.5f, diffIcon->getPositionY() });
						customSpr->setZOrder(5);

						layer->addChild(customSpr);
					}
				}

			}
		}

		return;
	}
};