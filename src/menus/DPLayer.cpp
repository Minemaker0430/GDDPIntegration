//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/utils/async.hpp>

#include <ctime>

#include "DPLayer.hpp"
#include "DPListLayer.hpp"
#include "RecommendedLayer.hpp"
#include "../popups/StatsPopup.hpp"
#include "../popups/SupportPopup.hpp"
#include "../popups/XPPopup.hpp"
#include "../popups/SearchPopup.hpp"
#include "../popups/RoulettePopup.hpp"
#include "../popups/dev/VerificationPopup.hpp"
#include "../XPUtils.hpp"
#include "../RecommendedUtils.hpp"
#include "DPPackCell.hpp"
#include "../DPUtils.hpp"
#include "../base64.h"

//geode namespace
using namespace geode::prelude;

$execute {
	Mod::get()->setSavedValue<bool>("in-gddp", false);
}

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

		if (!isInit && !m_error) m_list->removeAllChildrenWithCleanup(true);

		m_loadcircle = LoadingCircle::create();
		m_loadcircle->m_parentLayer = this;
		m_loadcircle->show();

		m_tabs->setVisible(false);
		m_errorText->setVisible(false);

		//list
		auto listReq = web::WebRequest();

		std::string dataURL = "https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/main-list.json";

		auto fileCheck = file::readDirectory(Mod::get()->getConfigDir()).unwrapOrDefault();
		auto secretFile = Mod::get()->getConfigDir() += std::filesystem::path("\\client_id.txt");
		auto accessCodeFile = Mod::get()->getConfigDir() += std::filesystem::path("\\access_token.txt");

		if (Mod::get()->getSavedValue<bool>("dev-preview", false) && std::find(fileCheck.begin(), fileCheck.end(), secretFile) != fileCheck.end()) {
			std::string accessCode;

			if (std::find(fileCheck.begin(), fileCheck.end(), accessCodeFile) != fileCheck.end()) {
				accessCode = file::readString(accessCodeFile).unwrapOr("");
				if (accessCode != "") {
					listReq.userAgent("GDDP Mod Database");
					listReq.header("Accept", "application/vnd.github+json");
					listReq.header("Authorization", fmt::format("Bearer {}", accessCode));
					listReq.header("X-GitHub-Api-Version", "2022-11-28");
					dataURL = "https://api.github.com/repos/Minemaker0430/gddp-mod-dev-data/contents/list.json";
				}
			}
		}
		
		m_listener.spawn(
			listReq.get(dataURL),
			[&](web::WebResponse value) {
				if (value.ok() && value.json().isOk() && !value.json().isErr()) {
					m_data = value.json().unwrapOrDefault();
					if (m_data.contains("content")) m_data = matjson::parse(base64_decode(m_data["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
					Mod::get()->setSavedValue<matjson::Value>("cached-data", m_data);

					m_tabs->setVisible(true);
					m_loadcircle->fadeAndRemove();

					m_finishedLoading = true;
					m_error = false;

					DPUtils::forceUpdateStatus();

					reloadList(m_currentTab);
				}
				else {
					m_loadcircle->fadeAndRemove();

					m_errorText->setCString(fmt::format("Something went wrong...\n(Code: {}, JSON Error: {})", value.code(), value.json().isErr()).c_str());
					m_errorText->setVisible(true);

					m_finishedLoading = true;
					m_error = true;
				}
			}
		);
	}

	return;
}

void DPLayer::openList(CCObject* sender) {
	auto parameters = static_cast<ListParameters*>(static_cast<CCNode*>(sender)->getUserObject());

	auto scene = CCScene::create(); // creates the scene
	auto dpLayer = DPListLayer::create(parameters->m_type.c_str(), parameters->m_index); //creates the layer

	scene->addChild(dpLayer);

	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition

	return;
}

void DPLayer::achievementsCallback(CCObject* sender) {
	if (m_finishedLoading && !m_error) StatsPopup::create()->show();

	return;
}

void DPLayer::supportCallback(CCObject* sender) {
	SupportPopup::create()->show();

	return;
}

void DPLayer::newsCallback(CCObject* sender) {
	//NewsPopup::create()->show();

	return;
}

void DPLayer::searchCallback(CCObject* sender) {
	if (m_finishedLoading && !m_error) SearchPopup::create()->show();

	return;
}

void DPLayer::rouletteCallback(CCObject* sender) {
	if (m_finishedLoading && !m_error) RoulettePopup::create()->show();

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

	Mod::get()->setSavedValue<bool>("in-gddp", true);

	auto menu = CCMenu::create();
	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	auto bg = createLayerBG();
	if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) bg->setColor({ 18, 18, 86 });
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

	if (Mod::get()->getSettingValue<bool>("show-monthly-timer")) this->schedule(schedule_selector(DPLayer::updateMonthlyTimer), 1.f);

	//back button
	auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backButton = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(DPLayer::backButton));
	m_backMenu = CCMenu::create();
	m_backMenu->addChild(backButton);
	m_backMenu->setPosition({ 25, size.height - 25 });
	m_backMenu->setID("back-menu");
	this->addChild(m_backMenu);

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

	this->addChild(listMiddle);
	this->addChild(listLeft);
	this->addChild(listRight);
	this->addChild(listTop);
	this->addChild(listBottom);

	//reload menu
	m_reload = CCMenu::create();
	m_reload->setPosition({ 0, 0 });
	auto reloadBtnSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
	auto reloadBtn = CCMenuItemSpriteExtra::create(reloadBtnSprite, this, menu_selector(DPLayer::reloadCallback));
	reloadBtn->setPosition({ 30.f, 30.f });
	m_reload->addChild(reloadBtn);
	m_reload->setID("reload-menu");
	m_reload->setZOrder(11);
	this->addChild(m_reload);

	//support menu
	auto supportMenu = CCMenu::create();
	supportMenu->setPosition({ 0, 0 });
	auto supportBtnSprite = CircleButtonSprite::createWithSpriteFrameName(("DP_Support.png"_spr), 1.0f, CircleBaseColor::Pink, CircleBaseSize::Small);
	auto supportBtn = CCMenuItemSpriteExtra::create(supportBtnSprite, this, menu_selector(DPLayer::supportCallback));
	supportBtn->setPosition({ 75.f, 30.f });
	supportMenu->addChild(supportBtn);
	supportMenu->setID("support-menu");
	supportMenu->setZOrder(11);
	if (Mod::get()->getSettingValue<bool>("show-support")) this->addChild(supportMenu);

	m_currentTab = (int)DPListType::Main;

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
	if (Mod::get()->getSettingValue<bool>("show-xp")) this->addChild(xpMenu);

	//list tabs
	m_tabs = CCMenu::create();
	m_tabs->setID("list-tabs");
	m_tabs->setZOrder(9);

	auto backTabSprite = CCSprite::createWithSpriteFrameName("DP_tabBack.png"_spr);
	backTabSprite->setZOrder(-1);
	backTabSprite->setAnchorPoint({ 0, 0 });

	auto mainPacksBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::Selected, "Main", this, menu_selector(DPLayer::onTab));
	mainPacksBtn->setPosition(-136.f, 133.5f);
	mainPacksBtn->setID("main");
	mainPacksBtn->setTag(static_cast<int>(DPListType::Main));
	mainPacksBtn->toggle(true);
	mainPacksBtn->addChild(backTabSprite);
	m_tabs->addChild(mainPacksBtn);
	m_tabBtns.push_back(mainPacksBtn);

	auto legacyPacksBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::Selected, "Legacy", this, menu_selector(DPLayer::onTab));
	legacyPacksBtn->setPosition(-45.f, 133.5f);
	legacyPacksBtn->setID("legacy");
	legacyPacksBtn->setTag(static_cast<int>(DPListType::Legacy));
	legacyPacksBtn->addChild(backTabSprite);
	m_tabs->addChild(legacyPacksBtn);
	m_tabBtns.push_back(legacyPacksBtn);

	auto bonusPacksBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::Selected, "Bonus", this, menu_selector(DPLayer::onTab));
	bonusPacksBtn->setPosition(45.f, 133.5f);
	bonusPacksBtn->setID("bonus");
	bonusPacksBtn->setTag(static_cast<int>(DPListType::Bonus));
	bonusPacksBtn->addChild(backTabSprite);
	m_tabs->addChild(bonusPacksBtn);
	m_tabBtns.push_back(bonusPacksBtn);

	auto monthlyPacksBtn = TabButton::create(TabBaseColor::Unselected, TabBaseColor::Selected, "Monthly", this, menu_selector(DPLayer::onTab));
	monthlyPacksBtn->setPosition(136.f, 133.5f);
	monthlyPacksBtn->setID("monthly");
	monthlyPacksBtn->setTag(static_cast<int>(DPListType::Monthly));
	monthlyPacksBtn->addChild(backTabSprite);
	m_tabs->addChild(monthlyPacksBtn);
	m_tabBtns.push_back(monthlyPacksBtn);

	m_tabs->setVisible(false);
	this->addChild(m_tabs);

	m_databaseVer = CCLabelBMFont::create("Loading...", "chatFont.fnt");
	m_databaseVer->setAnchorPoint({ 1, 1 });
	m_databaseVer->setPosition({ size.width - 1, size.height - 1 });
	m_databaseVer->setScale(0.5f);
	m_databaseVer->setZOrder(1);
	m_databaseVer->setVisible(Mod::get()->getSettingValue<bool>("show-database-version"));
	this->addChild(m_databaseVer);

	// mod/dev stuff
	auto secretFile = Mod::get()->getConfigDir() += std::filesystem::path("\\client_id.txt");
	auto fileCheck = file::readDirectory(Mod::get()->getConfigDir()).unwrapOrDefault();

	if (std::find(fileCheck.begin(), fileCheck.end(), secretFile) != fileCheck.end()) {
		//dev menu
		auto devMenu = CCMenu::create();
		devMenu->setPosition({ 0, 0 });
		
		auto devBtnSprite = CCSprite::createWithSpriteFrameName("GJ_starBtnMod_001.png");
		auto devBtn = CCMenuItemSpriteExtra::create(devBtnSprite, this, menu_selector(DPLayer::devCallback));
		devBtn->setPosition({ size.width - 30.f, size.height - 30.f });
		devBtn->setID("dev-menu-btn");
		devMenu->addChild(devBtn);

		auto previewToggleBtn = CCMenuItemToggler::create(
			CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
			CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
			this,
			menu_selector(DPLayer::onDevToggle)
		);
		previewToggleBtn->setPosition({ size.width - 30.f, size.height - 70.f });
		previewToggleBtn->setID("dev-preview-toggle");
		previewToggleBtn->toggle(Mod::get()->getSavedValue<bool>("dev-preview", false));
		devMenu->addChild(previewToggleBtn);

		devMenu->setID("dev-menu");
		this->addChild(devMenu);
	}

	m_completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

	// download data
	reloadData(true);

	this->setKeyboardEnabled(true);
	this->setKeypadEnabled(true);
	this->scheduleUpdate();

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
	datetime.tm_mon = m_currentMonth;
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

	if (diff == 0) m_monthlyTimer->setCString("Time until next Monthly:\nSoon...");
	else m_monthlyTimer->setCString(fmt::format("Time until next Monthly:\n{}d {}h {}m {}s", days, hours, minutes, seconds).c_str());
}

