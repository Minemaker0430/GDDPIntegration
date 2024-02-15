//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/cocos/include/ccTypes.h>

#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelListLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include <Geode/binding/LevelListLayer.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/GJListLayer.hpp>
#include <Geode/binding/BoomListView.hpp>
#include <Geode/binding/LoadingCircle.hpp>

#include <Geode/Enums.hpp>

#include <Geode/loader/Loader.hpp>

#include <Geode/ui/BasedButton.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/ui/ListView.hpp>
#include <Geode/binding/ListCell.hpp>

#include <Geode/utils/web.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/utils/string.hpp>

//dplayer callback
#include "DPLayer.hpp"

//geode namespace
using namespace geode::prelude;

/*
List IDs:

[UF] = Unofficial

Main:
25409 - Beginner
25408 - Bronze
25406 - Silver
25403 - Gold
25399 - Amber
24994 - Platinum
24993 - Sapphire
24991 - Jade
24989 - Emerald
24986 - Ruby
24721 - Diamond
24711 - Onyx
24702 - Amethyst
24694 - Azurite
24687 - Obsidian

Legacy:
? - Beginner
? - Bronze
? - Silver
? - Gold
? - Amber
161032 - Platinum
160741 - Sapphire
160743 - Jade
160745 - Emerald
? - Ruby
? - Diamond
? - Onyx
? - Amethyst
? - Azurite
? - Obsidian

Bonus:
? - 12DoC 2023
? - Minigame
? - #GD10
? - 12DoC 2022
? - Nine Circles
? - Memory I
? - Memory II
? - Memory III
? - 2 Player Solo
? - Invisible
? - 1.9 GDPS
? - XXL
? - Speedrun
? - Trusta
? - Temple
? - Coder
? - Kria
? - AeonAir
? - GDDL 1
? - GDDL 2
? - Apocalyptic Trilogy
? - Plasma
? - RGB
? - Sonic
? - Lightwave
? - Sonic Wave

Monthly:
? - Current

*/

//add button to creator layer
class $modify(CreatorLayer) {

	static void onModify(auto& self) {
        self.setHookPriority("CreatorLayer::init", -42);
    }

	bool init() {
        if (!CreatorLayer::init()) return false;

		Mod::get()->setSavedValue<bool>("in-gddp", false);

		auto menu = this->getChildByID("creator-buttons-menu");

		auto spr = CCSprite::create("DP_demonProgressionBtn.png"_spr);

		if (Mod::get()->getSettingValue<bool>("alt-button-texture")) {
			spr = CCSprite::create("DP_demonProgressionBtnAlt.png"_spr);
		}
		
		if (Loader::get()->isModLoaded("capeling.goodbye_unnecessary_buttons")) {
			spr->setScale(0.85f);
		} else {
			spr->setScale(0.8f);
		}

		auto dpBtn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(DPLayer::callback));
		dpBtn->setPosition({ 451, 153 });
		dpBtn->setID("demon-progression-button");
		menu->addChild(dpBtn);
		
		if (Loader::get()->isModLoaded("capeling.goodbye_unnecessary_buttons")) {
			menu->getChildByID("quests-button")->setPositionX(45.8);
			menu->getChildByID("daily-button")->setPositionX(135.4);
			menu->getChildByID("weekly-button")->setPositionX(225);
			menu->getChildByID("gauntlets-button")->setPositionX(314.6);
			menu->getChildByID("demon-progression-button")->setPositionX(406.2);
		} else {
			menu->getChildByID("map-button")->setPositionX(2.9);
			menu->getChildByID("daily-button")->setPositionX(91.5);
			menu->getChildByID("weekly-button")->setPositionX(181);
			menu->getChildByID("event-button")->setPositionX(270.5);
			menu->getChildByID("gauntlets-button")->setPositionX(360.1);
		}
		
		if (Loader::get()->isModLoaded("cvolton.betterinfo")) {
			log::info("{}", "BetterInfo Detected. Moved Button.");
			this->getChildByID("cvolton.betterinfo/center-right-menu")->setPositionY(260);
		}

		if (Loader::get()->isModLoaded("spaghettdev.gd-roulette")) {
			log::info("{}", "GD Roulette Detected. Moved Button.");
			this->getChildByID("spaghettdev.gd-roulette/creator-layer-menu")->setPositionY(92);
		}

        return true;
    }

};

//modify gddp list layout
class $modify(LevelListLayer) {
	static void onModify(auto& self) {
		self.setHookPriority("LevelListLayer::init", -42);
	}

