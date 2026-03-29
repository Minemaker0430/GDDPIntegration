//geode header
#include <Geode/Geode.hpp>

#include <Geode/binding/LevelSelectLayer.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/modify/LevelCell.hpp>
#include "../menus/DPLayer.hpp"
#include "../CustomText.hpp"
#include "../DPUtils.hpp"
#include "../XPUtils.hpp"
#include "../RecommendedUtils.hpp"

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
		
		auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("cached-data")["skillset-data"];

		FLAlertLayer::create(
			skillsetData[skillID]["display-name"].asString().unwrapOr("null").c_str(),
			skillsetData[skillID]["description"].asString().unwrapOr("erm that\'s awkward").c_str(),
			"OK"
		)->show();

		return;
	}

	void onRecommended(CCObject*) {
		RecommendedUtils::getSkillsForRecommendation(this->m_level->m_levelID.value());
		return;
	}

	void loadCustomLevelCell() {
		LevelCell::loadCustomLevelCell();
		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		bool inGDDP = (Mod::get()->getSavedValue<bool>("in-gddp") || Mod::get()->getSettingValue<bool>("show-outside-menus"));
		
		//check for errors
		auto jsonCheck = checkJson(data, "");

		if (!jsonCheck.ok()) {
			log::info("Something went wrong validating the GDDP list data.");

			return;
		}

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

					if (uniqueMonthly) return; 
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

					if (uniqueBonus) return; 
				}
			}

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);

			std::string saveID = (type == "main") ? data["main"][id]["saveID"].asString().unwrapOr("null") : "null";
			auto hasRank = (saveID != "null") ? Mod::get()->getSavedValue<ListSaveFormat>(saveID).hasRank : false;

			auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
			auto skillsetData = data["skillset-data"];

			int gddpDiff = 0;
			std::vector<std::string> skillsets = {};
			auto levelID = std::to_string(this->m_level->m_levelID.value());

			if (data["level-data"].contains(levelID)) {
				gddpDiff = data["level-data"][levelID]["difficulty"].as<int>().unwrapOr(0);
				skillsets = data["level-data"][levelID]["skillsets"].as<std::vector<std::string>>().unwrapOrDefault();

				if (this->m_level->m_normalPercent.value() >= 100) {
					auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

					if (!DPUtils::containsInt(completedLvls, this->m_level->m_levelID.value())) {
						completedLvls.insert(completedLvls.begin(), this->m_level->m_levelID.value());
						Mod::get()->setSavedValue<std::vector<int>>("completed-levels", completedLvls);
					}
				}
			}

			//skillset badges
			if (Mod::get()->getSettingValue<bool>("skillset-badges") && skillsets.size() > 0 && Mod::get()->getSettingValue<bool>("show-skills-in-list")) {
				auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer"));

				GJDifficultySprite* diffSpr;
				if (this->getChildByID("main-layer")->getChildByID("difficulty-container")) {
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
					if (!layer->getChildByID("level-place")->isVisible()) skillMenu->setPosition({ 25.f, -12.f });
					else skillMenu->setPosition({ 45.f, -12.f });
					skillMenu->setScale(0.4f);
				}
				else {
					skillMenu->setPosition({ diffSpr->getPositionX() + 32, diffSpr->getPositionY() + 6 });
					skillMenu->setScale(0.5f);
				}
				skillMenu->setZOrder(42);
				skillMenu->setContentSize({ 31.5f, 90.0f });
				if (skillsets.size() >= 3) skillMenu->setAnchorPoint({ 0.7f, 0.5f });
				else skillMenu->setAnchorPoint({ 0.5f, 0.5f });
				
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
				layer->addChild(skillMenu);

			}

			//fancy level name
			if (Mod::get()->getSettingValue<bool>("custom-level-name") && this->getChildByID("main-layer")->getChildByID("level-name")) {
				auto lvlName = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("main-layer")->getChildByID("level-name"));

				auto customLvlName = CustomText::create(lvlName->getString());
				customLvlName->addEffectsFromProperties(data["main"][gddpDiff]["textEffects"]);
				customLvlName->setPosition(lvlName->getPosition());
				customLvlName->setAnchorPoint(lvlName->getAnchorPoint());
				customLvlName->setScale(lvlName->getScale());
				customLvlName->setZOrder(5);
				customLvlName->setID("custom-level-name"_spr);

				lvlName->setOpacity(0);
				lvlName->setVisible(false);
				
				this->getChildByID("main-layer")->addChild(customLvlName);
			}

			//custom difficulty faces
			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) {

				if (this->getChildByID("main-layer")->getChildByID("difficulty-container")) {
					auto diffIcon = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("main-layer")->getChildByID("difficulty-container")->getChildByID("difficulty-sprite"));
					auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer")->getChildByID("difficulty-container"));
					typeinfo_cast<GJDifficultySprite*>(layer->getChildByID("difficulty-sprite"))->setOpacity(0);

					std::string sprite = data["main"][gddpDiff]["sprite"].asString().unwrapOr("DP_Unknown");
					std::string plusSprite = data["main"][gddpDiff]["plusSprite"].asString().unwrapOr("DP_Unknown");

					//fallback
					auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);

					std::string fullSpr = fmt::format("{}.png", sprite);
					std::string fullPlusSpr = fmt::format("{}.png", plusSprite);

					if (sprite != "DP_Invisible") {
						auto customSpr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(fullSpr).data(), unkSpr);

						if (this->m_level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible") {
							typeinfo_cast<CCSprite*>(diffIcon->getChildren()->objectAtIndex(0))->setVisible(false);
							customSpr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data(), unkSpr);
						}

						if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible") {
							customSpr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data(), unkSpr);
						}

						customSpr->setID("gddp-difficulty");
						customSpr->setAnchorPoint({ 0.5f, 0.5f });
						customSpr->setPosition({ diffIcon->getPositionX(), diffIcon->getPositionY() + 6.f });
						customSpr->setZOrder(5);

						layer->addChild(customSpr);
						
						//label
						auto demonLabel = CCLabelBMFont::create("Demon", "bigFont.fnt");
						auto customDemonLabel = CustomText::create("Demon");

						demonLabel->setID("demon-label"_spr);
						demonLabel->setAnchorPoint({ 0.5f, 0.5f });
						demonLabel->setPosition({ diffIcon->getPositionX(), diffIcon->getPositionY() - 16.f });
						demonLabel->setScale(0.35f);
						demonLabel->setZOrder(5);

						customDemonLabel->setID("custom-demon-label"_spr);
						customDemonLabel->setAnchorPoint({ 0.5f, 0.5f });
						customDemonLabel->setPosition({ diffIcon->getPositionX(), diffIcon->getPositionY() - 16.f });
						customDemonLabel->setScale(0.35f);
						customDemonLabel->setZOrder(5);

						customDemonLabel->addEffectsFromProperties(data["main"][gddpDiff]["textEffects"]);

						layer->addChild((Mod::get()->getSettingValue<bool>("custom-demon-labels")) ? (CCNode*)customDemonLabel : (CCNode*)demonLabel);

						//check if the level is recommended and the effect is enabled
						auto recommendations = Mod::get()->getSavedValue<matjson::Value>("recommended-levels");
						if (Mod::get()->getSettingValue<bool>("enable-recommendations")) {
							for (auto [key, value] : XPUtils::skills) {
								if (DPUtils::containsInt(recommendations[key].as<std::vector<int>>().unwrapOrDefault(), this->m_level->m_levelID.value())) {
									auto recommendedSpr = CCSprite::createWithSpriteFrameName("DP_RecommendGlow.png"_spr);
									recommendedSpr->setAnchorPoint({0.f, 0.f});
									recommendedSpr->setZOrder(6);
									if (!Mod::get()->getSettingValue<bool>("disable-recommended-effect")) customSpr->addChild(recommendedSpr);

									auto recommendedMenu = CCMenu::create();
									recommendedMenu->setID("recommended-menu");
									recommendedMenu->setContentSize({ 0.f, 0.f });
									recommendedMenu->setPosition(0.f, 45.f);
									recommendedMenu->setAnchorPoint({ 0.f, 0.f });
									recommendedMenu->setZOrder(7);

									if (Mod::get()->getSettingValue<bool>("enable-recommended-btn") && Mod::get()->getSettingValue<bool>("show-recommended-in-list")) {
										customSpr->addChild(recommendedMenu);

										auto starSpr = CCSprite::createWithSpriteFrameName("GJ_sStarsIcon_001.png");
										starSpr->setOpacity(200);

										auto recommendedBtn = CCMenuItemSpriteExtra::create(starSpr, this, menu_selector(DemonProgression::onRecommended));
										recommendedBtn->setAnchorPoint({ 0.f, 1.f });
										recommendedMenu->addChild(recommendedBtn);
									}

									break;
								}
							}
						}
					}
				}
			}
		}
		
		return;
	}
};