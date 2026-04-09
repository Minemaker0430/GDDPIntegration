//geode header
#include <Geode/Geode.hpp>

#include <Geode/utils/JsonValidation.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "../menus/DPLayer.hpp"
#include "../popups/XPPopup.hpp"
#include "../RecommendedUtils.hpp"
#include "../CustomText.hpp"
#include "../popups/OpenStartposPopup.hpp"
#include "../DPUtils.hpp"
#include "../XPUtils.hpp"

//geode namespace
using namespace geode::prelude;

//modify gddp level pages
class $modify(DemonProgression, LevelInfoLayer) {

	static void onModify(auto & self) {
		static_cast<void>(self.setHookPriority("LevelInfoLayer::init", -42));
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

		auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("cached-data")["skillset-data"];

		FLAlertLayer::create(
			skillsetData[skillID]["display-name"].asString().unwrapOr("null").c_str(),
			skillsetData[skillID]["description"].asString().unwrapOr("erm that\'s awkward").c_str(),
			"OK"
		)->show();

		return;
	}

	void openStartposCopy(CCObject* target) {
		auto popup = OpenStartposPopup::create(target->getTag());
		popup->setZOrder(100);
		this->addChild(popup);

		return;
	}

	void onRecommendedLvl(CCObject*) {
		RecommendedUtils::getSkillsForRecommendation(this->m_level->m_levelID.value());
		return;
	}

	//Normal Functions

	bool init(GJGameLevel* p0, bool p1) {
		if (!LevelInfoLayer::init(p0, p1)) return false;

		if (!this->m_level || this->m_level->m_levelID.value() < 0) return true;

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		//check for errors
		auto jsonCheck = checkJson(data, "");

		if (!jsonCheck.ok()) {
			log::info("Something went wrong validating the GDDP list data.");

			return true;
		}

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) inGDDP = true;

