// geode header
#include <Geode/Geode.hpp>

// other headers
#include <random>

#include "../menus/DPLayer.hpp"
#include "RoulettePopup.hpp"
#include "../RouletteUtils.hpp"
#include "../menus/RouletteSafeLayer.hpp"

// geode namespace
using namespace geode::prelude;

bool RoulettePopup::setup()
{
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Roulette");

	auto layer = typeinfo_cast<CCLayer *>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	// create save menu
	loadSaveMenu();

	return true;
}

void RoulettePopup::onClose(CCObject *sender)
{

	// log::info("roulette closed");
	m_loadingCancelled = true;
	saveProgress(false);

	// disable roulette mode
	Mod::get()->setSavedValue<bool>("in-roulette", false);

	// normal closing stuff
	// CloseEvent(this).post();
	this->setKeypadEnabled(false);
	this->setTouchEnabled(false);
	this->removeFromParentAndCleanup(true);

	return;
}

void RoulettePopup::onBack(CCObject* sender) {
	saveProgress(false);
	m_loadingCancelled = true;
	Mod::get()->setSavedValue<bool>("in-roulette", false);
	loadSaveMenu();

	return;
}

void RoulettePopup::loadSaveMenu() {

	this->setTitle("Roulette");
	m_mainLayer->removeAllChildrenWithCleanup(true);

	auto cells = CCArray::create();
	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});

	// create/import button
	auto newMenu = CCMenu::create();
	newMenu->setPosition({322.5f, -13.f});
	newMenu->setScale(0.75f);
	newMenu->setID("new-menu");

	auto newBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png"), this, menu_selector(RoulettePopup::onNewSave));
	newBtn->setID("new-btn");

	newMenu->addChild(newBtn);
	m_mainLayer->addChild(newMenu);

	// list
	if (rouletteSaves.empty())
	{
		auto emptyText = CCLabelBMFont::create("No Saves Found!\nCreate or Import One to Start!", "bigFont.fnt");
		emptyText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
		emptyText->setScale(0.65f);
		emptyText->setPosition({210.f, 125.f});
		emptyText->setID("empty-text");
		m_mainLayer->addChild(emptyText);
	}
	else
	{
		int id = 0;
		for (auto save : rouletteSaves)
		{
			CCNode *cell = ListCell::create();
			cell->setTag(id);

			// extract all data
			std::string name = save.name;
			std::string settings = save.settings;
			int seed = save.seed;
			std::vector<int> levels = save.levels;
			int progress = save.progress;
			int skips = save.skips;
			int score = save.score;

			auto settingsBool = RouletteUtils::fromFlags(settings);

			// name label
			auto nameLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
			nameLabel->setPosition({5.f, 50.f});
			nameLabel->setScale(0.6f);
			nameLabel->setAnchorPoint({0.f, 1.f});
			nameLabel->setID("name-label");
			cell->addChild(nameLabel);

			// show current progress and score
			auto progressLabel = CCLabelBMFont::create(fmt::format("{}%", progress).c_str(), "bigFont.fnt");
			progressLabel->setPosition({215.f, 45.f});
			progressLabel->setScale(0.5f);
			progressLabel->setAnchorPoint({0.5f, 1.f});
			progressLabel->setID("progress-label");
			cell->addChild(progressLabel);

			auto scoreLabel = CCLabelBMFont::create(fmt::format("Score: {}", score).c_str(), "bigFont.fnt");
			scoreLabel->setPosition({215.f, 5.f});
			scoreLabel->setScale(0.5f);
			scoreLabel->setAnchorPoint({0.5f, 0.f});
			scoreLabel->setID("score-label");
			cell->addChild(scoreLabel);

			// add labels for certain settings (skips enabled, perfect run enabled, etc)
			std::vector<std::string> settingTags = {"Skips", "Skips (+ Points)", "Perfect", "Gauntlet", "Completed Only", "Uncompleted Only"};
			std::string settingsTxt = "";
			if (settingsBool[0] && !settingsBool[1]) { 
				settingsTxt = settingTags[0];
			} 
			else if (settingsBool[1]) {
				settingsTxt = settingTags[1];
			}

			if (settingsBool[5]) {
				settingsTxt = fmt::format("{}\n{}", settingsTxt, settingTags[3]);
			}
			else if (settingsBool[4]) {
				settingsTxt = fmt::format("{}\n{}", settingsTxt, settingTags[2]);
			}

			if (settingsBool[2] && !settingsBool[3]) {
				settingsTxt = fmt::format("{}\n{}", settingsTxt, settingTags[4]);
			}
			else if (!settingsBool[2] && settingsBool[3]) {
				settingsTxt = fmt::format("{}\n{}", settingsTxt, settingTags[5]);
			}

			auto settingsLabel = CCLabelBMFont::create(settingsTxt.c_str(), "chatFont.fnt");
			settingsLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
			settingsLabel->setPosition({150.f, 5.f});
			settingsLabel->setScale(0.5f);
			settingsLabel->setAnchorPoint({0.5f, 0.f});
			settingsLabel->setID("settings-label");
			cell->addChild(settingsLabel);

			// if completed, change to gold text
			if (progress >= 100)
			{
				nameLabel->setFntFile("goldFont.fnt");
				progressLabel->setFntFile("goldFont.fnt");
				scoreLabel->setFntFile("goldFont.fnt");

				nameLabel->setScale(0.75f);
				progressLabel->setScale(0.65f);
				scoreLabel->setScale(0.65f);
			}

			// add buttons (play, copy, delete, export, rename)
			auto playMenu = CCMenu::create();
			playMenu->setPosition({0.f, 0.f});
			playMenu->setID("play-menu");
			cell->addChild(playMenu);

			auto playSpr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
			playSpr->setScale(0.6f);
			auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(RoulettePopup::onEnterSave));
			playBtn->setPosition({290.f, 25.f});
			playBtn->setTag(id);
			playBtn->setID("play-btn");
			playMenu->addChild(playBtn);

			auto actionsMenu = CCMenu::create();
			actionsMenu->setPosition({0.f, 0.f});
			actionsMenu->setID("actions-menu");
			cell->addChild(actionsMenu);

			auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
			editSpr->setScale(0.35f);
			auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(RoulettePopup::onRename));
			editBtn->setPosition({105.f, 15.f});
			editBtn->setTag(id);
			editBtn->setID("edit-btn");
			actionsMenu->addChild(editBtn);

			auto copySpr = CCSprite::createWithSpriteFrameName("GJ_duplicateBtn_001.png");
			copySpr->setScale(0.6f);
			auto copyBtn = CCMenuItemSpriteExtra::create(copySpr, this, menu_selector(RoulettePopup::onCopy));
			copyBtn->setPosition({75.f, 15.f});
			copyBtn->setTag(id);
			copyBtn->setID("copy-btn");
			actionsMenu->addChild(copyBtn);

			auto exportSpr = CCSprite::createWithSpriteFrameName("GJ_shareBtn_001.png");
			exportSpr->setScale(0.35f);
			auto exportBtn = CCMenuItemSpriteExtra::create(exportSpr, this, menu_selector(RoulettePopup::onExport));
			exportBtn->setPosition({45.f, 15.f});
			exportBtn->setTag(id);
			exportBtn->setID("export-btn");
			actionsMenu->addChild(exportBtn);

			auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
			deleteSpr->setScale(0.6f);
			auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(RoulettePopup::onDeleteSave));
			deleteBtn->setPosition({15.f, 15.f});
			deleteBtn->setTag(id);
			deleteBtn->setID("delete-btn");
			actionsMenu->addChild(deleteBtn);

			// add cell
			cells->addObject(cell);
			id += 1;
		}
	}

	auto saveMenu = CCMenu::create();
	saveMenu->setContentSize(m_mainLayer->getContentSize());
	saveMenu->setPosition({0.f, 0.f});
	saveMenu->setID("save-menu");

	m_list = ListView::create(cells, 50.f, 315.f, 180.f);
	m_list->setPosition({53.f, 32.f});
	saveMenu->addChild(m_list);

	m_mainLayer->addChild(saveMenu);

	return;
}