void DPLayer::reloadList(int type) {
	if (!m_finishedLoading) return;

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

	if (m_list) m_list->removeMeAndCleanup();

	//all save stuff
	auto localDatabaseVer = Mod::get()->getSavedValue<int>("database-version", 0);
	Mod::get()->setSavedValue<int>("database-version", m_data["database-version"].as<int>().unwrapOr(0));

	//do everything else
	std::string dataIdx = "";
	std::map<DPListType, std::string> indexes = {
		{DPListType::Main, "main"},
		{DPListType::Legacy, "legacy"},
		{DPListType::Bonus, "bonus"},
		{DPListType::Monthly, "monthly"}
	};

	dataIdx = indexes[(DPListType)type];

	if (!m_data.contains(dataIdx)) return;

	auto packs = m_data[dataIdx].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

	auto versionTxt = fmt::format("Database Version: {}", std::to_string(m_data["database-version"].as<int>().unwrapOr(0)));
	if (Mod::get()->getSavedValue<bool>("dev-preview", false)) versionTxt += " (DEV)";
	m_databaseVer->setCString(versionTxt.c_str());

	if (packs.empty()) return;

	m_monthlyTimer->setVisible(false);
	if (type == (int)DPListType::Monthly && Mod::get()->getSettingValue<bool>("show-monthly-timer")) {
		m_monthlyTimer->setVisible(true);

		if (m_data["monthly"][0]["name"].asString().unwrapOr("???") != "???") {
			m_currentMonth = m_data["monthly"][0]["month"].as<int>().unwrapOr(8);
			m_currentYear = m_data["monthly"][0]["year"].as<int>().unwrapOr(1987) - 1900;
		}
	}

	RecommendedUtils::validateLevels();

	//setup cells
	auto packListCells = CCArray::create();
	auto packID = 0;
	for (auto p : packs) {

		auto cell = ListCell::create();
		cell->addChild(DPPackCell::create(p, dataIdx, packID));

		packID++;
		packListCells->addObject(cell);
	}

	//list
	m_list = ListView::create(packListCells, 50.0f, 358.0f, 220.0f);
	m_list->setAnchorPoint({ 0.5f, 0.5f });
	m_list->setPosition({ (size.width / 2) - 180, (size.height / 2) - 115 });
	m_list->setID("list-menu");
	m_list->setZOrder(0);
	this->addChild(m_list);
	m_currentTab = type;

	return;
}

void DPLayer::onTab(CCObject* pSender) {
	auto btn = static_cast<TabButton*>(pSender);
	auto menuType = btn->getTag();

	for (auto b : m_tabBtns) if (b) static_cast<TabButton*>(b)->toggle(false);
	btn->m_toggled = false;

	reloadList(menuType);

	return;
}

void DPLayer::onDevToggle(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);

	Mod::get()->setSavedValue<bool>("dev-preview", !btn->isToggled());
	reloadData();
	
	return;
}

void DPLayer::update(float dt) {
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
	if (m_completedLvls == completedLvls || !m_finishedLoading) return;
	
	// update
	m_completedLvls = completedLvls;
	auto offs = m_list->m_tableView->m_contentLayer->getPositionY();
	reloadList(m_currentTab);
	m_list->m_tableView->m_contentLayer->setPositionY(offs);
	
	return;
}

DPLayer::~DPLayer() {
	this->removeAllChildrenWithCleanup(true);
}