	bool init(GJLevelList* p0) {
		if (!LevelListLayer::init(p0)) return false;

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (inGDDP) {

			log::info("{}", Mod::get()->getSavedValue<bool>("in-gddp"));

			//Gotta use getObjectAtIndex because there's no Node IDs here yet :v

			auto children = this->getChildren();

			if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
				auto bg = typeinfo_cast<CCSprite*>(children->objectAtIndex(0));
				bg->setColor({ 18, 18, 86 });
			}

			/*auto menu = dynamic_cast<CCMenu*>(children->objectAtIndex(4));
			auto menuChildren = menu->getChildren();

			auto commentBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(3));
			auto rateBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(4));
			auto copyBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(5));
			auto infoBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(6));
			auto favBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(7));

			commentBtn->setPosition({ 10000, 10000 });
			rateBtn->setPosition({ 10000, 10000 });
			copyBtn->setPosition({ 10000, 10000 });
			infoBtn->setPosition({ 10000, 10000 });
			favBtn->setPosition({ 10000, 10000 });*/

			//auto downloadIcon = typeinfo_cast<CCSprite*>(children->objectAtIndex(6));
			//auto downloadText = typeinfo_cast<CCLabelBMFont*>(children->objectAtIndex(7));
			//auto likeIcon = typeinfo_cast<CCSprite*>(children->objectAtIndex(8));
			//auto likeText = typeinfo_cast<CCLabelBMFont*>(children->objectAtIndex(9));

			//downloadIcon->removeMeAndCleanup();
			//downloadText->setPosition({10000, 10000});
			//likeIcon->removeMeAndCleanup();
			//likeText->setPosition({ 10000, 10000 });

			//auto diffIcon = typeinfo_cast<CCSprite*>(children->objectAtIndex(10));
			//diffIcon->setPosition({ 10000, 10000 });

			//auto dpIcon = CCSprite::create();

			//Get Completed Levels & Store in Save Data

			auto glm = GameLevelManager::sharedState();
			auto completedLevels = glm->getCompletedLevels(false);
			auto levels = p0->m_levels;

			auto packProgress = 0;

			/*auto progressText = static_cast<CCLabelBMFont*>(children->objectAtIndex(13));
			std::string s = progressText->getString();

			std::vector<std::string> res;
			std::string delim = "/";
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

			packProgress = std::stoi(res[0]);*/

			for (int i = 0; i < completedLevels->indexOfObject(completedLevels->lastObject()); i++) {
				auto lvl = static_cast<GJGameLevel*>(completedLevels->objectAtIndex(i));
				auto lvlID = lvl->m_levelID.value();

				for (int j = 0; j < levels.size(); j++) {
					if (lvlID == levels[j]) {
						packProgress += 1;
					}
				}
			}

			//all save stuff
			auto packProgress_main = Mod::get()->getSavedValue<matjson::Array>("pack-progress-main");
			auto packProgress_legacy = Mod::get()->getSavedValue<matjson::Array>("pack-progress-legacy");
			auto packProgress_bonus = Mod::get()->getSavedValue<matjson::Array>("pack-progress-bonus");
			auto packProgress_monthly = Mod::get()->getSavedValue<matjson::Array>("pack-progress-monthly");

			auto hasRank = Mod::get()->getSavedValue<matjson::Array>("has-rank");
			auto hasCompleted_main = Mod::get()->getSavedValue<matjson::Array>("has-completed-main");
			auto hasCompleted_legacy = Mod::get()->getSavedValue<matjson::Array>("has-completed-legacy");
			auto hasCompleted_bonus = Mod::get()->getSavedValue<matjson::Array>("has-completed-bonus");
			auto hasCompleted_monthly = Mod::get()->getSavedValue<matjson::Array>("has-completed-monthly");

			auto packsCompleted_main = Mod::get()->getSavedValue<int>("packs-completed-main", 0);
			auto packsCompleted_legacy = Mod::get()->getSavedValue<int>("packs-completed-legacy", 0);
			auto packsCompleted_bonus = Mod::get()->getSavedValue<int>("packs-completed-bonus", 0);
			auto packsCompleted_monthly = Mod::get()->getSavedValue<int>("packs-completed-monthly", 0);

			auto bronzeMedals = Mod::get()->getSavedValue<int>("bronze-medals", 0);
			auto silverMedals = Mod::get()->getSavedValue<int>("silver-medals", 0);
			auto goldMedals = Mod::get()->getSavedValue<int>("gold-medals", 0);

			auto localDatabaseVer = Mod::get()->getSavedValue<int>("database-version", 0);

			auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
			auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);
			auto reqLevels = Mod::get()->getSavedValue<int>("current-pack-requirement", 0);
			auto totalLevels = Mod::get()->getSavedValue<int>("current-pack-totalLvls", 0);

			log::info("{}", type);
			log::info("{}", id);
			log::info("{}", reqLevels);
			log::info("{}", totalLevels);

			if (type == "main") {
				packProgress_main[id] = packProgress;
				if ((packProgress >= reqLevels) && (!hasRank[id].as_bool())) {
					hasRank[id] = true;
				}
				else if (packProgress == totalLevels) {
					hasCompleted_main[id] = true;
				}
				else {
					hasCompleted_main[id] = false;
				}

				Mod::get()->setSavedValue("pack-progress-main", packProgress_main);
				Mod::get()->setSavedValue("has-rank", hasRank);
				Mod::get()->setSavedValue("has-completed-main", hasCompleted_main);
			}
			else if (type == "legacy") {
				packProgress_legacy[id] = packProgress;
				if (packProgress == totalLevels) {
					hasCompleted_legacy[id] = true;
				}
				else {
					hasCompleted_legacy[id] = false;
				}

				Mod::get()->setSavedValue("pack-progress-legacy", packProgress_legacy);
				Mod::get()->setSavedValue("has-completed-legacy", hasCompleted_legacy);
			}
			else if (type == "bonus") {
				packProgress_bonus[id] = packProgress;
				if (packProgress == totalLevels) {
					hasCompleted_bonus[id] = true;
				}
				else {
					hasCompleted_bonus[id] = false;
				}

				Mod::get()->setSavedValue("pack-progress-bonus", packProgress_bonus);
				Mod::get()->setSavedValue("has-completed-bonus", hasCompleted_bonus);
			}
			else if (type == "monthly") {
				packProgress_monthly[id] = packProgress;
				if (packProgress == 6) {
					hasCompleted_monthly[id] = true;
				}
				else {
					hasCompleted_monthly[id] = false;
				}

				Mod::get()->setSavedValue("pack-progress-monthly", packProgress_monthly);
				Mod::get()->setSavedValue("has-completed-monthly", hasCompleted_monthly);
			}

		}

		return true;
	}
};