void RoulettePopup::loadSettingsMenu() {

	this->setTitle("");
	m_mainLayer->removeAllChildrenWithCleanup(true);

	// back button
	auto backMenu = CCMenu::create();
	backMenu->setPosition({0.f, 0.f});
	backMenu->setID("back-menu");
	m_mainLayer->addChild(backMenu);

	auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backBtn = CCMenuItemSpriteExtra::create(backSpr, this, menu_selector(RoulettePopup::onBack));
	backBtn->setPosition(this->m_closeBtn->getPositionX(), this->m_closeBtn->getPositionY() - 40.f);
	backBtn->setID("back-btn");
	backMenu->addChild(backBtn);

	// name input
	m_value = TextInput::create(230.f, "Name", "bigFont.fnt");
	m_value->setCommonFilter(CommonFilter::Name);
	m_value->setMaxCharCount(16);
	m_value->setPosition({210.f, 225.f});
	m_value->setID("name-value");
	m_mainLayer->addChild(m_value);

	// settings button
	auto settingsMenu = CCMenu::create();
	settingsMenu->setPosition({ 0.f, 0.f });
	settingsMenu->setID("settings-menu");
	m_mainLayer->addChild(settingsMenu);

	auto settingsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
	settingsSpr->setScale(0.65f);
	auto settingsBtn = CCMenuItemSpriteExtra::create(settingsSpr, this, menu_selector(RoulettePopup::onSettings));
	settingsBtn->setPosition(400.f, 230.f);
	settingsBtn->setID("settings-btn");
	settingsMenu->addChild(settingsBtn);

	// check/uncheck all buttons
	auto checkMenu = CCMenu::create();
	checkMenu->setPosition({ -73.f, 65.f });
	checkMenu->setScale(0.65f);
	checkMenu->setZOrder(1);
	checkMenu->setID("check-menu");
	m_mainLayer->addChild(checkMenu);

	auto checkAllSpr = ButtonSprite::create("[] All", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto checkAllBox = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
	checkAllBox->setScale(0.75f);
	checkAllBox->setZOrder(2);
	checkAllBox->setPosition({ 15.f, 18.5f });
	checkAllSpr->addChild(checkAllBox);
	auto checkAllBtn = CCMenuItemSpriteExtra::create(checkAllSpr, this, menu_selector(RoulettePopup::onPackCheckAll));
	checkAllBtn->setPositionY(25.f);
	checkAllBtn->setID("check-all-btn");

	auto uncheckAllSpr = ButtonSprite::create("[] All", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto uncheckAllBox = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
	uncheckAllBox->setScale(0.75f);
	uncheckAllBox->setZOrder(2);
	uncheckAllBox->setPosition({ 15.f, 18.5f });
	uncheckAllSpr->addChild(uncheckAllBox);
	auto uncheckAllBtn = CCMenuItemSpriteExtra::create(uncheckAllSpr, this, menu_selector(RoulettePopup::onPackUncheckAll));
	uncheckAllBtn->setPositionY(-25.f);
	uncheckAllBtn->setID("uncheck-all-btn");

	checkMenu->addChild(checkAllBtn);
	checkMenu->addChild(uncheckAllBtn);

	//import menu
	auto importMenu = CCMenu::create();
	importMenu->setPosition({0.f, 0.f});
	importMenu->setID("import-menu");
	m_mainLayer->addChild(importMenu);

	auto importSpr = ButtonSprite::create("Import Settings", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	importSpr->setScale(0.65f);
	auto importBtn = CCMenuItemSpriteExtra::create(importSpr, this, menu_selector(RoulettePopup::onImportSettings));
	importBtn->setPosition(210.f, 265.f);
	importBtn->setID("import-btn");
	importMenu->addChild(importBtn);

	//confirm menu
	auto confirmMenu = CCMenu::create();
	confirmMenu->setPosition({0.f, 0.f});
	confirmMenu->setID("confirm-menu");
	m_mainLayer->addChild(confirmMenu);

	auto confirmSpr = ButtonSprite::create("Create", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(RoulettePopup::onFinalizeSave));
	confirmBtn->setPosition(210.f, 0.f);
	confirmBtn->setID("confirm-btn");
	confirmMenu->addChild(confirmBtn);

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	// set up pack selection
	auto cells = CCArray::create();

	// just copy stuff from the GDDP SearchPopup :v
	// main packs header
	auto mainPacksHeader = CCNode::create();
	auto mainPacksText = CCLabelBMFont::create("Main Packs", "bigFont.fnt");
	mainPacksText->setScale(0.4f);
	mainPacksText->setPosition({315.f / 2.f, 25.f / 2.f});
	mainPacksHeader->addChild(mainPacksText);
	cells->addObject(mainPacksHeader);

	int offs = 0;

	// main packs
	for (int i = 0; i < data["main"].asArray().unwrap().size(); i++)
	{
		auto packNode = CCNode::create();
		packNode->setID(fmt::format("main-pack-{}", i));
		packNode->setScale(0.75f);

		auto packData = data["main"][i];

		// sprite
		auto spriteName = fmt::format("{}.png", packData["plusSprite"].asString().unwrapOr("DP_Beginner")); // use plus sprite for main packs, normal for legacy
		CCSprite *sprite;
		if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr || spriteName == "DP_Invisible.png")
		{
			sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName("DP_Beginner.png").data());
			sprite->setVisible(false);
		}
		else
		{
			sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
		}
		sprite->setID("sprite");
		sprite->setScale(0.75f);
		sprite->setAnchorPoint({0.f, 0.5f});
		sprite->setPosition({5.f, 15.5f});

		// label
		auto label = CCLabelBMFont::create(fmt::format("{} Demons", packData["name"].asString().unwrapOr("null")).c_str(), "bigFont.fnt");
		label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
		label->setID("label");
		label->setScale(0.5f);
		label->setAnchorPoint({0.f, 0.5f});
		label->setPosition({40.f, 17.5f});

		// togglebox
		auto toggleMenu = CCMenu::create();
		toggleMenu->setScale(0.75f);
		toggleMenu->setPosition({330.f, -25.f});
		toggleMenu->setID("toggle-menu");

		auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
		auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(RoulettePopup::onPackToggle));
		toggle->setTag(i);
		toggle->setID("toggle");
		toggle->toggle(m_storedSettings[6 + i]);

		toggleMenu->addChild(toggle);

		// add children
		packNode->addChild(label);
		packNode->addChild(toggleMenu);
		packNode->addChild(sprite);

		cells->addObject(packNode);
	}

	offs = data["main"].asArray().unwrap().size();

	// legacy packs header
	auto legacyPacksHeader = CCNode::create();
	auto legacyPacksText = CCLabelBMFont::create("Legacy Packs", "bigFont.fnt");
	legacyPacksText->setScale(0.4f);
	legacyPacksText->setPosition({315.f / 2.f, 25.f / 2.f});
	legacyPacksHeader->addChild(legacyPacksText);
	cells->addObject(legacyPacksHeader);

	// legacy packs
	for (int i = 0; i < data["legacy"].asArray().unwrap().size(); i++)
	{
		auto packNode = CCNode::create();
		packNode->setID(fmt::format("legacy-pack-{}", i));
		packNode->setScale(0.75f);

		auto packData = data["legacy"][i];

		// sprite
		auto spriteName = fmt::format("{}.png", packData["sprite"].asString().unwrapOr("DP_Beginner")); // use plus sprite for main packs, normal for legacy
		CCSprite *sprite;
		if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr || spriteName == "DP_Invisible.png")
		{
			sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName("DP_Beginner.png").data());
			sprite->setVisible(false);
		}
		else
		{
			sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
		}
		sprite->setID("sprite");
		sprite->setScale(0.75f);
		sprite->setAnchorPoint({0.f, 0.5f});
		sprite->setPosition({5.f, 15.5f});

		// label
		auto label = CCLabelBMFont::create(fmt::format("{} Demons", packData["name"].asString().unwrapOr("null")).c_str(), "bigFont.fnt");
		label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
		label->setID("label");
		label->setScale(0.5f);
		label->setAnchorPoint({0.f, 0.5f});
		label->setPosition({40.f, 17.5f});

		// togglebox
		auto toggleMenu = CCMenu::create();
		toggleMenu->setScale(0.75f);
		toggleMenu->setPosition({330.f, -25.f});
		toggleMenu->setID("toggle-menu");

		auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
		auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(RoulettePopup::onPackToggle));
		toggle->setTag(i + offs);
		toggle->setID("toggle");
		toggle->toggle(m_storedSettings[6 + i + offs]);

		toggleMenu->addChild(toggle);

		// add children
		packNode->addChild(label);
		packNode->addChild(toggleMenu);
		packNode->addChild(sprite);

		cells->addObject(packNode);
	}

	offs = offs + data["legacy"].asArray().unwrap().size();

	// bonus packs header
	auto bonusPacksHeader = CCNode::create();
	auto bonusPacksText = CCLabelBMFont::create("Bonus Packs", "bigFont.fnt");
	bonusPacksText->setScale(0.4f);
	bonusPacksText->setPosition({315.f / 2.f, 25.f / 2.f});
	bonusPacksHeader->addChild(bonusPacksText);
	cells->addObject(bonusPacksHeader);

	// bonus packs
	for (int i = 0; i < data["bonus"].asArray().unwrap().size(); i++)
	{
		auto packNode = CCNode::create();
		packNode->setID(fmt::format("bonus-pack-{}", i));
		packNode->setScale(0.75f);

		auto packData = data["bonus"][i];

		// sprite
		auto spriteName = fmt::format("{}.png", packData["sprite"].asString().unwrapOr("DP_Beginner"));
		CCSprite *sprite;
		if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr || spriteName == "DP_Invisible.png")
		{
			sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName("DP_Beginner.png").data());
			sprite->setVisible(false);
		}
		else
		{
			sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
		}
		sprite->setID("sprite");
		sprite->setScale(0.75f);
		sprite->setAnchorPoint({0.f, 0.5f});
		sprite->setPosition({5.f, 15.5f});

		// label
		auto label = CCLabelBMFont::create(packData["name"].asString().unwrapOr("null").c_str(), "bigFont.fnt");
		label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
		label->setID("label");
		label->setScale(0.5f);
		label->setAnchorPoint({0.f, 0.5f});
		label->setPosition({40.f, 17.5f});

		// togglebox
		auto toggleMenu = CCMenu::create();
		toggleMenu->setScale(0.75f);
		toggleMenu->setPosition({330.f, -25.f});
		toggleMenu->setID("toggle-menu");

		auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
		auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(RoulettePopup::onPackToggle));
		toggle->setTag(i + offs);
		toggle->setID("toggle");
		toggle->toggle(m_storedSettings[6 + i + offs]);

		toggleMenu->addChild(toggle);

		// add children
		packNode->addChild(label);
		packNode->addChild(toggleMenu);
		packNode->addChild(sprite);

		cells->addObject(packNode);
	}

	auto packsMenu = CCMenu::create();
	packsMenu->setContentSize(m_mainLayer->getContentSize());
	packsMenu->setPosition({0.f, 0.f});
	packsMenu->setID("packs-menu");
	m_mainLayer->addChild(packsMenu);

	m_list = ListView::create(cells, 25.f, 315.f, 180.f);
	m_list->setPosition({52.f, 22.f});
	packsMenu->addChild(m_list);

	return;
}

