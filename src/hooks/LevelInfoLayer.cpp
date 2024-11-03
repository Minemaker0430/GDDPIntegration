//geode header
#include <Geode/Geode.hpp>

#include <Geode/utils/JsonValidation.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "../menus/DPLayer.hpp"
#include "../ListManager.hpp"
#include "../popups/XPPopup.hpp"
#include "../RecommendedUtils.hpp"

//geode namespace
using namespace geode::prelude;

//modify gddp level pages
class $modify(DemonProgression, LevelInfoLayer) {
	static void onModify(auto & self) {
		static_cast<void>(self.setHookPriority("LevelInfoLayer::init", -42));
		static_cast<void>(self.setHookPriority("LevelInfoLayer::updateLabelValues", -42));
	}

	//Custom Functions

	void openXPPopup(CCObject* target) {
		auto popup = DemonXPPopup::create(this->m_level->m_levelID.value());
		//log::info("{}", popup->m_levelID);
		popup->show();

		return;
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

	//Normal Functions

	bool init(GJGameLevel* p0, bool p1) {
		if (!LevelInfoLayer::init(p0, p1)) return false;

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		//check for errors
		auto jsonCheck = checkJson(data, "");

		if (!jsonCheck.ok()) {
			log::info("Something went wrong validating the GDDP list data.");

			return true;
		}

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) {
			inGDDP = true;
		}

		if (inGDDP && (data["level-data"].contains(std::to_string(this->m_level->m_levelID.value())) || Mod::get()->getSettingValue<bool>("all-demons-rated"))) {

			//if not on the GDDP or GDDL, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars == 10 && ListManager::getSpriteName(this->m_level) == "") {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return true;
				}
			}