//modify gddp level pages
class $modify(LevelInfoLayer) {
	static void onModify(auto & self) {
		self.setHookPriority("LevelInfoLayer::init", -42);
	}

	bool init(GJGameLevel* p0, bool p1) {
		if (!LevelInfoLayer::init(p0, p1)) return false;

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");

		if (inGDDP) {

			log::info("{}", Mod::get()->getSavedValue<bool>("in-gddp"));

			if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
				auto bg = typeinfo_cast<CCSprite*>(this->getChildByID("background"));
				bg->setColor({ 18, 18, 86 });
			}

			/*auto diffSpr = typeinfo_cast<CCSprite*>(this->getChildByID("difficulty-sprite"));

			auto customSpr = CCSprite::create("DP_Beginner.png"_spr);
			customSpr->setPosition(diffSpr->getPosition());
			//customSpr->addChild(newGlow);
			this->addChild(customSpr);

			diffSpr->setVisible(false);*/

			/*if (Loader::get()->isModLoaded("itzkiba.grandpa_demon")) {
				this->getChildByID("grd-difficulty")->setVisible(false);
			}*/

		}

		return true;
	}
};

//Main DP Layer
void DPLayer::callback(CCObject*) {
	auto scene = CCScene::create(); // creates the scene
	auto dpLayer = DPLayer::create(); //creates the layer

	scene->addChild(dpLayer);

	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition
}