void RoulettePopup::onSettings(CCObject*) {
	RouletteSettingsPopup::create(m_storedSeed, m_storedSettings)->show();
	
	return;
}

void RoulettePopup::onPackToggle(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto id = btn->getTag();

	m_storedSettings[id + 6] = !btn->isToggled();
	
	return;
}

void RoulettePopup::onPackCheckAll(CCObject*) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto totalPacks = data["main"].asArray().unwrap().size() + data["legacy"].asArray().unwrap().size() + data["bonus"].asArray().unwrap().size();
	
	for (int i = 0; i < totalPacks; i++) {
		m_storedSettings[i + 6] = true;
	}

	loadSettingsMenu();
	
	return;
}

void RoulettePopup::onPackUncheckAll(CCObject*) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto totalPacks = data["main"].asArray().unwrap().size() + data["legacy"].asArray().unwrap().size() + data["bonus"].asArray().unwrap().size();
	
	for (int i = 0; i < totalPacks; i++) {
		m_storedSettings[i + 6] = false;
	}

	loadSettingsMenu();
	
	return;
}

void RoulettePopup::onImportSettings(CCObject*) {
	RouletteImportPopup::create(true)->show();
	
	return;
}

void RoulettePopup::onFinalizeSave(CCObject*) {
	
	//check if at least one pack is selected
	auto packsOk = false;
	for (int i = 6; i < m_storedSettings.size(); i++) {
		if (m_storedSettings[i]) { //if at least one pack is selected, break and continue
			packsOk = true;
			break;
		}
	}

	if (!packsOk) { //if not, tell the player to select one
		FLAlertLayer::create(
			"Hold up!",
			"You need to select at least <cy>1</c> pack to create this save!",
			"OK"
			)->show(); 
        return; 
	}

	//create the save

	std::string name = "New Roulette";
	if (m_value->getString() != "") {
		name = m_value->getString();
	}

	auto settings = RouletteUtils::toFlags(m_storedSettings);

	auto seed = rand();
	if (m_storedSeed != -1) {
		seed = m_storedSeed;
	}

	//get packs and levels
    auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
    auto mainPacks = data["main"].asArray().unwrap();
    auto legacyPacks = data["legacy"].asArray().unwrap();
    auto bonusPacks = data["bonus"].asArray().unwrap();

    std::vector<matjson::Value> packs = {};
    for (int i = 0; i < mainPacks.size(); i++) {
        if (m_storedSettings[6 + i]) {
            packs.push_back(mainPacks[i]);
        }
    }
    for (int i = 0; i < legacyPacks.size(); i++) {
        if (m_storedSettings[6 + mainPacks.size() + i]) {
            packs.push_back(legacyPacks[i]);
        }
    }
    for (int i = 0; i < bonusPacks.size(); i++) {
        if (m_storedSettings[6 + mainPacks.size() + legacyPacks.size() + i]) {
            packs.push_back(bonusPacks[i]);
        }
    }

	auto levels = RouletteUtils::setupLevels(packs, settings, seed);

	if (levels.empty()) {
		FLAlertLayer::create(
			"Oops!",
			"Something went wrong creating your save!\nThis probably happened because you have Completed/Uncompleted Only on and don't have any matching levels.",
			"OK"
			)->show();
	
		loadSaveMenu();
	
		return;
	}

	RouletteUtils::createSave(name, levels, settings, seed);

	FLAlertLayer::create(
			"Save Created!",
			"Your save has been created successfully.",
			"OK"
			)->show();
	
	loadSaveMenu();
	
	return;
}

