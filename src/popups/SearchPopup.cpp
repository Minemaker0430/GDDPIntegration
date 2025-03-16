//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/JsonValidation.hpp>

#include "../menus/DPLayer.hpp"
#include "../XPUtils.hpp"
#include "../popups/XPPopup.hpp"
#include "SearchPopup.hpp"
#include "../menus/DPSearchLayer.hpp"

//geode namespace
using namespace geode::prelude;

bool SearchPopup::setup() {

	this->setTitle("Search");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	//restore filters
	restoreFilters();

	//create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	//create tabs
	auto tabs = CCMenu::create();

	auto difficultiesBtnSprOff = ButtonSprite::create("Difficulties", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	difficultiesBtnSprOff->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto difficultiesBtnSprOn = ButtonSprite::create("Difficulties", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	difficultiesBtnSprOn->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto difficultiesBtn = CCMenuItemToggler::create(difficultiesBtnSprOff, difficultiesBtnSprOn, this, menu_selector(SearchPopup::onTab));
	difficultiesBtn->setContentSize({ 145.f, 25.f });
	difficultiesBtn->setPosition({ 75.f, 45.f });
	difficultiesBtn->setID("difficulty");
	difficultiesBtn->setTag(static_cast<int>(SearchModes::Difficulty));
	difficultiesBtn->toggle(true);
	tabs->addChild(difficultiesBtn);

	auto packsBtnSprOff = ButtonSprite::create("Packs", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	packsBtnSprOff->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto packsBtnSprOn = ButtonSprite::create("Packs", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	packsBtnSprOn->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto packsBtn = CCMenuItemToggler::create(packsBtnSprOff, packsBtnSprOn, this, menu_selector(SearchPopup::onTab));
	packsBtn->setContentSize({ 145.f, 25.f });
	packsBtn->setPosition({ 75.f, 15.f });
	packsBtn->setID("packs");
	packsBtn->setTag(static_cast<int>(SearchModes::Packs));
	packsBtn->toggle(false);
	tabs->addChild(packsBtn);

	auto skillsBtnSprOff = ButtonSprite::create("Skills", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	skillsBtnSprOff->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto skillsBtnSprOn = ButtonSprite::create("Skills", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	skillsBtnSprOn->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto skillsBtn = CCMenuItemToggler::create(skillsBtnSprOff, skillsBtnSprOn, this, menu_selector(SearchPopup::onTab));
	skillsBtn->setContentSize({ 145.f, 25.f });
	skillsBtn->setPosition({ 75.f, -15.f });
	skillsBtn->setID("skills");
	skillsBtn->setTag(static_cast<int>(SearchModes::Skills));
	skillsBtn->toggle(false);
	tabs->addChild(skillsBtn);

	auto xpBtnSprOff = ButtonSprite::create("XP", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	xpBtnSprOff->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto xpBtnSprOn = ButtonSprite::create("XP", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	xpBtnSprOn->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto xpBtn = CCMenuItemToggler::create(xpBtnSprOff, xpBtnSprOn, this, menu_selector(SearchPopup::onTab));
	xpBtn->setContentSize({ 145.f, 25.f });
	xpBtn->setPosition({ 75.f, -45.f });
	xpBtn->setID("xp");
	xpBtn->setTag(static_cast<int>(SearchModes::XP));
	xpBtn->toggle(false);
	tabs->addChild(xpBtn);

	tabs->setPosition({ 0.f, 125.f });
	tabs->setContentSize({ 420.f, 50.f });

	tabs->setID("search-tabs");
	layer->addChild(tabs);

	m_tabs = tabs;

	//check and uncheck buttons
	auto checkMenu = CCMenu::create();
	checkMenu->setPosition({ 75.f, 200.f });
	checkMenu->setScale(0.75f);
	checkMenu->setContentSize({ 0.f, 0.f });
	checkMenu->setID("check-menu");
	layer->addChild(checkMenu);

	auto checkAllSpr = ButtonSprite::create("[] All", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto checkAllBox = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
	checkAllBox->setScale(0.75f);
	checkAllBox->setZOrder(2);
	checkAllBox->setPosition({15.f, 18.5f});
	checkAllSpr->addChild(checkAllBox);
	auto checkAllBtn = CCMenuItemSpriteExtra::create(checkAllSpr, this, menu_selector(SearchPopup::checkAll));
	checkAllBtn->setPositionX(-50.f);
	checkAllBtn->setID("check-all-btn");

	auto uncheckAllSpr = ButtonSprite::create("[] All", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto uncheckAllBox = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
	uncheckAllBox->setScale(0.75f);
	uncheckAllBox->setZOrder(2);
	uncheckAllBox->setPosition({ 15.f, 18.5f });
	uncheckAllSpr->addChild(uncheckAllBox);
	auto uncheckAllBtn = CCMenuItemSpriteExtra::create(uncheckAllSpr, this, menu_selector(SearchPopup::uncheckAll));
	uncheckAllBtn->setPositionX(50.f);
	uncheckAllBtn->setID("uncheck-all-btn");

	checkMenu->addChild(checkAllBtn);
	checkMenu->addChild(uncheckAllBtn);

	//completed and uncompleted checks
	auto completeMenu = CCMenu::create();
	completeMenu->setPosition({ 75.f, 35.f });
	completeMenu->setScale(0.75f);
	completeMenu->setContentSize({ 0.f, 0.f });
	completeMenu->setID("complete-filter-menu");
	layer->addChild(completeMenu);

	auto completedLabel = CCLabelBMFont::create("Completed", "goldFont.fnt");
	completedLabel->setPosition({ -40.f, 25.f });
	completedLabel->setScale(0.5f);
	completedLabel->setID("complete-label");

	auto uncompletedLabel = CCLabelBMFont::create("Uncompleted", "goldFont.fnt");
	uncompletedLabel->setPosition({ 40.f, 25.f });
	uncompletedLabel->setScale(0.5f);
	uncompletedLabel->setID("uncomplete-label");
	
	CCSprite* toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
	CCSprite* toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

	auto completedToggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(SearchPopup::onToggle));
	completedToggle->toggle(m_completed);
	completedToggle->setPositionX(-40.f);
	completedToggle->setID("complete-btn");

	auto uncompletedToggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(SearchPopup::onToggle));
	uncompletedToggle->toggle(m_uncompleted);
	uncompletedToggle->setPositionX(40.f);
	uncompletedToggle->setID("uncomplete-btn");

	completeMenu->addChild(completedLabel);
	completeMenu->addChild(uncompletedLabel);
	completeMenu->addChild(completedToggle);
	completeMenu->addChild(uncompletedToggle);

	//search button
	auto searchMenu = CCMenu::create();
	searchMenu->setID("search-menu");
	searchMenu->setPosition({ 210.f, 0.f });
	layer->addChild(searchMenu);

	auto searchSpr = ButtonSprite::create("Search", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto searchBtn = CCMenuItemSpriteExtra::create(searchSpr, this, menu_selector(SearchPopup::onSearch));
	searchMenu->addChild(searchBtn);

	//load default tab
	loadTab(static_cast<int>(SearchModes::Difficulty));

	return true;
}

void SearchPopup::restoreFilters() {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto skillsets = Mod::get()->getSavedValue<matjson::Value>("skillset-info");

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return;
	}

	auto jsonCheck2 = checkJson(skillsets, "");

	if (!jsonCheck2.ok()) {
		log::info("Something went wrong validating the skillset data.");

		return;
	}

	//fix filters if they're incorrect but otherwise restore them to what they were
	m_difficulties = Mod::get()->getSavedValue<std::vector<bool>>("search-difficulty", { true });
	m_packs = Mod::get()->getSavedValue<std::vector<bool>>("search-packs", { true });
	m_skills = Mod::get()->getSavedValue<std::vector<bool>>("search-skills", { true });
	m_xp = Mod::get()->getSavedValue<std::vector<int>>("search-xp", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
	m_xpToggle = Mod::get()->getSavedValue<std::vector<bool>>("search-xp-toggle", { false, false, false, false, false, false, false, false, false, false });
	m_xpMode = Mod::get()->getSavedValue<std::vector<int>>("search-xp-modes", { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 });
	m_completed = Mod::get()->getSavedValue<bool>("search-completed", true);
	m_uncompleted = Mod::get()->getSavedValue<bool>("search-uncompleted", true);

	//check difficulties
	if (m_difficulties.size() < data["main"].asArray().unwrap().size()) {
		if (m_difficulties.empty()) { m_difficulties = { true }; }

		for (int i = m_difficulties.size(); i < data["main"].asArray().unwrap().size(); i++) {
			m_difficulties.push_back(true);
		}
	}

	//check packs
	auto totalPacks = data["main"].asArray().unwrap().size() + data["legacy"].asArray().unwrap().size() + data["bonus"].asArray().unwrap().size(); //Monthly does not count
	if (m_packs.size() < totalPacks) {
		if (m_packs.empty()) { m_packs = { true }; }

		for (int i = m_packs.size(); i < totalPacks; i++) {
			m_packs.push_back(true);
		}
	}

	//check skills
	if (m_skills.size() < skillsets.size()) {
		if (m_skills.empty()) { m_skills = { true }; }

		for (int i = m_skills.size(); i < skillsets.size(); i++) {
			m_skills.push_back(true);
		}
	}

	//all filters should be restored/fixed now, so it's time to save them again
	saveFilters();
}

void SearchPopup::saveFilters() {
	Mod::get()->setSavedValue<std::vector<bool>>("search-difficulty", m_difficulties);
	Mod::get()->setSavedValue<std::vector<bool>>("search-packs", m_packs);
	Mod::get()->setSavedValue<std::vector<bool>>("search-skills", m_skills);
	Mod::get()->setSavedValue<std::vector<int>>("search-xp", m_xp);
	Mod::get()->setSavedValue<std::vector<bool>>("search-xp-toggle", m_xpToggle);
	Mod::get()->setSavedValue<std::vector<int>>("search-xp-modes", m_xpMode);
	Mod::get()->setSavedValue<bool>("search-completed", m_completed);
	Mod::get()->setSavedValue<bool>("search-uncompleted", m_uncompleted);
}

void SearchPopup::loadTab(int id) {

	m_currentTab = id;

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto skillsets = Mod::get()->getSavedValue<matjson::Value>("skillset-info");

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return;
	}

	auto jsonCheck2 = checkJson(skillsets, "");

	if (!jsonCheck2.ok()) {
		log::info("Something went wrong validating the skillset data.");

		return;
	}

	auto filterMenu = CCMenu::create();
	filterMenu->setID("filter-menu");
	filterMenu->setContentSize({ 255.f, 180.f });
	filterMenu->setPosition({ 155.5f, 25.f });

	auto nameHeader = CCLabelBMFont::create("Name", "goldFont.fnt");
	nameHeader->setScale(0.5f);
	nameHeader->setPosition({ 16.f, 188.f });

	auto toggleHeader = CCLabelBMFont::create("Toggle", "goldFont.fnt");
	toggleHeader->setScale(0.5f);
	toggleHeader->setPosition({ 240.f, 188.f });

	filterMenu->addChild(nameHeader);
	filterMenu->addChild(toggleHeader);

	auto cells = CCArray::create();

	switch (id) {
		case static_cast<int>(SearchModes::Difficulty):
		{
			for (int i = 0; i < data["main"].asArray().unwrap().size(); i++) {
				auto packNode = CCNode::create();
				packNode->setID(fmt::format("difficulty-{}", i));
				packNode->setScale(0.75f);

				auto packData = data["main"][i];

				//sprite
				auto spriteName = fmt::format("{}.png", packData["sprite"].asString().unwrapOr("DP_Beginner"));
				CCSprite* sprite;
				if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr || spriteName == "DP_Invisible.png") {
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName("DP_Beginner.png").data());
					sprite->setVisible(false);
				}
				else {
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
				}
				sprite->setID("sprite");
				sprite->setScale(0.75f);
				sprite->setAnchorPoint({ 0.f, 0.5f });
				sprite->setPosition({ 5.f, 15.5f });

				//label
				auto label = CCLabelBMFont::create(packData["name"].asString().unwrapOr("null").c_str(), "bigFont.fnt");
				label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
				label->setID("label");
				label->setScale(0.5f);
				label->setAnchorPoint({ 0.f, 0.5f });
				label->setPosition({ 40.f, 17.5f });

				//togglebox
				auto toggleMenu = CCMenu::create();
				toggleMenu->setScale(0.75f);
				toggleMenu->setPosition({ 250.f, -25.f });
				toggleMenu->setID("toggle-menu");

				auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
				auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
				auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(SearchPopup::onToggle));
				toggle->setTag(i);
				toggle->setID("toggle");
				toggle->toggle(m_difficulties[i]);

				toggleMenu->addChild(toggle);

				//add children
				packNode->addChild(label);
				packNode->addChild(toggleMenu);
				packNode->addChild(sprite);

				cells->addObject(packNode);
			}
			break;
		}
		case static_cast<int>(SearchModes::Packs):
		{
			//main packs header
			auto mainPacksHeader = CCNode::create();
			auto mainPacksText = CCLabelBMFont::create("Main Packs", "bigFont.fnt");
			mainPacksText->setScale(0.4f);
			mainPacksText->setPosition({ filterMenu->getContentWidth() / 2.f, 25.f / 2.f });
			mainPacksHeader->addChild(mainPacksText);
			cells->addObject(mainPacksHeader);

			int offs = 0;

			//main packs
			for (int i = 0; i < data["main"].asArray().unwrap().size(); i++) {
				auto packNode = CCNode::create();
				packNode->setID(fmt::format("main-pack-{}", i));
				packNode->setScale(0.75f);

				auto packData = data["main"][i];

				//sprite
				auto spriteName = fmt::format("{}.png", packData["plusSprite"].asString().unwrapOr("DP_Beginner")); //use plus sprite for main packs, normal for legacy
				CCSprite* sprite;
				if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr || spriteName == "DP_Invisible.png") {
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName("DP_Beginner.png").data());
					sprite->setVisible(false);
				}
				else {
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
				}
				sprite->setID("sprite");
				sprite->setScale(0.75f);
				sprite->setAnchorPoint({ 0.f, 0.5f });
				sprite->setPosition({ 5.f, 15.5f });

				//label
				auto label = CCLabelBMFont::create(fmt::format("{} Demons", packData["name"].asString().unwrapOr("null")).c_str(), "bigFont.fnt");
				label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
				label->setID("label");
				label->setScale(0.5f);
				label->setAnchorPoint({ 0.f, 0.5f });
				label->setPosition({ 40.f, 17.5f });

				//togglebox
				auto toggleMenu = CCMenu::create();
				toggleMenu->setScale(0.75f);
				toggleMenu->setPosition({ 250.f, -25.f });
				toggleMenu->setID("toggle-menu");

				auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
				auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
				auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(SearchPopup::onToggle));
				toggle->setTag(i);
				toggle->setID("toggle");
				toggle->toggle(m_packs[i]);

				toggleMenu->addChild(toggle);

				//add children
				packNode->addChild(label);
				packNode->addChild(toggleMenu);
				packNode->addChild(sprite);

				cells->addObject(packNode);
			}

			offs = data["main"].asArray().unwrap().size();

			//legacy packs header
			auto legacyPacksHeader = CCNode::create();
			auto legacyPacksText = CCLabelBMFont::create("Legacy Packs", "bigFont.fnt");
			legacyPacksText->setScale(0.4f);
			legacyPacksText->setPosition({ filterMenu->getContentWidth() / 2.f, 25.f / 2.f });
			legacyPacksHeader->addChild(legacyPacksText);
			cells->addObject(legacyPacksHeader);

			//legacy packs
			for (int i = 0; i < data["legacy"].asArray().unwrap().size(); i++) {
				auto packNode = CCNode::create();
				packNode->setID(fmt::format("legacy-pack-{}", i));
				packNode->setScale(0.75f);

				auto packData = data["legacy"][i];

				//sprite
				auto spriteName = fmt::format("{}.png", packData["sprite"].asString().unwrapOr("DP_Beginner")); //use plus sprite for main packs, normal for legacy
				CCSprite* sprite;
				if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr || spriteName == "DP_Invisible.png") {
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName("DP_Beginner.png").data());
					sprite->setVisible(false);
				}
				else {
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
				}
				sprite->setID("sprite");
				sprite->setScale(0.75f);
				sprite->setAnchorPoint({ 0.f, 0.5f });
				sprite->setPosition({ 5.f, 15.5f });

				//label
				auto label = CCLabelBMFont::create(fmt::format("{} Demons", packData["name"].asString().unwrapOr("null")).c_str(), "bigFont.fnt");
				label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
				label->setID("label");
				label->setScale(0.5f);
				label->setAnchorPoint({ 0.f, 0.5f });
				label->setPosition({ 40.f, 17.5f });

				//togglebox
				auto toggleMenu = CCMenu::create();
				toggleMenu->setScale(0.75f);
				toggleMenu->setPosition({ 250.f, -25.f });
				toggleMenu->setID("toggle-menu");

				auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
				auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
				auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(SearchPopup::onToggle));
				toggle->setTag(i + offs);
				toggle->setID("toggle");
				toggle->toggle(m_packs[i + offs]);

				toggleMenu->addChild(toggle);

				//add children
				packNode->addChild(label);
				packNode->addChild(toggleMenu);
				packNode->addChild(sprite);

				cells->addObject(packNode);
			}

			offs = offs + data["legacy"].asArray().unwrap().size();

			//bonus packs header
			auto bonusPacksHeader = CCNode::create();
			auto bonusPacksText = CCLabelBMFont::create("Bonus Packs", "bigFont.fnt");
			bonusPacksText->setScale(0.4f);
			bonusPacksText->setPosition({ filterMenu->getContentWidth() / 2.f, 25.f / 2.f });
			bonusPacksHeader->addChild(bonusPacksText);
			cells->addObject(bonusPacksHeader);

			//bonus packs
			for (int i = 0; i < data["bonus"].asArray().unwrap().size(); i++) {
				auto packNode = CCNode::create();
				packNode->setID(fmt::format("bonus-pack-{}", i));
				packNode->setScale(0.75f);

				auto packData = data["bonus"][i];

				//sprite
				auto spriteName = fmt::format("{}.png", packData["sprite"].asString().unwrapOr("DP_Beginner"));
				CCSprite* sprite;
				if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr || spriteName == "DP_Invisible.png") {
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName("DP_Beginner.png").data());
					sprite->setVisible(false);
				}
				else {
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
				}
				sprite->setID("sprite");
				sprite->setScale(0.75f);
				sprite->setAnchorPoint({ 0.f, 0.5f });
				sprite->setPosition({ 5.f, 15.5f });

				//label
				auto label = CCLabelBMFont::create(packData["name"].asString().unwrapOr("null").c_str(), "bigFont.fnt");
				label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
				label->setID("label");
				label->setScale(0.5f);
				label->setAnchorPoint({ 0.f, 0.5f });
				label->setPosition({ 40.f, 17.5f });

				//togglebox
				auto toggleMenu = CCMenu::create();
				toggleMenu->setScale(0.75f);
				toggleMenu->setPosition({ 250.f, -25.f });
				toggleMenu->setID("toggle-menu");

				auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
				auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
				auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(SearchPopup::onToggle));
				toggle->setTag(i + offs);
				toggle->setID("toggle");
				toggle->toggle(m_packs[i + offs]);

				toggleMenu->addChild(toggle);

				//add children
				packNode->addChild(label);
				packNode->addChild(toggleMenu);
				packNode->addChild(sprite);

				cells->addObject(packNode);
			}
			break;
		}
		case static_cast<int>(SearchModes::Skills):
		{
			std::vector<std::string> skillsetTypes = {"none", "gamemode", "misc", "platformer", "special"};

			int i = 0;
			for (auto type : skillsetTypes)
			{
				for (auto [key, value] : skillsets) {
					auto skillNode = CCNode::create();
					skillNode->setID(fmt::format("skill-{}", key));
					skillNode->setScale(0.75f);

					//log::info("i: {}", i);
					//log::info("key: {}", key);
					//log::info("value: {}", value);

					auto skillData = value;

					if (skillData["type"].asString().unwrapOr("none") != type) {
						continue;
					}

					//sprite
					auto spriteName = fmt::format("{}.png", skillData["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
					CCSprite* sprite;
					if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr) {
						spriteName = fmt::format("{}.png", skillsets["unknown"]["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
						sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
					}
					else {
						sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
					}
					sprite->setID("sprite");
					sprite->setScale(0.75f);
					sprite->setAnchorPoint({ 0.f, 0.5f });
					sprite->setPosition({ 5.f, 17.5f });

					//label
					auto label = CCLabelBMFont::create(skillData["display-name"].asString().unwrapOr("null").c_str(), "bigFont.fnt");
					label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
					label->setID("label");
					label->setScale(0.5f);
					label->setAnchorPoint({ 0.f, 0.5f });
					label->setPosition({ 30.f, 17.5f });

					//togglebox
					auto toggleMenu = CCMenu::create();
					toggleMenu->setScale(0.75f);
					toggleMenu->setPosition({ 250.f, -25.f });
					toggleMenu->setID("toggle-menu");

					auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
					auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
					auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(SearchPopup::onToggle));
					toggle->setTag(i);
					toggle->setID("toggle");
					toggle->toggle(m_skills[i]);

					toggleMenu->addChild(toggle);

					//add children
					skillNode->addChild(sprite);
					skillNode->addChild(label);
					skillNode->addChild(toggleMenu);

					cells->addObject(skillNode);

					i += 1;
				}
			}
			break;
		}
		case static_cast<int>(SearchModes::XP):
		{
			auto modeHeader = CCLabelBMFont::create("Mode", "goldFont.fnt");
			modeHeader->setScale(0.5f);
			modeHeader->setPosition({ 130.f, 188.f });
			
			auto valueHeader = CCLabelBMFont::create("Value", "goldFont.fnt");
			valueHeader->setScale(0.5f);
			valueHeader->setPosition({ 190.f, 188.f });
			
			filterMenu->addChild(modeHeader);
			filterMenu->addChild(valueHeader);
			
			for (int i = 0; i < XPUtils::skillIDs.size(); i++) {
				auto xpBox = CCNode::create();
				xpBox->setID(fmt::format("xp-{}", XPUtils::skillIDs[i]));
				xpBox->setScale(0.75f);

				//label
				auto label = CCLabelBMFont::create(skillNames[i].c_str(), "bigFont.fnt");
				label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
				label->setColor(skillColors[i]);
				label->setID("label");
				label->setScale(0.5f);
				label->setAnchorPoint({ 0.f, 0.5f });
				label->setPosition({ 5.f, 17.5f });

				//togglebox
				auto toggleMenu = CCMenu::create();
				toggleMenu->setScale(0.75f);
				toggleMenu->setPosition({ 250.f, -25.f });
				toggleMenu->setID("toggle-menu");

				auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
				auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
				auto toggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(SearchPopup::onToggle));
				toggle->setTag(i);
				toggle->setID("toggle");
				toggle->toggle(m_xpToggle[i]);
				
				toggleMenu->addChild(toggle);

				//value changer
				auto valueMenu = CCMenu::create();
				valueMenu->setID("value-menu");
				valueMenu->setScale(0.6f);
				valueMenu->setPosition({ 140.f, -48.f });

				auto valLeftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
				auto valLeft = CCMenuItemSpriteExtra::create(valLeftSpr, this, menu_selector(SearchPopup::onXpValue));
				valLeft->setTag(i);
				valLeft->setID("value-left");
				valLeft->setVisible(m_xp[i] > 0);
				valLeft->setPositionX(-60.f);
				valueMenu->addChild(valLeft);

				auto valRightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
				valRightSpr->setFlipX(true);
				auto valRight = CCMenuItemSpriteExtra::create(valRightSpr, this, menu_selector(SearchPopup::onXpValue));
				valRight->setTag(i);
				valRight->setID("value-right");
				valRight->setVisible(m_xp[i] < 3);
				valRight->setPositionX(60.f);
				valueMenu->addChild(valRight);

				std::vector<std::string> valueStrings = { "None", "Low", "Avg", "Max" };

				auto valLabel = CCLabelBMFont::create(valueStrings[m_xp[i]].c_str(), "bigFont.fnt");
				valLabel->setID("value-label");
				valLabel->setPositionY(2.f);
				valueMenu->addChild(valLabel);

				//value mode
				auto modeMenu = CCMenu::create();
				modeMenu->setID("mode-menu");
				modeMenu->setScale(0.6f);
				modeMenu->setPosition({ 60.f, -48.f });

				auto modeLeftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
				auto modeLeft = CCMenuItemSpriteExtra::create(modeLeftSpr, this, menu_selector(SearchPopup::onXpMode));
				modeLeft->setTag(i);
				modeLeft->setID("mode-left");
				modeLeft->setVisible(m_xpMode[i] > 0);
				modeLeft->setPositionX(-30.f);
				modeMenu->addChild(modeLeft);

				auto modeRightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
				modeRightSpr->setFlipX(true);
				auto modeRight = CCMenuItemSpriteExtra::create(valRightSpr, this, menu_selector(SearchPopup::onXpMode));
				modeRight->setTag(i);
				modeRight->setID("mode-right");
				modeRight->setVisible(m_xpMode[i] < 4);
				modeRight->setPositionX(30.f);
				modeMenu->addChild(modeRight);

				std::vector<std::string> modeStrings = {">=", ">", "=", "<", "<="};

				auto modeLabel = CCLabelBMFont::create(modeStrings[m_xpMode[i]].c_str(), "bigFont.fnt");
				modeLabel->setID("mode-label");
				modeLabel->setPositionY(2.f);
				modeMenu->addChild(modeLabel);

				//add children
				xpBox->addChild(label);
				xpBox->addChild(toggleMenu);
				xpBox->addChild(valueMenu);
				xpBox->addChild(modeMenu);

				cells->addObject(xpBox);
			}
			break;
		}
	}

	m_list = ListView::create(cells, 25.f, 255.f, 180.f);
	filterMenu->addChild(m_list);

	m_mainLayer->addChild(filterMenu);

	return;
}