		if (inGDDP && data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {

			//if a gddp level that's only in a monthly pack, return
			if (Mod::get()->getSettingValue<bool>("hide-monthly-outside") && !Mod::get()->getSavedValue<bool>("in-gddp")) {

				//check monthly, if check returns with nothing, skip the rest
				auto isMonthly = false;
				for (auto p : data["monthly"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
					auto pack = p["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
					if (DPUtils::containsInt(pack, this->m_level->m_levelID.value())) {
						isMonthly = true;
						break;
					}	
				}
				
				if (isMonthly) {
					auto uniqueMonthly = true; //false = level is in main/legacy/bonus, so don't return if false

					for (auto packs : {data["main"], data["legacy"], data["bonus"]}) {
						for (auto p : packs.as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
							auto pack = p["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
							if (DPUtils::containsInt(pack, this->m_level->m_levelID.value())) {
								uniqueMonthly = false;
								break;
							}
						}

						if (!uniqueMonthly) break;
					}

					if (uniqueMonthly) return true;
				}
			}

			//if a gddp level that's only in a bonus pack, return
			if (Mod::get()->getSettingValue<bool>("hide-bonus-outside") && !Mod::get()->getSavedValue<bool>("in-gddp")) {

				//check bonus, if check returns with nothing, skip the rest
				auto isBonus = false;
				for (auto p : data["bonus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
					auto pack = p["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
					if (DPUtils::containsInt(pack, this->m_level->m_levelID.value())) {
						isBonus = true;
						break;
					}	
				}
				
				if (isBonus) {
					auto uniqueBonus = true; //false = level is in main/legacy/monthly, so don't return if false

					for (auto packs : {data["main"], data["legacy"], data["monthly"]}) {
						for (auto p : packs.as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
							auto pack = p["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
							if (DPUtils::containsInt(pack, this->m_level->m_levelID.value())) {
								uniqueBonus = false;
								break;
							}
						}

						if (!uniqueBonus) break;
					}

					if (uniqueBonus) return true;
				}
			}

			//if gauntlet level, return
			if (this->m_level->m_gauntletLevel || this->m_level->m_gauntletLevel2) return true;

			if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
				if (auto bg = typeinfo_cast<CCSprite*>(this->getChildByID("background"))) {
					bg->setColor({ 18, 18, 86 });
				}
			}

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);

			std::string saveID = (type == "main") ? data["main"][id]["saveID"].asString().unwrapOr("null") : "null";
			auto hasRank = (saveID != "null") ? Mod::get()->getSavedValue<ListSaveFormat>(saveID).hasRank : false;

			GJDifficultySprite* diffSpr = GJDifficultySprite::create(10, GJDifficultyName::Long);
			if (this->getChildByID("difficulty-sprite")) diffSpr = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("difficulty-sprite"));
			
			auto skillsetData = data["skillset-data"];

			int gddpDiff = 0;
			std::vector<std::string> skillsets = {};
			auto levelID = std::to_string(this->m_level->m_levelID.value());

			if (data["level-data"].contains(levelID)) {
				gddpDiff = data["level-data"][levelID]["difficulty"].as<int>().unwrapOr(0);
				skillsets = data["level-data"][levelID]["skillsets"].as<std::vector<std::string>>().unwrapOrDefault();
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
				if (Mod::get()->getSettingValue<bool>("move-skill-badges")) { skillMenu->setPosition({ diffSpr->getPositionX() + 17, diffSpr->getPositionY() - 26 }); }
				skillMenu->setZOrder(42);
				skillMenu->setContentSize({ 31.5f, 65.0f });
				skillMenu->setAnchorPoint({ 1.0f, 0.5f });
				skillMenu->setScale(0.75f);

				//add skillset buttons
				for (int i = 0; i < skillsets.size(); i++) {

					std::string skillID = skillsets[i];

					//check data entry
					if (!skillsetData.contains(skillID)) skillID = "unknown";
					
					//fallback spr
					auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Skill_Unknown.png"_spr);

					//get data
					auto name = skillsetData[skillID]["display-name"].asString().unwrapOr("null");
					auto desc = skillsetData[skillID]["description"].asString().unwrapOr("erm that\'s awkward");
					auto spriteName = fmt::format("{}.png", skillsetData[skillID]["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
					
					CCSprite* sprite = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(spriteName).data(), unkSpr);

					auto skillsetBtn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(DemonProgression::skillInfoPopup));
					skillsetBtn->setID(skillID);
					skillMenu->addChild(skillsetBtn);
				}

				skillMenu->updateLayout(false);

				this->addChild(skillMenu);

			}

			auto xpMenu = CCMenu::create();
			xpMenu->setAnchorPoint({ 0.f, 0.f });
			xpMenu->setScale(0.65f);
			auto xpLayout = AxisLayout::create();
			xpLayout->setAxis(Axis::Column);
			xpLayout->setAxisReverse(true);
			xpMenu->setLayout(xpLayout, true);
			xpMenu->setPosition({ diffSpr->getPositionX() - 34.f, diffSpr->getPositionY() + 7.f });
			xpMenu->setID("xp-menu"_spr);
			if (Mod::get()->getSettingValue<bool>("lower-xp")) { xpMenu->setPosition({ diffSpr->getPositionX() - 37.f, diffSpr->getPositionY() - 24.f }); }

			//xp button & startpos button
			if (data["level-data"][levelID].contains("xp")) { // check for xp
				auto xpText = CCLabelBMFont::create("XP", "bigFont.fnt");
				auto xpSpr = CircleButtonSprite::create(xpText, CircleBaseColor::Green, CircleBaseSize::Small);
				typeinfo_cast<CCLabelBMFont*>(xpSpr->getChildren()->objectAtIndex(0))->setPosition({ 20.375f, 21.5f });
				auto xpBtn = CCMenuItemSpriteExtra::create(xpSpr, this, menu_selector(DemonProgression::openXPPopup));
				xpBtn->setID("xp-btn");
				if (Mod::get()->getSettingValue<bool>("show-xp")) xpMenu->addChild(xpBtn);
			}
			//check for a startpos copy
			if (data["level-data"][levelID]["startpos-copy"].as<int>().unwrapOr(0) > 0 && Mod::get()->getSettingValue<bool>("enable-practice")) {
				auto startposSpr = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
				startposSpr->setScale(0.6f);
				auto startposBtn = CCMenuItemSpriteExtra::create(startposSpr, this, menu_selector(DemonProgression::openStartposCopy));
				startposBtn->setID("startpos-btn");
				startposBtn->setTag(data["level-data"][levelID]["startpos-copy"].as<int>().unwrapOr(0));
					
				xpMenu->setAnchorPoint({ 0.5f, 0.5f });
				xpMenu->addChild(startposBtn);
				xpMenu->updateLayout(false);
			}
			this->addChild(xpMenu);

			//title label
			if (Mod::get()->getSettingValue<bool>("custom-level-name") && this->getChildByID("title-label")) {
				auto lvlName = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("title-label"));

				auto customLvlName = CustomText::create(lvlName->getString());
				customLvlName->addEffectsFromProperties(data["main"][gddpDiff]["textEffects"]);
				customLvlName->setPosition(lvlName->getPosition());
				customLvlName->setAnchorPoint(lvlName->getAnchorPoint());
				customLvlName->setScale(lvlName->getScale());
				customLvlName->setZOrder(5);
				customLvlName->setID("custom-level-title"_spr);

				lvlName->setOpacity(0);
				lvlName->setVisible(false);
				
				this->addChild(customLvlName);
			}

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) diffSpr->setOpacity(0);

			std::string sprite = data["main"][gddpDiff]["sprite"].asString().unwrapOr("DP_Unknown");
			std::string plusSprite = data["main"][gddpDiff]["plusSprite"].asString().unwrapOr("DP_Unknown");

			//fallback
			auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);

			std::string fullSpr = fmt::format("{}.png", sprite);
			std::string fullPlusSpr = fmt::format("{}.png", plusSprite);

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && sprite != "DP_Invisible") {
				auto customSpr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(fullSpr).data(), unkSpr);

				if (this->m_level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible") {
					typeinfo_cast<CCSprite*>(diffSpr->getChildren()->objectAtIndex(0))->setVisible(false);
					customSpr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data(), unkSpr);
				}