void RoulettePopup::loadRouletteSave(int id) {
	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto save = rouletteSaves[id];

	// extract all data
	std::string name = save.name; // name should be 16 characters AT MAX
	std::string settings = save.settings;
	int seed = save.seed;
	std::vector<int> levels = save.levels;
	int progress = save.progress;
	int skips = save.skips;
	int score = save.score;

	m_saveID = id;
	m_settings = RouletteUtils::fromFlags(settings);

	if (save.progress == 100)
	{
		loadWinScreen(id);
		return;
	}

	// find current index of the level you're on
	// if skips off or skips give points on: lvl = score
	// if skips give points off: lvl = score + skips
	int lvlIndex = score;
	if (m_settings[0] && !m_settings[1]) {
		lvlIndex += skips;
	}
	Mod::get()->setSavedValue<int>("roulette-lvl-id", levels[std::min(lvlIndex, static_cast<int>(levels.size() - 1))]);

	// calculate next percentage
	int percentJump = ceil(100.f / static_cast<float>(levels.size()));
	Mod::get()->setSavedValue<int>("roulette-next-goal", std::min(progress + percentJump, 100));
	if (lvlIndex == levels.size() - 1) { Mod::get()->setSavedValue<int>("roulette-next-goal", 100); }

	// set roulette vars
	Mod::get()->setSavedValue<bool>("in-roulette", true);
	Mod::get()->setSavedValue<int>("roulette-progress", 0);
	Mod::get()->setSavedValue<bool>("roulette-perfect", m_settings[4]);	 // perfect run enabled
	Mod::get()->setSavedValue<bool>("roulette-gauntlet", m_settings[5]); // the gauntlet enabled

	this->setTitle(name);
	m_mainLayer->removeAllChildrenWithCleanup(true);

	// back button
	auto backMenu = CCMenu::create();
	backMenu->setPosition({0.f, 0.f});
	backMenu->setID("back-menu");
	m_mainLayer->addChild(backMenu);

	auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backBtn = CCMenuItemSpriteExtra::create(backSpr, this, menu_selector(RoulettePopup::onBack));
	backBtn->setPosition(this->m_closeBtn->getPositionX(), this->m_closeBtn->getPositionY() - 40.f);
	backBtn->setID("back-btn");
	backMenu->addChild(backBtn);

	// actions
	auto actionsMenu = CCMenu::create();
	actionsMenu->setPosition({0.f, 0.f});
	actionsMenu->setID("actions-menu");
	actionsMenu->setVisible(false);
	m_mainLayer->addChild(actionsMenu);

	auto playSpr = ButtonSprite::create("Play", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(RoulettePopup::onPlay));
	playBtn->setPosition(130.f, 70.f);
	playBtn->setID("play-btn");
	actionsMenu->addChild(playBtn);

	auto nextSpr = ButtonSprite::create("Next", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto nextBtn = CCMenuItemSpriteExtra::create(nextSpr, this, menu_selector(RoulettePopup::onNext));
	nextBtn->setPosition(290.f, 70.f);
	nextBtn->setID("next-btn");
	actionsMenu->addChild(nextBtn);

	auto skipSpr = ButtonSprite::create("Skip", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto skipBtn = CCMenuItemSpriteExtra::create(skipSpr, this, menu_selector(RoulettePopup::onSkip));
	skipBtn->setPosition(210.f, 70.f);
	skipBtn->setID("skip-btn");
	skipBtn->setVisible(m_settings[0]);
	actionsMenu->addChild(skipBtn);

	// previous levels
	auto safeMenu = CCMenu::create();
	safeMenu->setPosition({0.f, 0.f});
	safeMenu->setID("safe-menu");
	m_mainLayer->addChild(safeMenu);

	auto safeSpr = CCSprite::createWithSpriteFrameName("GJ_safeBtn_001.png");
	auto safeBtn = CCMenuItemSpriteExtra::create(safeSpr, this, menu_selector(RoulettePopup::onSafe));
	safeBtn->setPosition(395.f, 225.f);
	safeBtn->setID("safe-btn");
	safeMenu->addChild(safeBtn);

	// visual stuff
	auto lvlLabel = CCLabelBMFont::create("Level Title", "bigFont.fnt");
	lvlLabel->setID("level-label");
	lvlLabel->setPosition(210.f, 200.f);
	lvlLabel->setScale(0.625f);
	lvlLabel->setZOrder(1);
	lvlLabel->setVisible(false);
	m_mainLayer->addChild(lvlLabel);

	auto creatorLabel = CCLabelBMFont::create("By Creator", "goldFont.fnt");
	creatorLabel->setID("creator-label");
	creatorLabel->setPosition(210.f, 185.f);
	creatorLabel->setScale(0.625f);
	creatorLabel->setZOrder(1);
	creatorLabel->setVisible(false);
	m_mainLayer->addChild(creatorLabel);

	// progress text
	int rouletteGoal = Mod::get()->getSavedValue<int>("roulette-next-goal", 100);
	if (m_settings[5]) { rouletteGoal = 100; }
	auto progressText = CCLabelBMFont::create(fmt::format("Get to {}%\nScore: {}", rouletteGoal, save.score).c_str(), "goldFont.fnt");
	if (m_settings[0]) { progressText->setCString(fmt::format("Get to {}%\nScore: {}\nSkips Used: {}", rouletteGoal, save.score, save.skips).c_str()); }
	progressText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	progressText->setPosition(210.f, 5.f);
	progressText->setScale(0.55f);
	progressText->setAnchorPoint({0.5f, 0.f});
	progressText->setID("progress-label");
	m_mainLayer->addChild(progressText);

	fetchLevel(Mod::get()->getSavedValue<int>("roulette-lvl-id"));

	return;
}

void RoulettePopup::loadWinScreen(int id)
{
	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto save = rouletteSaves[id];

	// extract all data
	std::string name = save.name; // name should be 16 characters AT MAX
	std::string settings = save.settings;
	int seed = save.seed;
	std::vector<int> levels = save.levels;
	int progress = save.progress;
	int skips = save.skips;
	int score = save.score;

	m_saveID = id;
	m_settings = RouletteUtils::fromFlags(settings);

	// set roulette vars
	Mod::get()->setSavedValue<bool>("in-roulette", true);
	Mod::get()->setSavedValue<int>("roulette-progress", 0);
	Mod::get()->setSavedValue<bool>("roulette-perfect", m_settings[4]);	 // perfect run enabled
	Mod::get()->setSavedValue<bool>("roulette-gauntlet", m_settings[5]); // the gauntlet enabled

	this->setTitle(name);
	m_mainLayer->removeAllChildrenWithCleanup(true);

	// back button
	auto backMenu = CCMenu::create();
	backMenu->setPosition({0.f, 0.f});
	backMenu->setID("back-menu");
	m_mainLayer->addChild(backMenu);

	auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backBtn = CCMenuItemSpriteExtra::create(backSpr, this, menu_selector(RoulettePopup::onBack));
	backBtn->setPosition(this->m_closeBtn->getPositionX(), this->m_closeBtn->getPositionY() - 40.f);
	backBtn->setID("back-btn");
	backMenu->addChild(backBtn);

	// previous levels
	auto safeMenu = CCMenu::create();
	safeMenu->setPosition({0.f, 0.f});
	safeMenu->setID("safe-menu");
	m_mainLayer->addChild(safeMenu);

	auto safeSpr = CCSprite::createWithSpriteFrameName("GJ_safeBtn_001.png");
	auto safeBtn = CCMenuItemSpriteExtra::create(safeSpr, this, menu_selector(RoulettePopup::onSafe));
	safeBtn->setPosition(395.f, 225.f);
	safeBtn->setID("safe-btn");
	safeMenu->addChild(safeBtn);

	// final grade

	auto percent = (static_cast<float>(save.score) / static_cast<float>(save.levels.size())) * 100.f;

	/*
	S (Perfect!) - 100%
	A (Excellent) - 95% to 99%
	B (Good) - 90% to 94%
	C (Okay) - 80% to 89%
	D (Poor) - 70% to 79%
	F (Fail!) - 0% to 69%
	*/

	std::map<int, int> rankReq = {{100, 0}, {95, 1}, {90, 2}, {80, 3}, {70, 4}, {0, 5}};
	auto letterGrade = 5; // F by default
	for (auto [req, grade] : rankReq)
	{
		if (percent >= req)
		{
			letterGrade = grade;
		}
	}

	std::vector<std::string> ranks = {"S", "A", "B", "C", "D", "F"};
	std::vector<std::string> subtitle = {"Perfect!", "Excellent", "Good", "Okay", "Poor", "Fail!"};
	std::vector<ccColor3B> rankColors = {{255, 0, 0}, {255, 135, 0}, {255, 255, 0}, {0, 255, 0}, {41, 41, 255}, {255, 255, 255}};

	auto rankStr = ranks[letterGrade];
	if (m_settings[4] || m_settings[5]) { rankStr = fmt::format("{}+", ranks[letterGrade]); }
	if (m_settings[1]) { rankStr = fmt::format("{}-", ranks[letterGrade]); }

	auto rankText = CCLabelBMFont::create(rankStr.c_str(), "bigFont.fnt");
	auto subtitleText = CCLabelBMFont::create(subtitle[letterGrade].c_str(), "bigFont.fnt");
	auto scoreText = CCLabelBMFont::create(fmt::format("{}/{}", save.score, save.levels.size()).c_str(), "bigFont.fnt");

	rankText->setPosition({210.f, 158.f});
	subtitleText->setPosition({210.f, 110.f});
	scoreText->setPosition({210.f, 85.f});

	rankText->setScale(2.5f);
	subtitleText->setScale(0.9f);
	scoreText->setScale(0.5f);

	rankText->setColor(rankColors[letterGrade]);
	subtitleText->setColor(rankColors[letterGrade]);
	scoreText->setColor(rankColors[letterGrade]);

	rankText->setID("rank-letter");
	subtitleText->setID("rank-subtitle");
	scoreText->setID("final-score");

	m_mainLayer->addChild(rankText);
	m_mainLayer->addChild(subtitleText);
	m_mainLayer->addChild(scoreText);

	// skips text
	auto skipsText = CCLabelBMFont::create(fmt::format("Skips Used: {}", save.skips).c_str(), "goldFont.fnt");
	if (!m_settings[0]) {
		skipsText->setVisible(false);
	}
	skipsText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	skipsText->setPosition(210.f, 5.f);
	skipsText->setScale(0.55f);
	skipsText->setAnchorPoint({0.5f, 0.f});
	skipsText->setID("skips-label");
	m_mainLayer->addChild(skipsText);

	return;
}

void RoulettePopup::onSafe(CCObject *sender)
{
	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto save = rouletteSaves[m_saveID];

	std::vector<int> safeLevels = {};
	for (int id : save.levels)
	{
		if (save.progress == 100) {
			if (m_settings[0] && !m_settings[1]) {
				if (safeLevels.size() == (save.score + save.skips)) { 
					break; 
				}
			}
			else if (safeLevels.size() == save.score) { 
				break;
			}
		}
		else if (m_settings[0] && !m_settings[1]) {
			if (safeLevels.size() > (save.score + save.skips)) { 
				break; 
			}
		}
		else if (safeLevels.size() > save.score) { 
			break;
		}
		
		safeLevels.push_back(id);
	}

	auto scene = CCScene::create();					   // creates the scene
	auto safe = RouletteSafeLayer::create(safeLevels); // creates the layer

	scene->addChild(safe);

	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition

	return;
}

void RoulettePopup::fetchLevel(int levelID) {

	m_loadingCancelled = false;

	// visual stuff
	m_loadcircle = LoadingCircle::create();
	m_loadcircle->m_parentLayer = this;
	m_loadcircle->show();

	CCLabelBMFont* lvlLabel = typeinfo_cast<CCLabelBMFont *>(m_mainLayer->getChildByID("level-label"));
	CCLabelBMFont* creatorLabel = typeinfo_cast<CCLabelBMFont *>(m_mainLayer->getChildByID("creator-label"));

	m_mainLayer->getChildByID("actions-menu")->setVisible(false);

	lvlLabel->setVisible(false);
	creatorLabel->setVisible(false);

	if (m_mainLayer->getChildByID("difficulty-sprite"))
	{
		m_mainLayer->getChildByID("difficulty-sprite")->removeMeAndCleanup();
	}

	// borrowed some stuff from integrated demon list
	auto glm = GameLevelManager::sharedState();
	glm->m_levelManagerDelegate = this;
	auto searchObject = GJSearchObject::create(SearchType::Type19, std::to_string(levelID));
	auto storedLevels = glm->getStoredOnlineLevels(searchObject->getKey());

	if (storedLevels)
	{
		loadLevelsFinished(storedLevels, "");
	}
	else
	{
		glm->getOnlineLevels(searchObject);
	}

	return;
}

void RoulettePopup::loadLevelsFinished(CCArray* levels, const char *) {
	if (m_loadingCancelled) { return; }

	auto level = typeinfo_cast<GJGameLevel*>(levels->objectAtIndex(0));
	m_currentLevel = level;

	m_levelLoaded = true;
	m_loadcircle->fadeAndRemove();

	CCLabelBMFont* lvlLabel = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("level-label"));
	CCLabelBMFont* creatorLabel = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("creator-label"));
	CCSprite* diffSpr = CCSprite::createWithSpriteFrameName("DP_Beginner.png"_spr);

	auto featuredSpr = CCSprite::createWithSpriteFrameName("GJ_featuredCoin_001.png");
	featuredSpr->setPosition({0.75f, -0.25f});
	featuredSpr->setAnchorPoint({0.f, 0.15f});
	featuredSpr->setZOrder(-1);
	featuredSpr->setID("featured-spr");
	featuredSpr->setVisible(false);

	auto epicSpr = CCSprite::createWithSpriteFrameName("GJ_epicCoin_001.png");
	epicSpr->setPosition({0.f, -2.f});
	epicSpr->setAnchorPoint({0.05f, 0.2f});
	epicSpr->setZOrder(-1);
	epicSpr->setID("epic-spr");
	epicSpr->setVisible(false);

	auto legendarySpr = CCSprite::createWithSpriteFrameName("GJ_epicCoin2_001.png"); // imma be honest idk if the legendary/mythic ones will be used but whatever :v
	legendarySpr->setPosition({-0.5f, -2.f});
	legendarySpr->setAnchorPoint({0.05f, 0.2f});
	legendarySpr->setZOrder(-1);
	legendarySpr->setID("legendary-spr");
	legendarySpr->setVisible(false);
	// diffSpr->addChild(legendarySpr);

	auto mythicSpr = CCSprite::createWithSpriteFrameName("GJ_epicCoin3_001.png");
	mythicSpr->setPosition({-2.f, -2.f});
	mythicSpr->setAnchorPoint({0.1f, 0.2f});
	mythicSpr->setZOrder(-1);
	mythicSpr->setID("mythic-spr");
	mythicSpr->setVisible(false);
	// diffSpr->addChild(mythicSpr);

	lvlLabel->setCString(level->m_levelName.c_str());
	lvlLabel->setVisible(true);

	creatorLabel->setCString(fmt::format("By {}", level->m_creatorName).c_str());
	creatorLabel->setVisible(true);

	// find out what pack the level is part of
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto mainPacks = data["main"];
	auto legacyPacks = data["legacy"];
	auto bonusPacks = data["bonus"];

	auto levelFound = false;

	for (auto pack : mainPacks)
	{
		for (auto lvl : pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault())
		{
			if (level->m_levelID == lvl)
			{
				// change sprite
				if (pack["sprite"].asString().unwrapOr("DP_Invisible") != "DP_Invisible" && pack["plusSprite"].asString().unwrapOr("DP_Invisible") != "DP_Invisible")
				{
					if (level->m_isEpic && Mod::get()->getSettingValue<bool>("replace-epic"))
					{
						auto sprite = fmt::format("{}.png", pack["plusSprite"].asString().unwrapOr("DP_Beginner"));
						diffSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprite).data());
					}
					else
					{
						auto sprite = fmt::format("{}.png", pack["sprite"].asString().unwrapOr("DP_Beginner"));
						diffSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprite).data());

						if (level->m_isEpic)
						{
							epicSpr->setVisible(true);
							diffSpr->addChild(epicSpr);
						}
						else if (level->m_featured != 0)
						{
							featuredSpr->setVisible(true);
							diffSpr->addChild(featuredSpr);
						}
					}
				}
				else
				{
					diffSpr->setOpacity(0);
				}

				levelFound = true;
				break;
			}
		}

		if (levelFound)
		{
			break;
		}
	}

	for (auto pack : legacyPacks)
	{
		if (levelFound)
		{
			break;
		}

		for (auto lvl : pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault())
		{
			if (level->m_levelID == lvl)
			{
				// change sprite
				if (pack["sprite"].asString().unwrapOr("DP_Invisible") != "DP_Invisible" && data["main"][pack["mainPack"].asInt().unwrapOr(0)]["plusSprite"].asString().unwrapOr("DP_Invisible") != "DP_Invisible")
				{
					if (level->m_isEpic && Mod::get()->getSettingValue<bool>("replace-epic"))
					{
						auto sprite = fmt::format("{}.png", data["main"][pack["mainPack"].asInt().unwrapOr(0)]["plusSprite"].asString().unwrapOr("DP_Beginner"));
						diffSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprite).data());
					}
					else
					{
						auto sprite = fmt::format("{}.png", pack["sprite"].asString().unwrapOr("DP_Beginner"));
						diffSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprite).data());

						if (level->m_isEpic)
						{
							epicSpr->setVisible(true);
							diffSpr->addChild(epicSpr);
						}
						else if (level->m_featured != 0)
						{
							featuredSpr->setVisible(true);
							diffSpr->addChild(featuredSpr);
						}
					}
				}
				else
				{
					diffSpr->setOpacity(0);
				}

				levelFound = true;
				break;
			}
		}
	}

	for (auto pack : bonusPacks)
	{
		if (levelFound)
		{
			break;
		}
		for (auto lvl : pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault())
		{
			if (level->m_levelID == lvl)
			{
				// change sprite
				if (pack["sprite"].asString().unwrapOr("DP_Invisible") != "DP_Invisible")
				{
					auto sprite = fmt::format("{}.png", pack["sprite"].asString().unwrapOr("DP_Beginner"));
					diffSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprite).data());

					if (level->m_isEpic)
					{
						epicSpr->setVisible(true);
						diffSpr->addChild(epicSpr);
					}
					else if (level->m_featured != 0)
					{
						featuredSpr->setVisible(true);
						diffSpr->addChild(featuredSpr);
					}
				}
				else
				{
					diffSpr->setOpacity(0);
				}

				levelFound = true;
				break;
			}
		}
	}

	diffSpr->setPosition({210.f, 135.f});
	diffSpr->setScale(2.f);
	diffSpr->setID("difficulty-sprite");
	m_mainLayer->addChild(diffSpr);

	m_mainLayer->getChildByID("actions-menu")->setVisible(true);

	return;
}

