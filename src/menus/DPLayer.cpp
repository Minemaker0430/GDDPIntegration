//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/JsonValidation.hpp>

#include <ctime>

#include "DPLayer.hpp"
#include "DPListLayer.hpp"
#include "RecommendedLayer.hpp"
#include "../popups/StatsPopup.hpp"
#include "../popups/SupportPopup.hpp"
#include "../popups/NewsPopup.hpp"
#include "../popups/XPPopup.hpp"
#include "../popups/SearchPopup.hpp"
#include "../popups/RoulettePopup.hpp"
#include "../popups/dev/VerificationPopup.hpp"
#include "../Utils.hpp"
#include "../XPUtils.hpp"
#include "../RecommendedUtils.hpp"
#include "../CustomText.hpp"

//geode namespace
using namespace geode::prelude;

/*
Save Format:

"listID": {
	"type": "main",
	"completed": false,
	"progress": 2,
	"has-rank": false
}

*/

//Main DP Layer
void DPLayer::callback(CCObject*) {
	auto scene = CCScene::create(); // creates the scene
	auto dpLayer = DPLayer::create(); //creates the layer

	scene->addChild(dpLayer);

	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition

	return;
}

void DPLayer::keyBackClicked() {

	this->unschedule(schedule_selector(DPLayer::updateMonthlyTimer));
	Mod::get()->setSavedValue<bool>("in-gddp", false);
	CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);

	return;
}

