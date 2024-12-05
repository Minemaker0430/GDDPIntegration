//geode header
#include <Geode/Geode.hpp>

#include <Geode/utils/JsonValidation.hpp>
#include <Geode/modify/LevelCell.hpp>
#include "../menus/DPLayer.hpp"
//#include "../ListManager.hpp"

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
			skillsetData[skillID]["display-name"].asString().unwrapOr("null").c_str(),
			skillsetData[skillID]["description"].asString().unwrapOr("erm that\'s awkward").c_str(),
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
		auto jsonCheck = checkJson(data, "");

		if (!jsonCheck.ok()) {
			log::info("Something went wrong validating the GDDP list data.");

			return;
		}

		//log::info("{}", inGDDP);

		if (inGDDP && data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {

			//if not on the GDDP or GDDL, return
			/*if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars == 10 && ListManager::getSpriteName(this->m_level) == "") {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return true;
				}
			}

			//if not a non-demon level that's registered on the gddp, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars != 10) {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return true;
				}
			}*/

			//if a gddp level that's only in a monthly pack, return
			if (Mod::get()->getSettingValue<bool>("hide-monthly-outside") && !Mod::get()->getSavedValue<bool>("in-gddp")) {

				//check monthly, if check returns with nothing, skip the rest
				auto isMonthly = false;
				for (int i = 0; i < data["monthly"].as<std::vector<matjson::Value>>().unwrapOrDefault().size(); i++) {
					auto monthlyPack = data["monthly"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
					if (std::find(monthlyPack.begin(), monthlyPack.end(), this->m_level->m_levelID.value()) != monthlyPack.end()) {
						isMonthly = true;
						break;
					}	
				}
				
				if (isMonthly) {
					auto uniqueMonthly = true; //false = level is in main/legacy/bonus, so don't return if false

					//check main
					for (int i = 0; i < data["main"].as<std::vector<matjson::Value>>().unwrapOrDefault().size(); i++) {
						auto pack = data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
						if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
							uniqueMonthly = false;
							break;
						}
					}

					//check legacy
					if (uniqueMonthly) {
						for (int i = 0; i < data["legacy"].as<std::vector<matjson::Value>>().unwrapOrDefault().size(); i++) {
							auto pack = data["legacy"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
							if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
								uniqueMonthly = false;
								break;
							}
						}
					}

					//check bonus
					if (uniqueMonthly) {
						for (int i = 0; i < data["bonus"].as<std::vector<matjson::Value>>().unwrapOrDefault().size(); i++) {
							auto pack = data["bonus"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
							if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
								uniqueMonthly = false;
								break;
							}
						}
					}

					if (uniqueMonthly) { return; }
				}
			}

			//if a gddp level that's only in a bonus pack, return
			if (Mod::get()->getSettingValue<bool>("hide-bonus-outside") && !Mod::get()->getSavedValue<bool>("in-gddp")) {

				//check bonus, if check returns with nothing, skip the rest
				auto isBonus = false;
				for (int i = 0; i < data["bonus"].as<std::vector<matjson::Value>>().unwrapOrDefault().size(); i++) {
					auto bonusPack = data["bonus"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
					if (std::find(bonusPack.begin(), bonusPack.end(), this->m_level->m_levelID.value()) != bonusPack.end()) {
						isBonus = true;
						break;
					}	
				}
				
				if (isBonus) {
					auto uniqueBonus = true; //false = level is in main/legacy, so don't return if false

					//check main
					for (int i = 0; i < data["main"].as<std::vector<matjson::Value>>().unwrapOrDefault().size(); i++) {
						auto pack = data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
						if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
							uniqueBonus = false;
							break;
						}
					}

					//check legacy
					if (uniqueBonus) {
						for (int i = 0; i < data["legacy"].as<std::vector<matjson::Value>>().unwrapOrDefault().size(); i++) {
							auto pack = data["legacy"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
							if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
								uniqueBonus = false;
								break;
							}
						}
					}

					if (uniqueBonus) { return; }
				}
			}

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);

			std::string saveID = "null";
			if (type == "main") {
				if (!data["main"][id]["saveID"].isNull()) { saveID = data["main"][id]["saveID"].asString().unwrapOr("null"); }
			}

			auto hasRank = Mod::get()->getSavedValue<ListSaveFormat>(saveID).hasRank;

			auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
			auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("skillset-info", matjson::makeObject({
				{"unknown", matjson::makeObject({
						{ "display-name", "Unknown" },
						{ "description", "This skill does not have a description." },
						{ "sprite", "DP_Skill_Unknown" }
					})
				}
			}));

			//check for errors
			auto jsonCheck2 = checkJson(skillsetData, "");

			if (!jsonCheck2.ok()) {
				log::info("Something went wrong validating the skillset data.");

				return;
			}

			int gddpDiff = 0;
			std::vector<std::string> skillsets = {};
			auto levelID = std::to_string(this->m_level->m_levelID.value());

			if (data["level-data"].contains(levelID)) {
				if (!data["level-data"][levelID]["difficulty"].isNull()) { gddpDiff = data["level-data"][levelID]["difficulty"].as<int>().unwrapOr(0); }
				if (!data["level-data"][levelID]["skillsets"].isNull()) { skillsets = data["level-data"][levelID]["skillsets"].as<std::vector<std::string>>().unwrapOrDefault(); }

				if (typeinfo_cast<CCSprite*>(this->m_mainLayer->getChildByID("completed-icon"))) {
					auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

					if (std::find(completedLvls.begin(), completedLvls.end(), this->m_level->m_levelID.value()) == completedLvls.end()) {
						completedLvls.insert(completedLvls.begin(), this->m_level->m_levelID.value());
						Mod::get()->setSavedValue<std::vector<int>>("completed-levels", completedLvls);
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
				skillLayout->setAxisReverse(true);
				skillMenu->setLayout(skillLayout, true, false);
				skillMenu->setID("skillset-menu"_spr);
				if (layer->getChildByID("level-place")) {
					if (!layer->getChildByID("level-place")->isVisible()) {
						skillMenu->setPosition({ 25.f, -12.f });
					}
					else {
						skillMenu->setPosition({ 45.f, -12.f });
					}
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

					std::string skillID = skillsets[i];

					//check data entry
					if (!skillsetData.contains(skillID)) {
						skillID = "unknown";
					}

					//get data
					auto name = skillsetData[skillID]["display-name"].asString().unwrapOr("null");
					auto desc = skillsetData[skillID]["description"].asString().unwrapOr("erm that\'s awkward");
					auto spriteName = fmt::format("{}.png", skillsetData[skillID]["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
					
					CCSprite* sprite;
					if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr) {
						spriteName = fmt::format("{}.png", skillsetData["unknown"]["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
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
						if (layer->getChildByType<CCSprite>(i)) {
							if (!(layer->getChildByType<CCSprite>(i)->getID() != "") && layer->getChildByType<CCSprite>(i)->getTag() != 69420) {
								layer->getChildByType<CCSprite>(i)->setID("grd-difficulty-face");
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

					/*if (Mod::get()->getSettingValue<bool>("all-demons-rated") && !data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
						sprite = ListManager::getSpriteName(this->m_level);
						plusSprite = fmt::format("{}Plus", sprite);
					}*/
					sprite = data["main"][gddpDiff]["sprite"].asString().unwrapOr("DP_Beginner");
					plusSprite = data["main"][gddpDiff]["plusSprite"].asString().unwrapOr("DP_BeginnerPlus");
					
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

						//check if the level is recommended and the effect is enabled
						auto recommendations = Mod::get()->getSavedValue<std::vector<int>>("recommended-levels");
						if (!Mod::get()->getSettingValue<bool>("disable-recommended-effect") && std::find(recommendations.begin(), recommendations.end(), this->m_level->m_levelID.value()) != recommendations.end()) {
							auto recommendedSpr = CCSprite::createWithSpriteFrameName("DP_RecommendGlow.png"_spr);
							recommendedSpr->setPosition({ 37.f, 37.f });
							recommendedSpr->setZOrder(6);
							customSpr->addChild(recommendedSpr);
						}
					}
				}
				//typical list layer
				else if (this->getChildByID("main-layer")->getChildByID("difficulty-container")) {
					auto diffIcon = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("main-layer")->getChildByID("difficulty-container")->getChildByID("difficulty-sprite"));
					auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer")->getChildByID("difficulty-container"));
					typeinfo_cast<GJDifficultySprite*>(layer->getChildByID("difficulty-sprite"))->setOpacity(0);

					std::string sprite = "DP_Beginner";
					std::string plusSprite = "DP_BeginnerPlus";

					/*if (Mod::get()->getSettingValue<bool>("all-demons-rated") && !data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
						sprite = ListManager::getSpriteName(this->m_level);
						plusSprite = fmt::format("{}Plus", sprite);
					}*/
					sprite = data["main"][gddpDiff]["sprite"].asString().unwrapOr("DP_Beginner");
					plusSprite = data["main"][gddpDiff]["plusSprite"].asString().unwrapOr("DP_BeginnerPlus");

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

						//check if the level is recommended and the effect is enabled
						auto recommendations = Mod::get()->getSavedValue<std::vector<int>>("recommended-levels");
						if (!Mod::get()->getSettingValue<bool>("disable-recommended-effect") && std::find(recommendations.begin(), recommendations.end(), this->m_level->m_levelID.value()) != recommendations.end()) {
							auto recommendedSpr = CCSprite::createWithSpriteFrameName("DP_RecommendGlow.png"_spr);
							recommendedSpr->setPosition({ 37.f, 37.f });
							recommendedSpr->setZOrder(6);
							customSpr->addChild(recommendedSpr);
						}
					}
				}

			}
		}

		return;
	}
};