void RoulettePopup::loadLevelsFailed(const char *)
{
	if (m_loadingCancelled)
	{
		return;
	}

	m_levelLoaded = true;
	m_loadcircle->fadeAndRemove();

	auto alert = FLAlertLayer::create("ERROR", "Something went wrong...", "OK");
	alert->setParent(this);
	alert->show();

	return;
}

void RoulettePopup::onPlay(CCObject *sender)
{
	auto scene = CCScene::create();								// creates the scene
	auto layer = LevelInfoLayer::create(m_currentLevel, false); // creates the layer

	scene->addChild(layer);

	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition

	return;
}

void RoulettePopup::onSkip(CCObject *sender)
{
	geode::createQuickPopup(
		"Skip Level",										 // title
		"Are you sure you want to <cr>skip</c> this level?", // content
		"No", "Yes",										 // buttons
		[&](auto, bool btn2)
		{
			if (btn2)
			{
				saveProgress(true);

				bool inRoulette = Mod::get()->getSavedValue<bool>("in-roulette", false);
				int currentLvlID = Mod::get()->getSavedValue<int>("roulette-lvl-id", -1);
				int rouletteProgress = Mod::get()->getSavedValue<int>("roulette-progress", 0);
				int rouletteGoal = Mod::get()->getSavedValue<int>("roulette-next-goal", 100);

				bool perfectEnabled = Mod::get()->getSavedValue<bool>("roulette-perfect", false);
				bool gauntletEnabled = Mod::get()->getSavedValue<bool>("roulette-gauntlet", false);

				auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
				auto save = rouletteSaves[m_saveID];

				auto progressText = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("progress-label"));

				Mod::get()->setSavedValue<int>("roulette-progress", 0);

				if (gauntletEnabled) { rouletteGoal = 100; }
				progressText->setCString(fmt::format("Get to {}%\nScore: {}", rouletteGoal, save.score).c_str());
				if (m_settings[0]) { progressText->setCString(fmt::format("Get to {}%\nScore: {}\nSkips Used: {}", rouletteGoal, save.score, save.skips).c_str()); }

				int lvlIndex = save.score;
				if (m_settings[0] && !m_settings[1])
				{
					lvlIndex += save.skips;
				}
				auto levelID = save.levels[std::min(lvlIndex, static_cast<int>(save.levels.size() - 1))];

				if (save.progress < 100)
				{
					fetchLevel(levelID);
					Mod::get()->setSavedValue<int>("roulette-lvl-id", levelID);
				}
				else
				{
					// you beat the roulette :D
					log::info("GG, you skipped the last level though :v");
					loadWinScreen(m_saveID);
				}
			}
		});

	return;
}

