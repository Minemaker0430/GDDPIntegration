//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

#include "DPLayer.hpp"
#include "Settings.hpp"
#include "ListManager.hpp"
#include "StatsPopup.hpp"
#include "Utils.hpp"

//geode namespace
using namespace geode::prelude;

SettingNode* SectionSettingValue::createNode(float width) {
	return SectionSettingNode::create(this, width);
}

$on_mod(Loaded) {
	Mod::get()->addCustomSetting<SectionSettingValue>("bypass-section", "");
	Mod::get()->addCustomSetting<SectionSettingValue>("menu-section", "");
	Mod::get()->addCustomSetting<SectionSettingValue>("cosmetic-section", "");
	Mod::get()->addCustomSetting<SectionSettingValue>("compatibility-section", "");

	ListManager::init();
}

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
}

void DPLayer::keyBackClicked() {

	Mod::get()->setSavedValue<bool>("in-gddp", false);
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

void DPLayer::infoCallback(CCObject*) { //unused
	FLAlertLayer::create(nullptr, "GDDP Info", "OK", nullptr, "Beat the required number of levels in each pack to get a special rank and move on to the next pack.\n\nMod Developer: Minemaker0430\nGDDP Creator: Trusta\nSpecial Thanks: FireMario211 (Firee), HJFod, The GDP Discord Server, & The Geode Discord Server")->show();
}

void DPLayer::soonCallback(CCObject*) {
	FLAlertLayer::create("Coming Soon!", "This feature hasn't been implemented yet but will be in the future!", "OK")->show();
}

void DPLayer::reloadCallback(CCObject*) {
	reloadData(false);
}

void DPLayer::reloadData(bool isInit) {

	if (m_finishedLoading || isInit) {

		m_finishedLoading = false;

		if (!isInit) {
			m_list->removeAllChildrenWithCleanup(true);
			m_list->removeMeAndCleanup();
		}
		
		m_loadcircle = LoadingCircle::create();
		m_loadcircle->m_parentLayer = this;
		m_loadcircle->show();

		//m_reload->setVisible(false);
		m_tabs->setVisible(false);
		//m_backMenu->setVisible(false);

		//this->setKeyboardEnabled(false);
		//this->setKeypadEnabled(false);

		if (!Mod::get()->getSettingValue<bool>("enable-cache")) {
			Mod::get()->setSavedValue<matjson::Value>("cached-data", {});
		}

		std::string dataURL;

		//log::info("{}", GameManager::sharedState()->m_playerName);
		if (GameManager::sharedState()->m_playerName == "Minemaker0430") {
			log::info("Hello, me");
			dataURL = "https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/dev-list.json";
		}
		else {
			dataURL = "https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/main-list.json";
		}

		// download data

		//list
		m_listListener.bind([&](web::WebTask::Event* e) {
			if (auto res = e->getValue()) {
				//log::info("{}", res->string().unwrapOr("Uh oh!"));
				if (res->ok() && res->json().isOk()) {
					Mod::get()->setSavedValue<matjson::Value>("cached-data", res->json().unwrap());
					m_data = Mod::get()->getSavedValue<matjson::Value>("cached-data", matjson::parse("{\"main\": [], \"legacy\": [], \"bonus\": [], \"monthly\": [], \"database-version\": 0, \"level-data\": {}}"));
					reloadList(m_currentTab);

					m_tabs->setVisible(true);
					m_loadcircle->fadeAndRemove();

					m_finishedLoading = true;
					log::info("List data loaded!");
				}
				else {
					m_loadcircle->fadeAndRemove();
					FLAlertLayer::create("ERROR", fmt::format("Something went wrong getting the List Data. ({})", res->code()), "OK")->show();
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
				if (res->ok() && res->json().isOk()) {
					if (res->json().unwrap() != Mod::get()->getSavedValue<matjson::Value>("skillset-info", {})) {
						Mod::get()->setSavedValue<matjson::Value>("skillset-info", res->json().unwrap());
						log::info("Updated skillset info.");
					}
					else {
						log::info("No skillset updates found.");
					}
				}
				else {
					FLAlertLayer::create("ERROR", fmt::format("Something went wrong getting the Skillset Data. ({})", res->code()), "OK")->show();
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

	if (type == "main-practice") {
		Mod::get()->setSavedValue<bool>("is-practice", true);
		type = "main";
	}
	else if (type == "legacy-practice") {
		Mod::get()->setSavedValue<bool>("is-practice", true);
		type = "legacy";
	}
	else {
		Mod::get()->setSavedValue<bool>("is-practice", false);
	}

	auto listID = m_data[type][id]["listID"].as_int();
	auto reqLevels = 0;
	if (type == "main") { reqLevels = m_data[type][id]["reqLevels"].as_int(); }
	auto totalLevels = 0;
	if (type != "monthly") { totalLevels = m_data[type][id]["totalLevels"].as_int(); }
	auto hasPractice = false;
	if (type == "main") { hasPractice = m_data[type][id]["practice"].as_bool(); }
	auto mainPack = 0;
	if (type == "legacy") { mainPack = m_data[type][id]["mainPack"].as_int(); }
	auto practiceID = 0;
	if (type == "main") { practiceID = m_data[type][id]["practiceID"].as_int(); }
	if (type == "legacy") { practiceID = m_data["main"][mainPack]["practiceID"].as_int(); }

	Mod::get()->setSavedValue<std::string>("current-pack-type", type);
	Mod::get()->setSavedValue<int>("current-pack-index", id);
	Mod::get()->setSavedValue<int>("current-pack-requirement", reqLevels);
	Mod::get()->setSavedValue<int>("current-pack-totalLvls", totalLevels);
	Mod::get()->setSavedValue<bool>("current-pack-hasPractice", hasPractice);

	auto fetchID = 0;

	if ((type == "main" || type == "legacy") && Mod::get()->getSavedValue<bool>("is-practice", false)) {
		fetchID = practiceID;
	}
	else {
		fetchID = listID;
	}

	m_loadcircle = LoadingCircle::create();
	m_loadcircle->m_parentLayer = this;
	m_loadcircle->show();

	std::string const& url = "https://www.boomlings.com/database/getGJLevelLists.php";
	std::string const& fields = "secret=Wmfd2893gb7&type=0&diff=-&len=-&count=1&str=" + std::to_string(fetchID); //thank you gd cologne :pray:

	m_listener.bind([this](web::WebTask::Event* e) {
		if (web::WebResponse* res = e->getValue()) {
			//log::info("{}", res->string().unwrapOr("Uh oh!"));
			if (res->ok() && res->string().isOk()) {

				auto response = res->string().unwrap();

				if (response != "-1") {
					auto scene = CCScene::create();

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

					auto cachedData = Mod::get()->getSavedValue<matjson::Value>("cached-data");
					auto cachedType = Mod::get()->getSavedValue<std::string>("current-pack-type");
					auto cachedID = Mod::get()->getSavedValue<int>("current-pack-index");

					auto mainPack = 0;
					if (cachedType == "legacy") { mainPack = cachedData[cachedType][cachedID]["mainPack"].as_int(); }
					auto totalLevels = 0;
					if (cachedType != "monthly") { totalLevels = cachedData[cachedType][cachedID]["totalLevels"].as_int(); }

					auto data = Utils::substring(response, ":");
					log::info("{}", data);

					gd::vector<std::string> levelIDstr = Utils::substring(data[20], ",");
					log::info("{}", levelIDstr);

					std::vector<int> IDs;

					if (cachedType == "main" && Mod::get()->getSavedValue<bool>("is-practice", false)) {
						for (int i = 0; i < totalLevels; i++)
						{
							int num = atoi(levelIDstr.at(i).c_str());
							IDs.push_back(num);
						}
					}
					else if (cachedType == "legacy" && Mod::get()->getSavedValue<bool>("is-practice", false)) {
						for (int i = cachedData["main"][mainPack]["totalLevels"].as_int(); i < levelIDstr.size(); i++)
						{
							int num = atoi(levelIDstr.at(i).c_str());
							IDs.push_back(num);
						}
					}
					else {
						for (int i = 0; i < levelIDstr.size(); i++)
						{
							int num = atoi(levelIDstr.at(i).c_str());
							IDs.push_back(num);
						}
					}

					//gd::string desc = ZipUtils::base64URLDecode(data[5]);

					gd::string packTitle = "null";

					if (cachedType == "main" || cachedType == "legacy") {
						if (Mod::get()->getSavedValue<bool>("is-practice", false)) {
							packTitle = fmt::format("{} Demons (Practice)", cachedData[cachedType][cachedID]["name"].as_string());
						}
						else {
							packTitle = fmt::format("{} Demons", cachedData[cachedType][cachedID]["name"].as_string());
						}
					}
					else {
						packTitle = cachedData[cachedType][cachedID]["name"].as_string();
					}

					gd::string description = cachedData[cachedType][cachedID]["description"].as_string();

					auto list = GJLevelList::create();
					list->m_listID = std::stoi(data[1]);
					list->m_listName = packTitle;
					//list->m_listName = data[3];
					list->m_downloads = std::stoi(data[13]);
					list->m_likes = std::stoi(data[17]);
					//list->m_creatorName = data[29];
					list->m_levels = IDs;
					list->m_listDesc = ZipUtils::base64URLEncode(description); //LevelTools::base64EncodeString(description); 

					auto layer = LevelListLayer::create(list);

					m_loadcircle->fadeAndRemove();

					scene->addChild(layer);
					CCDirector::sharedDirector()->pushScene(cocos2d::CCTransitionFade::create(0.5f, scene));
				}
				else {
					FLAlertLayer::create("ERROR", "This pack doesn't exist! Check back later.", "OK")->show();
					m_loadcircle->fadeAndRemove();
				}
			}
			else {
				m_loadcircle->fadeAndRemove();
				FLAlertLayer::create("ERROR", fmt::format("Something went wrong opening the list. ({})", res->code()), "OK")->show();
			}
		}
		else if (e->isCancelled()) {
			log::info("Cancelled open list request.");
		}
	});

	auto req = web::WebRequest();
	req.bodyString(fields);
	m_listener.setFilter(req.get(url));
}

void DPLayer::achievementsCallback(CCObject* sender) {
	if (m_finishedLoading) {
		StatsPopup::create()->show();
	}
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
		//backMenu->setVisible(false);
		this->addChild(backMenu);
		m_backMenu = backMenu;

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

		//reload menu
		auto reloadMenu = CCMenu::create();
		reloadMenu->setPosition({ 0, 0 });
		auto reloadBtnSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
		auto reloadBtn = CCMenuItemSpriteExtra::create(reloadBtnSprite, this, menu_selector(DPLayer::reloadCallback));
		reloadBtn->setPosition({ 30, 30 });
		reloadMenu->addChild(reloadBtn);
		reloadMenu->setID("reload-menu");
		this->addChild(reloadMenu);
		m_reload = reloadMenu;
		//m_reload->setVisible(false);

		m_currentTab = static_cast<int>(DPListType::Main);

		if (!Mod::get()->getSettingValue<bool>("enable-cache")) {
			Mod::get()->setSavedValue<matjson::Value>("cached-data", {});
		}

		//extra buttons
		auto achievementBtnSprite = CCSprite::createWithSpriteFrameName("GJ_achBtn_001.png");
		auto leaderboardsBtnSprite = CCSprite::createWithSpriteFrameName("GJ_statsBtn_001.png");
		auto leaderboardButton = CCMenuItemSpriteExtra::create(leaderboardsBtnSprite, this, menu_selector(DPLayer::soonCallback));
		auto achievementButton = CCMenuItemSpriteExtra::create(achievementBtnSprite, this, menu_selector(DPLayer::achievementsCallback));
		achievementButton->setPosition({ size.width - 30, 30 });
		leaderboardButton->setPosition({ size.width - 30, 80 });
		auto extrasMenu = CCMenu::create();
		extrasMenu->setPosition({ 0, 0 });
		//extrasMenu->addChild(leaderboardButton);
		extrasMenu->addChild(achievementButton);
		extrasMenu->setID("extras-menu");
		this->addChild(extrasMenu);

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

		if (!Mod::get()->getSettingValue<bool>("show-database-version")) {
			m_databaseVer->setVisible(false);
		}

		this->addChild(m_databaseVer);

		// download data
		reloadData(true);

		this->setKeyboardEnabled(true);
		this->setKeypadEnabled(true);

        return true;
}

void DPLayer::reloadList(int type) {

	//all save stuff
	auto bronzeMedals = Mod::get()->getSavedValue<int>("bronze-medals", 0);
	auto silverMedals = Mod::get()->getSavedValue<int>("silver-medals", 0);
	auto goldMedals = Mod::get()->getSavedValue<int>("gold-medals", 0);

	auto demonCountMain = Mod::get()->getSavedValue<int>("demon-count-main", 0);
	auto demonCountLegacy = Mod::get()->getSavedValue<int>("demon-count-legacy", 0);
	auto demonCountBonus = Mod::get()->getSavedValue<int>("demon-count-bonus", 0);
	auto demonCountMonthly = Mod::get()->getSavedValue<int>("demon-count-monthly", 0);

	auto localDatabaseVer = Mod::get()->getSavedValue<int>("database-version", 0);

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
	
	auto packs = m_data[dataIdx].as_array();

	auto versionTxt = fmt::format("Database Version: {}", std::to_string(m_data["database-version"].as_int()));
	m_databaseVer->setCString(versionTxt.c_str());

	if (m_data[dataIdx].as_array().size() <= 0) { return; }

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
		bool hasPractice = false; //Main Only
		int mainPack = 0; //Legacy Only

		name = m_data[dataIdx][i]["name"].as_string();
		sprite = m_data[dataIdx][i]["sprite"].as_string();
		if (type == static_cast<int>(DPListType::Main)) { plusSprite = m_data[dataIdx][i]["plusSprite"].as_string(); }
		listID = m_data[dataIdx][i]["listID"].as_int();
		if (type == static_cast<int>(DPListType::Main)) { reqLevels = m_data[dataIdx][i]["reqLevels"].as_int(); }
		if (type != static_cast<int>(DPListType::Monthly)) { totalLevels = m_data[dataIdx][i]["totalLevels"].as_int(); }
		if (type == static_cast<int>(DPListType::Monthly)) { month = m_data[dataIdx][i]["month"].as_int(); }
		if (type == static_cast<int>(DPListType::Monthly)) { year = m_data[dataIdx][i]["year"].as_int(); }
		if (type == static_cast<int>(DPListType::Main)) { hasPractice = m_data[dataIdx][i]["practice"].as_bool(); }
		if (type == static_cast<int>(DPListType::Legacy)) { mainPack = m_data[dataIdx][i]["mainPack"].as_int(); }

		//get list save
		auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(listID));

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
		if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSprite).data()) == nullptr) {
			fullSprite = "DP_Invisible.png";
		}

		if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSprite).data()) == nullptr) {
			fullPlusSprite = "DP_Invisible.png";
		}

		CCNode* cell = ListCell::create();

		CCLabelBMFont* packText = CCLabelBMFont::create(fullTitle.c_str(), "bigFont.fnt");
		packText->setScale(0.65f);
		if (fullTitle.length() > 18) { packText->setScale(0.50f); }
		if (fullTitle.length() > 25) { packText->setScale(0.425f); }
		packText->setAnchorPoint({ 0, 1 });
		packText->setPosition({ 53, 49 });
		packText->setID("pack-text");

		if (listSave.completed) {
			packText->setFntFile("goldFont.fnt");
		}

		CCNode* packSpr = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");

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
		packProgressBack->setAnchorPoint({0, 0.5});
		packProgressBack->setPosition({53, 15});
		packProgressBack->setColor({0, 0, 0});
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
		else if (type == static_cast<int>(DPListType::Main) && !listSave.hasRank) {
			packProgressFront->setColor({ 255, 84, 50 });
		}
		else {
			packProgressFront->setColor({ 80, 190, 255 });
		}

		auto progressPercent = 0.0f;

		if (type == static_cast<int>(DPListType::Monthly)) {
			if (listSave.progress >= 5) {
				progressPercent = (float)listSave.progress / 6.f;
			}
			else {
				progressPercent = (float)listSave.progress / 5.f;
			}
		}
		else if (listSave.hasRank || type != static_cast<int>(DPListType::Main)) {
			progressPercent = (float)listSave.progress / (float)totalLevels;
		}
		else {
			progressPercent = (float)listSave.progress / (float)reqLevels;
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

		if (type == static_cast<int>(DPListType::Main)) {
			if (!listSave.hasRank) {
				std::string nextTier = "???";
				if (i + 1 < packs.size()) {
					nextTier = m_data[dataIdx][i + 1]["name"].as_string();
				}
				progStr = fmt::format("{}/{} to {} Tier", std::to_string(listSave.progress), std::to_string(reqLevels), nextTier);
			}
			else if (listSave.completed) {
				progStr = "100% Complete!";
				progText->setFntFile("goldFont.fnt");
				packPlusSpr->setVisible(true);
			}
			else {
				progStr = fmt::format("{}/{} to Completion", std::to_string(listSave.progress), std::to_string(totalLevels));
				packPlusSpr->setVisible(true);
			}
		}
		else if (type == static_cast<int>(DPListType::Legacy) || type == static_cast<int>(DPListType::Bonus)) {
			if (listSave.completed) {
				progStr = "100% Complete!";
				progText->setFntFile("goldFont.fnt");
			}
			else {
				progStr = fmt::format("{}/{} to Completion", std::to_string(listSave.progress), std::to_string(totalLevels));
			}
		}
		else if (type == static_cast<int>(DPListType::Monthly)) {

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
		}

		progText->setCString(progStr.c_str());
		progText->setPosition({ 155, 15 });
		progText->setAnchorPoint({ 0.5, 0.5 });
		progText->setScale(0.30f);
		progText->setID("progress-text");

		if (type == static_cast<int>(DPListType::Monthly)) {
			std::string months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
			ccColor3B monthColor[12] = { {255, 0, 0}, {255, 128, 0}, {255, 255, 0}, {128, 255, 0}, {0, 255, 0}, {0, 255, 128}, {0, 255, 255}, {0, 128, 255}, {0, 0, 255}, {128, 0, 255}, {255, 0, 255}, {255, 0, 128} };
			std::string monthlyDisp = fmt::format("{} {}", months[month - 1], std::to_string(year));

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

		if (type == static_cast<int>(DPListType::Main) && hasPractice && Mod::get()->getSettingValue<bool>("enable-practice")) {
			auto practiceSpr = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
			auto practiceBtn = CCMenuItemSpriteExtra::create(practiceSpr, this, menu_selector(DPLayer::openList));
			practiceBtn->setPosition({ 288, 14 });
			practiceSpr->setScale(0.45f);
			practiceBtn->setTag(i);
			practiceBtn->setID("main-practice");
			cellMenu->addChild(practiceBtn);
		}
		else if (type == static_cast<int>(DPListType::Legacy) && m_data["main"][mainPack]["practice"].as_bool() && Mod::get()->getSettingValue<bool>("enable-practice")) {
			auto practiceSpr = CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
			auto practiceBtn = CCMenuItemSpriteExtra::create(practiceSpr, this, menu_selector(DPLayer::openList));
			practiceBtn->setPosition({ 288, 14 });
			practiceSpr->setScale(0.45f);
			practiceBtn->setTag(i);
			practiceBtn->setID("legacy-practice");
			cellMenu->addChild(practiceBtn);
		}

		if (i == 0 && type == static_cast<int>(DPListType::Monthly)) {
			auto goldBG = CCLayerColor::create({255, 200, 0, 255});
			goldBG->setID("gold-bg");
			cell->addChild(goldBG);

			cellMenu->setZOrder(1);
			packText->setZOrder(1);
			packSpr->setZOrder(1);
			progText->setZOrder(1);
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
			michiHeart->setColor({0, 0, 0});
			michiHeart->setID("michigun-heart");

			cell->addChild(michiSpikes);
			cell->addChild(michiHeart);
		}

		if (i > 0 && type == static_cast<int>(DPListType::Main) && !Mod::get()->getSettingValue<bool>("unlock-all-tiers")) {

			//get rank of previous main pack
			auto prevListID = m_data[dataIdx][i - 1]["listID"].as_int();
			auto rankCheck = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(prevListID)).hasRank;

			if (!rankCheck) {
				auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
				lockIcon->setPosition({180, 20});
				lockIcon->setAnchorPoint({0.5f, 0});
				lockIcon->setID("lock-icon");

				std::string rankText = "???";
				if (i > 1) {

					//check for the one before that
					prevListID = m_data[dataIdx][i - 2]["listID"].as_int();
					rankCheck = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(prevListID)).hasRank;

					if (rankCheck) {
						rankText = m_data[dataIdx][i - 1]["name"].as_string();
					}
				}
				else {
					rankText = m_data[dataIdx][i - 1]["name"].as_string();
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
				cell->addChild(packSpr);
				cell->addChild(packPlusSpr);
				cell->addChild(packProgressBack);
				cell->addChild(progText);
			}
		}
		else if (type == static_cast<int>(DPListType::Legacy) && !Mod::get()->getSettingValue<bool>("unlock-all-legacy")) {
			
			//get main pack list ID
			auto mainListID = m_data["main"][mainPack]["listID"].as_int();
			auto rankCheck = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(mainListID)).hasRank;
			
			if (!rankCheck) {
				auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
				lockIcon->setPosition({ 180, 20 });
				lockIcon->setAnchorPoint({ 0.5f, 0 });
				lockIcon->setID("lock-icon");

				std::string rankText = "???";
				if (mainPack > 0) {

					//get previous main pack
					auto prevListID = m_data["main"][mainPack - 1]["listID"].as_int();
					rankCheck = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(prevListID)).hasRank;

					if (rankCheck) {
						rankText = m_data["main"][mainPack]["name"].as_string();
					}
				}
				else {
					rankText = m_data["main"][mainPack]["name"].as_string();
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
				cell->addChild(packSpr);
				cell->addChild(packPlusSpr);
				cell->addChild(packProgressBack);
				cell->addChild(progText);
			}
		}
		else {
			cell->addChild(cellMenu);
			cell->addChild(packText);
			cell->addChild(packSpr);
			cell->addChild(packPlusSpr);
			cell->addChild(packProgressBack);
			cell->addChild(progText);
		}

		packListCells->addObject(cell);
	};

	Mod::get()->setSavedValue<int>("demon-count-main", demonCountMain);
	Mod::get()->setSavedValue<int>("demon-count-legacy", demonCountLegacy);
	Mod::get()->setSavedValue<int>("demon-count-bonus", demonCountBonus);
	Mod::get()->setSavedValue<int>("demon-count-monthly", demonCountMonthly);

	Mod::get()->setSavedValue<int>("total-demon-count", demonCountMain + demonCountLegacy + demonCountBonus + demonCountMonthly);

	//list
	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	ListView* packListMenu = ListView::create(packListCells, 50.0f, 358.0f, 220.0f);
	packListMenu->setAnchorPoint({ 0.5f, 0.5f });
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

	if (m_list) {
		m_list->removeAllChildrenWithCleanup(true);
		m_list->removeMeAndCleanup();
	}

	if (menuType == static_cast<int>(DPListType::Main)) {
		log::info("Switched to Main Tab");

		btn->toggle(true);
		static_cast<TabButton*>(legacybtn)->toggle(false);
		static_cast<TabButton*>(bonusbtn)->toggle(false);
		static_cast<TabButton*>(monthlybtn)->toggle(false);

		reloadList(static_cast<int>(DPListType::Main));
	}
	else if (menuType == static_cast<int>(DPListType::Legacy)) {
		log::info("Switched to Legacy Tab");
		
		btn->toggle(true);
		static_cast<TabButton*>(mainbtn)->toggle(false);
		static_cast<TabButton*>(bonusbtn)->toggle(false);
		static_cast<TabButton*>(monthlybtn)->toggle(false);

		reloadList(static_cast<int>(DPListType::Legacy));
	}
	else if (menuType == static_cast<int>(DPListType::Bonus)) {
		log::info("Switched to Bonus Tab");
		
		btn->toggle(true);
		static_cast<TabButton*>(legacybtn)->toggle(false);
		static_cast<TabButton*>(mainbtn)->toggle(false);
		static_cast<TabButton*>(monthlybtn)->toggle(false);

		reloadList(static_cast<int>(DPListType::Bonus));
	}
	else if (menuType == static_cast<int>(DPListType::Monthly)) {
		log::info("Switched to Monthly Tab");
		
		btn->toggle(true);
		static_cast<TabButton*>(legacybtn)->toggle(false);
		static_cast<TabButton*>(bonusbtn)->toggle(false);
		static_cast<TabButton*>(mainbtn)->toggle(false);

		reloadList(static_cast<int>(DPListType::Monthly));
	}
}

DPLayer::~DPLayer() {
    this->removeAllChildrenWithCleanup(true);
}