void SearchPopup::onTab(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto menuType = btn->getTag();

	auto difficultybtn = m_tabs->getChildByID("difficulty");
	auto packsbtn = m_tabs->getChildByID("packs");
	auto skillsbtn = m_tabs->getChildByID("skills");
	auto xpbtn = m_tabs->getChildByID("xp");

	if (btn->isToggled()) {
		btn->toggle(false);
	}

	if (m_mainLayer) {
		m_mainLayer->removeAllChildrenWithCleanup(true);
	}

	if (menuType == static_cast<int>(SearchModes::Difficulty)) {

		static_cast<CCMenuItemToggler*>(packsbtn)->toggle(false);
		static_cast<CCMenuItemToggler*>(skillsbtn)->toggle(false);
		static_cast<CCMenuItemToggler*>(xpbtn)->toggle(false);

		loadTab(static_cast<int>(SearchModes::Difficulty));
	}
	else if (menuType == static_cast<int>(SearchModes::Packs)) {

		static_cast<CCMenuItemToggler*>(difficultybtn)->toggle(false);
		static_cast<CCMenuItemToggler*>(skillsbtn)->toggle(false);
		static_cast<CCMenuItemToggler*>(xpbtn)->toggle(false);

		loadTab(static_cast<int>(SearchModes::Packs));
	}
	else if (menuType == static_cast<int>(SearchModes::Skills)) {

		static_cast<CCMenuItemToggler*>(difficultybtn)->toggle(false);
		static_cast<CCMenuItemToggler*>(packsbtn)->toggle(false);
		static_cast<CCMenuItemToggler*>(xpbtn)->toggle(false);

		loadTab(static_cast<int>(SearchModes::Skills));
	}
	else if (menuType == static_cast<int>(SearchModes::XP)) {

		static_cast<CCMenuItemToggler*>(difficultybtn)->toggle(false);
		static_cast<CCMenuItemToggler*>(packsbtn)->toggle(false);
		static_cast<CCMenuItemToggler*>(skillsbtn)->toggle(false);

		loadTab(static_cast<int>(SearchModes::XP));
	}

	return;
}