void RoulettePopup::onNext(CCObject *sender) {
	bool inRoulette = Mod::get()->getSavedValue<bool>("in-roulette", false);
	int currentLvlID = Mod::get()->getSavedValue<int>("roulette-lvl-id", -1);
	int rouletteProgress = Mod::get()->getSavedValue<int>("roulette-progress", 0);
	int rouletteGoal = Mod::get()->getSavedValue<int>("roulette-next-goal", 100);

	bool perfectEnabled = Mod::get()->getSavedValue<bool>("roulette-perfect", false);
	bool gauntletEnabled = Mod::get()->getSavedValue<bool>("roulette-gauntlet", false);

	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto save = rouletteSaves[m_saveID];

	// check if the player is able to move on
	bool canProceed = false;
	if (gauntletEnabled && rouletteProgress == 100) {
		canProceed = true;
	}
	else if (perfectEnabled && rouletteProgress == rouletteGoal) {
		canProceed = true;
	}
	else if (!gauntletEnabled && !perfectEnabled && rouletteProgress >= rouletteGoal) {
		canProceed = true;
	}

	if (canProceed) {
		saveProgress(false);
		rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
		save = rouletteSaves[m_saveID];

		Mod::get()->setSavedValue<int>("roulette-progress", 0);
		rouletteGoal = Mod::get()->getSavedValue<int>("roulette-next-goal", 100);

		auto progressText = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("progress-label"));

		if (gauntletEnabled) { rouletteGoal = 100; }
		progressText->setCString(fmt::format("Get to {}%\nScore: {}", rouletteGoal, save.score).c_str());
		if (m_settings[0]) { progressText->setCString(fmt::format("Get to {}%\nScore: {}\nSkips Used: {}", rouletteGoal, save.score, save.skips).c_str()); }

		int lvlIndex = save.score;
		if (m_settings[0] && !m_settings[1]) { 
			lvlIndex += save.skips;
		}
		auto levelID = save.levels[std::min(lvlIndex, static_cast<int>(save.levels.size() - 1))];

		if (save.progress < 100) {
			fetchLevel(levelID);
			Mod::get()->setSavedValue<int>("roulette-lvl-id", levelID);
		}
		else {
			// you beat the roulette :D
			log::info("GG");
			loadWinScreen(m_saveID);
		}
	}
	else {
		if (gauntletEnabled) { rouletteGoal = 100; }

		std::vector<std::string> titleStrings = {
			"You shall not pass!",
			"L Bozo",
			"JUDGEMENT!",
			"nuh uh",
			"You\'re not winnin\' son",
			"Stop! You have violated the law!",
			"August 12th, 2036, Heat Death of the Universe",
			"oopsie whoopsie!~ :3c"
		};

		std::vector<std::string> subtitleStrings = {
			fmt::format("You need at least {}% to move on!", rouletteGoal),
			fmt::format("Get at least {}%, then come talk to me.", rouletteGoal),
			fmt::format("You gotta beat {}% at the very least.", rouletteGoal),
			fmt::format("Don't worry, you can get {}%!", rouletteGoal),
			fmt::format("Hey there, I can't let you pass until you get {}%.", rouletteGoal),
			fmt::format("You need {}%.", rouletteGoal),
			fmt::format("oopsie!~ s-sowwy, but wou need {}% to move on!~ >w<", rouletteGoal),
			fmt::format("Yeaaa, I'm sorry, but you need {}% to move forward.", rouletteGoal)
		};

		std::vector<std::string> confirmStrings = {
			"OK",
			"Fine",
			"Alright",
			"Whatever",
			"Bruh",
			":(",
			"Oh come on",
			"Really?"
		};

		//randomize stuff
    	std::seed_seq seed{rand()};
    	std::mt19937 gen(seed);
    	std::uniform_int_distribution<int> distribution(0, titleStrings.size() - 1);

		FLAlertLayer::create(
			titleStrings[distribution(gen)].c_str(),
			subtitleStrings[distribution(gen)].c_str(),
			confirmStrings[distribution(gen)].c_str()
			)->show(); 
	}

	return;
}