void DPLayer::backButton(CCObject*) {

	Mod::get()->setSavedValue<bool>("in-gddp", false);
	keyBackClicked();

	return;
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

void DPLayer::soonCallback(CCObject*) {
	FLAlertLayer::create("Coming Soon!", "This feature hasn't been implemented yet but will be in the future!", "OK")->show();
	return;
}

void DPLayer::reloadCallback(CCObject*) {
	reloadData(false);
	return;
}

void DPLayer::reloadData(bool isInit) {

	if (m_finishedLoading || isInit) {

		m_finishedLoading = false;

		if (!isInit && !m_error) {
			m_list->removeAllChildrenWithCleanup(true);
			m_list->removeMeAndCleanup();
		}

		m_loadcircle = LoadingCircle::create();
		m_loadcircle->m_parentLayer = this;
		m_loadcircle->show();

		//m_reload->setVisible(false);
		m_tabs->setVisible(false);
		//m_backMenu->setVisible(false);
		m_errorText->setVisible(false);

		//this->setKeyboardEnabled(false);
		//this->setKeypadEnabled(false);

		std::string dataURL = "https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/main-list.json";

		//log::info("{}", GameManager::sharedState()->m_playerName);
		/*if (GameManager::sharedState()->m_playerName == "Minemaker0430") {
			log::info("Hello, me");
			dataURL = "https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/dev-list.json";
		}
		else {*/
		
		//}

		// download data

		//list
		m_listListener.bind([&](web::WebTask::Event* e) {
			if (auto res = e->getValue()) {
				//log::info("{}", res->string().unwrapOr("Uh oh!"));
				if (res->ok() && res->json().isOk() && !res->json().isErr()) {
					m_data = res->json().unwrapOrDefault();
					Mod::get()->setSavedValue<matjson::Value>("cached-data", m_data);

					m_tabs->setVisible(true);
					m_loadcircle->fadeAndRemove();

					m_finishedLoading = true;
					m_error = false;
					log::info("List data loaded!");

					// check completed levels in player's save file
					auto glm = GameLevelManager::sharedState();
					auto glmCompletedLvls = glm->getCompletedLevels(false);

					if (glmCompletedLvls->count() > 0) {
						for (int i = 0; i < glmCompletedLvls->indexOfObject(glmCompletedLvls->lastObject()); i++) {
							auto lvl = static_cast<GJGameLevel*>(glmCompletedLvls->objectAtIndex(i));
							auto lvlID = lvl->m_levelID.value();

							if (m_data["level-data"].contains(std::to_string(lvlID)) && lvl->m_normalPercent.value() == 100) {
								auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
								if (std::find(completedLvls.begin(), completedLvls.end(), lvlID) == completedLvls.end()) {
									completedLvls.insert(completedLvls.begin(), lvlID);
									Mod::get()->setSavedValue<std::vector<int>>("completed-levels", completedLvls);
								}
							}
						}
					}

					reloadList(m_currentTab);
				}
				else {
					m_loadcircle->fadeAndRemove();

					m_errorText->setCString(fmt::format("Something went wrong...\n(Code: {}, JSON Error: {})", res->code(), res->json().isErr()).c_str());
					m_errorText->setVisible(true);

					m_finishedLoading = true;
					m_error = true;
				}
			}
			else if (e->isCancelled()) {
				log::info("Cancelled List request.");
			}
			});

		auto listReq = web::WebRequest();
		m_listListener.setFilter(listReq.get(dataURL));

		//skillsets
		m_skillListener.bind([&](web::WebTask::Event* e) {
			if (web::WebResponse* res = e->getValue()) {
				//log::info("{}", res->string().unwrapOr("Uh oh!"));
				if (res->ok() && res->json().isOk() && !res->json().isErr()) {
					Mod::get()->setSavedValue<matjson::Value>("skillset-info", res->json().unwrapOrDefault());
					log::info("Updated skillset info.");
				}
				else {
					log::info("Something went wrong getting the Skillset Data. ({}, {})", res->code(), res->json().isErr());
				}
			}
			else if (e->isCancelled()) {
				log::info("Cancelled Skillsets request.");
			}
			});

		auto skillReq = web::WebRequest();
		m_skillListener.setFilter(skillReq.get("https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/skill-badges.json"));

	}

	return;
}

void DPLayer::openList(CCObject* sender) {
	//FLAlertLayer::create("the", "bingle bong", "OK")->show();
	auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
	auto id = btn->getTag();
	auto type = btn->getID();

	auto isPractice = false;

	if (type == "main-practice") {
		isPractice = true;
		type = "main";
	}
	else if (type == "legacy-practice") {
		isPractice = true;
		type = "legacy";
	}

	//auto listID = m_data[type][id]["listID"].as_int();
	//auto reqLevels = 0;
	//if (type == "main") { reqLevels = m_data[type][id]["reqLevels"].as<int>().unwrapOr(999); }
	//auto hasPractice = false;
	//if (type == "main") { hasPractice = m_data[type][id]["practice"].asBool().unwrapOr(false); }
	//auto mainPack = 0;
	//if (type == "legacy") { mainPack = m_data[type][id]["mainPack"].as<int>().unwrapOr(0); }
	//std::vector<int> levelIDs = {};
	//levelIDs = m_data[type][id]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
	//std::vector<int> practiceIDs = {};
	//if (type == "main") { practiceIDs = m_data["main"][id]["practiceIDs"].as<std::vector<int>>().unwrapOrDefault(); }
	//if (type == "legacy") { practiceIDs = m_data["main"][mainPack]["practiceIDs"].as<std::vector<int>>().unwrapOrDefault(); }

	auto scene = CCScene::create(); // creates the scene
	auto dpLayer = DPListLayer::create(type.c_str(), id, isPractice); //creates the layer
	dpLayer->m_isPractice = isPractice;

	scene->addChild(dpLayer);

	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition

	return;
}

void DPLayer::achievementsCallback(CCObject* sender) {
	if (m_finishedLoading && !m_error) {
		StatsPopup::create()->show();
	}

	return;
}

void DPLayer::supportCallback(CCObject* sender) {
	SupportPopup::create()->show();

	return;
}

void DPLayer::newsCallback(CCObject* sender) {
	NewsPopup::create()->show();

	return;
}

void DPLayer::searchCallback(CCObject* sender) {
	if (m_finishedLoading && !m_error) {
		SearchPopup::create()->show();
	}

	return;
}

void DPLayer::rouletteCallback(CCObject* sender) {
	if (m_finishedLoading && !m_error) {
		RoulettePopup::create()->show();
	}

	return;
}

void DPLayer::recommendedCallback(CCObject* sender) {
	if (m_finishedLoading && !m_error) {
		RecommendedUtils::generateRecommendations();

		auto scene = CCScene::create(); // creates the scene
		auto dpLayer = RecommendedLayer::create(); //creates the layer

		scene->addChild(dpLayer);

		CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition
	}

	return;
}

void DPLayer::xpCallback(CCObject* sender) {
	if (m_finishedLoading && !m_error) {
		//get xp values
		XPUtils::getLevels();

		XPPopup::create()->show();
	}

	return;
}

bool DPLayer::init() {
	if (!CCLayer::init()) return false;

	log::info("Opened the Demon Progression menu.");

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
	rCornerSprite->setPosition({ size.width, 0 });
	rCornerSprite->setFlipX(true);
	this->addChild(rCornerSprite);

	this->addChild(menu);

	//error text
	m_errorText = CCLabelBMFont::create("Something went wrong...\n(Code: 200, JSON Error: false)", "bigFont.fnt");
	m_errorText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	m_errorText->setPosition({ size.width / 2, size.height / 2 });
	m_errorText->setScale(0.6f);
	m_errorText->setZOrder(100);
	m_errorText->setVisible(false);
	m_errorText->setID("error-text");
	this->addChild(m_errorText);

	//monthly timer
	m_monthlyTimer = CCLabelBMFont::create("Time until next Monthly:\n0d 0h 0m 0s", "goldFont.fnt");
	m_monthlyTimer->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	m_monthlyTimer->setPosition({ size.width / 2, 35.f });
	m_monthlyTimer->setScale(0.6f);
	m_monthlyTimer->setZOrder(100);
	m_monthlyTimer->setVisible(false);
	m_monthlyTimer->setID("monthly-text");
	this->addChild(m_monthlyTimer);

	if (Mod::get()->getSettingValue<bool>("show-monthly-timer")) { this->schedule(schedule_selector(DPLayer::updateMonthlyTimer), 1.f); }

	//back button
	auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backButton = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(DPLayer::backButton));
	auto backMenu = CCMenu::create();
	backMenu->addChild(backButton);
	backMenu->setPosition({ 25, size.height - 25 });
	backMenu->setID("back-menu");
	//backMenu->setVisible(false);
	this->addChild(backMenu);
	m_backMenu = backMenu;

	//list bg
	auto listMiddle = CCLayerColor::create({ 194, 114, 62, 255 });
	auto listLeft = CCSprite::createWithSpriteFrameName("GJ_table_side_001.png");
	auto listRight = CCSprite::createWithSpriteFrameName("GJ_table_side_001.png");
	auto listTop = CCSprite::createWithSpriteFrameName("GJ_table_top02_001.png");
	auto listBottom = CCSprite::createWithSpriteFrameName("GJ_table_bottom_001.png");

	listMiddle->setAnchorPoint({ 0.5, 0.5 });
	listMiddle->setPosition({ (size.width / 2) - 180, (size.height / 2) - 115 });
	listMiddle->setContentSize({ 370, 230 });
	listMiddle->setZOrder(-1);

	listLeft->setPosition({ (size.width / 2) - 183.5f, (size.height / 2) });
	listRight->setPosition({ (size.width / 2) + 183.5f, (size.height / 2) });
	listTop->setPosition({ size.width / 2, (size.height / 2) + 120 });
	listBottom->setPosition({ size.width / 2, (size.height / 2) - 125 });

	listRight->setScaleX(-1);
	listRight->setScaleY(3.6f);
	listLeft->setScaleY(3.6f);

	listLeft->setZOrder(9);
	listRight->setZOrder(9);
	listTop->setZOrder(10);
	listBottom->setZOrder(10);

	listLeft->setID("list-left");
	listRight->setID("list-right");
	listTop->setID("list-top");
	listBottom->setID("list-bottom");

	//if (!Loader::get()->isModLoaded("alphalaneous.transparent_lists")) {}
	this->addChild(listMiddle);
	this->addChild(listLeft);
	this->addChild(listRight);
	this->addChild(listTop);
	this->addChild(listBottom);

	//reload menu
	auto reloadMenu = CCMenu::create();
	reloadMenu->setPosition({ 0, 0 });
	auto reloadBtnSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
	auto reloadBtn = CCMenuItemSpriteExtra::create(reloadBtnSprite, this, menu_selector(DPLayer::reloadCallback));
	reloadBtn->setPosition({ 30.f, 30.f });
	reloadMenu->addChild(reloadBtn);
	reloadMenu->setID("reload-menu");
	reloadMenu->setZOrder(11);
	this->addChild(reloadMenu);
	m_reload = reloadMenu;

	//support menu
	auto supportMenu = CCMenu::create();
	supportMenu->setPosition({ 0, 0 });
	auto supportBtnSprite = CircleButtonSprite::createWithSpriteFrameName(("DP_Support.png"_spr), 1.0f, CircleBaseColor::Pink, CircleBaseSize::Small);
	auto supportBtn = CCMenuItemSpriteExtra::create(supportBtnSprite, this, menu_selector(DPLayer::supportCallback));
	supportBtn->setPosition({ 75.f, 30.f });
	supportMenu->addChild(supportBtn);
	supportMenu->setID("support-menu");
	supportMenu->setZOrder(11);
	if (Mod::get()->getSettingValue<bool>("show-support")) { this->addChild(supportMenu); }

	//news menu
	auto newsMenu = CCMenu::create();
	newsMenu->setPosition({ 0, 0 });
	auto newsBtnSprite = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
	auto newsBtn = CCMenuItemSpriteExtra::create(newsBtnSprite, this, menu_selector(DPLayer::newsCallback));
	newsBtn->setPosition({ 30.f, 80.f });
	newsMenu->addChild(newsBtn);
	newsMenu->setID("news-menu");
	//this->addChild(newsMenu);

	m_currentTab = static_cast<int>(DPListType::Main);

	//extra buttons
	auto achievementBtnSprite = CCSprite::createWithSpriteFrameName("GJ_achBtn_001.png");
	auto leaderboardsBtnSprite = CCSprite::createWithSpriteFrameName("GJ_statsBtn_001.png");
	auto leaderboardButton = CCMenuItemSpriteExtra::create(leaderboardsBtnSprite, this, menu_selector(DPLayer::soonCallback));
	auto achievementButton = CCMenuItemSpriteExtra::create(achievementBtnSprite, this, menu_selector(DPLayer::achievementsCallback));
	achievementButton->setPosition({ size.width - 30, 30 });
	leaderboardButton->setPosition({ size.width - 30, 80 });
	achievementButton->setID("stats-btn");
	leaderboardButton->setID("leaderboards-btn");
	auto extrasMenu = CCMenu::create();
	extrasMenu->setPosition({ 0, 0 });
	//extrasMenu->addChild(leaderboardButton);
	extrasMenu->addChild(achievementButton);
	extrasMenu->setID("extras-menu");
	extrasMenu->setZOrder(11);
	this->addChild(extrasMenu);

	//utility tabs
	auto skillsetsSpr = CircleButtonSprite::createWithSpriteFrameName("DP_Search.png"_spr);
	auto rouletteSpr = CircleButtonSprite::createWithSpriteFrameName("DP_Roulette.png"_spr);
	auto recommendedSpr = CircleButtonSprite::createWithSpriteFrameName("DP_Recommended.png"_spr);

	auto skillsetsBtn = CCMenuItemSpriteExtra::create(skillsetsSpr, this, menu_selector(DPLayer::searchCallback));
	auto rouletteBtn = CCMenuItemSpriteExtra::create(rouletteSpr, this, menu_selector(DPLayer::rouletteCallback));
	auto recommendedBtn = CCMenuItemSpriteExtra::create(recommendedSpr, this, menu_selector(DPLayer::recommendedCallback));

	skillsetsBtn->setID("skillsets-btn");
	rouletteBtn->setID("roulette-btn");
	recommendedBtn->setID("recommended-btn");

	auto utilityLayout = AxisLayout::create(Axis::Column);
	utilityLayout->setAxisReverse(true);

	auto utilityMenu = CCMenu::create();
	utilityMenu->setPosition({ listLeft->getPositionX() - 35.f, size.height / 2 });
	if (Mod::get()->getSettingValue<bool>("enable-search")) { utilityMenu->addChild(skillsetsBtn); }
	if (Mod::get()->getSettingValue<bool>("enable-roulette")) { utilityMenu->addChild(rouletteBtn); }
	if (Mod::get()->getSettingValue<bool>("enable-recommendations")) { utilityMenu->addChild(recommendedBtn); }
	utilityMenu->setID("utility-menu");
	utilityMenu->setLayout(utilityLayout, true);
	this->addChild(utilityMenu);

	//xp button
	auto xpText = CCLabelBMFont::create("XP", "bigFont.fnt");
	auto xpSpr = CircleButtonSprite::create(xpText);
	typeinfo_cast<CCLabelBMFont*>(xpSpr->getChildren()->objectAtIndex(0))->setPosition({ 24.375f, 25.5f });
	auto xpBtn = CCMenuItemSpriteExtra::create(xpSpr, this, menu_selector(DPLayer::xpCallback));
	xpBtn->setID("xp-btn");

	auto xpMenu = CCMenu::create();
	xpMenu->setPosition({ listRight->getPositionX() + 35.f, size.height / 2 });
	xpMenu->addChild(xpBtn);
	xpMenu->setID("xp-menu");
	if (Mod::get()->getSettingValue<bool>("show-xp")) { this->addChild(xpMenu); }

	//list tabs
	auto listTabs = CCMenu::create();
	listTabs->setID("list-tabs");

	auto backTabSprite = CCSprite::createWithSpriteFrameName("DP_tabBack.png"_spr);
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

	listTabs->setVisible(false);
	this->addChild(listTabs);
	m_tabs = listTabs;

	m_databaseVer = CCLabelBMFont::create("Loading...", "chatFont.fnt");
	m_databaseVer->setAnchorPoint({ 1, 1 });
	m_databaseVer->setPosition({ size.width - 1, size.height - 1 });
	m_databaseVer->setScale(0.5f);
	m_databaseVer->setZOrder(1);

	if (!Mod::get()->getSettingValue<bool>("show-database-version")) {
		m_databaseVer->setVisible(false);
	}

	this->addChild(m_databaseVer);

	// mod/dev stuff
	auto secretFile = Mod::get()->getConfigDir() += std::filesystem::path("\\client_id.txt");

	auto fileCheck = file::readDirectory(Mod::get()->getConfigDir()).unwrapOrDefault();
	log::info("{}, {}", fileCheck, secretFile);

	if (std::find(fileCheck.begin(), fileCheck.end(), secretFile) != fileCheck.end()) {
		log::info("Found Client ID File.");

		//dev menu
		auto devMenu = CCMenu::create();
		devMenu->setPosition({ 0, 0 });
		auto devBtnSprite = CCSprite::createWithSpriteFrameName("GJ_starBtnMod_001.png");
		auto devBtn = CCMenuItemSpriteExtra::create(devBtnSprite, this, menu_selector(DPLayer::devCallback));
		devBtn->setPosition({ size.width - 30.f, size.height - 30.f });
		devMenu->addChild(devBtn);
		devMenu->setID("dev-menu");
		this->addChild(devMenu);
	}
	else {
		//log::info("Client ID File not found, don't add mod button.");
	}

	// download data
	reloadData(true);

	this->setKeyboardEnabled(true);
	this->setKeypadEnabled(true);

	return true;
}

