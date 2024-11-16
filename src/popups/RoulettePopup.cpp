//geode header
#include <Geode/Geode.hpp>

//other headers
#include "../menus/DPLayer.hpp"
#include "RoulettePopup.hpp"
#include "../RouletteUtils.hpp"

//geode namespace
using namespace geode::prelude;

bool RoulettePopup::setup() {
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Roulette");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	//create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	//create save menu
	loadSaveMenu();

	return true;
}

void RoulettePopup::loadSaveMenu() {

	this->setTitle("Roulette");
	m_mainLayer->removeAllChildrenWithCleanup(true);

	auto cells = CCArray::create();
	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});

	//create/import button
	auto newMenu = CCMenu::create();
	newMenu->setPosition({ 322.5f, -13.f });
	newMenu->setScale(0.75f);
	newMenu->setID("new-menu");

	auto newBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png"), this, menu_selector(RoulettePopup::onNewSave));
	newBtn->setID("new-btn");

	newMenu->addChild(newBtn);
	m_mainLayer->addChild(newMenu);

	//list
	if (rouletteSaves.size() < 1) {
		auto emptyText = CCLabelBMFont::create("No Saves Found!\nCreate or Import One to Start!", "bigFont.fnt");
		emptyText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
		emptyText->setScale(0.65f);
		emptyText->setPosition({ 210.f, 125.f });
		emptyText->setID("empty-text");
		m_mainLayer->addChild(emptyText);
	}
	else {
		int id = 0;
		for (auto save : rouletteSaves) {
			CCNode* cell = ListCell::create();
			cell->setTag(id);

			//extract all data
			std::string name = save.name;
			int settings = save.settings;
    		int seed = save.seed;
    		std::vector<int> levels = save.levels;
    		int progress = save.progress;
    		int skips = save.skips;
    		int score = save.score;

			//auto settingsBool = RouletteUtils::fromFlags(settings);

			//name label
			auto nameLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
			nameLabel->setPosition({ 5.f, 50.f });
			nameLabel->setScale(0.6f);
			nameLabel->setAnchorPoint({ 0.f, 1.f });
			nameLabel->setID("name-label");
			cell->addChild(nameLabel);

			//show current progress and score
			auto progressLabel = CCLabelBMFont::create(fmt::format("{}%", progress).c_str(), "bigFont.fnt");
			progressLabel->setPosition({ 215.f, 45.f });
			progressLabel->setScale(0.5f);
			progressLabel->setAnchorPoint({ 0.5f, 1.f });
			progressLabel->setID("progress-label");
			cell->addChild(progressLabel);

			auto scoreLabel = CCLabelBMFont::create(fmt::format("Score: {}", score).c_str(), "bigFont.fnt");
			scoreLabel->setPosition({ 215.f, 5.f });
			scoreLabel->setScale(0.5f);
			scoreLabel->setAnchorPoint({ 0.5f, 0.f });
			scoreLabel->setID("score-label");
			cell->addChild(scoreLabel);

			//add labels for certain settings (skips enabled, perfect run enabled, etc)
			//TODO

			//if completed, change to gold text
			if (progress >= 100) {
				//...
			}

			//add buttons (play, copy, delete, export)
			auto playMenu = CCMenu::create();
			playMenu->setPosition({ 0.f, 0.f });
			playMenu->setID("play-menu");
			cell->addChild(playMenu);

			auto playSpr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
			playSpr->setScale(0.6f);
			auto playBtn = CCMenuItemSpriteExtra::create(playSpr, this, menu_selector(RoulettePopup::placeholder));
			playBtn->setPosition({ 290.f, 25.f });
			playBtn->setID("play-btn");
			playMenu->addChild(playBtn);

			auto actionsMenu = CCMenu::create();
			actionsMenu->setPosition({ 0.f, 0.f });
			actionsMenu->setID("actions-menu");
			cell->addChild(actionsMenu);

			auto copySpr = CCSprite::createWithSpriteFrameName("GJ_duplicateBtn_001.png");
			copySpr->setScale(0.6f);
			auto copyBtn = CCMenuItemSpriteExtra::create(copySpr, this, menu_selector(RoulettePopup::onCopy));
			copyBtn->setPosition({ 75.f, 15.f });
			copyBtn->setTag(id);
			copyBtn->setID("copy-btn");
			actionsMenu->addChild(copyBtn);

			auto exportSpr = CCSprite::createWithSpriteFrameName("GJ_shareBtn_001.png");
			exportSpr->setScale(0.35f);
			auto exportBtn = CCMenuItemSpriteExtra::create(exportSpr, this, menu_selector(RoulettePopup::onExport));
			exportBtn->setPosition({ 45.f, 15.f });
			exportBtn->setTag(id);
			exportBtn->setID("export-btn");
			actionsMenu->addChild(exportBtn);

			auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
			deleteSpr->setScale(0.6f);
			auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(RoulettePopup::onDeleteSave));
			deleteBtn->setPosition({ 15.f, 15.f });
			deleteBtn->setTag(id);
			deleteBtn->setID("delete-btn");
			actionsMenu->addChild(deleteBtn);

			//add cell
			cells->addObject(cell);
			id += 1;
		}
	}

	auto saveMenu = CCMenu::create();
	saveMenu->setContentSize(m_mainLayer->getContentSize());
	saveMenu->setPosition({ 0.f, 0.f });
	saveMenu->setID("save-menu");

	m_list = ListView::create(cells, 50.f, 315.f, 180.f);
	m_list->setPosition({ 53.f, 32.f });
	saveMenu->addChild(m_list);

	m_mainLayer->addChild(saveMenu);

	return;
}

void RoulettePopup::placeholder(CCObject* sender) {
	return;
}

void RoulettePopup::onNewSave(CCObject* sender) {

	//TODO: Add Prompt for New Save / Import Save
	
	//PLACEHOLDER STUFF
	RouletteUtils::createSave("test", {0}, 0, rand());
	loadSaveMenu();

	return;
}

void RoulettePopup::onCopy(CCObject* sender) {
	auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);

	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto id = btn->getTag();

	auto pos = rouletteSaves.begin() + id;
	rouletteSaves.insert(pos, rouletteSaves[id]);
	Mod::get()->setSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", rouletteSaves);
	loadSaveMenu();

	return;
}

void RoulettePopup::onExport(CCObject* sender) {
	auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);

	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto id = btn->getTag();

	//TODO: Add Prompt
	//Just export to clipboard for now

	matjson::Value save = rouletteSaves[id];
	RouletteUtils::exportSave(save);

	auto tap = TextAlertPopup::create("Save Copied to Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");
	this->addChild(tap);
}

void RoulettePopup::onDeleteSave(CCObject* sender) {
	auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);

	auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
	auto id = btn->getTag();

    auto pos = rouletteSaves.begin() + id;
	rouletteSaves.erase(pos);
	Mod::get()->setSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", rouletteSaves);
	loadSaveMenu();
}

RoulettePopup* RoulettePopup::create() {
	auto ret = new RoulettePopup();
	if (ret && ret->initAnchored(420.f, 250.f)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

RoulettePopup::~RoulettePopup() {
	this->removeAllChildrenWithCleanup(true);
}