void SearchPopup::onSearch(CCObject* sender) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto skillsets = Mod::get()->getSavedValue<matjson::Value>("skillset-info");
	
	std::vector<int> selectedLevels = {};

	//get all packs
	auto mainPackSize = data["main"].asArray().unwrap().size();
	auto legacyPackSize = data["legacy"].asArray().unwrap().size();
	auto bonusPackSize = data["bonus"].asArray().unwrap().size();
	std::vector<matjson::Value> packs(mainPackSize + legacyPackSize + bonusPackSize, data["main"][0]);

	auto offs = 0;

	for (int i = 0; i < mainPackSize; i++) {
		packs[i + offs] = data["main"][i];
	}

	offs = mainPackSize;

	for (int i = 0; i < legacyPackSize; i++) {
		packs[i + offs] = data["legacy"][i];
	}

	offs += legacyPackSize;

	for (int i = 0; i < bonusPackSize; i++) {
		packs[i + offs] = data["bonus"][i];
	}

	//iterate through packs
	for (int i = 0; i < packs.size(); i++) {

		//if pack isn't checked, skip
		if (!m_packs[i]) { continue; }

		//iterate through levels
		auto levels = packs[i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
		for (auto lvlID : levels) {
			auto lvlData = data["level-data"][std::to_string(lvlID)];

			//check if difficulty is ok
			if (!m_difficulties[lvlData["difficulty"].as<int>().unwrapOr(0)]) { continue; }

			//check if skillsets are ok
			auto skillsetsOk = false;
			int j = 0;
			auto lvlSkillsets = lvlData["skillsets"].as<std::vector<std::string>>().unwrapOrDefault();
			for (auto& [key, value] : skillsets) {
				
				for (auto skillsetID : lvlSkillsets) {
					if (key == skillsetID && m_skills[j]) {
						skillsetsOk = true;
					}
				}

				j += 1;
			}
			if (!skillsetsOk) { continue; }

			//check if xp satisfies filter
			// 1. if at least one xp filter is enabled, skip any levels with no xp values
			// 2. completely skip any disabled filter
			// 3. check if filters are satisfied, if not, don't accept level
			auto xpOk = true;
			for (int skill = 0; skill < XPUtils::skillIDs.size(); skill++) {

				if (m_xpToggle.size() < XPUtils::skillIDs.size()) { m_xpToggle.push_back(false); }
				if (m_xpMode.size() < XPUtils::skillIDs.size()) { m_xpMode.push_back(2); }
				if (m_xp.size() < XPUtils::skillIDs.size()) { m_xp.push_back(0); }

				if (m_xpToggle[skill]) {
					if (!lvlData["xp"][XPUtils::skillIDs[skill]].isNumber()) { 
						xpOk = false; 
						break;
					}
					else {
						auto xpValue = lvlData["xp"][XPUtils::skillIDs[skill]].as<int>().unwrapOr(0);
            			switch (m_xpMode[skill]) {
                			case 0: // >=
                    			if (!(xpValue >= m_xp[skill])) {
                        			xpOk = false;
                        			break;
                    			}
                    			break;
                			case 1: // >
                    			if (!(xpValue > m_xp[skill])) {
                        			xpOk = false;
                        			break;
                    			}
                    			break;
                			case 2: // ==
                    			if (!(xpValue == m_xp[skill])) {
                        			xpOk = false;
                        			break;
                    			}
                    			break;
                			case 3: // <
                    			if (!(xpValue < m_xp[skill])) {
                       				xpOk = false;
                        			break;
                    			}
                    			break;
                			case 4: // <=
                    			if (!(xpValue <= m_xp[skill])) {
                        			xpOk = false;
                        			break;
                    			}
                    			break;
            			}
					}
				}
			}
			if (!xpOk) { continue; }

			//check if completed/uncompleted and completed/uncompleted filter is on
			auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
			if (!(m_completed == m_uncompleted)) {
				if (m_uncompleted && std::find(completedLvls.begin(), completedLvls.end(), lvlID) != completedLvls.end()) {
					continue;
				}
				else if (m_completed && std::find(completedLvls.begin(), completedLvls.end(), lvlID) == completedLvls.end()) {
					continue;
				}
			}

			//check if level is already in the list
			if (std::find(selectedLevels.begin(), selectedLevels.end(), lvlID) != selectedLevels.end()) { continue; }

			//if all checks are ok, add it to the list
			selectedLevels.push_back(lvlID);
		}
	}

	//create DPSearchLayer with results
	auto scene = CCScene::create(); // creates the scene
	auto dpLayer = DPSearchLayer::create(selectedLevels); //creates the layer

	scene->addChild(dpLayer);

	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition

	return;
}
void SearchPopup::onToggle(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemToggler*>(sender);
	auto id = btn->getID();
	auto tag = btn->getTag();

	if (id == "complete-btn") {
		m_completed = !m_completed;
	}
	else if (id == "uncomplete-btn") {
		m_uncompleted = !m_uncompleted;
	}
	else {
		switch (m_currentTab) {
    		case static_cast<int>(SearchModes::Difficulty):
        		m_difficulties[tag] = !m_difficulties[tag];
        		break;
    		case static_cast<int>(SearchModes::Packs):
        		m_packs[tag] = !m_packs[tag];
        		break;
    		case static_cast<int>(SearchModes::Skills):
        		m_skills[tag] = !m_skills[tag];
        		break;
    		case static_cast<int>(SearchModes::XP):
        		m_xpToggle[tag] = !m_xpToggle[tag];
        		break;
		}
	}

	//save filters
	saveFilters();

	return;
}
void SearchPopup::onXpValue(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto id = btn->getID();
	auto tag = btn->getTag();

	if (id == "value-left" && m_xp[tag] > 0) {
		m_xp[tag] = m_xp[tag] - 1;
	}
	else if (id == "value-right" && m_xp[tag] < 3) {
		m_xp[tag] = m_xp[tag] + 1;
	}

	//update nodes
	auto menu = btn->getParent();
	auto left = menu->getChildByID("value-left");
	auto right = menu->getChildByID("value-right");
	auto label = typeinfo_cast<CCLabelBMFont*>(menu->getChildByID("value-label"));

	std::vector<std::string> valueStrings = { "None", "Low", "Avg", "Max" };

	left->setVisible(m_xp[tag] > 0);
	right->setVisible(m_xp[tag] < 3);
	label->setCString(valueStrings[m_xp[tag]].c_str());
	
	//save filters
	saveFilters();

	return;
}
void SearchPopup::onXpMode(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto id = btn->getID();
	auto tag = btn->getTag();

	if (id == "mode-left" && m_xpMode[tag] > 0) {
		m_xpMode[tag] = m_xpMode[tag] - 1;
	}
	else if (id == "mode-right" && m_xpMode[tag] < 4) {
		m_xpMode[tag] = m_xpMode[tag] + 1;
	}

	//update nodes
	auto menu = btn->getParent();
	auto left = menu->getChildByID("mode-left");
	auto right = menu->getChildByID("mode-right");
	auto label = typeinfo_cast<CCLabelBMFont*>(menu->getChildByID("mode-label"));

	std::vector<std::string> modeStrings = { ">=", ">", "=", "<", "<=" };

	left->setVisible(m_xpMode[tag] > 0);
	right->setVisible(m_xpMode[tag] < 4);
	label->setCString(modeStrings[m_xpMode[tag]].c_str());

	//save filters
	saveFilters();

	return;
}
void SearchPopup::checkAll(CCObject* sender) {
	if (m_mainLayer) {
		m_mainLayer->removeAllChildrenWithCleanup(true);
	}

	switch (m_currentTab) {
    	case static_cast<int>(SearchModes::Difficulty):
        	for (int i = 0; i < m_difficulties.size(); i++) {
				m_difficulties[i] = true;
			}
        	break;
   		case static_cast<int>(SearchModes::Packs):
        	for (int i = 0; i < m_packs.size(); i++) {
				m_packs[i] = true;
			}
        	break;
    	case static_cast<int>(SearchModes::Skills):
        	for (int i = 0; i < m_skills.size(); i++) {
				m_skills[i] = true;
			}
        	break;
    	case static_cast<int>(SearchModes::XP):
        	for (int i = 0; i < m_xpToggle.size(); i++) {
				m_xpToggle[i] = true;
			}
        	break;
	}

	//save filters
	saveFilters();

	//reload tab
	loadTab(m_currentTab);

	return;
}
void SearchPopup::uncheckAll(CCObject* sender) {
	if (m_mainLayer) {
		m_mainLayer->removeAllChildrenWithCleanup(true);
	}
	
	switch (m_currentTab) {
    	case static_cast<int>(SearchModes::Difficulty):
        	for (int i = 0; i < m_difficulties.size(); i++) {
				m_difficulties[i] = false;
			}
        	break;
   		case static_cast<int>(SearchModes::Packs):
        	for (int i = 0; i < m_packs.size(); i++) {
				m_packs[i] = false;
			}
        	break;
    	case static_cast<int>(SearchModes::Skills):
        	for (int i = 0; i < m_skills.size(); i++) {
				m_skills[i] = false;
			}
        	break;
    	case static_cast<int>(SearchModes::XP):
        	for (int i = 0; i < m_xpToggle.size(); i++) {
				m_xpToggle[i] = false;
			}
        	break;
	}

	//save filters
	saveFilters();

	//reload tab
	loadTab(m_currentTab);
	
	return;
}

SearchPopup* SearchPopup::create() {
	auto ret = new SearchPopup();
	if (ret && ret->initAnchored(420.f, 250.f)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

SearchPopup::~SearchPopup() {
	this->removeAllChildrenWithCleanup(true);
}