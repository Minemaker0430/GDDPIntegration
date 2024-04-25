//geode header
#include <Geode/Geode.hpp>

#include <Geode/modify/LevelInfoLayer.hpp>
#include "DPLayer.hpp"

//geode namespace
using namespace geode::prelude;

//modify gddp level pages
class $modify(LevelInfoLayer) {
	static void onModify(auto & self) {
		static_cast<void>(self.setHookPriority("LevelInfoLayer::init", -42));
	}

	bool init(GJGameLevel * p0, bool p1) {
		if (!LevelInfoLayer::init(p0, p1)) return false;

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) {
			inGDDP = true;
		}

		if (inGDDP && data["level-data"].contains(std::to_string(p0->m_levelID.value()))) {

			log::info("{}", Mod::get()->getSavedValue<bool>("in-gddp"));

			if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
				auto bg = typeinfo_cast<CCSprite*>(this->getChildByID("background"));
				bg->setColor({ 18, 18, 86 });
			}

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);
			auto reqLevels = Mod::get()->getSavedValue<int>("current-pack-requirement", 0);
			auto totalLevels = Mod::get()->getSavedValue<int>("current-pack-totalLvls", 0);

			auto hasRank = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(data[type][id]["listID"].as_int())).hasRank;

			auto diffSpr = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("difficulty-sprite"));

			auto gddpDiff = data["level-data"][std::to_string(p0->m_levelID.value())]["difficulty"].as_int();

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) {
				diffSpr->setOpacity(0);
			}

			std::string sprite = "DP_Beginner";
			std::string plusSprite = "DP_BeginnerPlus";

			sprite = data["main"][gddpDiff]["sprite"].as_string();
			plusSprite = data["main"][gddpDiff]["plusSprite"].as_string();

			std::string fullSpr = sprite + "Text.png";
			std::string fullPlusSpr = plusSprite + "Text.png";

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && sprite != "DP_Invisible.png") {
				auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr.c_str()));

				if (p0->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible.png") {
					typeinfo_cast<CCSprite*>(diffSpr->getChildren()->objectAtIndex(0))->setVisible(false);
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr.c_str()));
				}

				if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible.png") {
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr.c_str()));
				}

				customSpr->setID("gddp-difficulty");
				customSpr->setAnchorPoint({ 0.5f, 1 });
				customSpr->setPosition({ diffSpr->getPositionX() + 0.25f, diffSpr->getPositionY() + 30 });
				customSpr->setZOrder(5);

				this->addChild(customSpr);

				if (this->getChildByID("grd-difficulty") && !Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
					customSpr->setVisible(false);
				}
			}

			if (Mod::get()->getSettingValue<bool>("disable-grandpa-demon-effects") && Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) {
				if (Loader::get()->isModLoaded("itzkiba.grandpa_demon")) {
					if (this->getChildByID("grd-infinity")) {
						if (getChildOfType<CCSprite>(this, 11) && getChildOfType<CCSprite>(this, 12)) {
							auto grdBG1 = getChildOfType<CCSprite>(this, 11);
							auto grdBG2 = getChildOfType<CCSprite>(this, 12);

							grdBG1->setVisible(false);
							grdBG2->setVisible(false);

							typeinfo_cast<CCSprite*>(this->getChildByID("background"))->setOpacity(255);
							typeinfo_cast<CCSprite*>(this->getChildByID("bottom-left-art"))->setOpacity(255);
							typeinfo_cast<CCSprite*>(this->getChildByID("bottom-right-art"))->setOpacity(255);
						}
						if (getChildOfType<CCParticleSystemQuad>(this, 0) && getChildOfType<CCParticleSystemQuad>(this, 1)) {
							auto grdParticles1 = getChildOfType<CCParticleSystemQuad>(this, 0);
							auto grdParticles2 = getChildOfType<CCParticleSystemQuad>(this, 1);

							grdParticles1->setVisible(false);
							grdParticles2->setVisible(false);
						}
					}
					else {
						if (auto grdBG = getChildOfType<CCSprite>(this, 11)) {
							grdBG->setVisible(false);
							typeinfo_cast<CCSprite*>(this->getChildByID("background"))->setOpacity(255);
							typeinfo_cast<CCSprite*>(this->getChildByID("bottom-left-art"))->setOpacity(255);
							typeinfo_cast<CCSprite*>(this->getChildByID("bottom-right-art"))->setOpacity(255);
						}
						if (auto grdParticles = getChildOfType<CCParticleSystemQuad>(this, 0)) {
							grdParticles->setVisible(false);
						}
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

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) {
			inGDDP = true;
		}

		if (inGDDP && data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");

			if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
				if (Loader::get()->isModLoaded("itzkiba.grandpa_demon") && this->getChildByID("grd-difficulty")) {
					this->getChildByID("grd-difficulty")->setVisible(false);
					if (this->getChildByID("grd-infinity")) { this->getChildByID("grd-infinity")->setVisible(false); }

					this->getChildByID("grd-difficulty")->removeMeAndCleanup();
				}
			}
		}
	}


};