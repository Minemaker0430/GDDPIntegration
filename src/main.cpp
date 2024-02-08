//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/cocos/include/ccTypes.h>

#include <Geode/modify/CreatorLayer.hpp>

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
			m_data = matjson::parse(response);
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

	std::string const& url = "https://www.boomlings.com/database/getGJLevelLists.php";
	std::string const& fields = "secret=Wmfd2893gb7&type=0&diff=-&len=-&count=1&str=" + std::to_string(id); //thank you gd cologne :pray:
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
			list->m_levels = IDs;
			//list->m_listDesc = desc;

			auto layer = LevelListLayer::create(list);
			auto layerChildren = layer->getChildren();

			auto bg = typeinfo_cast<CCSprite*>(layerChildren->objectAtIndex(0));
			bg->setColor({18, 18, 86});

			auto menu = typeinfo_cast<CCMenu*>(layerChildren->objectAtIndex(4));
			/*auto menuChildren = menu->getChildren();

			auto commentBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(3));
			auto rateBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(4));
			auto copyBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(5));
			auto infoBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(6));
			auto favBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(menuChildren->objectAtIndex(7));*/

			/*commentBtn->setVisible(false);
			rateBtn->setVisible(false);
			copyBtn->setVisible(false);
			infoBtn->setVisible(false);
			favBtn->setVisible(false);*/

			auto downloadIcon = typeinfo_cast<CCSprite*>(layerChildren->objectAtIndex(6));
			auto downloadText = typeinfo_cast<CCLabelBMFont*>(layerChildren->objectAtIndex(7));
			auto likeIcon = typeinfo_cast<CCSprite*>(layerChildren->objectAtIndex(8));
			auto likeText = typeinfo_cast<CCLabelBMFont*>(layerChildren->objectAtIndex(9));

			downloadIcon->setVisible(false);
			//downloadText->setVisible(false);
			likeIcon->setVisible(false);
			//likeText->setVisible(false);

			auto diffIcon = typeinfo_cast<CCSprite*>(layerChildren->objectAtIndex(10));
			//diffIcon->setVisible(false);

			//auto dpIcon = CCSprite::create();

			scene->addChild(layer);
			CCDirector::sharedDirector()->pushScene(cocos2d::CCTransitionFade::create(0.5f, scene));
		}
		else {
			FLAlertLayer::create("ERROR", "This list doesn't exist! This is probably the mod developer's fault.", "OK")->show();
		}
		}).expect([](std::string const& error) {
			FLAlertLayer::create("ERROR", "Something went wrong! (" + error + ")", "OK")->show();
		});
}

bool DPLayer::init() {
	if (!CCLayer::init()) return false;

        log::info("{}", "Opened the Demon Progression menu.");

        //auto testMenu = CCMenu::create();

        auto menu = CCMenu::create();
        auto director = CCDirector::sharedDirector();
	    auto size = director->getWinSize();

	    auto bg = createLayerBG();
		bg->setColor({18, 18, 86});
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

		listMiddle->setPosition({100, 40});
		listMiddle->setContentSize({370, 230});

		listLeft->setPosition({101, 155});
		listRight->setPosition({469, 155});
		listTop->setPosition({size.width / 2, 280});
		listBottom->setPosition({size.width / 2, 30});

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
				m_data = matjson::parse(response);
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
		bonusPacksBtn->setVisible(false);

		auto monthlyPacksBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::Selected, "Monthly", this, menu_selector(DPLayer::onTab));
		monthlyPacksBtn->setPosition(136.f, 133.5f);
		monthlyPacksBtn->setID("monthly");
		monthlyPacksBtn->setTag(static_cast<int>(DPListType::Monthly));
		monthlyPacksBtn->addChild(backTabSprite);
		listTabs->addChild(monthlyPacksBtn);
		monthlyPacksBtn->setVisible(false);

		this->addChild(listTabs);
		m_tabs = listTabs;

		this->setKeyboardEnabled(true);
		this->setKeypadEnabled(true);

        return true;
}