void DPLayer::devCallback(CCObject*) {
	auto popup = VerificationPopup::create();
	popup->show();

	return;
}

void DPLayer::updateMonthlyTimer(float dt) {
	//imma be honest i copied this from w3schools but it works soooo
	time_t now;
	time_t nextMonthly;
	struct tm datetime;

	now = time(NULL);
	datetime = *localtime(&now);
	datetime.tm_year = m_currentYear;
	datetime.tm_mon = m_currentMonth + 1;
	datetime.tm_mday = 1;
	datetime.tm_hour = 0; 
	datetime.tm_min = 0; 
	datetime.tm_sec = 0;
	//datetime.is_dst = -1;
	nextMonthly = mktime(&datetime);

	int diff = difftime(nextMonthly, now); //secs until next monthly, now we convert it
	diff = std::max(diff, 0);

    int days = diff / 86400;
    int hours = (diff / 3600) % 24;
    int minutes = (diff / 60) % 60;
	int seconds = diff % 60;

	if (diff == 0) {
		m_monthlyTimer->setCString("Time until next Monthly:\nSoon...");
	}
	else {
		m_monthlyTimer->setCString(fmt::format("Time until next Monthly:\n{}d {}h {}m {}s", days, hours, minutes, seconds).c_str());
	}
}