void RoulettePopup::saveProgress(bool skip) {

	bool inRoulette = Mod::get()->getSavedValue<bool>("in-roulette", false);
	int currentLvlID = Mod::get()->getSavedValue<int>("roulette-lvl-id", -1);
	int rouletteProgress = Mod::get()->getSavedValue<int>("roulette-progress", 0);
	int rouletteGoal = Mod::get()->getSavedValue<int>("roulette-next-goal", 100);

	bool perfectEnabled = Mod::get()->getSavedValue<bool>("roulette-perfect", false);
	bool gauntletEnabled = Mod::get()->getSavedValue<bool>("roulette-gauntlet", false);

	if (!inRoulette) { return; }

	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto save = rouletteSaves[m_saveID];

	if (skip) {
		save.skips += 1;
		save.progress = rouletteGoal;
		if (m_settings[1]) { save.score += 1; }
	}
	else if (gauntletEnabled && rouletteProgress == 100) {
		save.score += 1;
		save.progress = rouletteGoal;
	}
	else if ((rouletteProgress >= rouletteGoal) || (perfectEnabled && rouletteProgress == rouletteGoal))
	{
		save.score += 1;
		save.progress = rouletteProgress;
	}

	// calculate next percentage
	int percentJump = ceil(100.f / static_cast<float>(save.levels.size()));
	Mod::get()->setSavedValue<int>("roulette-next-goal", std::min(save.progress + percentJump, 100));
	if (currentLvlID == save.levels[save.levels.size() - 1]) { Mod::get()->setSavedValue<int>("roulette-next-goal", 100); }

	rouletteSaves[m_saveID] = save;
	Mod::get()->setSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", rouletteSaves);

	return;
}

void RoulettePopup::onEnterSave(CCObject *sender) {
	auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);
	auto id = btn->getTag();

	loadRouletteSave(id);
	return;
}

void RoulettePopup::onNewSave(CCObject *sender)
{

	RouletteNewPopup::create()->show();

	// PLACEHOLDER STUFF
	/*auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto mainPacks = data["main"].as<std::vector<matjson::Value>>().unwrap();
	auto legacyPacks = data["legacy"].as<std::vector<matjson::Value>>().unwrap();
	auto bonusPacks = data["bonus"].as<std::vector<matjson::Value>>().unwrap();
	//auto packs = mainPacks + legacyPacks + bonusPacks;
	auto settings = 0;
	auto seed = rand();
	RouletteUtils::createSave("test", RouletteUtils::setupLevels(mainPacks, settings, seed), settings, seed);
	loadSaveMenu();*/

	return;
}

void RoulettePopup::onCopy(CCObject *sender)
{
	auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);

	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto id = btn->getTag();

	m_saveID = id;

	geode::createQuickPopup(
		"Copy Save",										// title
		"Are you sure you want to <cy>copy</c> this save?", // content
		"No", "Yes",										// buttons
		[&](auto, bool btn2)
		{
			if (btn2)
			{
				RouletteUtils::copySaveAtPos(m_saveID);
				loadSaveMenu();
			}
		});

	return;
}

void RoulettePopup::onExport(CCObject *sender)
{
	auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);

	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto id = btn->getTag();

	// TODO: Add Prompt
	// Just export to clipboard for now

	matjson::Value save = rouletteSaves[id];
	RouletteUtils::exportSave(save);

	auto tap = TextAlertPopup::create("Save Copied to Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");
	this->addChild(tap);
}

void RoulettePopup::onDeleteSave(CCObject *sender)
{
	auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);

	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto id = btn->getTag();

	m_saveID = id;

	geode::createQuickPopup(
		"Delete Save",																				 // title
		"Are you sure you want to <cr>delete</c> this save?\n<cr>This action cannot be undone.</c>", // content
		"No", "Yes",																				 // buttons
		[&](auto, bool btn2)
		{
			if (btn2)
			{
				RouletteUtils::deleteSaveAtPos(m_saveID);
				loadSaveMenu();
			}
		});

	return;
}

void RoulettePopup::onRename(CCObject *sender)
{
	auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);
	auto id = btn->getTag();

	RouletteRenamePopup::create(id)->show();

	return;
}

