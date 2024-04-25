//geode header
#include <Geode/Geode.hpp>

#include <Geode/modify/LevelCell.hpp>
#include "DPLayer.hpp"

//geode namespace
using namespace geode::prelude;

//modify level cells
class $modify(LevelCell) {

	void loadCustomLevelCell() {
		LevelCell::loadCustomLevelCell();

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (Mod::get()->getSettingValue<bool>("show-outside-menus")) {
			inGDDP = true;
		}

		//log::info("{}", inGDDP);

		if (inGDDP && data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);
			auto reqLevels = Mod::get()->getSavedValue<int>("current-pack-requirement", 0);
			auto totalLevels = Mod::get()->getSavedValue<int>("current-pack-totalLvls", 0);

			auto hasRank = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(data[type][id]["listID"].as_int())).hasRank;

			auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

			auto gddpDiff = data["level-data"][std::to_string(this->m_level->m_levelID.value())]["difficulty"].as_int();

			if (!Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) {
				return;
			}

			//check for grandpa demon
			if (this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer") && Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
				auto diffIcon = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer")->getChildByID("difficulty-sprite"));
				auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer"));

				diffIcon->setVisible(false);

				if (layer->getChildByID("grd-infinity")) { layer->getChildByID("grd-infinity")->setVisible(false); }
				if (getChildOfType(layer, 3)) {
					if (getChildOfType(layer, 3)->getTag() == 69420 && getChildOfType(layer, 4)) {
						if ((this->m_level->m_isEpic == 1) && Mod::get()->getSettingValue<bool>("replace-epic")) { getChildOfType(layer, 3)->setVisible(false); }
						getChildOfType(layer, 4)->setVisible(false);
					}
					else {
						getChildOfType(layer, 3)->setVisible(false);
					}
				}

				std::string sprite = "DP_Beginner";
				std::string plusSprite = "DP_BeginnerPlus";

				sprite = data["main"][gddpDiff]["sprite"].as_string();
				plusSprite = data["main"][gddpDiff]["plusSprite"].as_string();

				std::string fullSpr = sprite + "SmallText.png";
				std::string fullPlusSpr = plusSprite + "SmallText.png";

				if (sprite != "DP_Invisible.png") {
					auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr.c_str()));

					if (this->m_level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible.png") {
						typeinfo_cast<CCSprite*>(diffIcon->getChildren()->objectAtIndex(0))->setVisible(false);
						customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr.c_str()));
					}

					if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible.png") {
						customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr.c_str()));
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

				sprite = data["main"][gddpDiff]["sprite"].as_string();
				plusSprite = data["main"][gddpDiff]["plusSprite"].as_string();

				std::string fullSpr = sprite + "SmallText.png";
				std::string fullPlusSpr = plusSprite + "SmallText.png";

				if (sprite != "DP_Invisible.png") {
					auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr.c_str()));

					if (this->m_level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible.png") {
						typeinfo_cast<CCSprite*>(diffIcon->getChildren()->objectAtIndex(0))->setVisible(false);
						customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr.c_str()));
					}

					if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible.png") {
						customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr.c_str()));
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
};