void DPLayer::reloadList(int type) {

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	//check for errors
	auto jsonCheck = checkJson(m_data, "");

	if (!jsonCheck.ok()) {
		auto alert = FLAlertLayer::create("ERROR", "Something went wrong validating the GDDP list data.", "OK");
		alert->setParent(this);
		alert->show();

		return;
	}

	//all save stuff
	auto localDatabaseVer = Mod::get()->getSavedValue<int>("database-version", 0);

	Mod::get()->setSavedValue<int>("database-version", m_data["database-version"].as<int>().unwrapOr(0));
	log::info("{}", Mod::get()->getSavedValue<int>("database-version"));

	//do everything else
	auto dataIdx = "";

	switch(type) {
		case static_cast<int>(DPListType::Main):
		{
			dataIdx = "main";
			break;
		}
		case static_cast<int>(DPListType::Legacy):
		{
			dataIdx = "legacy";
			break;
		}
		case static_cast<int>(DPListType::Bonus):
		{
			dataIdx = "bonus";
			break;
		}
		case static_cast<int>(DPListType::Monthly):
		{
			dataIdx = "monthly";
			break;
		}
	}

	if (!m_data.contains(dataIdx)) { return; }

	auto packs = m_data[dataIdx].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

	auto versionTxt = fmt::format("Database Version: {}", std::to_string(m_data["database-version"].as<int>().unwrapOr(0)));
	m_databaseVer->setCString(versionTxt.c_str());

	if (packs.empty()) { return; }

	m_monthlyTimer->setVisible(false);
	if (type == static_cast<int>(DPListType::Monthly) && Mod::get()->getSettingValue<bool>("show-monthly-timer")) {
		m_monthlyTimer->setVisible(true);

		if (m_data["monthly"][0]["name"].asString().unwrapOr("???") != "???") {
			m_currentMonth = m_data["monthly"][0]["month"].as<int>().unwrapOr(1) - 1;
			m_currentYear = m_data["monthly"][0]["year"].as<int>().unwrapOr(1987) - 1900;
		}
	}

	RecommendedUtils::validateLevels();

	//setup cells
	auto packListCells = CCArray::create();
	for (int i = 0; i < packs.size(); i++) {

		std::string name = "null";
		std::string sprite = "DP_Beginner";
		std::string plusSprite = "DP_Beginner"; //Main Only
		//int listID = 0;
		std::string saveID = "null";
		std::vector<int> levelIDs = {};
		std::vector<int> practiceIDs = {};
		int reqLevels = 0; //Main Only
		int month = 1; //Monthly Only
		int year = 0; //Monthly Only
		//bool hasPractice = false; //Main Only
		int mainPack = 0; //Legacy Only

		if (!m_data[dataIdx][i]["name"].isNull()) { name = m_data[dataIdx][i]["name"].asString().unwrapOr("null"); }
		if (!m_data[dataIdx][i]["sprite"].isNull()) { sprite = m_data[dataIdx][i]["sprite"].asString().unwrapOr("DP_Unknown"); }
		if (type == static_cast<int>(DPListType::Main) && !m_data[dataIdx][i]["plusSprite"].isNull()) { plusSprite = m_data[dataIdx][i]["plusSprite"].asString().unwrapOr("DP_Unknown"); }
		//listID = m_data[dataIdx][i]["listID"].as_int(); //only used to obtain old saves
		if (type != static_cast<int>(DPListType::Monthly) && !m_data[dataIdx][i]["saveID"].isNull()) { saveID = m_data[dataIdx][i]["saveID"].asString().unwrapOr("null"); }
		if (!m_data[dataIdx][i]["levelIDs"].isNull()) { levelIDs = m_data[dataIdx][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault(); }
		if ((type == static_cast<int>(DPListType::Main) || type == static_cast<int>(DPListType::Legacy)) && !m_data[dataIdx][i]["practiceIDs"].isNull()) { practiceIDs = m_data[dataIdx][i]["practiceIDs"].as<std::vector<int>>().unwrapOrDefault(); }
		if (type == static_cast<int>(DPListType::Main) && !m_data[dataIdx][i]["reqLevels"].isNull()) { reqLevels = m_data[dataIdx][i]["reqLevels"].as<int>().unwrapOr(999); }
		if (type == static_cast<int>(DPListType::Monthly) && !m_data[dataIdx][i]["month"].isNull()) { month = m_data[dataIdx][i]["month"].as<int>().unwrapOr(1); }
		if (type == static_cast<int>(DPListType::Monthly) && !m_data[dataIdx][i]["year"].isNull()) { year = m_data[dataIdx][i]["year"].as<int>().unwrapOr(1987); }
		//if (type == static_cast<int>(DPListType::Main) && !m_data[dataIdx][i]["practice"].isNull()) { hasPractice = m_data[dataIdx][i]["practice"].asBool().unwrapOrDefault(); }
		if (type == static_cast<int>(DPListType::Legacy) && !m_data[dataIdx][i]["mainPack"].isNull()) { mainPack = m_data[dataIdx][i]["mainPack"].as<int>().unwrapOr(0); }

		if (type == static_cast<int>(DPListType::Monthly)) { saveID = fmt::format("{}-{}", month, year); }

		//get list save
		auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

		//check save
		//get completed levels
		auto progress = 0;
		auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
		for (auto const& level : levelIDs) {
			if (std::find(completedLvls.begin(), completedLvls.end(), level) != completedLvls.end()) {
				progress += 1;
			}
		}

		/*if (type == static_cast<int>(DPListType::Main) && !listSave.hasRank && progress < listSave.progress) { //If you don't have the rank, any progress you have will be maintained even if a level is moved to legacy
			progress = listSave.progress;
		}*/

		//update status

		auto hasRank = listSave.hasRank;

		if ((progress >= reqLevels) && type == static_cast<int>(DPListType::Main)) {
			hasRank = true;
		}

		auto completed = listSave.completed;

		if (progress == levelIDs.size() && type != static_cast<int>(DPListType::Monthly)) {
			completed = true;
			if (type == static_cast<int>(DPListType::Main)) {
				hasRank = true;
			}
		}
		else if (type == static_cast<int>(DPListType::Monthly) && (progress > 5)) {
			completed = true;
		}
		else {
			completed = false;
		}

		if (type == static_cast<int>(DPListType::Monthly) && progress >= 5) {
			auto completedMonthlies = Mod::get()->getSavedValue<std::vector<std::string>>("monthly-completions");

			if (std::find(completedMonthlies.begin(), completedMonthlies.end(), saveID) == completedMonthlies.end()) {
				completedMonthlies.insert(completedMonthlies.begin(), saveID);
				Mod::get()->setSavedValue<std::vector<std::string>>("monthly-completions", completedMonthlies);
			}
		}

		//save
		listSave.progress = progress;
		listSave.completed = completed;
		listSave.hasRank = hasRank;
		Mod::get()->setSavedValue<ListSaveFormat>(saveID, ListSaveFormat{ .progress = progress, .completed = completed, .hasRank = hasRank });

		auto fullTitle = name;
		if (type == static_cast<int>(DPListType::Main) || type == static_cast<int>(DPListType::Legacy)) {
			if (listSave.hasRank && type == static_cast<int>(DPListType::Main)) {
				fullTitle = fmt::format("{}+ Demons", name);
			}
			else {
				fullTitle = fmt::format("{} Demons", name);
			}
		}
		if (type == static_cast<int>(DPListType::Monthly) && listSave.progress >= 5) {
			fullTitle = fmt::format("{} +", name);
		}
		auto fullSprite = fmt::format("{}.png", sprite);
		auto fullPlusSprite = fmt::format("{}.png", plusSprite);

		//fallbacks
		if (!CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSprite).data())) {
			fullSprite = "DP_Unknown.png";
		}

		if (!CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSprite).data())) {
			fullPlusSprite = "DP_Unknown.png";
		}

		CCNode* cell = ListCell::create();
		
		CCLabelBMFont* packText = CCLabelBMFont::create(fullTitle.c_str(), "bigFont.fnt");
		packText->setScale(0.65f);
		if (fullTitle.length() > 18) { packText->setScale(0.50f); }
		if (fullTitle.length() > 25) { packText->setScale(0.425f); }
		packText->setAnchorPoint({ 0, 1 });
		packText->setPosition({ 53, 49 });
		packText->setID("pack-text");

		//log::info("{}", DPTextEffects.dump());

		//custom pack text
		auto customPackText = CustomText::create(fullTitle);
		if (Mod::get()->getSettingValue<bool>("custom-pack-text") && DPTextEffects.contains(saveID)) {
			customPackText->addEffectsFromProperties(DPTextEffects[saveID].as<matjson::Value>().unwrapOrDefault());
			customPackText->setScale(0.65f);
			if (fullTitle.length() > 18) { customPackText->setScale(0.50f); }
			if (fullTitle.length() > 25) { customPackText->setScale(0.425f); }
			customPackText->setAnchorPoint({ 0, 1 });
			customPackText->setPosition({ 53, 49 });
			customPackText->setID("custom-pack-text");

			packText->setVisible(false);
		}
		else {
			customPackText->setVisible(false);
		}

		if (type == static_cast<int>(DPListType::Bonus) && Mod::get()->getSettingValue<bool>("disable-fancy-bonus-text")) {
			packText->setVisible(true);
			customPackText->setVisible(false);	
		}

		if (listSave.completed) {
			packText->setFntFile("goldFont.fnt");
			packText->setScale(0.85f);
			if (fullTitle.length() > 18) { packText->setScale(0.65f); }
			if (fullTitle.length() > 25) { packText->setScale(0.55f); }

			if (!Mod::get()->getSettingValue<bool>("override-gold-text")) {
				customPackText->setVisible(false);
				packText->setVisible(true);
			}
		}

		CCNode* packSpr = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);

		if (fullSprite != "DP_Invisible.png") {
			packSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSprite).data());
			packSpr->setScale(1.0f);
			packSpr->setAnchorPoint({ 0.5, 0.5 });
			packSpr->setPosition({ 28.5, 25 });
		}
		else {
			packSpr->setVisible(false);
		}
		packSpr->setID("pack-sprite");

		CCNode* packPlusSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSprite).data());
		packPlusSpr->setScale(1.0f);
		packPlusSpr->setAnchorPoint({ 0.5, 0.5 });
		packPlusSpr->setPosition({ 28.5, 25 });
		packPlusSpr->setVisible(false);
		packPlusSpr->setID("pack-plus-sprite");

		auto packProgressBack = CCSprite::create("GJ_progressBar_001.png");
		packProgressBack->setAnchorPoint({ 0, 0.5 });
		packProgressBack->setPosition({ 53, 15 });
		packProgressBack->setColor({ 0, 0, 0 });
		packProgressBack->setOpacity(128);
		packProgressBack->setID("progress-bar");

		auto packProgressFront = CCSprite::create("GJ_progressBar_001.png");
		packProgressFront->setAnchorPoint({ 0, 0.5 });
		packProgressFront->setPosition({ 0.0f, 10.f });
		packProgressFront->setScaleX(0.98f);
		packProgressFront->setScaleY(0.75f);
		packProgressFront->setZOrder(1);

		if (listSave.completed) {
			packProgressFront->setColor({ 255, 255, 0 });
		}
		else if ((type == static_cast<int>(DPListType::Main) && !listSave.hasRank) || (type == static_cast<int>(DPListType::Monthly) && listSave.progress < 5)) {
			packProgressFront->setColor({ 255, 84, 50 });
		}
		else {
			packProgressFront->setColor({ 80, 190, 255 });
		}

		auto progressPercent = 0.0f;

		if (type == static_cast<int>(DPListType::Monthly)) {
			if (listSave.progress >= 5) {
				progressPercent = static_cast<float>(listSave.progress) / 6.f;
			}
			else {
				progressPercent = static_cast<float>(listSave.progress) / 5.f;
			}
		}
		else if (listSave.hasRank || type != static_cast<int>(DPListType::Main)) {
			progressPercent = static_cast<float>(listSave.progress) / static_cast<float>(levelIDs.size());
		}
		else {
			progressPercent = static_cast<float>(listSave.progress) / static_cast<float>(reqLevels);
		}

		auto clippingNode = CCClippingNode::create();
		auto stencil = CCScale9Sprite::create("square02_001.png");
		stencil->setAnchorPoint({ 0, 0.5f });
		stencil->setContentWidth(packProgressFront->getScaledContentSize().width);
		stencil->setScaleX(progressPercent);
		stencil->setContentHeight(100);
		clippingNode->setStencil(stencil);
		clippingNode->setAnchorPoint({ 0, 0.5f });
		clippingNode->setPosition({ 3.25f, 10.5f });
		clippingNode->setContentWidth(packProgressFront->getContentWidth() - 2.f);
		clippingNode->setContentHeight(20);
		clippingNode->addChild(packProgressFront);
		packProgressBack->addChild(clippingNode);

		packProgressBack->setScaleX(0.6f);
		packProgressBack->setScaleY(0.65f);

		std::string progStr = "...";
		CCLabelBMFont* progText = CCLabelBMFont::create("...", "bigFont.fnt");

		switch(type) {
			case static_cast<int>(DPListType::Main):
			{
				if (!listSave.hasRank) {
					std::string nextTier = "???";
					if (i + 1 < packs.size()) {
						nextTier = m_data[dataIdx][i + 1]["name"].asString().unwrapOr("???");
					}
					progStr = fmt::format("{}/{} to {} Tier", std::to_string(listSave.progress), std::to_string(reqLevels), nextTier);
				}
				else if (listSave.completed && progressPercent == 1.0f) {
					progStr = "100% Complete!";
					progText->setFntFile("goldFont.fnt");
					packPlusSpr->setVisible(true);
				}
				else {
					progStr = fmt::format("{}/{} to Completion", std::to_string(listSave.progress), std::to_string(levelIDs.size()));
					packPlusSpr->setVisible(true);
				}
				break;
			}
			case static_cast<int>(DPListType::Legacy): 
			{
				if (listSave.completed) {
					progStr = "100% Complete!";
					progText->setFntFile("goldFont.fnt");
				}
				else {
					progStr = fmt::format("{}/{} to Completion", std::to_string(listSave.progress), std::to_string(levelIDs.size()));
				}
				break;
			}
			case static_cast<int>(DPListType::Bonus):
			{
				if (listSave.completed) {
					progStr = "100% Complete!";
					progText->setFntFile("goldFont.fnt");
				}
				else {
					progStr = fmt::format("{}/{} to Completion", std::to_string(listSave.progress), std::to_string(levelIDs.size()));
				}
				break;
			}
			case static_cast<int>(DPListType::Monthly):
			{
				auto epicSprite = CCSprite::createWithSpriteFrameName("GJ_epicCoin_001.png");
				epicSprite->setPosition({ 21.75f, 18.f });
				epicSprite->setZOrder(-1);

				if (listSave.completed) {
					progStr = "100% Complete!";
					progText->setFntFile("goldFont.fnt");
					packSpr->addChild(epicSprite);
				}
				else if (listSave.progress < 5) {
					progStr = fmt::format("{}/5 to Partial Completion", std::to_string(listSave.progress));
				}
				else {
					progStr = fmt::format("{}/6 to Completion", std::to_string(listSave.progress));
					packSpr->addChild(epicSprite);
				}
				break;
			}
		}

		progText->setCString(progStr.c_str());
		progText->setPosition({ 155.f, 16.f });
		progText->setAnchorPoint({ 0.5, 0.5 });
		progText->setScale(0.35f);
		if (listSave.completed) { progText->setScale(0.45f); }
		progText->setID("progress-text");

		if (type == static_cast<int>(DPListType::Monthly)) {
			std::string months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
			ccColor3B monthColor[12] = { {255, 0, 0}, {255, 128, 0}, {255, 255, 0}, {128, 255, 0}, {0, 255, 0}, {0, 255, 128}, {0, 255, 255}, {0, 128, 255}, {0, 0, 255}, {128, 0, 255}, {255, 0, 255}, {255, 0, 128} };
			std::string monthlyDisp = fmt::format("{} {}", months[month - 1], std::to_string(year));

			auto yearBg = CCLayerColor::create({monthColor[year % 13].r, monthColor[year % 13].g, monthColor[year % 13].b, 75});
			yearBg->setContentHeight(50.f);
			yearBg->setZOrder(-2);
			yearBg->setID("year-bg");
			if (!Mod::get()->getSettingValue<bool>("disable-year-color")) { cell->addChild(yearBg); }

			auto monthlyText = CCLabelBMFont::create(monthlyDisp.c_str(), "bigFont.fnt");
			monthlyText->setColor(monthColor[month - 1]);
			monthlyText->setScale(0.35f);
			monthlyText->setAnchorPoint({ 0, 1 });
			monthlyText->setPosition({ 53, 32 });
			monthlyText->setZOrder(1);
			monthlyText->setID("monthly-text");
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

		if ((type == static_cast<int>(DPListType::Main) || type == static_cast<int>(DPListType::Legacy)) && Mod::get()->getSettingValue<bool>("enable-practice")) {
			auto practiceSpr = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
			auto practiceBtn = CCMenuItemSpriteExtra::create(practiceSpr, this, menu_selector(DPLayer::openList));
			practiceBtn->setPosition({ 288, 14 });
			practiceSpr->setScale(0.45f);
			practiceBtn->setTag(i);

			switch(type) {
				case static_cast<int>(DPListType::Main):
				{
					practiceBtn->setID("main-practice");
					break;
				}
				case static_cast<int>(DPListType::Legacy):
				{
					practiceBtn->setID("legacy-practice");
					break;
				}
			}

			if (!practiceIDs.empty()) {
				cellMenu->addChild(practiceBtn);
			}
		}

		if (i == 0 && type == static_cast<int>(DPListType::Monthly)) {
			auto goldBG = CCLayerColor::create({ 255, 200, 0, 255 });
			//if (Loader::get()->isModLoaded("alphalaneous.transparent_lists")) { goldBG->setOpacity(50); }
			goldBG->setID("gold-bg");
			goldBG->setContentHeight(50.f);
			goldBG->setZOrder(-1);
			cell->addChild(goldBG);

			/*cellMenu->setZOrder(1);
			packText->setZOrder(1);
			packSpr->setZOrder(1);
			progText->setZOrder(1);*/
		}

		if (name == "The Temple Series") {
			auto michiSpikes = CCSprite::createWithSpriteFrameName("communityIcon_03_001.png");
			auto michiHeart = CCSprite::createWithSpriteFrameName("d_heart01_001.png");

			michiSpikes->setScale(0.5f);
			michiSpikes->setPositionX(packText->getContentWidth() - 42);
			michiSpikes->setPositionY(35);
			michiSpikes->setID("michigun-spikes");

			michiHeart->setScale(0.35f);
			michiHeart->setPositionX(packText->getContentWidth() - 42);
			michiHeart->setPositionY(44);
			michiHeart->setColor({ 0, 0, 0 });
			michiHeart->setID("michigun-heart");

			cell->addChild(michiSpikes);
			cell->addChild(michiHeart);
		}

		switch(type) {
			case static_cast<int>(DPListType::Main):
			{
				if (i > 0 && !Mod::get()->getSettingValue<bool>("unlock-all-tiers")) {

					//get rank of previous main pack
					auto prevSaveID = m_data[dataIdx][i - 1]["saveID"].asString().unwrapOr("null");
					auto rankCheck = Mod::get()->getSavedValue<ListSaveFormat>(prevSaveID).hasRank;

					if (!rankCheck) {
						auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
						lockIcon->setPosition({ 180, 20 });
						lockIcon->setAnchorPoint({ 0.5f, 0 });
						lockIcon->setID("lock-icon");

						std::string rankText = "???";
						if (i > 1) {

							//check for the one before that
							prevSaveID = m_data[dataIdx][i - 2]["saveID"].asString().unwrapOr("null");
							rankCheck = Mod::get()->getSavedValue<ListSaveFormat>(prevSaveID).hasRank;

							if (rankCheck) {
								rankText = m_data[dataIdx][i - 1]["name"].asString().unwrapOr("???");
							}
						}
						else {
							rankText = m_data[dataIdx][i - 1]["name"].asString().unwrapOr("null");
						}

						std::string fullLockText = fmt::format("Get the {} Rank to unlock!", rankText);

						auto lockText = CCLabelBMFont::create(fullLockText.c_str(), "bigFont.fnt");
						lockText->setPosition({ 180, 5 });
						lockText->setAnchorPoint({ 0.5f, 0 });
						lockText->setScale(0.5f);
						lockText->setID("lock-text");

						cell->addChild(lockIcon);
						cell->addChild(lockText);
					}
					else {
						cell->addChild(cellMenu);
						cell->addChild(packText);
						cell->addChild(customPackText);
						cell->addChild(packSpr);
						cell->addChild(packPlusSpr);
						cell->addChild(packProgressBack);
						cell->addChild(progText);
					}
				}
				else {
					cell->addChild(cellMenu);
					cell->addChild(packText);
					cell->addChild(customPackText);
					cell->addChild(packSpr);
					cell->addChild(packPlusSpr);
					cell->addChild(packProgressBack);
					cell->addChild(progText);
				}
				break;
			}
			case static_cast<int>(DPListType::Legacy):
			{
				if (!Mod::get()->getSettingValue<bool>("unlock-all-legacy")) {
					//get main pack list ID
					auto mainSaveID = m_data["main"][mainPack]["saveID"].asString().unwrapOr("null");
					auto rankCheck = Mod::get()->getSavedValue<ListSaveFormat>(mainSaveID).hasRank;

					if (!rankCheck) {
						auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
						lockIcon->setPosition({ 180, 20 });
						lockIcon->setAnchorPoint({ 0.5f, 0 });
						lockIcon->setID("lock-icon");

						std::string rankText = "???";
						if (mainPack > 0) {

							//get previous main pack
							auto prevSaveID = m_data["main"][mainPack - 1]["saveID"].asString().unwrapOr("null");
							rankCheck = Mod::get()->getSavedValue<ListSaveFormat>(prevSaveID).hasRank;

							if (rankCheck) {
								rankText = m_data["main"][mainPack]["name"].asString().unwrapOr("???");
							}
						}
						else {
							rankText = m_data["main"][mainPack]["name"].asString().unwrapOr("???");
						}
						std::string fullLockText = fmt::format("Get the {} Rank to unlock!", rankText);

						auto lockText = CCLabelBMFont::create(fullLockText.c_str(), "bigFont.fnt");
						lockText->setPosition({ 180, 5 });
						lockText->setAnchorPoint({ 0.5f, 0 });
						lockText->setScale(0.5f);
						lockText->setID("lock-text");

						cell->addChild(lockIcon);
						cell->addChild(lockText);
					}
					else {
						cell->addChild(cellMenu);
						cell->addChild(packText);
						cell->addChild(customPackText);
						cell->addChild(packSpr);
						cell->addChild(packPlusSpr);
						cell->addChild(packProgressBack);
						cell->addChild(progText);
					}
				}
				else {
					cell->addChild(cellMenu);
					cell->addChild(packText);
					cell->addChild(customPackText);
					cell->addChild(packSpr);
					cell->addChild(packPlusSpr);
					cell->addChild(packProgressBack);
					cell->addChild(progText);
				}
				break;
			}
			default:
			{
				cell->addChild(cellMenu);
				cell->addChild(packText);
				cell->addChild(customPackText);
				cell->addChild(packSpr);
				cell->addChild(packPlusSpr);
				cell->addChild(packProgressBack);
				cell->addChild(progText);
				break;
			}
		}

		packListCells->addObject(cell);
	};

	//list
	ListView* packListMenu = ListView::create(packListCells, 50.0f, 358.0f, 220.0f);
	packListMenu->setAnchorPoint({ 0.5f, 0.5f });
	packListMenu->setPosition({ (size.width / 2) - 180, (size.height / 2) - 115 });
	packListMenu->setID("list-menu");
	packListMenu->setZOrder(0);
	this->addChild(packListMenu);
	m_list = packListMenu;
	m_currentTab = type;

	//scrollbar everywhere compatibility
	/*if (auto scrollbar = this->getChildByID("user95401.scrollbar_everywhere/scrollbar")) {
		scrollbar->setPosition({ (size.width / 2) + 185.f, size.height / 2});
	}*/

	return;
}