			//if not a non-demon level that's registered on the gddp, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars != 10) {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return true;
				}
			}

			//if a gddp level that's only in a monthly pack, return
			if (Mod::get()->getSettingValue<bool>("hide-monthly-outside") && !Mod::get()->getSavedValue<bool>("in-gddp")) {

				//check monthly, if check returns with nothing, skip the rest
				auto isMonthly = false;
				for (int i = 0; i < data["monthly"].as_array().size(); i++) {
					auto monthlyPack = data["monthly"][i]["levelIDs"].as_array();
					if (std::find(monthlyPack.begin(), monthlyPack.end(), this->m_level->m_levelID.value()) != monthlyPack.end()) {
						isMonthly = true;
						break;
					}	
				}
				
				if (isMonthly) {
					auto uniqueMonthly = true; //false = level is in main/legacy/bonus, so don't return if false

					//check main
					for (int i = 0; i < data["main"].as_array().size(); i++) {
						auto pack = data["main"][i]["levelIDs"].as_array();
						if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
							uniqueMonthly = false;
							break;
						}
					}

					//check legacy
					if (uniqueMonthly) {
						for (int i = 0; i < data["legacy"].as_array().size(); i++) {
							auto pack = data["legacy"][i]["levelIDs"].as_array();
							if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
								uniqueMonthly = false;
								break;
							}
						}
					}

					//check bonus
					if (uniqueMonthly) {
						for (int i = 0; i < data["bonus"].as_array().size(); i++) {
							auto pack = data["bonus"][i]["levelIDs"].as_array();
							if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
								uniqueMonthly = false;
								break;
							}
						}
					}

					if (uniqueMonthly) { return true; }
				}
			}

			//if gauntlet level, return
			if (this->m_level->m_gauntletLevel || this->m_level->m_gauntletLevel2) {
				return true;
			}

			log::info("{}", Mod::get()->getSavedValue<bool>("in-gddp"));

			if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
				auto bg = typeinfo_cast<CCSprite*>(this->getChildByID("background"));
				bg->setColor({ 18, 18, 86 });
			}

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);

			std::string saveID = "null";
			if (type == "main") {
				if (!data["main"][id]["saveID"].is_null()) { saveID = data["main"][id]["saveID"].as_string(); }
			}

			auto hasRank = Mod::get()->getSavedValue<ListSaveFormat>(saveID).hasRank;

			auto diffSpr = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("difficulty-sprite"));
			
			auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("skillset-info", matjson::parse("{\"unknown\": {\"display-name\": \"Unknown\",\"description\": \"This skill does not have a description.\",\"sprite\": \"DP_Skill_Unknown\"}}"));

			//check for errors
			auto jsonCheck2 = checkJson(skillsetData, "");

			if (!jsonCheck2.ok()) {
				log::info("Something went wrong validating the skillset data.");

				return true;
			}

			int gddpDiff = 0;
			matjson::Array skillsets = {};
			auto levelID = std::to_string(this->m_level->m_levelID.value());

			if (data["level-data"].contains(levelID)) {
				if (!data["level-data"][levelID]["difficulty"].is_null()) { gddpDiff = data["level-data"][levelID]["difficulty"].as_int(); }
				if (!data["level-data"][levelID]["skillsets"].is_null()) { skillsets = data["level-data"][levelID]["skillsets"].as_array(); }

				if (this->m_level->m_normalPercent.value() == 100) {
					auto completedLvls = Mod::get()->getSavedValue<matjson::Array>("completed-levels");

					if (std::find(completedLvls.begin(), completedLvls.end(), this->m_level->m_levelID.value()) == completedLvls.end()) {
						completedLvls.insert(completedLvls.begin(), this->m_level->m_levelID.value());
						Mod::get()->setSavedValue<matjson::Array>("completed-levels", completedLvls);
					}
				}
			}

			//skillset badges
			if (Mod::get()->getSettingValue<bool>("skillset-badges") && skillsets.size() > 0) {

				//create the skillset menu
				auto skillMenu = CCMenu::create();
				auto skillLayout = AxisLayout::create();
				skillLayout->setAxis(Axis::Column);
				skillLayout->setAxisReverse(true);
				skillMenu->setLayout(skillLayout, true, false);
				skillMenu->setID("skillset-menu"_spr);
				skillMenu->setPosition({ diffSpr->getPositionX() + 14, diffSpr->getPositionY() - 26 });
				skillMenu->setZOrder(42);
				skillMenu->setContentSize({ 31.5f, 65.0f });
				skillMenu->setAnchorPoint({ 1.0f, 0.5f });
				skillMenu->setScale(0.75f);

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

				this->addChild(skillMenu);

			}

			//xp button
			if (data["level-data"][levelID]["xp"]["chokepoints"].is_number()) { //just use chokepoints, all demons with xp will have this value so it's fine
				auto xpText = CCLabelBMFont::create("XP", "bigFont.fnt");
				auto xpSpr = CircleButtonSprite::create(xpText, CircleBaseColor::Green, CircleBaseSize::Small);
				typeinfo_cast<CCLabelBMFont*>(xpSpr->getChildren()->objectAtIndex(0))->setPosition({ 20.375f, 21.5f });
				auto xpBtn = CCMenuItemSpriteExtra::create(xpSpr, this, menu_selector(DemonProgression::openXPPopup));
				xpBtn->setID("xp-btn");

				auto xpMenu = CCMenu::create();
				xpMenu->setAnchorPoint({ 0.f, 0.f });
				xpMenu->setScale(0.65f);
				xpMenu->setPosition({ diffSpr->getPositionX() - 34.f, diffSpr->getPositionY() + 7.f });
				if (Mod::get()->getSettingValue<bool>("lower-xp")) { xpMenu->setPosition({ diffSpr->getPositionX() - 34.f, diffSpr->getPositionY() - 24.f }); }
				xpMenu->addChild(xpBtn);
				xpMenu->setID("gddp-xp-menu");
				if (Mod::get()->getSettingValue<bool>("show-xp")) { this->addChild(xpMenu); }
			}

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) {
				diffSpr->setOpacity(0);
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

			std::string fullSpr = fmt::format("{}Text.png", sprite);
			std::string fullPlusSpr = fmt::format("{}Text.png", plusSprite);

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && sprite != "DP_Invisible") {
				auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr).data());

				if (this->m_level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible") {
					typeinfo_cast<CCSprite*>(diffSpr->getChildren()->objectAtIndex(0))->setVisible(false);
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
				}

				if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible") {
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
				}

				customSpr->setID("gddp-difficulty");
				customSpr->setAnchorPoint({ 0.5f, 1 });
				customSpr->setPosition({ diffSpr->getPositionX() + 0.25f, diffSpr->getPositionY() + 30 });
				customSpr->setZOrder(5);

				this->addChild(customSpr);

				//check if the level is recommended and the effect is enabled
				auto recommendations = Mod::get()->getSavedValue<matjson::Array>("recommended-levels");
				if (!Mod::get()->getSettingValue<bool>("disable-recommended-effect") && std::find(recommendations.begin(), recommendations.end(), this->m_level->m_levelID.value()) != recommendations.end()) {
					auto recommendedSpr = CCSprite::createWithSpriteFrameName("DP_RecommendGlow.png"_spr);
					recommendedSpr->setPosition({ 37.f, 37.f });
					if (gddpDiff >= 11) { recommendedSpr->setPosition({ 37.f, 40.f }); }
					recommendedSpr->setZOrder(6);
					customSpr->addChild(recommendedSpr);
				}

				if (this->getChildByID("grd-difficulty") && !Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
					customSpr->setVisible(false);
				}
			}

			//GrD Effects
			if (Mod::get()->getSettingValue<bool>("disable-grandpa-demon-effects") && Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) {
				if (Loader::get()->isModLoaded("itzkiba.grandpa_demon")) {
					int num = 0;

					for (int i = 0; i < this->getChildrenCount(); i++) {
						if (this->getChildByType<CCSprite>(i)) {
							if (!(this->getChildByType<CCSprite>(i)->getID() != "") && (this->getChildByType<CCSprite>(i)->getTag() != 69420) && (this->getChildByType<CCSprite>(i)->getContentHeight() >= 750.0f)) {
								num += 1;
								this->getChildByType<CCSprite>(i)->setID(fmt::format("grd-bg-{}", num));
							}
						}

						if (num == 2) {
							break;
						}
					}

					num = 0;

					for (int i = 0; i < this->getChildrenCount(); i++) {
						if (this->getChildByType<CCParticleSystemQuad>(i)) {
							if (!(this->getChildByType<CCParticleSystemQuad>(i)->getID() != "") && (this->getChildByType<CCParticleSystemQuad>(i)->getPositionY() >= 230)) {
								num += 1;
								this->getChildByType<CCParticleSystemQuad>(i)->setID(fmt::format("grd-particles-{}", num));
							}
						}

						if (num == 2) {
							break;
						}
					}

					if (this->getChildByID("grd-bg-1")) {
						this->getChildByID("grd-bg-1")->setVisible(false);
						

						typeinfo_cast<CCSprite*>(this->getChildByID("background"))->setOpacity(255);
						typeinfo_cast<CCSprite*>(this->getChildByID("bottom-left-art"))->setOpacity(255);
						typeinfo_cast<CCSprite*>(this->getChildByID("bottom-right-art"))->setOpacity(255);
					}

					if (this->getChildByID("grd-bg-2")) {
						this->getChildByID("grd-bg-2")->setVisible(false);
					}

					if (this->getChildByID("grd-particles-1")) {
						this->getChildByID("grd-particles-1")->setVisible(false);
					}

					if (this->getChildByID("grd-particles-2")) {
						this->getChildByID("grd-particles-2")->setVisible(false);
					}
				}
			}

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && Mod::get()->getSettingValue<bool>("override-grandpa-demon") && (type == "main" || type == "legacy")) {
				if (Loader::get()->isModLoaded("itzkiba.grandpa_demon") && this->getChildByID("grd-difficulty")) {
					this->getChildByID("grd-difficulty")->setVisible(false);
					if (this->getChildByID("grd-infinity")) { this->getChildByID("grd-infinity")->setVisible(false); }

					this->getChildByID("grd-difficulty")->removeMeAndCleanup();
				}
			}

		}

		return true;
	}

	void updateLabelValues() {
		LevelInfoLayer::updateLabelValues();

		if (this->m_level->m_stars != 10) {
			return;
		}

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		//check for errors
		auto jsonCheck = checkJson(data, "");

		if (!jsonCheck.ok()) {
			log::info("Something went wrong validating the GDDP list data.");

			return;
		}

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) {
			inGDDP = true;
		}

		if (inGDDP && (data["level-data"].contains(std::to_string(this->m_level->m_levelID.value())) || Mod::get()->getSettingValue<bool>("all-demons-rated"))) {
			
			//if not on the GDDP or GDDL, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars == 10 && ListManager::getSpriteName(this->m_level) == "") {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return;
				}
			}

			//if not a non-demon level that's registered on the gddp, return
			if (Mod::get()->getSettingValue<bool>("all-demons-rated") && this->m_level->m_stars != 10) {
				if (!data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
					return;
				}
			}
			
			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
				if (Loader::get()->isModLoaded("itzkiba.grandpa_demon") && this->getChildByID("grd-difficulty")) {
					this->getChildByID("grd-difficulty")->setVisible(false);
					if (this->getChildByID("grd-infinity")) { this->getChildByID("grd-infinity")->setVisible(false); }

					this->getChildByID("grd-difficulty")->removeMeAndCleanup();
				}
			}
		}

		return;
	}

	void onBack(CCObject* sender) {
		LevelInfoLayer::onBack(sender);

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		//check for errors
		auto jsonCheck = checkJson(data, "");

		if (!jsonCheck.ok()) {
			log::info("Something went wrong validating the GDDP list data.");

			return;
		}

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) {
			inGDDP = true;
		}

		//if gauntlet level, return
		if (this->m_level->m_gauntletLevel || this->m_level->m_gauntletLevel2) {
			return;
		}

		if (inGDDP && data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
			if (this->m_level->m_normalPercent.value() == 100) {
				auto completedLvls = Mod::get()->getSavedValue<matjson::Array>("completed-levels");

				if (std::find(completedLvls.begin(), completedLvls.end(), this->m_level->m_levelID.value()) == completedLvls.end()) {
					completedLvls.insert(completedLvls.begin(), this->m_level->m_levelID.value());
					Mod::get()->setSavedValue<matjson::Array>("completed-levels", completedLvls);
				}
			}

			RecommendedUtils::validateLevels();
		}

		return;
	}
};