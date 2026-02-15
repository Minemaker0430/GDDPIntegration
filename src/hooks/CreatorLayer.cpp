//geode header
#include <Geode/Geode.hpp>

#include <Geode/modify/CreatorLayer.hpp>
#include "../menus/DPLayer.hpp"

//geode namespace
using namespace geode::prelude;

//add button to creator layer
class $modify(CreatorLayer) {

	static void onModify(auto & self) {
		static_cast<void>(self.setHookPriority("CreatorLayer::init", -42));
	}

	bool init() {
		if (!CreatorLayer::init()) return false;
		
		if (Mod::get()->getSavedValue<int>("database-version", 161) < 161) { 
			auto alert = FLAlertLayer::create(
			"IMPORTANT",
			"Your GDDP Data is outdated. Go into the Progression Menu to refresh it.",
			"OK"
			); 
			alert->m_scene = this;
			alert->show();
		}

		Mod::get()->setSavedValue<bool>("in-gddp", false);

		auto menu = this->getChildByID("creator-buttons-menu");

		if (Mod::get()->getSettingValue<bool>("compact-button")) {
			auto spr = CircleButtonSprite::createWithSpriteFrameName("DP_Logo.png"_spr);

			if (Mod::get()->getSettingValue<bool>("alt-button-texture")) {
				spr = CircleButtonSprite::createWithSpriteFrameName("DP_BeginnerPlus.png"_spr);
			}

			auto dpBtn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(DPLayer::callback));
			dpBtn->setID("demon-progression-button");

			if (Loader::get()->isModLoaded("cvolton.betterinfo")) {
				log::info("BetterInfo Detected. Added Button.");

				if(auto menu = this->getChildByID("cvolton.betterinfo/center-right-menu")){
					menu->addChild(dpBtn);
					menu->setLayout(AxisLayout::create(Axis::Column), true, true);
				}
			}
			else {
				auto size = CCDirector::sharedDirector()->getWinSize();
				auto sideMenu = CCMenu::create();
				sideMenu->setPositionX(size.width - 22);
				sideMenu->setAnchorPoint({ 0, 0 });
				sideMenu->setID("center-right-menu"_spr);
				sideMenu->setLayout(AxisLayout::create(Axis::Column), true, true);
				sideMenu->addChild(dpBtn);
				this->addChild(sideMenu);
			}
		}
		else if (Mod::get()->getSettingValue<bool>("replace-map-packs")) {
			auto spr = CCSprite::createWithSpriteFrameName("DP_demonProgressionBtn.png"_spr);

			if (Mod::get()->getSettingValue<bool>("alt-button-texture")) {
				spr = CCSprite::createWithSpriteFrameName("DP_demonProgressionBtnAlt.png"_spr);
			}

			if (Loader::get()->isModLoaded("capeling.goodbye_unnecessary_buttons")) {
				spr->setScale(0.85f);
			}
			else {
				spr->setScale(0.8f);
			}

			auto dpBtn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(DPLayer::callback));
			dpBtn->setContentSize({ 83.6f, 83.6f });
			dpBtn->setPosition({ 450.f, 155.f });
			dpBtn->setID("demon-progression-button");
			typeinfo_cast<CCSprite*>(dpBtn->getChildren()->objectAtIndex(0))->setPositionX(44.f);
			menu->addChild(dpBtn);

			if (menu->getChildByID("map-packs-button")) {
				dpBtn->setPosition({ menu->getChildByID("map-packs-button")->getPositionX(), menu->getChildByID("map-packs-button")->getPositionY() });

				if (Loader::get()->isModLoaded("alphalaneous.pages_api")) {
					menu->getChildByID("map-packs-button")->removeMeAndCleanup();
				}
				else {
					menu->getChildByID("map-packs-button")->setVisible(false);
				}
			}
		}
		else {
			auto spr = CCSprite::createWithSpriteFrameName("DP_demonProgressionBtn.png"_spr);

			if (Mod::get()->getSettingValue<bool>("alt-button-texture")) {
				spr = CCSprite::createWithSpriteFrameName("DP_demonProgressionBtnAlt.png"_spr);
			}

			if (Loader::get()->isModLoaded("capeling.goodbye_unnecessary_buttons")) {
				spr->setScale(0.85f);
			}
			else {
				spr->setScale(0.8f);
			}

			auto dpBtn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(DPLayer::callback));
			dpBtn->setContentSize({ 83.6f, 83.6f });
			dpBtn->setPosition({ 450.f, 155.f });
			dpBtn->setID("demon-progression-button");
			typeinfo_cast<CCSprite*>(dpBtn->getChildren()->objectAtIndex(0))->setPositionX(44.f);
			menu->addChild(dpBtn);

			if (Loader::get()->isModLoaded("capeling.goodbye_unnecessary_buttons")) {
				menu->getChildByID("quests-button")->setPositionX(45.8);
				menu->getChildByID("daily-button")->setPositionX(135.4);
				menu->getChildByID("weekly-button")->setPositionX(225);
				menu->getChildByID("gauntlets-button")->setPositionX(314.6);
				menu->getChildByID("demon-progression-button")->setPositionX(406.2);
			}
			else {
				menu->getChildByID("map-button")->setPositionX(2.9);
				menu->getChildByID("daily-button")->setPositionX(91.5);
				menu->getChildByID("weekly-button")->setPositionX(181);
				menu->getChildByID("event-button")->setPositionX(270.5);
				menu->getChildByID("gauntlets-button")->setPositionX(360.1);
			}

			if (Loader::get()->isModLoaded("cvolton.betterinfo") && !Mod::get()->getSettingValue<bool>("dont-move-side-btns")) {
				log::info("BetterInfo Detected. Moved Button.");
				if (auto menu = this->getChildByID("cvolton.betterinfo/center-right-menu")){
					menu->setPositionY(260);
				}
			}

			if (Loader::get()->isModLoaded("spaghettdev.gd-roulette") && !Mod::get()->getSettingValue<bool>("dont-move-side-btns")) {
				log::info("GD Roulette Detected. Moved Button.");
				if (auto menu = this->getChildByID("spaghettdev.gd-roulette/creator-layer-menu")) {
					menu->setPositionY(92);
				}
				else if (auto menu = this->getChildByID("spaghettdev.gd-roulette/center-left-menu")){
					menu->setPositionY(255);
				}
			}
		}

		return true;
	}
};