void DPLayer::onTab(CCObject* pSender) {
	auto btn = static_cast<TabButton*>(pSender);
	auto menuType = btn->getTag();

	auto mainbtn = m_tabs->getChildByID("main");
	auto legacybtn = m_tabs->getChildByID("legacy");
	auto bonusbtn = m_tabs->getChildByID("bonus");
	auto monthlybtn = m_tabs->getChildByID("monthly");

	if (m_list) {
		m_list->removeAllChildrenWithCleanup(true);
		m_list->removeMeAndCleanup();
	}

	switch(menuType) {
		case static_cast<int>(DPListType::Main):
		{
			log::info("Switched to Main Tab");

			btn->toggle(true);
			static_cast<TabButton*>(legacybtn)->toggle(false);
			static_cast<TabButton*>(bonusbtn)->toggle(false);
			static_cast<TabButton*>(monthlybtn)->toggle(false);

			break;
		}
		case static_cast<int>(DPListType::Legacy):
		{
			log::info("Switched to Legacy Tab");

			btn->toggle(true);
			static_cast<TabButton*>(mainbtn)->toggle(false);
			static_cast<TabButton*>(bonusbtn)->toggle(false);
			static_cast<TabButton*>(monthlybtn)->toggle(false);

			break;
		}
		case static_cast<int>(DPListType::Bonus):
		{
			log::info("Switched to Bonus Tab");

			btn->toggle(true);
			static_cast<TabButton*>(legacybtn)->toggle(false);
			static_cast<TabButton*>(mainbtn)->toggle(false);
			static_cast<TabButton*>(monthlybtn)->toggle(false);

			break;
		}
		case static_cast<int>(DPListType::Monthly):
		{
			log::info("Switched to Monthly Tab");
		
			btn->toggle(true);
			static_cast<TabButton*>(legacybtn)->toggle(false);
			static_cast<TabButton*>(bonusbtn)->toggle(false);
			static_cast<TabButton*>(mainbtn)->toggle(false);

			break;
		}
	}

	reloadList(menuType);

	return;
}

DPLayer::~DPLayer() {
	this->removeAllChildrenWithCleanup(true);
}