void DPLayer::keyBackClicked() {
	CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

void DPLayer::backButton(CCObject*) {
	Mod::get()->setSavedValue<bool>("in-gddp", false);
	keyBackClicked();
}

DPLayer* DPLayer::create() {
	auto pRet = new DPLayer();
	if (pRet && pRet->init()) {
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet); //don't crash if it fails
	return nullptr;
}

void DPLayer::infoCallback(CCObject*) {
	FLAlertLayer::create(nullptr, "GDDP Info", "OK", nullptr, "Beat the required number of levels in each pack to get a special rank and move on to the next pack.\n\nMod Developer: Minemaker0430\nGDDP Creator: Trusta\nSpecial Thanks: FireMario211 (Firee), HJFod, The GDP Discord Server, & The Geode Discord Server")->show();
}

void DPLayer::soonCallback(CCObject*) {
	FLAlertLayer::create("Coming Soon!", "This feature hasn't been implemented yet but will be in the future!", "OK")->show();
}

std::vector<std::string> getWords(std::string s, std::string d) {
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
}

void DPLayer::reloadData(CCObject* sender) {
	m_list->removeAllChildrenWithCleanup(true);
	m_list->removeMeAndCleanup();

	m_loadcircle = LoadingCircle::create();
	m_loadcircle->show();

	m_reload->setVisible(false);

	// download data
	web::AsyncWebRequest()
		.fetch("https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/main-list.json")
		.text()
		.then([&](std::string const& response) {
			Mod::get()->setSavedValue<matjson::Value>("cached-data", matjson::parse(response));

			m_data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
			reloadList(m_currentTab);
			m_loadcircle->fadeAndRemove();
			m_reload->setVisible(true);
		})
		.expect([&](std::string const& error) {
			FLAlertLayer::create("ERROR", "Something went wrong getting the List Data. (" + error + ")", "OK")->show();
			m_loadcircle->fadeAndRemove();
			m_reload->setVisible(true);
		});
}

void DPLayer::openList(CCObject* sender) {
	//FLAlertLayer::create("the", "bingle bong", "OK")->show();
	auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
	auto id = btn->getTag();
	auto type = btn->getID();

	auto listID = m_data[type][id]["listID"].as_int();
	auto reqLevels = 0;
	if (type == "main") { reqLevels = m_data[type][id]["reqLevels"].as_int(); }
	auto totalLevels = 0;
	if (type != "monthly") { totalLevels = m_data[type][id]["totalLevels"].as_int(); }

	Mod::get()->setSavedValue<std::string>("current-pack-type", type);
	Mod::get()->setSavedValue<int>("current-pack-index", id);
	Mod::get()->setSavedValue<int>("current-pack-requirement", reqLevels);
	Mod::get()->setSavedValue<int>("current-pack-totalLvls", totalLevels);

	std::string const& url = "https://www.boomlings.com/database/getGJLevelLists.php";
	std::string const& fields = "secret=Wmfd2893gb7&type=0&diff=-&len=-&count=1&str=" + std::to_string(listID); //thank you gd cologne :pray:
	web::AsyncWebRequest()
		.bodyRaw(fields)
		.postRequest()
		.fetch(url).text()
		.then([&](std::string& response) {
		//std::cout << response << std::endl;
		if (response != "-1") {
			auto scene = CCScene::create();
			//auto dict = glm->responseToDict(gd::string(response), false);

			/*
			Response IDs:
			1 - List ID (int)
			2 - Name (str)
			3 - Description? (encoded with Base64)
			4 - ???
			5 - Version (int)
			7 - ??? (int)
			10 - Downloads (int)
			14 - Likes (int)
			19 - ??? (empty)
			49 - ??? (int)
			50 - Account Name (str)
			51 - ID List (ints)
			55 - ??? (int)
			56 - ??? (int)

			(i'll probably only use 1, 2, 3, and 51)
			*/

			auto data = getWords(response, ":");
			log::info("{}", data);

			gd::vector<std::string> levelIDstr = getWords(data[20], ",");
			log::info("{}", levelIDstr);

			std::vector<int> IDs;
			for (int i = 0; i < levelIDstr.size(); i++)
			{
				int num = atoi(levelIDstr.at(i).c_str());
				IDs.push_back(num);
			}

			//gd::string desc = ZipUtils::base64URLDecode(data[5]);

			auto list = GJLevelList::create();
			list->m_listID = std::stoi(data[1]);
			list->m_listName = data[3];
			list->m_downloads = std::stoi(data[13]);
			list->m_likes = std::stoi(data[17]);
			//list->m_creatorName = data[29];
			list->m_levels = IDs;
			//list->m_listDesc = desc;

			auto layer = LevelListLayer::create(list);

			scene->addChild(layer);
			CCDirector::sharedDirector()->pushScene(cocos2d::CCTransitionFade::create(0.5f, scene));
		}
		else {
			FLAlertLayer::create("ERROR", "This pack doesn't exist! Check back later.", "OK")->show();
		}
		}).expect([](std::string const& error) {
			FLAlertLayer::create("ERROR", "Something went wrong! (" + error + ")", "OK")->show();
		});
}

bool DPLayer::init() {
	if (!CCLayer::init()) return false;

        log::info("{}", "Opened the Demon Progression menu.");

		Mod::get()->setSavedValue<bool>("in-gddp", true);
		log::info("{}", Mod::get()->getSavedValue<bool>("in-gddp"));

        auto menu = CCMenu::create();
        auto director = CCDirector::sharedDirector();
	    auto size = director->getWinSize();

	    auto bg = createLayerBG();
		if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
			bg->setColor({ 18, 18, 86 });
		}
		bg->setZOrder(-10);
        this->addChild(bg);

	    auto lCornerSprite = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
	    lCornerSprite->setAnchorPoint({ 0, 0 });
	    this->addChild(lCornerSprite);

	    auto rCornerSprite = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
	    rCornerSprite->setAnchorPoint({ 1, 0 });
	    rCornerSprite->setPosition({ size.width - 70, 0 });
	    rCornerSprite->setScaleX(-1);
	    this->addChild(rCornerSprite);

        this->addChild(menu);

		//back button
		auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
		auto backButton = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(DPLayer::backButton));
		auto backMenu = CCMenu::create();
		backMenu->addChild(backButton);
		backMenu->setPosition({25, size.height - 25});
		this->addChild(backMenu);

		//list bg
		auto listMiddle = CCLayerColor::create({194, 114, 62, 255});
		auto listLeft = CCSprite::createWithSpriteFrameName("GJ_table_side_001.png");
		auto listRight = CCSprite::createWithSpriteFrameName("GJ_table_side_001.png");
		auto listTop = CCSprite::createWithSpriteFrameName("GJ_table_top02_001.png");
		auto listBottom = CCSprite::createWithSpriteFrameName("GJ_table_bottom_001.png");

		listMiddle->setAnchorPoint({ 0.5, 0.5 });
		listMiddle->setPosition({ (size.width / 2) - 180, (size.height / 2) - 115 });
		listMiddle->setContentSize({370, 230});
		listMiddle->setZOrder(-1);

		listLeft->setPosition({(size.width / 2) - 183.5f, (size.height / 2)});
		listRight->setPosition({ (size.width / 2) + 183.5f, (size.height / 2)});
		listTop->setPosition({size.width / 2, (size.height / 2) + 120});
		listBottom->setPosition({size.width / 2, (size.height / 2) - 125});

		listRight->setScaleX(-1);
		listRight->setScaleY(3.6f);
		listLeft->setScaleY(3.6f);

		listLeft->setZOrder(9);
		listRight->setZOrder(9);
		listTop->setZOrder(10);
		listBottom->setZOrder(10);

		this->addChild(listMiddle);
		this->addChild(listLeft);
		this->addChild(listRight);
		this->addChild(listTop);
		this->addChild(listBottom);

		m_loadcircle = LoadingCircle::create();
        m_loadcircle->show();

		//reload menu
		auto reloadMenu = CCMenu::create();
		reloadMenu->setPosition({ 0, 0 });
		auto reloadBtnSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
		auto reloadBtn = CCMenuItemSpriteExtra::create(reloadBtnSprite, this, menu_selector(DPLayer::reloadData));
		reloadBtn->setPosition({ 30, 30 });
		reloadMenu->addChild(reloadBtn);
		reloadMenu->setID("reload-menu");
		this->addChild(reloadMenu);
		m_reload = reloadMenu;
		m_reload->setVisible(false);

		m_currentTab = static_cast<int>(DPListType::Main);

		// download data
		web::AsyncWebRequest()
			.fetch("https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/main-list.json")
			.text()
			.then([&](std::string const& response) {
				Mod::get()->setSavedValue<matjson::Value>("cached-data", matjson::parse(response));

				m_data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
				reloadList(static_cast<int>(DPListType::Main));
				m_loadcircle->fadeAndRemove();
				m_reload->setVisible(true);
			})
			.expect([&](std::string const& error) {
				FLAlertLayer::create("ERROR", "Something went wrong getting the List Data. (" + error + ")", "OK")->show();
				m_loadcircle->fadeAndRemove();
				m_reload->setVisible(true);
			});
		
		//extra buttons
		auto achievementBtnSprite = CCSprite::createWithSpriteFrameName("GJ_achBtn_001.png");
		auto leaderboardsBtnSprite = CCSprite::createWithSpriteFrameName("GJ_statsBtn_001.png");
		auto monthlyPackBtnSprite = CCSprite::createWithSpriteFrameName("GJ_rateDiffBtnMod_001.png");
		auto leaderboardButton = CCMenuItemSpriteExtra::create(leaderboardsBtnSprite, this, menu_selector(DPLayer::soonCallback));
		auto achievementButton = CCMenuItemSpriteExtra::create(achievementBtnSprite, this, menu_selector(DPLayer::soonCallback));
		auto monthlyPackButton = CCMenuItemSpriteExtra::create(monthlyPackBtnSprite, this, menu_selector(DPLayer::soonCallback));
		achievementButton->setPosition({ size.width - 30, 30 });
		leaderboardButton->setPosition({ size.width - 30, 80 });
		monthlyPackButton->setPosition({ size.width - 80, 30 });
		auto extrasMenu = CCMenu::create();
		extrasMenu->setPosition({ 0, 0 });
		extrasMenu->addChild(leaderboardButton);
		extrasMenu->addChild(achievementButton);
		extrasMenu->addChild(monthlyPackButton);
		extrasMenu->setID("extras-menu");
		//this->addChild(extrasMenu);

		//list tabs
		auto listTabs = CCMenu::create();
		listTabs->setID("list-tabs");

		auto backTabSprite = CCSprite::create("DP_tabBack.png"_spr);
		backTabSprite->setZOrder(-1);
		backTabSprite->setAnchorPoint({ 0, 0 });
		
		auto mainPacksBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::Selected, "Main", this, menu_selector(DPLayer::onTab));
		mainPacksBtn->setPosition(-136.f, 133.5f);
		mainPacksBtn->setID("main");
		mainPacksBtn->setTag(static_cast<int>(DPListType::Main));
		mainPacksBtn->toggle(true);
		mainPacksBtn->addChild(backTabSprite);
		listTabs->addChild(mainPacksBtn);

		auto legacyPacksBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::Selected, "Legacy", this, menu_selector(DPLayer::onTab));
		legacyPacksBtn->setPosition(-45.f, 133.5f);
		legacyPacksBtn->setID("legacy");
		legacyPacksBtn->setTag(static_cast<int>(DPListType::Legacy));
		legacyPacksBtn->addChild(backTabSprite);
		listTabs->addChild(legacyPacksBtn);

		auto bonusPacksBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::Selected, "Bonus", this, menu_selector(DPLayer::onTab));
		bonusPacksBtn->setPosition(45.f, 133.5f);
		bonusPacksBtn->setID("bonus");
		bonusPacksBtn->setTag(static_cast<int>(DPListType::Bonus));
		bonusPacksBtn->addChild(backTabSprite);
		listTabs->addChild(bonusPacksBtn);

		auto monthlyPacksBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::Selected, "Monthly", this, menu_selector(DPLayer::onTab));
		monthlyPacksBtn->setPosition(136.f, 133.5f);
		monthlyPacksBtn->setID("monthly");
		monthlyPacksBtn->setTag(static_cast<int>(DPListType::Monthly));
		monthlyPacksBtn->addChild(backTabSprite);
		listTabs->addChild(monthlyPacksBtn);

		this->addChild(listTabs);
		m_tabs = listTabs;

		m_databaseVer = CCLabelBMFont::create("Loading...", "chatFont.fnt");
		m_databaseVer->setAnchorPoint({ 1, 1 });
		m_databaseVer->setPosition({ size.width - 1, size.height - 1 });
		m_databaseVer->setScale(0.5f);

		if (!Mod::get()->getSettingValue<bool>("show-database-version")) {
			m_databaseVer->setVisible(false);
		}

		this->addChild(m_databaseVer);

		this->setKeyboardEnabled(true);
		this->setKeypadEnabled(true);

        return true;
}