				if (Mod::get()->getSettingValue<bool>("override-ratings") && hasRank && plusSprite != "DP_Invisible") {
					customSpr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data(), unkSpr);
				}

				customSpr->setID("gddp-difficulty");
				customSpr->setAnchorPoint({ 0.5f, 1 });
				customSpr->setPosition({ diffSpr->getPositionX() + 0.25f, diffSpr->getPositionY() + 30 });
				customSpr->setZOrder(5);

				this->addChild(customSpr);

				//label
				auto demonLabelName = CCLabelBMFont::create(data["main"][gddpDiff]["name"].asString().unwrapOr("???").c_str(), "bigFont.fnt");
				auto customDemonLabelName = CustomText::create(data["main"][gddpDiff]["name"].asString().unwrapOr("???"));
				auto demonLabel = CCLabelBMFont::create("Demon", "bigFont.fnt");
				auto customDemonLabel = CustomText::create("Demon");

				demonLabelName->setID("demon-label"_spr);
				demonLabelName->setAnchorPoint({ 0.5f, 0.5f });
				demonLabelName->setPosition({ diffSpr->getPositionX(), diffSpr->getPositionY() - 16.f });
				demonLabelName->setScale(0.35f);
				demonLabelName->setZOrder(5);

				customDemonLabelName->setID("custom-demon-label"_spr);
				customDemonLabelName->setAnchorPoint({ 0.5f, 0.5f });
				customDemonLabelName->setPosition({ diffSpr->getPositionX(), diffSpr->getPositionY() - 16.f });
				customDemonLabelName->setScale(0.35f);
				customDemonLabelName->setZOrder(5);

				demonLabel->setID("demon-label-bottom"_spr);
				demonLabel->setAnchorPoint({ 0.5f, 0.5f });
				demonLabel->setPosition({ diffSpr->getPositionX(), diffSpr->getPositionY() - 26.f });
				demonLabel->setScale(0.35f);
				demonLabel->setZOrder(5);

				customDemonLabel->setID("custom-demon-label-bottom"_spr);
				customDemonLabel->setAnchorPoint({ 0.5f, 0.5f });
				customDemonLabel->setPosition({ diffSpr->getPositionX(), diffSpr->getPositionY() - 26.f });
				customDemonLabel->setScale(0.35f);
				customDemonLabel->setZOrder(5);

				customDemonLabelName->addEffectsFromProperties(data["main"][gddpDiff]["textEffects"]);
				customDemonLabel->addEffectsFromProperties(data["main"][gddpDiff]["textEffects"]);

				if (Mod::get()->getSettingValue<bool>("custom-demon-labels")) {
					this->addChild(customDemonLabelName);
					this->addChild(customDemonLabel);
				} 
				else {
					this->addChild(demonLabelName);
					this->addChild(demonLabel);
				}

				//check if the level is recommended and the effect is enabled
				auto recommendations = Mod::get()->getSavedValue<matjson::Value>("recommended-levels");
				if (Mod::get()->getSettingValue<bool>("enable-recommendations")) {
					for (auto [key, value] : XPUtils::skills) {
						if (DPUtils::containsInt(recommendations[key].as<std::vector<int>>().unwrapOrDefault(), this->m_level->m_levelID.value())) {
							auto recommendedSpr = CCSprite::createWithSpriteFrameName("DP_RecommendGlow.png"_spr);
							recommendedSpr->setAnchorPoint({0.f, 0.f});
							if (gddpDiff >= 11) recommendedSpr->setPositionY(3.f);
							recommendedSpr->setZOrder(6);
							if (!Mod::get()->getSettingValue<bool>("disable-recommended-effect")) customSpr->addChild(recommendedSpr);

							auto recommendedMenu = CCMenu::create();
							recommendedMenu->setID("recommended-menu");
							recommendedMenu->setContentSize({ 0.f, 0.f });
							recommendedMenu->setPosition(0.f, 45.f);
							recommendedMenu->setAnchorPoint({ 0.f, 0.f });
							recommendedMenu->setZOrder(7);

							if (Mod::get()->getSettingValue<bool>("enable-recommended-btn")) {
								customSpr->addChild(recommendedMenu);

								auto starSpr = CCSprite::createWithSpriteFrameName("GJ_sStarsIcon_001.png");
								starSpr->setOpacity(200);

								auto recommendedBtn = CCMenuItemSpriteExtra::create(starSpr, this, menu_selector(DemonProgression::onRecommendedLvl));
								recommendedBtn->setAnchorPoint({ 0.f, 1.f });
								recommendedMenu->addChild(recommendedBtn);
							}

							break;
						}
					}
				}
			}
			
			DPUtils::verifyCompletedLevels();
		}

		return true;
	}

	void onBack(CCObject* sender) {
		LevelInfoLayer::onBack(sender);

		DPUtils::verifyCompletedLevels();

		return;
	}
};