void DPLayer::reloadList(int type) {
	
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
	
	auto packs = m_data[dataIdx].as_array();

	//setup cells
	auto packListCells = CCArray::create();
	for (int i = 0; i < packs.size(); i++) {

		std::string name = "null";
		std::string sprite = "DP_Beginner";
		std::string plusSprite = "DP_BeginnerPlus"; //Main Only
		int listID = 0;
		int reqLevels = 0; //Main Only
		int month = 1; //Monthly Only
		int year = 2024; //Monthly Only
		bool official = true; //Bonus Only

		name = m_data[dataIdx][i]["name"].as_string();
		sprite = m_data[dataIdx][i]["sprite"].as_string();
		if (type == static_cast<int>(DPListType::Main)) { plusSprite = m_data[dataIdx][i]["plusSprite"].as_string(); }
		listID = m_data[dataIdx][i]["listID"].as_int();
		if (type == static_cast<int>(DPListType::Main)) { reqLevels = m_data[dataIdx][i]["reqLevels"].as_int(); }
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
		packSpr->setScale(1.3f);
		packSpr->setAnchorPoint({ 0.5, 0.5 });
		packSpr->setPosition({ 28.5, 25 });

		//CCNode* packPlusSpr = CCSprite::create(Mod::get()->expandSpriteName(fullPlusSprite.c_str()));

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

		if (type == static_cast<int>(DPListType::Main)) {
			std::string reqStr = "Complete " + std::to_string(reqLevels) + " to move on to the next Tier.";
			CCNode* tempText = CCLabelBMFont::create(reqStr.c_str(), "bigFont.fnt");
			tempText->setPosition({ 53, 16 });
			tempText->setAnchorPoint({ 0, 0.5 });
			tempText->setScale(0.30f);
			cell->addChild(tempText);
		}

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
		viewBtn->setTag(listID);
		viewSpr->addChild(viewText);
		cellMenu->addChild(viewBtn);

		if (i == 0 && type == static_cast<int>(DPListType::Monthly)) {
			auto goldBG = CCLayerColor::create({255, 200, 0, 255});
			cell->addChild(goldBG);

			cellMenu->setZOrder(1);
			packText->setZOrder(1);
			packSpr->setZOrder(1);
		}

		//packProgressBack->addChild(packProgressFront);
		//packProgressBack->addChild(packProgressText);
		cell->addChild(cellMenu);
		cell->addChild(packText);
		cell->addChild(packSpr);
		//cell->addChild(packProgressBack);
		packListCells->addObject(cell);
	};

	//list
	ListView* packListMenu = ListView::create(packListCells, 50.0, 358.0, 220.0);
	packListMenu->setAnchorPoint({ 0.5, 0.5 });
	packListMenu->setPosition({ 104.5, 44.0 });
	packListMenu->setID("list-menu");
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

	if (menuType == static_cast<int>(DPListType::Main)) {
		log::info("{}", "Switched to Main Tab");

		btn->toggle(true);
		static_cast<TabButton*>(legacybtn)->toggle(false);
		static_cast<TabButton*>(bonusbtn)->toggle(false);
		static_cast<TabButton*>(monthlybtn)->toggle(false);

		m_list->removeAllChildrenWithCleanup(true);
		m_list->removeMeAndCleanup();
		reloadList(static_cast<int>(DPListType::Main));
	}
	else if (menuType == static_cast<int>(DPListType::Legacy)) {
		log::info("{}", "Switched to Legacy Tab");
		
		btn->toggle(true);
		static_cast<TabButton*>(mainbtn)->toggle(false);
		static_cast<TabButton*>(bonusbtn)->toggle(false);
		static_cast<TabButton*>(monthlybtn)->toggle(false);

		m_list->removeAllChildrenWithCleanup(true);
		m_list->removeMeAndCleanup();
		reloadList(static_cast<int>(DPListType::Legacy));
	}
	else if (menuType == static_cast<int>(DPListType::Bonus)) {
		log::info("{}", "Switched to Bonus Tab");
		
		btn->toggle(true);
		static_cast<TabButton*>(legacybtn)->toggle(false);
		static_cast<TabButton*>(mainbtn)->toggle(false);
		static_cast<TabButton*>(monthlybtn)->toggle(false);

		m_list->removeAllChildrenWithCleanup(true);
		m_list->removeMeAndCleanup();
		reloadList(static_cast<int>(DPListType::Bonus));
	}
	else if (menuType == static_cast<int>(DPListType::Monthly)) {
		log::info("{}", "Switched to Monthly Tab");
		
		btn->toggle(true);
		static_cast<TabButton*>(legacybtn)->toggle(false);
		static_cast<TabButton*>(bonusbtn)->toggle(false);
		static_cast<TabButton*>(mainbtn)->toggle(false);

		m_list->removeAllChildrenWithCleanup(true);
		m_list->removeMeAndCleanup();
		reloadList(static_cast<int>(DPListType::Monthly));
	}
}

DPLayer::~DPLayer() {
    this->removeAllChildrenWithCleanup(true);
}