void DPLayer::reloadList(int type) {

	//all save stuff
	auto packProgress_main = Mod::get()->getSavedValue<matjson::Array>("pack-progress-main");
	auto packProgress_legacy = Mod::get()->getSavedValue<matjson::Array>("pack-progress-legacy");
	auto packProgress_bonus = Mod::get()->getSavedValue<matjson::Array>("pack-progress-bonus");
	auto packProgress_monthly = Mod::get()->getSavedValue<matjson::Array>("pack-progress-monthly");

	auto hasRank = Mod::get()->getSavedValue<matjson::Array>("has-rank");
	auto hasCompleted_main = Mod::get()->getSavedValue<matjson::Array>("has-completed-main");
	auto hasCompleted_legacy = Mod::get()->getSavedValue<matjson::Array>("has-completed-legacy");
	auto hasCompleted_bonus = Mod::get()->getSavedValue<matjson::Array>("has-completed-bonus");
	auto hasCompleted_monthly = Mod::get()->getSavedValue<matjson::Array>("has-completed-monthly");

	auto packsCompleted_main = Mod::get()->getSavedValue<int>("packs-completed-main", 0);
	auto packsCompleted_legacy = Mod::get()->getSavedValue<int>("packs-completed-legacy", 0);
	auto packsCompleted_bonus = Mod::get()->getSavedValue<int>("packs-completed-bonus", 0);
	auto packsCompleted_monthly = Mod::get()->getSavedValue<int>("packs-completed-monthly", 0);

	auto bronzeMedals = Mod::get()->getSavedValue<int>("bronze-medals", 0);
	auto silverMedals = Mod::get()->getSavedValue<int>("silver-medals", 0);
	auto goldMedals = Mod::get()->getSavedValue<int>("gold-medals", 0);

	auto localDatabaseVer = Mod::get()->getSavedValue<int>("database-version", 0);

	//check if database got an update
	/*
	Main Pack Update - Reset Main Pack Save Data
	Legacy Pack Update - Reset Legacy Save Data
	Bonus Pack Update - Doesn't need to be checked
	Monthly Pack Update - Move all data up one array index and add a new slot at index 0
	*/
	
	if (packProgress_main.size() < m_data["main"].as_array().size()) { //check main packs
		//"erase" data
		packProgress_main.clear();
		hasCompleted_main.clear();
		hasRank.clear();

		//insert dummy save data
		for (int i = 0; i < m_data["main"].as_array().size(); i++) {
			packProgress_main.push_back(0);
			hasCompleted_main.push_back(false);
			hasRank.push_back(false);
		}

		//push save data
		Mod::get()->setSavedValue("pack-progress-main", packProgress_main);
		Mod::get()->setSavedValue("has-completed-main", hasCompleted_main);
		Mod::get()->setSavedValue("has-rank", hasRank);
			
		log::info("{}", "Found new Main Pack(s).");
	}

	if (packProgress_legacy.size() < m_data["legacy"].as_array().size()) { //check legacy packs
		//"erase" data
		packProgress_legacy.clear();
		hasCompleted_legacy.clear();
			
		//insert dummy save data
		for (int i = 0; i < m_data["legacy"].as_array().size(); i++) {
			packProgress_legacy.push_back(0);
			hasCompleted_legacy.push_back(false);
		}

		//push save data
		Mod::get()->setSavedValue("pack-progress-legacy", packProgress_legacy);
		Mod::get()->setSavedValue("has-completed-legacy", hasCompleted_legacy);
			
		log::info("{}", "Found new Legacy Pack(s).");
	}

	if (packProgress_bonus.size() < m_data["bonus"].as_array().size()) { //"check" bonus packs
		matjson::Array progress = packProgress_bonus;
		matjson::Array completed = hasCompleted_bonus;

		auto sizeDiff = m_data["bonus"].as_array().size() - packProgress_bonus.size();

		//insert dummy save data
		for (int i = 0; i < sizeDiff; i++) {
			progress.push_back(0);
			completed.push_back(false);
		}

		//push save data
		Mod::get()->setSavedValue("pack-progress-bonus", progress);
		Mod::get()->setSavedValue("has-completed-bonus", completed);
			
		log::info("{}", "Found new Bonus Pack(s).");
	}

	if (packProgress_monthly.size() < m_data["monthly"].as_array().size()) { //check monthly packs
		matjson::Array progress = packProgress_monthly;
		matjson::Array completed = hasCompleted_monthly;

		auto sizeDiff = m_data["monthly"].as_array().size() - packProgress_monthly.size();

		//insert dummy save data
		for (int i = 0; i < sizeDiff; i++) {
			progress.insert(progress.begin(), 0);
			completed.insert(completed.begin(), false);
		}

		//push save data
		Mod::get()->setSavedValue("pack-progress-monthly", progress);
		Mod::get()->setSavedValue("has-completed-monthly", completed);

		log::info("{}", "Found new Monthly Pack(s).");
	}

		Mod::get()->setSavedValue<int>("database-version", m_data["database-version"].as_int());
		log::info("{}", Mod::get()->getSavedValue<int>("database-version"));

	//do everything else
	auto dataIdx = "";

	if (type == static_cast<int>(DPListType::Main)) {
		dataIdx = "main";
	} 
	else if (type == static_cast<int>(DPListType::Legacy)) {
		dataIdx = "legacy";
	}
	else if (type == static_cast<int>(DPListType::Bonus)) {
		dataIdx = "bonus";
	}
	else if (type == static_cast<int>(DPListType::Monthly)) {
		dataIdx = "monthly";
	}

	if (!m_data["bonus-available"].as_bool()) {
		m_tabs->getChildByID("bonus")->setVisible(false);
	}

	if (!m_data["monthly-available"].as_bool()) {
		m_tabs->getChildByID("monthly")->setVisible(false);
	}
	
	auto packs = m_data[dataIdx].as_array();

	auto versionTxt = "Database Version: " + std::to_string(m_data["database-version"].as_int());
	m_databaseVer->setCString(versionTxt.c_str());

	//setup cells
	auto packListCells = CCArray::create();
	for (int i = 0; i < packs.size(); i++) {

		std::string name = "null";
		std::string sprite = "DP_Beginner";
		std::string plusSprite = "DP_BeginnerPlus"; //Main Only
		int listID = 0;
		int reqLevels = 0; //Main Only
		int totalLevels = 0; //Excludes Monthly
		int month = 1; //Monthly Only
		int year = 2024; //Monthly Only
		bool official = true; //Bonus Only

		name = m_data[dataIdx][i]["name"].as_string();
		sprite = m_data[dataIdx][i]["sprite"].as_string();
		if (type == static_cast<int>(DPListType::Main)) { plusSprite = m_data[dataIdx][i]["plusSprite"].as_string(); }
		listID = m_data[dataIdx][i]["listID"].as_int();
		if (type == static_cast<int>(DPListType::Main)) { reqLevels = m_data[dataIdx][i]["reqLevels"].as_int(); }
		if (type != static_cast<int>(DPListType::Monthly)) { totalLevels = m_data[dataIdx][i]["totalLevels"].as_int(); }
		if (type == static_cast<int>(DPListType::Monthly)) { month = m_data[dataIdx][i]["month"].as_int(); }
		if (type == static_cast<int>(DPListType::Monthly)) { year = m_data[dataIdx][i]["year"].as_int(); }
		if (type == static_cast<int>(DPListType::Bonus)) { official = m_data[dataIdx][i]["official"].as_bool(); }

		auto fullTitle = name;
		if (type == static_cast<int>(DPListType::Main) || type == static_cast<int>(DPListType::Legacy)) { fullTitle = name + " Demons"; }
		auto fullSprite = sprite + ".png";
		auto fullPlusSprite = plusSprite + ".png";

		CCNode* cell = ListCell::create();

		CCNode* packText = CCLabelBMFont::create(fullTitle.c_str(), "bigFont.fnt");
		packText->setScale(0.65f);
		if (fullTitle.length() > 18) { packText->setScale(0.50f); }
		if (fullTitle.length() > 25) { packText->setScale(0.425f); }
		packText->setAnchorPoint({ 0, 1 });
		packText->setPosition({ 53, 49 });

		CCNode* packSpr = CCSprite::create(Mod::get()->expandSpriteName(fullSprite.c_str()));
		packSpr->setScale(1.0f);
		packSpr->setAnchorPoint({ 0.5, 0.5 });
		packSpr->setPosition({ 28.5, 25 });

		CCNode* packPlusSpr = CCSprite::create(Mod::get()->expandSpriteName(fullPlusSprite.c_str()));
		packPlusSpr->setScale(1.0f);
		packPlusSpr->setAnchorPoint({ 0.5, 0.5 });
		packPlusSpr->setPosition({ 28.5, 25 });
		packPlusSpr->setVisible(false);

		/*auto packProgressBack = CCSprite::create("GJ_progressBar_001.png");
		packProgressBack->setAnchorPoint({0, 0.5});
		packProgressBack->setPosition({53, 15});
		packProgressBack->setScaleX(0.48f);
		packProgressBack->setScaleY(0.64f);
		packProgressBack->setColor({0, 0, 0});

		auto packProgressFront = CCSprite::create("GJ_progressBar_001.png");
		packProgressFront->setAnchorPoint({0, 0.5});
		packProgressFront->setPosition({3.74f, 10});
		packProgressFront->setScaleX(0.975f);
		packProgressFront->setScaleY(0.76f);
		packProgressFront->setColor({255, 84, 50});

		CCNode* packProgressText = CCLabelBMFont::create("0/13", "bigFont.fnt");
		packProgressText->setPosition({170, 11});
		packProgressFront->setScaleX(1);
		packProgressFront->setScaleY(0.65f);*/

		std::string progStr = "...";
		CCLabelBMFont* progText = CCLabelBMFont::create("...", "bigFont.fnt");

		if (type == static_cast<int>(DPListType::Main) && !hasRank[i].as_bool()) {
			std::string nextTier = "???";
			if (i + 1 < packs.size()) {
				nextTier = m_data[dataIdx][i + 1]["name"].as_string();
			}
			progStr = std::to_string(packProgress_main[i].as_int()) + "/" + std::to_string(reqLevels) + " to " + nextTier + " Tier.";
		}
		else if (type == static_cast<int>(DPListType::Main)) {
			progStr = std::to_string(packProgress_main[i].as_int()) + "/" + std::to_string(totalLevels) + " to Completion.";
			packPlusSpr->setVisible(true);
		}
		else if (type == static_cast<int>(DPListType::Legacy)) {
			progStr = std::to_string(packProgress_legacy[i].as_int()) + "/" + std::to_string(totalLevels) + " to Completion.";
		}
		else if (type == static_cast<int>(DPListType::Bonus)) {
			progStr = std::to_string(packProgress_bonus[i].as_int()) + "/" + std::to_string(totalLevels) + " to Completion.";
		}
		else if (type == static_cast<int>(DPListType::Monthly) && (packProgress_monthly[i] < 5)) {
			progStr = std::to_string(packProgress_monthly[i].as_int()) + "/5 to Partial Completion.";
		}
		else if (type == static_cast<int>(DPListType::Monthly)) {
			progStr = std::to_string(packProgress_monthly[i].as_int()) + "/6 to Completion.";
		}

		progText->setCString(progStr.c_str());
		progText->setPosition({ 53, 16 });
		progText->setAnchorPoint({ 0, 0.5 });
		progText->setScale(0.30f);

		if (type == static_cast<int>(DPListType::Monthly)) {
			std::string months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
			std::string monthlyDisp = months[month - 1] + " " + std::to_string(year);

			CCNode* monthlyText = CCLabelBMFont::create(monthlyDisp.c_str(), "bigFont.fnt");
			monthlyText->setScale(0.35f);
			monthlyText->setAnchorPoint({ 0, 1 });
			monthlyText->setPosition({ 53, 30 });
			monthlyText->setZOrder(1);
			cell->addChild(monthlyText);
		}

		auto cellMenu = CCMenu::create();
		cellMenu->setID("cell-menu");
		cellMenu->setPosition({ 0, 0 });
		auto viewSpr = extension::CCScale9Sprite::create("GJ_button_01.png");
		viewSpr->setPosition({ 35, 16 });
		viewSpr->setContentSize({ 66, 30 });
		auto viewText = CCLabelBMFont::create("View", "bigFont.fnt");
		viewText->setPosition({ 32, 16 });
		viewText->setScale(0.6f);
		auto viewBtn = CCMenuItemSpriteExtra::create(viewSpr, this, menu_selector(DPLayer::openList));
		viewBtn->setPosition({ 320, 25 });
		viewBtn->setTag(i);
		viewBtn->setID(dataIdx);
		viewSpr->addChild(viewText);
		cellMenu->addChild(viewBtn);

		if (i == 0 && type == static_cast<int>(DPListType::Monthly)) {
			auto goldBG = CCLayerColor::create({255, 200, 0, 255});
			cell->addChild(goldBG);

			cellMenu->setZOrder(1);
			packText->setZOrder(1);
			packSpr->setZOrder(1);
			progText->setZOrder(1);
		}

		if (i != 0 && type == static_cast<int>(DPListType::Main) && !Mod::get()->getSettingValue<bool>("unlock-all-tiers")) {
			if (!hasRank[i - 1].as_bool()) {
				auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
				lockIcon->setPosition({180, 20});
				lockIcon->setAnchorPoint({0.5f, 0});

				auto lockText = CCLabelBMFont::create("Get the Previous Rank to unlock!", "bigFont.fnt");
				lockText->setPosition({ 180, 5 });
				lockText->setAnchorPoint({ 0.5f, 0 });
				lockText->setScale(0.5f);

				cell->addChild(lockIcon);
				cell->addChild(lockText);
			}
			else {
				//packProgressBack->addChild(packProgressFront);
				//packProgressBack->addChild(packProgressText);
				cell->addChild(cellMenu);
				cell->addChild(packText);
				cell->addChild(packSpr);
				cell->addChild(packPlusSpr);
				//cell->addChild(packProgressBack);
				cell->addChild(progText);
			}
		}
		else {
			//packProgressBack->addChild(packProgressFront);
			//packProgressBack->addChild(packProgressText);
			cell->addChild(cellMenu);
			cell->addChild(packText);
			cell->addChild(packSpr);
			cell->addChild(packPlusSpr);
			//cell->addChild(packProgressBack);
			cell->addChild(progText);
		}

		packListCells->addObject(cell);
	};

	//list
	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	ListView* packListMenu = ListView::create(packListCells, 50.0, 358.0, 220.0);
	packListMenu->setAnchorPoint({ 0.5, 0.5 });
	packListMenu->setPosition({ (size.width / 2) - 180, (size.height / 2) - 115});
	packListMenu->setID("list-menu");
	packListMenu->setZOrder(0);
	this->addChild(packListMenu);
	m_list = packListMenu;
	m_currentTab = type;
}