RoulettePopup *RoulettePopup::create()
{
	auto ret = new RoulettePopup();
	if (ret && ret->initAnchored(420.f, 250.f))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

RoulettePopup::~RoulettePopup()
{
	this->removeAllChildrenWithCleanup(true);
}

/*
===========================
RouletteRenamePopup
===========================
*/

bool RouletteRenamePopup::setup()
{
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Edit Save");

	auto layer = typeinfo_cast<CCLayer *>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto save = rouletteSaves[m_saveID];

	// add text edit area
	m_value = TextInput::create(230.f, "Name", "bigFont.fnt");
	m_value->setString(save.name);
	m_value->setCommonFilter(CommonFilter::Name);
	m_value->setMaxCharCount(16);
	m_value->setPosition({130.f, 135.f});
	m_value->setID("name-value");
	m_mainLayer->addChild(m_value);

	// add export settings button
	auto exportMenu = CCMenu::create();
	exportMenu->setPosition({0.f, 0.f});
	exportMenu->setID("export-menu");
	m_mainLayer->addChild(exportMenu);

	auto exportSpr = ButtonSprite::create("Copy Settings", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto exportBtn = CCMenuItemSpriteExtra::create(exportSpr, this, menu_selector(RouletteRenamePopup::onExportSettings));
	exportBtn->setPosition(130.f, 83.f);
	exportBtn->setID("export-settings-btn");
	exportMenu->addChild(exportBtn);

	// add confirm rename button
	auto confirmMenu = CCMenu::create();
	confirmMenu->setPosition({0.f, 0.f});
	confirmMenu->setID("confirm-menu");
	m_mainLayer->addChild(confirmMenu);

	auto confirmSpr = ButtonSprite::create("OK", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(RouletteRenamePopup::onConfirm));
	confirmBtn->setPosition(130.f, 30.f);
	confirmBtn->setID("confirm-btn");
	confirmMenu->addChild(confirmBtn);

	return true;
}

void RouletteRenamePopup::onExportSettings(CCObject *)
{
	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	RouletteSaveFormat save = rouletteSaves[m_saveID];
	RouletteUtils::exportSettings(save.settings, save.seed);

	auto tap = TextAlertPopup::create("Settings Copied to Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");
	this->addChild(tap);

	return;
}

void RouletteRenamePopup::onConfirm(CCObject *)
{
	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	RouletteSaveFormat save = rouletteSaves[m_saveID];

	if (m_value->getString() == "")
	{
		save.name = "New Roulette";
	}
	else
	{
		save.name = m_value->getString();
	}

	rouletteSaves[m_saveID] = save;
	Mod::get()->setSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", rouletteSaves);

	RoulettePopup *popup = this->getParent()->getChildByType<RoulettePopup>(0);
	popup->loadSaveMenu();

	this->removeMeAndCleanup();

	return;
}

RouletteRenamePopup *RouletteRenamePopup::create(int id)
{
	auto ret = new RouletteRenamePopup();
	ret->m_saveID = id;
	if (ret && ret->initAnchored(260.f, 200.f))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

RouletteRenamePopup::~RouletteRenamePopup()
{
	this->removeAllChildrenWithCleanup(true);
}

/*
===========================
RouletteNewPopup
===========================
*/

bool RouletteNewPopup::setup()
{
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("New Save");

	auto layer = typeinfo_cast<CCLayer *>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	// add buttons
	auto menu = CCMenu::create();
	menu->setPosition({0.f, 0.f});
	menu->setID("new-menu");
	m_mainLayer->addChild(menu);

	auto newSpr = ButtonSprite::create("Create New", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto newBtn = CCMenuItemSpriteExtra::create(newSpr, this, menu_selector(RouletteNewPopup::onCreate));
	newBtn->setPosition(130.f, 90.f);
	newBtn->setID("create-btn");
	menu->addChild(newBtn);

	auto importSpr = ButtonSprite::create("Import Save", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto importBtn = CCMenuItemSpriteExtra::create(importSpr, this, menu_selector(RouletteNewPopup::onImport));
	importBtn->setPosition(130.f, 45.f);
	importBtn->setID("import-btn");
	menu->addChild(importBtn);

	return true;
}

void RouletteNewPopup::onCreate(CCObject*) {
	RoulettePopup* popup = this->getParent()->getChildByType<RoulettePopup>(0);
	popup->m_storedSettings = RouletteUtils::fromFlags("001100"); // default flags
	popup->m_storedSeed = -1; // default seed, -1 means random
	
	popup->loadSettingsMenu();	

	this->removeMeAndCleanup();

	return;
}

void RouletteNewPopup::onImport(CCObject*) {
	RouletteImportPopup::create()->show();

	this->removeMeAndCleanup();

	return;
}

RouletteNewPopup *RouletteNewPopup::create()
{
	auto ret = new RouletteNewPopup();
	if (ret && ret->initAnchored(260.f, 150.f))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

RouletteNewPopup::~RouletteNewPopup()
{
	this->removeAllChildrenWithCleanup(true);
}

/*
===========================
RouletteImportPopup
===========================
*/

bool RouletteImportPopup::setup()
{
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Enter Save Text");
	if (m_isSettings) { this->setTitle("Enter Settings Text"); }

	auto layer = typeinfo_cast<CCLayer *>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	m_value = TextInput::create(400.f, "Enter Save Text", "bigFont.fnt");
	if (m_isSettings) { m_value->setPlaceholder("Enter Settings Text"); }
	m_value->setString(clipboard::read());
	m_value->setCommonFilter(CommonFilter::Base64URL);
	m_value->setMaxCharCount(0);
	m_value->setPosition({210.f, 75.f});
	m_value->setID("import-value");
	m_mainLayer->addChild(m_value);

	// add confirm button
	auto confirmMenu = CCMenu::create();
	confirmMenu->setPosition({0.f, 0.f});
	confirmMenu->setID("confirm-menu");
	m_mainLayer->addChild(confirmMenu);

	auto confirmSpr = ButtonSprite::create("OK", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(RouletteImportPopup::onConfirm));
	confirmBtn->setPosition(210.f, 30.f);
	confirmBtn->setID("confirm-btn");
	confirmMenu->addChild(confirmBtn);

	// add paste button
	auto pasteMenu = CCMenu::create();
	pasteMenu->setPosition({0.f, 0.f});
	pasteMenu->setID("paste-menu");
	m_mainLayer->addChild(pasteMenu);

	auto pasteSpr = ButtonSprite::create("Paste", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto pasteBtn = CCMenuItemSpriteExtra::create(pasteSpr, this, menu_selector(RouletteImportPopup::onPaste));
	pasteBtn->setPosition(370.f, 30.f);
	pasteBtn->setID("paste-btn");
	pasteMenu->addChild(pasteBtn);

	return true;
}

void RouletteImportPopup::onPaste(CCObject*) {
	m_value->setString(clipboard::read());

	auto tap = TextAlertPopup::create("Pasted From Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");
	this->addChild(tap);

	return;
}

void RouletteImportPopup::onConfirm(CCObject*) {

	if (m_isSettings) {
		auto result = RouletteUtils::importSettings(m_value->getString());
		auto seed = std::stoi(result[1]);
		auto settings = result[0];

		RoulettePopup* popup = this->getParent()->getChildByType<RoulettePopup>(0);
		popup->m_storedSeed = seed;
		popup->m_storedSettings = RouletteUtils::fromFlags(settings);
		popup->loadSettingsMenu();
	}
	else {
		RouletteUtils::importSave(m_value->getString());

		RoulettePopup* popup = this->getParent()->getChildByType<RoulettePopup>(0);
		popup->loadSaveMenu();
	}

	this->removeMeAndCleanup();

	return;
}

RouletteImportPopup* RouletteImportPopup::create(bool isSettings) {
	auto ret = new RouletteImportPopup();
	ret->m_isSettings = isSettings;
	if (ret && ret->initAnchored(420.f, 125.f))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

RouletteImportPopup::~RouletteImportPopup() {
	this->removeAllChildrenWithCleanup(true);
}

/*
===========================
RouletteSettingsPopup
===========================
*/

bool RouletteSettingsPopup::setup() {
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Roulette Settings");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	// seed value
	m_value = TextInput::create(350.f, "Custom Seed (Leave Blank for Random)", "bigFont.fnt");
	if (m_seed > -1) { m_value->setString(std::to_string(m_seed)); }
	m_value->setCommonFilter(CommonFilter::Int);
	m_value->setMaxCharCount(9);
	m_value->setPosition({210.f, 200.f});
	m_value->setID("name-value");
	m_mainLayer->addChild(m_value);

	// add buttons
	auto menu = CCMenu::create();
	menu->setPosition({0.f, 0.f});
	menu->setID("settings-menu");
	m_mainLayer->addChild(menu);

	std::vector<std::string> names = {
		"Enable Skips",
		"Skips Award Points",
		"Completed Levels",
		"Uncompleted Levels",
		"Perfect Run",
		"The Gauntlet"
	};

	std::vector<std::string> descriptions = {
		"Allows you to <cr>Skip</c> levels.",
		"Skipping a level still gives you a point.\n<cr>Note that this is counted as Cheating!</c>",
		"Completed levels can be added to the Roulette.",
		"Uncompleted levels can be added to the Roulette.",
		"<cy>Challenge Mode</c>\nNo skipping Percentages. You have to get the exact percent for every level, every time.",
		"<cp>Challenge Mode</c>\nBeat every single level. Your only goal is 100%."
	};

	std::vector<ccColor3B> colors = {
		{ 255, 255, 255 },
		{ 255, 0, 0 },
		{ 255, 255, 255 },
		{ 255, 255, 255 },
		{ 255, 255, 0 },
		{ 255, 0, 255 }
	};

	std::vector<CCPoint> positions = {
		{ 105.f, 155.f },
		{ 310.f, 155.f },
		{ 105.f, 100.f },
		{ 310.f, 100.f },
		{ 105.f, 45.f },
		{ 310.f, 45.f }
	};

	for (int i = 0; i < names.size(); i++) {
		auto name = names[i];
		auto description = descriptions[i];
		auto color = colors[i];
		auto position = positions[i];

		auto parent = CCNode::create();
		parent->setPosition(position);
		parent->setContentSize({ 0.f, 0.f });
		parent->setScale(0.8f);
		parent->setID(fmt::format("setting-{}", i));
		menu->addChild(parent);

		auto label = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
		label->setPosition({ -60.f, 2.5f });
		label->setAnchorPoint({ 0.f, 0.5f });
		label->setScale(0.65f);
		if (name.size() > 12) { label->setScale(0.55f); }
		label->setColor(color);
		label->setID("label");
		parent->addChild(label);

		auto toggleMenu = CCMenu::create();
		toggleMenu->setPosition({ 0.f, 0.f });
		toggleMenu->setID("toggle-menu");
		parent->addChild(toggleMenu);

		auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
		auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(RouletteSettingsPopup::onToggle));
		toggle->setPosition({ -80.f, 0.f });
		toggle->setTag(i);
		toggle->setID("toggle-btn");
		toggle->toggle(m_settings[i]);
		toggleMenu->addChild(toggle);

		auto infoMenu = CCMenu::create();
		infoMenu->setPosition({ -65.f, 13.5f });
		infoMenu->setZOrder(1);
		infoMenu->setID("info-menu");
		parent->addChild(infoMenu);

		auto infoBtn = InfoAlertButton::create(name, description, 0.65f);
		infoBtn->setID("info-btn");
		infoMenu->addChild(infoBtn);
	}

	return true;
}

void RouletteSettingsPopup::onClose(CCObject *sender) {
	if (m_value->getString() == "" || m_value->getString().size() > 9) {
		m_seed = -1;
	}
	else {
		m_seed = abs(std::stoi(m_value->getString()));
	}

	RoulettePopup *popup = this->getParent()->getChildByType<RoulettePopup>(0);
	popup->m_storedSeed = m_seed;

	if (!m_settings[0] && m_settings[1]) { m_settings[0] = true; }
	if (m_settings[5] == m_settings[4]) { m_settings[4] = false; }
	if (m_settings[2] == m_settings[3]) { 
		m_settings[2] = true; 
		m_settings[3] = true;
	}
	
	popup->m_storedSettings = m_settings;

	// normal closing stuff
	this->setKeypadEnabled(false);
	this->setTouchEnabled(false);
	this->removeFromParentAndCleanup(true);

	return;
}

void RouletteSettingsPopup::onToggle(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto id = btn->getTag();

	m_settings[id] = !btn->isToggled();
	
	return;
}

RouletteSettingsPopup *RouletteSettingsPopup::create(int seed, std::vector<bool> settings)
{
	auto ret = new RouletteSettingsPopup();
	ret->m_seed = seed;
	ret->m_settings = settings;
	if (ret && ret->initAnchored(420.f, 250.f))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

RouletteSettingsPopup::~RouletteSettingsPopup() {
	this->removeAllChildrenWithCleanup(true);
}