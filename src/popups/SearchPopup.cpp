//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/JsonValidation.hpp>

#include "../menus/DPLayer.hpp"
#include "../XPUtils.hpp"
#include "SearchPopup.hpp"
#include "../menus/DPSearchLayer.hpp"
#include "../DPUtils.hpp"

//geode namespace
using namespace geode::prelude;

bool SearchPopup::init() {
	if (!Popup::init(420.f, 250.f)) return false;
	this->setTitle("Search");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	//restore filters
	m_filter = Mod::get()->getSavedValue<matjson::Value>("search-filter", m_filter);

	//create main layer
	m_mainLayer = CCLayer::create();
	m_mainLayer->setID("main-layer");

	layer->addChild(m_mainLayer);

	//create tabs
	auto m_tabs = CCMenu::create();

	auto difficultiesBtnSprOff = ButtonSprite::create("Difficulties", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	difficultiesBtnSprOff->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto difficultiesBtnSprOn = ButtonSprite::create("Difficulties", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	difficultiesBtnSprOn->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto difficultiesBtn = CCMenuItemToggler::create(difficultiesBtnSprOff, difficultiesBtnSprOn, this, menu_selector(SearchPopup::onTab));
	difficultiesBtn->setContentSize({ 145.f, 25.f });
	difficultiesBtn->setPosition({ 75.f, 45.f });
	difficultiesBtn->setID("difficulty");
	difficultiesBtn->setTag((int)SearchModes::Difficulty);
	difficultiesBtn->toggle(true);
	m_tabs->addChild(difficultiesBtn);
	m_tabBtns.push_back(difficultiesBtn);

	auto packsBtnSprOff = ButtonSprite::create("Packs", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	packsBtnSprOff->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto packsBtnSprOn = ButtonSprite::create("Packs", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	packsBtnSprOn->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto packsBtn = CCMenuItemToggler::create(packsBtnSprOff, packsBtnSprOn, this, menu_selector(SearchPopup::onTab));
	packsBtn->setContentSize({ 145.f, 25.f });
	packsBtn->setPosition({ 75.f, 15.f });
	packsBtn->setID("packs");
	packsBtn->setTag((int)SearchModes::Packs);
	packsBtn->toggle(false);
	m_tabs->addChild(packsBtn);
	m_tabBtns.push_back(packsBtn);

	auto skillsBtnSprOff = ButtonSprite::create("Skills", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	skillsBtnSprOff->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto skillsBtnSprOn = ButtonSprite::create("Skills", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	skillsBtnSprOn->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto skillsBtn = CCMenuItemToggler::create(skillsBtnSprOff, skillsBtnSprOn, this, menu_selector(SearchPopup::onTab));
	skillsBtn->setContentSize({ 145.f, 25.f });
	skillsBtn->setPosition({ 75.f, -15.f });
	skillsBtn->setID("skills");
	skillsBtn->setTag((int)SearchModes::Skills);
	skillsBtn->toggle(false);
	m_tabs->addChild(skillsBtn);
	m_tabBtns.push_back(skillsBtn);

	auto xpBtnSprOff = ButtonSprite::create("XP", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	xpBtnSprOff->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto xpBtnSprOn = ButtonSprite::create("XP", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	xpBtnSprOn->m_BGSprite->setContentSize({ 145.f, 25.f });

	auto xpBtn = CCMenuItemToggler::create(xpBtnSprOff, xpBtnSprOn, this, menu_selector(SearchPopup::onTab));
	xpBtn->setContentSize({ 145.f, 25.f });
	xpBtn->setPosition({ 75.f, -45.f });
	xpBtn->setID("xp");
	xpBtn->setTag((int)SearchModes::XP);
	xpBtn->toggle(false);
	m_tabs->addChild(xpBtn);
	m_tabBtns.push_back(xpBtn);

	m_tabs->setPosition({ 0.f, 125.f });
	m_tabs->setContentSize({ 420.f, 50.f });

	m_tabs->setID("search-tabs");
	layer->addChild(m_tabs);

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
	completedToggle->toggle(m_filter["completed"].as<bool>().unwrapOr(true));
	completedToggle->setPositionX(-40.f);
	completedToggle->setID("complete-btn");
	completedToggle->setUserObject(new SetToggleValue("completed"));

	auto uncompletedToggle = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(SearchPopup::onToggle));
	uncompletedToggle->toggle(m_filter["uncompleted"].as<bool>().unwrapOr(false));
	uncompletedToggle->setPositionX(40.f);
	uncompletedToggle->setID("uncomplete-btn");
	uncompletedToggle->setUserObject(new SetToggleValue("uncompleted"));

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

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return;
	}

	//fix filters if they're incorrect but otherwise restore them to what they were
	m_filter = Mod::get()->getSavedValue<matjson::Value>("search-filter", m_filter);

	//all filters should be restored/fixed now, so it's time to save them again
	saveFilters();
}

void SearchPopup::saveFilters() {
	Mod::get()->setSavedValue<matjson::Value>("search-filter", m_filter);
}

void SearchPopup::loadTab(int id) {

	m_currentTab = id;

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

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
		case (int)SearchModes::Difficulty:
		{
			for (auto pack : data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto sprite = fmt::format("{}.png", pack["sprite"].asString().unwrapOr("DP_Unknown"));
				auto name = pack["name"].asString().unwrapOr("null");

				auto filterToggle = m_filter["difficulties"][saveID].asBool().unwrapOr(true);
				
				auto packNode = CCNode::create();
				packNode->setID(fmt::format("difficulty-{}", saveID));
				packNode->setScale(0.75f);

				//sprite
				auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);
				CCSprite* spr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(sprite).data(), unkSpr);
				if (sprite == "DP_Invisible.png") spr->setVisible(false);
				spr->setID("sprite");
				spr->setScale(0.75f);
				spr->setAnchorPoint({ 0.f, 0.5f });
				spr->setPosition({ 5.f, 15.5f });

				//label
				auto label = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
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
				toggle->setID("toggle");
				toggle->toggle(filterToggle);
				toggle->setUserObject(new SetToggleValue(saveID));

				toggleMenu->addChild(toggle);

				//add children
				packNode->addChild(label);
				packNode->addChild(toggleMenu);
				packNode->addChild(spr);

				cells->addObject(packNode);
			}
			break;
		}
		case (int)SearchModes::Packs:
		{
			std::vector<std::string> headerTitles = {"Main", "Legacy", "Bonus"};
			std::vector<std::string> indexIDs = {"main", "legacy", "bonus"};

			for (int i = 0; i < indexIDs.size(); i++) {
				auto index = indexIDs[i];

				// packs header
				auto packsHeader = CCNode::create();
				auto packsText = CCLabelBMFont::create(fmt::format("{} Packs", headerTitles[i]).c_str(), "bigFont.fnt");
				packsText->setScale(0.4f);
				packsText->setPosition({ filterMenu->getContentWidth() / 2.f, 25.f / 2.f });
				packsHeader->addChild(packsText);
				cells->addObject(packsHeader);

				// packs
				for (auto pack : data[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
					auto saveID = pack["saveID"].asString().unwrapOr("null");
					auto sprite = fmt::format("{}.png", pack[(index == "main") ? "plusSprite" : "sprite"].asString().unwrapOr("DP_Unknown"));
					auto name = pack["name"].asString().unwrapOr("null");

					auto filterToggle = m_filter["packs"][saveID].asBool().unwrapOr(true);
					
					auto packNode = CCNode::create();
					packNode->setID(fmt::format("{}-pack-{}", index, saveID));
					packNode->setScale(0.75f);

					//sprite
					auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);
					CCSprite* spr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(sprite).data(), unkSpr);
					if (sprite == "DP_Invisible.png") spr->setVisible(false);
					spr->setID("sprite");
					spr->setScale(0.75f);
					spr->setAnchorPoint({ 0.f, 0.5f });
					spr->setPosition({ 5.f, 15.5f });

					//label
					auto label = CCLabelBMFont::create((index == "bonus") ? name.c_str() : fmt::format("{} Demons", name).c_str(), "bigFont.fnt");
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
					toggle->setID("toggle");
					toggle->toggle(filterToggle);
					toggle->setUserObject(new SetToggleValue(saveID));

					toggleMenu->addChild(toggle);

					//add children
					packNode->addChild(label);
					packNode->addChild(toggleMenu);
					packNode->addChild(spr);

					cells->addObject(packNode);
				}
			}
			break;
		}
		case (int)SearchModes::Skills:
		{
			std::vector<std::string> skillsetTypes = {"none", "gamemode", "misc", "platformer", "special"};

			int i = 0;
			for (auto skillType : skillsetTypes)
			{
				for (auto [key, value] : data["skillset-data"]) {
					auto skillNode = CCNode::create();
					skillNode->setID(fmt::format("skill-{}", key));
					skillNode->setScale(0.75f);

					auto type = value["type"].asString().unwrapOr("none");
					auto sprite = value["sprite"].asString().unwrapOr("DP_Skill_Unknown");
					auto name = value["display-name"].asString().unwrapOr("Unknown");
					auto filterToggle = m_filter["skills"][key].asBool().unwrapOr(true);

					if (type != skillType) continue;

					//sprite
					auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Skill_Unknown.png"_spr);
					auto spSpr = CCSprite::createWithSpriteFrameName("DP_Skill_Special.png"_spr);
					CCSprite* spr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", sprite)).data(), (type == "special") ? spSpr : unkSpr);
					spr->setID("sprite");
					spr->setScale(0.75f);
					spr->setAnchorPoint({ 0.f, 0.5f });
					spr->setPosition({ 5.f, 17.5f });

					//label
					auto label = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
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
					toggle->setID("toggle");
					toggle->toggle(filterToggle);
					toggle->setUserObject(new SetToggleValue(key));

					toggleMenu->addChild(toggle);

					//add children
					skillNode->addChild(spr);
					skillNode->addChild(label);
					skillNode->addChild(toggleMenu);

					cells->addObject(skillNode);
				}
			}
			break;
		}
		case (int)SearchModes::XP:
		{
			auto modeHeader = CCLabelBMFont::create("Mode", "goldFont.fnt");
			modeHeader->setScale(0.5f);
			modeHeader->setPosition({ 130.f, 188.f });
			
			auto valueHeader = CCLabelBMFont::create("Value", "goldFont.fnt");
			valueHeader->setScale(0.5f);
			valueHeader->setPosition({ 190.f, 188.f });
			
			filterMenu->addChild(modeHeader);
			filterMenu->addChild(valueHeader);
			
			for (auto [key, value] : XPUtils::skills) {
				auto name = value["name"].asString().unwrapOr("???");
				auto color = value["color"].as<ccColor3B>().unwrapOrDefault();
				auto filterToggle = m_filter["xp"][key]["toggled"].asBool().unwrapOr(false);
				auto filterValue = m_filter["xp"][key]["value"].as<int>().unwrapOr(0);
				auto filterMode = m_filter["xp"][key]["mode"].as<int>().unwrapOr(2);
				
				auto xpBox = CCNode::create();
				xpBox->setID(fmt::format("xp-{}", key));
				xpBox->setScale(0.75f);

				//label
				auto label = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
				label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
				label->setColor(color);
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
				toggle->setID("toggle");
				toggle->toggle(filterToggle);
				toggle->setUserObject(new SetToggleValue(key));
				
				toggleMenu->addChild(toggle);

				//value changer
				auto valueMenu = CCMenu::create();
				valueMenu->setID("value-menu");
				valueMenu->setScale(0.6f);
				valueMenu->setPosition({ 140.f, -48.f });

				auto valLeftSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
				auto valLeft = CCMenuItemSpriteExtra::create(valLeftSpr, this, menu_selector(SearchPopup::onXpValue));
				valLeft->setID("value-left");
				valLeft->setVisible(filterValue > 0);
				valLeft->setUserObject(new SetXPValue(key,  -1));
				valLeft->setPositionX(-60.f);
				valueMenu->addChild(valLeft);

				auto valRightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
				valRightSpr->setFlipX(true);
				auto valRight = CCMenuItemSpriteExtra::create(valRightSpr, this, menu_selector(SearchPopup::onXpValue));
				valRight->setID("value-right");
				valRight->setVisible(filterValue < 3);
				valRight->setUserObject(new SetXPValue(key, 1));
				valRight->setPositionX(60.f);
				valueMenu->addChild(valRight);

				std::vector<std::string> valueStrings = { "None", "Low", "Avg", "Max" };

				auto valLabel = CCLabelBMFont::create(valueStrings[filterValue].c_str(), "bigFont.fnt");
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
				modeLeft->setID("mode-left");
				modeLeft->setVisible(filterMode > 0);
				modeLeft->setUserObject(new SetXPValue(key, -1));
				modeLeft->setPositionX(-30.f);
				modeMenu->addChild(modeLeft);

				auto modeRightSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
				modeRightSpr->setFlipX(true);
				auto modeRight = CCMenuItemSpriteExtra::create(modeRightSpr, this, menu_selector(SearchPopup::onXpMode));
				modeRight->setID("mode-right");
				modeRight->setVisible(filterMode < 4);
				modeRight->setUserObject(new SetXPValue(key, 1));
				modeRight->setPositionX(30.f);
				modeMenu->addChild(modeRight);

				std::vector<std::string> modeStrings = {">=", ">", "=", "<", "<="};

				auto modeLabel = CCLabelBMFont::create(modeStrings[filterMode].c_str(), "bigFont.fnt");
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

	for (auto b : m_tabBtns) if (b) static_cast<CCMenuItemToggler*>(b)->toggle(false);
	btn->m_toggled = false;

	if (m_mainLayer) m_mainLayer->removeAllChildrenWithCleanup(true);
	
	loadTab(menuType);

	return;
}

void SearchPopup::onSearch(CCObject* sender) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	
	std::vector<int> selectedLevels = {};

	auto completed = m_filter["completed"].asBool().unwrapOr(true);
	auto uncompleted = m_filter["uncompleted"].asBool().unwrapOr(true);

	// get difficulties
	std::vector<bool> difficulties = {};
	for (auto [key, value] : m_filter["difficulties"]) difficulties.push_back(value.asBool().unwrapOr(true));

	// get all packs
	matjson::Value packs;
	for (auto p : data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) packs.set(p["saveID"].asString().unwrapOr("null"), p);
	for (auto p : data["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) packs.set(p["saveID"].asString().unwrapOr("null"), p);
	for (auto p : data["bonus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) packs.set(p["saveID"].asString().unwrapOr("null"), p);

	// iterate through packs
	for (auto [key, value] : m_filter["packs"]) {

		// if pack isn't checked, skip
		if (!value.asBool().unwrapOr(true)) continue; 

		auto pack = packs[key];

		// iterate through levels
		auto levels = pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
		for (auto lvlID : levels) {
			auto lvlData = data["level-data"][std::to_string(lvlID)];

			// check if difficulty is ok
			if (!difficulties[lvlData["difficulty"].as<int>().unwrapOr(0)]) continue;

			// check if skillsets are ok
			auto skillsetsOk = false;
			auto lvlSkillsets = lvlData["skillsets"].as<std::vector<std::string>>().unwrapOrDefault();
			for (auto [key, value] : data["skillset-data"]) {
				if (skillsetsOk) break;
				for (auto skillsetID : lvlSkillsets) {
					if (key == skillsetID && m_filter["skills"][key].asBool().unwrapOr(true)) {
						skillsetsOk = true; 
						break;
					}
				}
			}
			if (!skillsetsOk) { continue; }

			//check if xp satisfies filter
			// 1. if at least one xp filter is enabled, skip any levels with no xp values
			// 2. completely skip any disabled filter
			// 3. check if filters are satisfied, if not, don't accept level
			auto xpOk = true;
			for (auto [key, value] : XPUtils::skills) {

				auto toggled = m_filter["xp"][key]["toggled"].asBool().unwrapOr(false);
				auto mode = m_filter["xp"][key]["mode"].as<int>().unwrapOr(2);
				auto val = m_filter["xp"][key]["value"].as<int>().unwrapOr(0);

				if (toggled) {
					if (!lvlData.contains("xp")) { 
						xpOk = false; 
						break;
					}
					
					auto xpValue = lvlData["xp"][key].as<int>().unwrapOr(0);
            		switch (mode) {
                		case 0: // >=
                    		if (!(xpValue >= val)) {
                        		xpOk = false;
                        		break;
                    		}
                    		break;
                		case 1: // >
                    		if (!(xpValue > val)) {
                        		xpOk = false;
                        		break;
                    		}
                    		break;
                		case 2: // ==
                    		if (!(xpValue == val)) {
                     			xpOk = false;
                     			break;
                    		}
                    		break;
                		case 3: // <
                    		if (!(xpValue < val)) {
                    				xpOk = false;
                     			break;
                    		}
                    		break;
                		case 4: // <=
                    		if (!(xpValue <= val)) {
                     			xpOk = false;
                     			break;
                    		}
                    		break;
            		}
				}
			}
			if (!xpOk) continue;

			//check if completed/uncompleted and completed/uncompleted filter is on
			auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
			if (!(completed == uncompleted)) {
				if (uncompleted && DPUtils::containsInt(completedLvls, lvlID)) continue;
				else if (completed && !DPUtils::containsInt(completedLvls, lvlID)) continue;
			}

			//check if level is already in the list
			if (DPUtils::containsInt(selectedLevels, lvlID)) continue;

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
	auto parameters = static_cast<SetToggleValue*>(static_cast<CCNode*>(sender)->getUserObject());

	if (parameters->m_id == "completed") m_filter.set("completed", !btn->isToggled());
	else if (parameters->m_id == "uncompleted") m_filter.set("uncompleted", !btn->isToggled());
	else {
		switch (m_currentTab) {
    		case static_cast<int>(SearchModes::Difficulty):
        		m_filter["difficulties"].set(parameters->m_id, !btn->isToggled());
        		break;
    		case static_cast<int>(SearchModes::Packs):
        		m_filter["packs"].set(parameters->m_id, !btn->isToggled());
        		break;
    		case static_cast<int>(SearchModes::Skills):
        		m_filter["skills"].set(parameters->m_id, !btn->isToggled());
        		break;
    		case static_cast<int>(SearchModes::XP):
        		m_filter["xp"][parameters->m_id].set("toggled", !btn->isToggled());
        		break;
		}
	}

	//save filters
	Mod::get()->setSavedValue<matjson::Value>("search-filter", m_filter);

	return;
}

void SearchPopup::onXpValue(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto parameters = static_cast<SetXPValue*>(static_cast<CCNode*>(sender)->getUserObject());

	auto id = parameters->m_id;
	auto oldValue = m_filter["xp"][id]["value"].as<int>().unwrapOr(0);
	auto newValue = oldValue + parameters->m_value;
	m_filter["xp"][id].set("value", newValue);

	log::info("{}: {}", id, newValue);

	//update nodes
	auto menu = btn->getParent();
	auto left = menu->getChildByID("value-left");
	auto right = menu->getChildByID("value-right");
	auto label = typeinfo_cast<CCLabelBMFont*>(menu->getChildByID("value-label"));

	std::vector<std::string> valueStrings = { "None", "Low", "Avg", "Max" };

	left->setVisible(newValue > 0);
	right->setVisible(newValue < 3);
	label->setCString(valueStrings[newValue].c_str());
	
	//save filters
	Mod::get()->setSavedValue<matjson::Value>("search-filter", m_filter);

	return;
}
void SearchPopup::onXpMode(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto parameters = static_cast<SetXPValue*>(static_cast<CCNode*>(sender)->getUserObject());

	auto id = parameters->m_id;
	auto oldValue = m_filter["xp"][id]["mode"].as<int>().unwrapOr(0);
	auto newValue = oldValue + parameters->m_value;
	m_filter["xp"][id].set("mode", newValue);

	log::info("{}: {}", id, newValue);

	//update nodes
	auto menu = btn->getParent();
	auto left = menu->getChildByID("mode-left");
	auto right = menu->getChildByID("mode-right");
	auto label = typeinfo_cast<CCLabelBMFont*>(menu->getChildByID("mode-label"));

	std::vector<std::string> modeStrings = { ">=", ">", "=", "<", "<=" };

	left->setVisible(newValue > 0);
	right->setVisible(newValue < 4);
	label->setCString(modeStrings[newValue].c_str());

	//save filters
	Mod::get()->setSavedValue<matjson::Value>("search-filter", m_filter);

	return;
}
void SearchPopup::checkAll(CCObject* sender) {
	if (m_mainLayer) m_mainLayer->removeAllChildrenWithCleanup(true);

	switch (m_currentTab) {
    	case static_cast<int>(SearchModes::Difficulty):
        	for (auto [key, value] : m_filter["difficulties"]) m_filter["difficulties"].set(key, true);
        	break;
   		case static_cast<int>(SearchModes::Packs):
        	for (auto [key, value] : m_filter["packs"]) m_filter["packs"].set(key, true);
        	break;
    	case static_cast<int>(SearchModes::Skills):
        	for (auto [key, value] : m_filter["skills"]) m_filter["skills"].set(key, true);
        	break;
    	case static_cast<int>(SearchModes::XP):
        	for (auto [key, value] : m_filter["xp"]) m_filter["xp"][key].set("toggled", true);
        	break;
	}

	//save filters
	Mod::get()->setSavedValue<matjson::Value>("search-filter", m_filter);

	//save offset
	auto offs = m_list->m_tableView->m_contentLayer->getPositionY();

	//reload tab
	loadTab(m_currentTab);
	m_list->m_tableView->m_contentLayer->setPositionY(offs);

	return;
}
void SearchPopup::uncheckAll(CCObject* sender) {
	if (m_mainLayer) m_mainLayer->removeAllChildrenWithCleanup(true);
	
	switch (m_currentTab) {
    	case static_cast<int>(SearchModes::Difficulty):
        	for (auto [key, value] : m_filter["difficulties"]) m_filter["difficulties"].set(key, false);
        	break;
   		case static_cast<int>(SearchModes::Packs):
        	for (auto [key, value] : m_filter["packs"]) m_filter["packs"].set(key, false);
        	break;
    	case static_cast<int>(SearchModes::Skills):
        	for (auto [key, value] : m_filter["skills"]) m_filter["skills"].set(key, false);
        	break;
    	case static_cast<int>(SearchModes::XP):
        	for (auto [key, value] : m_filter["xp"]) m_filter["xp"][key].set("toggled", false);
        	break;
	}

	//save filters
	Mod::get()->setSavedValue<matjson::Value>("search-filter", m_filter);

	//save offset
	auto offs = m_list->m_tableView->m_contentLayer->getPositionY();

	//reload tab
	loadTab(m_currentTab);
	m_list->m_tableView->m_contentLayer->setPositionY(offs);
	
	return;
}

SearchPopup* SearchPopup::create() {
	auto ret = new SearchPopup();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

SearchPopup::~SearchPopup() {
	this->removeAllChildrenWithCleanup(true);
}