void DPLayer::onTab(CCObject* pSender) {
	auto btn = static_cast<TabButton*>(pSender);
	auto menuType = btn->getTag();

	auto mainbtn = m_tabs->getChildByID("main");
	auto legacybtn = m_tabs->getChildByID("legacy");
	auto bonusbtn = m_tabs->getChildByID("bonus");
	auto monthlybtn = m_tabs->getChildByID("monthly");

	m_list->removeAllChildrenWithCleanup(true);
	m_list->removeMeAndCleanup();

	if (menuType == static_cast<int>(DPListType::Main)) {
		log::info("{}", "Switched to Main Tab");

		btn->toggle(true);
		static_cast<TabButton*>(legacybtn)->toggle(false);
		static_cast<TabButton*>(bonusbtn)->toggle(false);
		static_cast<TabButton*>(monthlybtn)->toggle(false);

		reloadList(static_cast<int>(DPListType::Main));
	}
	else if (menuType == static_cast<int>(DPListType::Legacy)) {
		log::info("{}", "Switched to Legacy Tab");
		
		btn->toggle(true);
		static_cast<TabButton*>(mainbtn)->toggle(false);
		static_cast<TabButton*>(bonusbtn)->toggle(false);
		static_cast<TabButton*>(monthlybtn)->toggle(false);

		reloadList(static_cast<int>(DPListType::Legacy));
	}
	else if (menuType == static_cast<int>(DPListType::Bonus)) {
		log::info("{}", "Switched to Bonus Tab");
		
		btn->toggle(true);
		static_cast<TabButton*>(legacybtn)->toggle(false);
		static_cast<TabButton*>(mainbtn)->toggle(false);
		static_cast<TabButton*>(monthlybtn)->toggle(false);

		if (!m_data["bonus-available"].as_bool()) {
			FLAlertLayer::create("Nice Try.", "nuh uh", "L")->show();
			reloadList(static_cast<int>(DPListType::Main));
		}
		else {
			reloadList(static_cast<int>(DPListType::Bonus));
		}
	}
	else if (menuType == static_cast<int>(DPListType::Monthly)) {
		log::info("{}", "Switched to Monthly Tab");
		
		btn->toggle(true);
		static_cast<TabButton*>(legacybtn)->toggle(false);
		static_cast<TabButton*>(bonusbtn)->toggle(false);
		static_cast<TabButton*>(mainbtn)->toggle(false);

		if (!m_data["monthly-available"].as_bool()) {
			FLAlertLayer::create("Nice Try.", "nuh uh", "L")->show();
			reloadList(static_cast<int>(DPListType::Main));
		}
		else {
			reloadList(static_cast<int>(DPListType::Monthly));
		}
	}
}

DPLayer::~DPLayer() {
    this->removeAllChildrenWithCleanup(true);
}