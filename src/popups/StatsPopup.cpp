//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/JsonValidation.hpp>

#include "../menus/DPLayer.hpp"
#include "StatsPopup.hpp"

//geode namespace
using namespace geode::prelude;

bool StatsPopup::init() {
	if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Stats");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));
	
	//create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	
	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	//create tabs
	auto tabs = CCMenu::create();
	
	auto mainBtnSprOff = ButtonSprite::create("Summary", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	mainBtnSprOff->m_BGSprite->setContentSize({ 115.f, 25.f });

	auto mainBtnSprOn = ButtonSprite::create("Summary", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	mainBtnSprOn->m_BGSprite->setContentSize({ 115.f, 25.f });

	auto mainBtn = CCMenuItemToggler::create(mainBtnSprOff, mainBtnSprOn, this, menu_selector(StatsPopup::onTab));
	mainBtn->setContentSize({ 115.f, 25.f });
	mainBtn->setPosition({ 90.f, 0.f });
	mainBtn->setID("main");
	mainBtn->setTag(static_cast<int>(StatsTab::Main));
	mainBtn->toggle(true);
	tabs->addChild(mainBtn);

	auto ranksBtnSprOff = ButtonSprite::create("Ranks", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	ranksBtnSprOff->m_BGSprite->setContentSize({ 115.f, 25.f });

	auto ranksBtnSprOn = ButtonSprite::create("Ranks", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	ranksBtnSprOn->m_BGSprite->setContentSize({ 115.f, 25.f });

	auto ranksBtn = CCMenuItemToggler::create(ranksBtnSprOff, ranksBtnSprOn, this, menu_selector(StatsPopup::onTab));
	ranksBtn->setContentSize({ 115.f, 25.f });
	ranksBtn->setPosition({ 210.f, 0.f });
	ranksBtn->setID("ranks");
	ranksBtn->setTag(static_cast<int>(StatsTab::Ranks));
	ranksBtn->toggle(false);
	tabs->addChild(ranksBtn);

	auto medalsBtnSprOff = ButtonSprite::create("Medals", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	medalsBtnSprOff->m_BGSprite->setContentSize({ 115.f, 25.f });

	auto medalsBtnSprOn = ButtonSprite::create("Medals", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	medalsBtnSprOn->m_BGSprite->setContentSize({ 115.f, 25.f });

	auto medalsBtn = CCMenuItemToggler::create(medalsBtnSprOff, medalsBtnSprOn, this, menu_selector(StatsPopup::onTab));
	medalsBtn->setContentSize({ 115.f, 25.f });
	medalsBtn->setPosition({ 330.f, 0.f });
	medalsBtn->setID("medals");
	medalsBtn->setTag(static_cast<int>(StatsTab::Medals));
	medalsBtn->toggle(false);
	tabs->addChild(medalsBtn);

	tabs->setPosition({0.f, 200.f});
	tabs->setContentSize({420.f, 50.f});

	tabs->setID("stat-tabs");
	layer->addChild(tabs);

	m_tabs = tabs;

	loadTab(static_cast<int>(StatsTab::Main));

	return true;
}

void StatsPopup::loadTab(int id) {

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return;
	}

	switch(id) {
		case static_cast<int>(StatsTab::Main):
		{
			
			//Score
			auto scoreSection = CCNode::create();
			scoreSection->setPosition({ 90.f, 150.f });
			scoreSection->setID("score-section");

			auto scoreHeader = CCLabelBMFont::create("Score", "bigFont.fnt");
			scoreHeader->setPositionY(20.f);
			scoreHeader->setScale(0.65f);
			scoreHeader->setID("score-header");

			auto scoreValue = CCLabelBMFont::create(std::to_string(getScore()).c_str(), "bigFont.fnt");
			scoreValue->setPositionY(0.f);
			scoreValue->setScale(1.f);
			scoreValue->setID("score-value");

			auto infoMenu = CCMenu::create();
			infoMenu->setPosition({ 37.f, 25.f });
			infoMenu->setContentSize({ 0.f, 0.f });
			infoMenu->setScale(0.6f);
			infoMenu->setZOrder(1);
			infoMenu->setID("info-menu");

			auto scoreInfoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::onScoreInfo));
			scoreInfoBtn->setID("info-btn");
			infoMenu->addChild(scoreInfoBtn);

			auto milestone = floor(getScore() / 25);
			auto milestoneStr = fmt::format("Next Milestone at {}", (milestone + 1) * 25);
			if (milestone >= 14) {
				milestoneStr = "All Milestones Reached!";
			}

			auto scoreMilestone = CCLabelBMFont::create(milestoneStr.c_str(), "bigFont.fnt");
			scoreMilestone->setPositionY(-20.f);
			scoreMilestone->setScale(0.25f);
			scoreMilestone->setID("score-milestone");

			scoreSection->addChild(scoreHeader);
			scoreSection->addChild(scoreValue);
			scoreSection->addChild(scoreMilestone);
			scoreSection->addChild(infoMenu);

			m_mainLayer->addChild(scoreSection);

			//Monthly Check
			auto listSaveID = fmt::format("{}-{}", data["monthly"][0]["month"].asInt().unwrapOr(1), data["monthly"][0]["year"].asInt().unwrapOr(1987));
			auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(listSaveID);

			auto monthlySection = CCNode::create();
			monthlySection->setPosition({ 210.f, 150.f });
			monthlySection->setID("monthly-section");

			auto monthlyHeader = CCLabelBMFont::create("Monthly", "bigFont.fnt");
			monthlyHeader->setPositionY(20.f);
			monthlyHeader->setScale(0.65f);
			monthlyHeader->setID("monthly-header");

			auto monthlySprite = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
			monthlySprite->setPositionY(-5.f);
			monthlySprite->setScale(1.f);
			monthlySprite->setID("monthly-sprite");

			auto epicSprite = CCSprite::createWithSpriteFrameName("GJ_epicCoin_001.png");
			epicSprite->setPosition({ 11.5f, 7.5f });
			epicSprite->setZOrder(-1);

			if (listSave.progress == 6) {
				monthlySprite->addChild(epicSprite);
			}
			else if (listSave.progress < 5) {
				monthlySprite->setColor({ 0, 0, 0 });
			}

			auto monthlyInfoMenu = CCMenu::create();
			monthlyInfoMenu->setPosition({ 49.f, 25.f });
			monthlyInfoMenu->setContentSize({ 0.f, 0.f });
			monthlyInfoMenu->setScale(0.6f);
			monthlyInfoMenu->setZOrder(1);
			monthlyInfoMenu->setID("info-menu");

			auto monthlyInfoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::onMonthlyInfo));
			monthlyInfoBtn->setID("info-btn");
			monthlyInfoMenu->addChild(monthlyInfoBtn);

			monthlySection->addChild(monthlyHeader);
			monthlySection->addChild(monthlySprite);
			monthlySection->addChild(monthlyInfoMenu);

			m_mainLayer->addChild(monthlySection);

			//Monthly Completions
			auto monthlyCompletionsSection = CCNode::create();
			monthlyCompletionsSection->setPosition({ 330.f, 150.f });
			monthlyCompletionsSection->setID("monthly-completions-section");

			auto monthlyCompletionsHeader = CCLabelBMFont::create("Monthly Completions", "bigFont.fnt");
			monthlyCompletionsHeader->setPositionY(20.f);
			monthlyCompletionsHeader->setScale(0.35f);
			monthlyCompletionsHeader->setID("monthly-completions-header");

			auto monthlyCompletionsValue = CCLabelBMFont::create(std::to_string(Mod::get()->getSavedValue<std::vector<std::string>>("monthly-completions").size()).c_str(), "bigFont.fnt");
			monthlyCompletionsValue->setPositionY(0.f);
			monthlyCompletionsValue->setScale(1.f);
			monthlyCompletionsValue->setID("monthly-completions-value");

			std::vector<int> milestoneList = { 1, 2, 3, 4, 5, 10, 15, 25 };
			milestone = milestoneList[0];

			for (int i = 0; i < milestoneList.size(); i++) {
				if (Mod::get()->getSavedValue<std::vector<std::string>>("monthly-completions").size() < milestoneList[i]) {
					milestone = milestoneList[i];
					break;
				}

				if (Mod::get()->getSavedValue<std::vector<std::string>>("monthly-completions").size() > milestoneList[milestoneList.size()]) {
					milestone = -1; //all milestones reached
				}
			}

			milestoneStr = fmt::format("Next Milestone at {}", milestone);
			if (milestone == -1) {
				milestoneStr = "All Milestones Reached!";
			}

			auto monthlyCompletionsMilestone = CCLabelBMFont::create(milestoneStr.c_str(), "bigFont.fnt");
			monthlyCompletionsMilestone->setPositionY(-20.f);
			monthlyCompletionsMilestone->setScale(0.25f);
			monthlyCompletionsMilestone->setID("monthly-completions-milestone");

			monthlyCompletionsSection->addChild(monthlyCompletionsHeader);
			monthlyCompletionsSection->addChild(monthlyCompletionsValue);
			monthlyCompletionsSection->addChild(monthlyCompletionsMilestone);

			m_mainLayer->addChild(monthlyCompletionsSection);

			//Highest Rank
			auto highestRankSection = CCNode::create();
			highestRankSection->setPosition({ 60.f, 90.f });
			highestRankSection->setID("highest-rank-section");

			auto highestRankHeader = CCLabelBMFont::create("Highest Rank", "bigFont.fnt");
			highestRankHeader->setPositionY(20.f);
			highestRankHeader->setScale(0.35f);
			highestRankHeader->setID("highest-rank-header");

			auto highestRankSprite = CCSprite::createWithSpriteFrameName("DP_Beginner.png"_spr);

			for (int i = 0; i < data["main"].asArray().unwrap().size(); i++) {
				auto saveID = data["main"][i]["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (listSave.hasRank) {
					auto sprName = fmt::format("{}.png", data["main"][i]["sprite"].asString().unwrapOr("DP_Beginner"));
					highestRankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
				}
				else {
					if (i == 0) {
						highestRankSprite->setColor({ 0, 0, 0 });
					}
				}
			}

			highestRankSprite->setPositionY(-10.f);
			highestRankSprite->setScale(1.f);
			highestRankSprite->setID("highest-rank-sprite");

			highestRankSection->addChild(highestRankHeader);
			highestRankSection->addChild(highestRankSprite);

			m_mainLayer->addChild(highestRankSection);

			//Highest Plus Rank
			auto highestPlusSection = CCNode::create();
			highestPlusSection->setPosition({ 160.f, 90.f });
			highestPlusSection->setID("highest-plus-rank-section");

			auto highestPlusHeader = CCLabelBMFont::create("Highest Plus Rank", "bigFont.fnt");
			highestPlusHeader->setPositionY(20.f);
			highestPlusHeader->setScale(0.35f);
			highestPlusHeader->setID("highest-plus-rank-header");

			auto highestPlusSprite = CCSprite::createWithSpriteFrameName("DP_BeginnerPlus.png"_spr);

			for (int i = 0; i < data["main"].asArray().unwrap().size(); i++) {
				auto saveID = data["main"][i]["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (listSave.completed) {
					auto sprName = fmt::format("{}.png", data["main"][i]["plusSprite"].asString().unwrapOr("DP_Beginner"));
					highestPlusSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
				}
				else {
					if (i == 0) {
						highestPlusSprite->setColor({ 0, 0, 0 });
					}
				}
			}

			highestPlusSprite->setPositionY(-10.f);
			highestPlusSprite->setScale(1.f);
			highestPlusSprite->setID("highest-plus-rank-sprite");

			highestPlusSection->addChild(highestPlusHeader);
			highestPlusSection->addChild(highestPlusSprite);

			m_mainLayer->addChild(highestPlusSection);

			//Next Rank
			auto nextRankSection = CCNode::create();
			nextRankSection->setPosition({ 260.f, 90.f });
			nextRankSection->setID("next-rank-section");

			auto nextRankHeader = CCLabelBMFont::create("Next Rank", "bigFont.fnt");
			nextRankHeader->setPositionY(20.f);
			nextRankHeader->setScale(0.35f);
			nextRankHeader->setID("next-rank-header");

			auto nextRankSprite = CCSprite::createWithSpriteFrameName("DP_Beginner.png"_spr);

			for (int i = 0; i < data["main"].asArray().unwrap().size(); i++) {

				auto saveID = data["main"][i]["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (i + 1 >= data["main"].asArray().unwrap().size() && listSave.hasRank) {
					nextRankSprite = CCSprite::createWithSpriteFrameName("DP_Obsidian.png"_spr);
					nextRankSprite->setColor({ 0, 0, 0 });
					break;
				}
				else if (listSave.hasRank) {
					auto sprName = fmt::format("{}.png", data["main"][i + 1]["sprite"].asString().unwrapOr("DP_Beginner"));
					nextRankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
				}
			}

			nextRankSprite->setPositionY(-10.f);
			nextRankSprite->setScale(1.f);
			nextRankSprite->setID("next-rank-sprite");

			nextRankSection->addChild(nextRankHeader);
			nextRankSection->addChild(nextRankSprite);

			m_mainLayer->addChild(nextRankSection);

			//Next Plus Rank
			auto nextPlusSection = CCNode::create();
			nextPlusSection->setPosition({ 360.f, 90.f });
			nextPlusSection->setID("next-plus-rank-section");

			auto nextPlusHeader = CCLabelBMFont::create("Next Plus Rank", "bigFont.fnt");
			nextPlusHeader->setPositionY(20.f);
			nextPlusHeader->setScale(0.35f);
			nextPlusHeader->setID("next-plus-rank-header");

			auto nextPlusSprite = CCSprite::createWithSpriteFrameName("DP_BeginnerPlus.png"_spr);

			for (int i = 0; i < data["main"].asArray().unwrap().size(); i++) {

				auto saveID = data["main"][i]["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (i + 1 >= data["main"].asArray().unwrap().size() && listSave.completed) {
					nextPlusSprite = CCSprite::createWithSpriteFrameName("DP_ObsidianPlus.png"_spr);
					nextPlusSprite->setColor({ 0, 0, 0 });
					break;
				}
				else if (listSave.completed) {
					auto sprName = fmt::format("{}.png", data["main"][i + 1]["plusSprite"].asString().unwrapOr("DP_Beginner"));
					nextPlusSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
				}
			}

			nextPlusSprite->setPositionY(-10.f);
			nextPlusSprite->setScale(1.f);
			nextPlusSprite->setID("next-plus-rank-sprite");

			nextPlusSection->addChild(nextPlusHeader);
			nextPlusSection->addChild(nextPlusSprite);

			m_mainLayer->addChild(nextPlusSection);

			//Medal
			auto medalSection = CCNode::create();
			medalSection->setPosition({ 120.f, 35.f });
			medalSection->setID("medal-section");

			auto medalHeader = CCLabelBMFont::create("Your Medal", "bigFont.fnt");
			medalHeader->setPositionY(20.f);
			medalHeader->setScale(0.35f);
			medalHeader->setID("medal-header");

			auto highestMedal = -1;
			for (int i = 0; i < NormalMedals.size(); i++) {
				if (getPercentToRank(NormalMedalRequirements[i], false) >= 1.f) {
					highestMedal = i;
				}
			}

			auto medalStr = "None";
			ccColor3B medalColor = { 130, 130, 130 };
			if (highestMedal != -1) {
				medalStr = NormalMedals[highestMedal].c_str();
				medalColor = NormalMedalColors[highestMedal];
			}

			auto medalValue = CCLabelBMFont::create(medalStr, "bigFont.fnt");
			medalValue->setColor(medalColor);
			medalValue->setPositionY(0.f);
			medalValue->setScale(0.7f);
			medalValue->setID("medal-value");

			medalSection->addChild(medalHeader);
			medalSection->addChild(medalValue);

			m_mainLayer->addChild(medalSection);

			//Plus Medal
			auto medalPlusSection = CCNode::create();
			medalPlusSection->setPosition({ 300.f, 35.f });
			medalPlusSection->setID("plus-medal-section");

			auto medalPlusHeader = CCLabelBMFont::create("Your Plus Medal", "bigFont.fnt");
			medalPlusHeader->setPositionY(20.f);
			medalPlusHeader->setScale(0.35f);
			medalPlusHeader->setID("plus-medal-header");

			auto highestPlusMedal = -1;
			for (int i = 0; i < PlusMedals.size(); i++) {
				if (getPercentToRank(PlusMedalRequirements[i], true) >= 1.f) {
					highestPlusMedal = i;
				}
			}

			//Absolute Perfection Progress
			auto progressPercent = getPercentToRank(data["main"].asArray().unwrap().size() - 1, true);

			auto bonusProgress = 0;
			auto bonusTotalLevels = 0;
			for (int i = 0; i < data["bonus"].asArray().unwrap().size(); i++) {
				auto saveID = data["bonus"][i]["saveID"].asString().unwrapOr("null");
				auto totalLevels = data["bonus"][i]["levelIDs"].asArray().unwrap().size();
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				bonusProgress += listSave.progress;
				bonusTotalLevels += totalLevels;
			}

			auto bonusPercent = static_cast<float>(bonusProgress) / static_cast<float>(bonusTotalLevels);
			auto totalPercent = (progressPercent + bonusPercent) / 2.f;

			if (totalPercent >= 1.f) {
				highestPlusMedal = -2;
			}

			auto plusMedalStr = "None";
			ccColor3B plusMedalColor = { 130, 130, 130 };
			if (highestPlusMedal > -1) {
				plusMedalStr = PlusMedals[highestPlusMedal].c_str();
				plusMedalColor = PlusMedalColors[highestPlusMedal];
			}
			else if (highestPlusMedal == -2) {
				plusMedalStr = "ABSOLUTE PERFECTION";
				plusMedalColor = { 255, 190, 255 };
			}

			auto plusMedalValue = CCLabelBMFont::create(plusMedalStr, "bigFont.fnt");
			plusMedalValue->setColor(plusMedalColor);
			plusMedalValue->setPositionY(0.f);
			plusMedalValue->setScale(0.7f);
			if (highestPlusMedal == -2) {
				plusMedalValue->setScale(0.425f);
			}
			plusMedalValue->setID("plus-medal-value");

			medalPlusSection->addChild(medalPlusHeader);
			medalPlusSection->addChild(plusMedalValue);

			m_mainLayer->addChild(medalPlusSection);

			break;
		}
		case static_cast<int>(StatsTab::Ranks):
		{
			auto scrollLayer = ScrollLayer::create({ 400.f, 175.f }, true, true);
			scrollLayer->setPosition({ 10.25f, 10.f });
			scrollLayer->setID("scroll-layer");
			m_mainLayer->addChild(scrollLayer);

			//Ranks
			auto ranksSection = CCNode::create();
			ranksSection->setPosition({ 200.f, 138.f });
			ranksSection->setID("ranks-section");

			auto ranksHeader = CCLabelBMFont::create("Ranks", "bigFont.fnt");
			ranksHeader->setPositionY(50.f);
			ranksHeader->setScale(0.65f);
			ranksHeader->setID("header");

			auto ranksBG = CCScale9Sprite::create("square02_001.png");
			ranksBG->setPositionY(-10.f);
			ranksBG->setContentSize({ 400.f, 90.f });
			ranksBG->setOpacity(128);
			ranksBG->setID("bg");

			auto ranksContainer = CCMenu::create();
			ranksContainer->setPosition({ 0.f, -10.f });
			ranksContainer->setContentSize({ 400.f, 95.5f });
			ranksContainer->setID("container");

			auto containerLayout = AxisLayout::create();
			containerLayout->setGrowCrossAxis(true);
			containerLayout->setGap(-2.f);
			containerLayout->setAxisAlignment(AxisAlignment::Center);
			containerLayout->setCrossAxisAlignment(AxisAlignment::Center);
			containerLayout->setCrossAxisLineAlignment(AxisAlignment::Start);
			containerLayout->setAxis(Axis::Row);

			ranksContainer->setLayout(containerLayout, true);

			for (int i = 0; i < data["main"].asArray().unwrap().size(); i++) {

				auto saveID = data["main"][i]["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				auto sprName = fmt::format("{}.png", data["main"][i]["sprite"].asString().unwrapOr("DP_Invisible"));
				auto plusSprName = fmt::format("{}.png", data["main"][i]["plusSprite"].asString().unwrapOr("DP_Invisible"));

				auto rankSprite = CCSprite::createWithSpriteFrameName("DP_Beginner.png"_spr);

				if (sprName != "DP_Invisible.png") {
					rankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
				}
				else {
					rankSprite->setOpacity(0);
				}

				if (!listSave.hasRank && !listSave.completed) {
					rankSprite->setColor({ 0, 0, 0 });
				}
				else if (listSave.completed && sprName != "DP_Invisible.png") {
					rankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(plusSprName).data());
				}

				auto infoMenu = CCMenu::create();
				infoMenu->setPosition({ 10.f, 10.f });
				infoMenu->setContentSize({ 0.f, 0.f });
				infoMenu->setScale(0.6f);
				infoMenu->setZOrder(1);
				infoMenu->setID("info-menu");

				auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::rankInfoCallback));
				infoBtn->setID("main");
				infoBtn->setTag(i);
				infoMenu->addChild(infoBtn);

				rankSprite->addChild(infoMenu);

				ranksContainer->addChild(rankSprite);
			}

			ranksContainer->updateLayout(false);

			ranksSection->addChild(ranksBG);
			ranksSection->addChild(ranksHeader);
			ranksSection->addChild(ranksContainer);

			//Legacy Ranks
			auto legacyRanksSection = CCNode::create();
			legacyRanksSection->setPosition({ 200.f, 20.f });
			legacyRanksSection->setID("legacy-ranks-section");

			auto legacyRanksHeader = CCLabelBMFont::create("Legacy Ranks", "bigFont.fnt");
			legacyRanksHeader->setPositionY(50.f);
			legacyRanksHeader->setScale(0.65f);
			legacyRanksHeader->setID("header");

			auto legacyRanksBG = CCScale9Sprite::create("square02_001.png");
			legacyRanksBG->setPositionY(-10.f);
			legacyRanksBG->setContentSize({ 400.f, 90.f });
			legacyRanksBG->setOpacity(128);
			legacyRanksBG->setID("bg");

			auto legacyRanksContainer = CCMenu::create();
			legacyRanksContainer->setPosition({ 0.f, -10.f });
			legacyRanksContainer->setContentSize({ 400.f, 95.5f });
			legacyRanksContainer->setID("container");

			legacyRanksContainer->setLayout(containerLayout, true);

			for (int i = 0; i < data["legacy"].asArray().unwrap().size(); i++) {

				auto saveID = data["legacy"][i]["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				auto mainPack = data["legacy"][i]["mainPack"].asInt().unwrapOr(0);

				auto sprName = fmt::format("{}.png", data["main"][mainPack]["sprite"].asString().unwrapOr("DP_Invisible"));

				auto rankSprite = CCSprite::createWithSpriteFrameName("DP_Beginner.png"_spr);

				if (sprName != "DP_Invisible.png") {
					rankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
				}
				else {
					rankSprite->setOpacity(0);
				}

				if (!listSave.hasRank && !listSave.completed) {
					rankSprite->setColor({ 0, 0, 0 });
				}

				auto infoMenu = CCMenu::create();
				infoMenu->setPosition({ 10.f, 10.f });
				infoMenu->setContentSize({ 0.f, 0.f });
				infoMenu->setScale(0.6f);
				infoMenu->setZOrder(1);
				infoMenu->setID("info-menu");

				auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::rankInfoCallback));
				infoBtn->setID("legacy");
				infoBtn->setTag(i);
				infoMenu->addChild(infoBtn);

				rankSprite->addChild(infoMenu);

				legacyRanksContainer->addChild(rankSprite);
			}

			legacyRanksContainer->updateLayout(false);

			legacyRanksSection->addChild(legacyRanksBG);
			legacyRanksSection->addChild(legacyRanksHeader);
			legacyRanksSection->addChild(legacyRanksContainer);

			//Bonus Ranks
			auto bonusRanksSection = CCNode::create();
			bonusRanksSection->setPosition({ 200.f, -97.f });
			bonusRanksSection->setID("bonus-ranks-section");

			auto bonusRanksHeader = CCLabelBMFont::create("Bonus Packs", "bigFont.fnt");
			bonusRanksHeader->setPositionY(50.f);
			bonusRanksHeader->setScale(0.65f);
			bonusRanksHeader->setID("header");

			auto bonusRanksBG = CCScale9Sprite::create("square02_001.png");
			bonusRanksBG->setPositionY(-(bonusRanksHeader->getPositionY() - bonusRanksHeader->getContentHeight()) - 35.f);
			bonusRanksBG->setContentSize({ 400.f, (floor(data["bonus"].asArray().unwrap().size() / 9.f) * 60.f) });
			bonusRanksBG->setOpacity(128);
			bonusRanksBG->setID("bg");

			auto bonusRanksContainer = CCMenu::create();
			bonusRanksContainer->setPosition({ 0.f, bonusRanksBG->getPositionY()});
			bonusRanksContainer->setContentSize({ 400.f, bonusRanksBG->getContentHeight() });
			bonusRanksContainer->setID("container");

			bonusRanksContainer->setLayout(containerLayout, true);

			for (int i = 0; i < data["bonus"].asArray().unwrap().size(); i++) {

				auto saveID = data["bonus"][i]["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				auto sprName = fmt::format("{}.png", data["bonus"][i]["sprite"].asString().unwrapOr("DP_Invisible"));

				auto rankSprite = CCSprite::createWithSpriteFrameName("DP_Beginner.png"_spr);

				if (sprName != "DP_Invisible.png") {
					rankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
				}
				else {
					rankSprite->setOpacity(0);
				}

				if (!listSave.hasRank && !listSave.completed) {
					rankSprite->setColor({ 0, 0, 0 });
				}

				auto infoMenu = CCMenu::create();
				infoMenu->setPosition({ 10.f, 10.f });
				infoMenu->setContentSize({ 0.f, 0.f });
				infoMenu->setScale(0.6f);
				infoMenu->setZOrder(1);
				infoMenu->setID("info-menu");

				auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::rankInfoCallback));
				infoBtn->setID("bonus");
				infoBtn->setTag(i);
				infoMenu->addChild(infoBtn);

				rankSprite->addChild(infoMenu);

				bonusRanksContainer->addChild(rankSprite);
			}

			bonusRanksContainer->updateLayout(false);

			bonusRanksSection->addChild(bonusRanksBG);
			bonusRanksSection->addChild(bonusRanksHeader);
			bonusRanksSection->addChild(bonusRanksContainer);

			//Add Sections
			auto contentLayer = CCLayer::create();
			contentLayer->setID("content-layer");

			contentLayer->addChild(ranksSection);
			contentLayer->addChild(legacyRanksSection);
			contentLayer->addChild(bonusRanksSection);

			contentLayer->setPositionY(250.f);
			scrollLayer->m_contentLayer->setPositionY(-250.f);
			scrollLayer->m_contentLayer->setContentHeight(450.f);
			scrollLayer->scrollToTop();

			scrollLayer->m_contentLayer->addChild(contentLayer);
			break;
		}
		case static_cast<int>(StatsTab::Medals):
		{
			auto scrollLayer = ScrollLayer::create({ 400.f, 175.f }, true, true);
			scrollLayer->setPosition({ 10.25f, 10.f });
			scrollLayer->setID("scroll-layer");
			m_mainLayer->addChild(scrollLayer);

			//Normal Medals
			auto normalSection = CCNode::create();
			normalSection->setPosition({ 200.f, 138.f });
			normalSection->setID("normal-section");

			auto normalHeader = CCLabelBMFont::create("Medals", "bigFont.fnt");
			normalHeader->setPositionY(50.f);
			normalHeader->setScale(0.65f);
			normalHeader->setID("normal-header");
			normalSection->addChild(normalHeader);

			for (int i = 0; i < NormalMedals.size(); i++) {
				auto medal = CCNode::create();
				medal->setPosition({ 0.f, (15.f - (30.f * i))});
				medal->setID(fmt::format("normal-medal-{}", i + 1));

				auto medalHeader = CCLabelBMFont::create(NormalMedals[i].c_str(), "bigFont.fnt");
				medalHeader->setPositionY(1.f);
				medalHeader->setScale(0.75f);
				medalHeader->setColor(NormalMedalColors[i]);
				medalHeader->setID("medal-header");

				auto packProgressBack = CCSprite::create("GJ_progressBar_001.png");
				packProgressBack->setAnchorPoint({ 0, 0.5 });
				packProgressBack->setPosition({ -102.f, -15.f });
				packProgressBack->setColor({ 0, 0, 0 });
				packProgressBack->setOpacity(128);
				packProgressBack->setID("progress-bar");

				auto packProgressFront = CCSprite::create("GJ_progressBar_001.png");
				packProgressFront->setAnchorPoint({ 0, 0.5 });
				packProgressFront->setPosition({ 0.0f, 10.f });
				packProgressFront->setScaleX(0.98f);
				packProgressFront->setScaleY(0.75f);
				packProgressFront->setZOrder(1);
				packProgressFront->setColor(NormalMedalColors[i]);

				auto progressPercent = getPercentToRank(NormalMedalRequirements[i], false);

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

				auto progressLabel = CCLabelBMFont::create(fmt::format("{}%", clampf(floor(progressPercent * 100), 0, 100)).c_str(), "bigFont.fnt");
				progressLabel->setPosition({ 170.f, 12.f });
				progressLabel->setScale(0.65f);
				progressLabel->setZOrder(2);
				progressLabel->setID("progress-label");
				packProgressBack->addChild(progressLabel);

				auto infoMenu = CCMenu::create();
				infoMenu->setPosition({ 110.f, -15.f });
				infoMenu->setContentSize({ 0.f, 0.f });
				infoMenu->setScale(0.6f);
				infoMenu->setZOrder(1);
				infoMenu->setID("info-menu");

				auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::medalInfoCallback));
				infoBtn->setID("normal");
				infoBtn->setTag(i);
				infoMenu->addChild(infoBtn);

				medal->addChild(packProgressBack);
				medal->addChild(medalHeader);
				medal->addChild(infoMenu);

				normalSection->addChild(medal);
			}

			//Plus Medals
			auto plusSection = CCNode::create();
			plusSection->setPosition({ 200.f, -60.f });
			plusSection->setID("plus-section");

			auto plusHeader = CCLabelBMFont::create("Plus Medals", "bigFont.fnt");
			plusHeader->setPositionY(50.f);
			plusHeader->setScale(0.65f);
			plusHeader->setID("plus-header");
			plusSection->addChild(plusHeader);

			for (int i = 0; i < PlusMedals.size(); i++) {
				auto medal = CCNode::create();
				medal->setPosition({ 0.f, (15.f - (30.f * i)) });
				medal->setID(fmt::format("plus-medal-{}", i + 1));
				medal->setTag(i);

				auto medalHeader = CCLabelBMFont::create(PlusMedals[i].c_str(), "bigFont.fnt");
				medalHeader->setPositionY(1.f);
				medalHeader->setScale(0.75f);
				medalHeader->setColor(PlusMedalColors[i]);
				medalHeader->setID("medal-header");

				auto packProgressBack = CCSprite::create("GJ_progressBar_001.png");
				packProgressBack->setAnchorPoint({ 0, 0.5 });
				packProgressBack->setPosition({ -102.f, -15.f });
				packProgressBack->setColor({ 0, 0, 0 });
				packProgressBack->setOpacity(128);
				packProgressBack->setID("progress-bar");

				auto packProgressFront = CCSprite::create("GJ_progressBar_001.png");
				packProgressFront->setAnchorPoint({ 0, 0.5 });
				packProgressFront->setPosition({ 0.0f, 10.f });
				packProgressFront->setScaleX(0.98f);
				packProgressFront->setScaleY(0.75f);
				packProgressFront->setZOrder(1);
				packProgressFront->setColor(PlusMedalColors[i]);

				auto progressPercent = getPercentToRank(PlusMedalRequirements[i], true);

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

				auto progressLabel = CCLabelBMFont::create(fmt::format("{}%", clampf(floor(progressPercent * 100), 0, 100)).c_str(), "bigFont.fnt");
				progressLabel->setPosition({ 170.f, 12.f });
				progressLabel->setScale(0.65f);
				progressLabel->setZOrder(2);
				progressLabel->setID("progress-label");
				packProgressBack->addChild(progressLabel);

				auto infoMenu = CCMenu::create();
				infoMenu->setPosition({ 110.f, -15.f });
				infoMenu->setContentSize({ 0.f, 0.f });
				infoMenu->setScale(0.6f);
				infoMenu->setZOrder(1);
				infoMenu->setID("info-menu");

				auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::medalInfoCallback));
				infoBtn->setID("plus");
				infoBtn->setTag(i);
				infoMenu->addChild(infoBtn);

				medal->addChild(packProgressBack);
				medal->addChild(medalHeader);
				medal->addChild(infoMenu);

				plusSection->addChild(medal);
			}

			//Absolute Perfection
			auto medal = CCNode::create();
			medal->setPosition({ 0.f, (15.f - (30.f * PlusMedals.size() + 1)) });
			medal->setID(fmt::format("plus-medal-{}", PlusMedals.size() + 1));
			medal->setTag(-1);

			auto medalHeader = CCLabelBMFont::create("ABSOLUTE PERFECTION", "bigFont.fnt");
			medalHeader->setPositionY(1.f);
			medalHeader->setScale(0.75f);
			medalHeader->setColor({ 255, 190, 255 });
			medalHeader->setID("medal-header");

			auto packProgressBack = CCSprite::create("GJ_progressBar_001.png");
			packProgressBack->setAnchorPoint({ 0, 0.5 });
			packProgressBack->setPosition({ -102.f, -15.f });
			packProgressBack->setColor({ 0, 0, 0 });
			packProgressBack->setOpacity(128);
			packProgressBack->setID("progress-bar");

			auto packProgressFront = CCSprite::create("GJ_progressBar_001.png");
			packProgressFront->setAnchorPoint({ 0, 0.5 });
			packProgressFront->setPosition({ 0.0f, 10.f });
			packProgressFront->setScaleX(0.98f);
			packProgressFront->setScaleY(0.75f);
			packProgressFront->setZOrder(1);
			packProgressFront->setColor({ 255, 190, 255 });

			auto progressPercent = getPercentToRank(data["main"].asArray().unwrap().size() - 1, true);

			//Get All Bonus Progress
			auto bonusProgress = 0;
			auto bonusTotalLevels = 0;
			for (auto pack : data["bonus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto totalLevels = pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault().size();
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				bonusProgress += listSave.progress;
				bonusTotalLevels += totalLevels;
			}

			auto bonusPercent = static_cast<float>(bonusProgress) / static_cast<float>(bonusTotalLevels);
			auto totalPercent = (progressPercent + bonusPercent) / 2.f;

			auto clippingNode = CCClippingNode::create();
			auto stencil = CCScale9Sprite::create("square02_001.png");
			stencil->setAnchorPoint({ 0, 0.5f });
			stencil->setContentWidth(packProgressFront->getScaledContentSize().width);
			stencil->setScaleX(totalPercent);
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

			auto progressLabel = CCLabelBMFont::create(fmt::format("{}%", clampf(floor(totalPercent * 100), 0, 100)).c_str(), "bigFont.fnt");
			progressLabel->setPosition({ 170.f, 12.f });
			progressLabel->setScale(0.65f);
			progressLabel->setZOrder(2);
			progressLabel->setID("progress-label");
			packProgressBack->addChild(progressLabel);

			auto infoMenu = CCMenu::create();
			infoMenu->setPosition({ 110.f, -15.f });
			infoMenu->setContentSize({ 0.f, 0.f });
			infoMenu->setScale(0.6f);
			infoMenu->setZOrder(1);
			infoMenu->setID("info-menu");

			auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::medalInfoCallback));
			infoBtn->setID("plus");
			infoBtn->setTag(-1);
			infoMenu->addChild(infoBtn);

			medal->addChild(packProgressBack);
			medal->addChild(medalHeader);
			medal->addChild(infoMenu);

			plusSection->addChild(medal);

			//Add Sections
			auto contentLayer = CCLayer::create();
			contentLayer->setID("content-layer");

			contentLayer->addChild(normalSection);
			contentLayer->addChild(plusSection);

			contentLayer->setPositionY(375.f);
			contentLayer->setContentHeight(1000.f);
			scrollLayer->m_contentLayer->setPositionY(-405.f);
			scrollLayer->m_contentLayer->setContentHeight(580.f);

			scrollLayer->m_contentLayer->addChild(contentLayer);
			break;
		}
	}

	return;
}

void StatsPopup::onScoreInfo(CCObject* sender) {
	FLAlertLayer::create("Score Info", "Your GDDP Score is based on how many <cy>Main Pack</c> levels you've beaten.", "OK")->show();

	return;
}

void StatsPopup::onMonthlyInfo(CCObject* sender) {
	FLAlertLayer::create("Monthly Info", "Complete at least <cy>5 Levels</c> of this month's Monthly Pack.", "OK")->show();

	return;
}

void StatsPopup::medalInfoCallback(CCObject* sender) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto medalType = btn->getID();
	auto medalID = btn->getTag();

	if (medalID == -1) { //Special Case for ABSOLUTE PERFECTION
		//Absolute Perfection Progress is just score, so borrow from that
		auto progress = getScore();

		//get total level count
		auto totalLevels = 0;
		for (auto pack : data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
			totalLevels += pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault().size();
		}
		
		//Get All Bonus Progress
		auto bonusProgress = 0;
		auto bonusTotalLevels = 0;
		for (auto pack : data["bonus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
			auto saveID = pack["saveID"].asString().unwrapOr("null");
			auto total = pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault().size();
			auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

			bonusProgress += listSave.progress;
			bonusTotalLevels += total;
		}

		std::string progressStr = fmt::format("{}/{} Main Levels\n{}/{} Bonus Levels", progress, totalLevels, bonusProgress, bonusTotalLevels);

		if (progress == totalLevels && bonusProgress != bonusTotalLevels) {
			progressStr = fmt::format("<cy>{}/{} Main Levels</c>\n{}/{} Bonus Levels", progress, totalLevels, bonusProgress, bonusTotalLevels);
		}
		else if (progress != totalLevels && bonusProgress == bonusTotalLevels) {
			progressStr = fmt::format("{}/{} Main Levels\n<cy>{}/{} Bonus Levels</c>", progress, totalLevels, bonusProgress, bonusTotalLevels);
		}
		else if (progress == totalLevels && bonusProgress == bonusTotalLevels) {
			progressStr = fmt::format("<cy>{}/{} Main Levels</c>\n<cy>{}/{} Bonus Levels</c>", progress, totalLevels, bonusProgress, bonusTotalLevels);
		}

		FLAlertLayer::create("Medal Info", fmt::format("Nope, this is not happening, there is no way anyone will ever do this, absolutely no way. Wont happen.\n\n (Beat every Level in every Tier & Bonus Pack on the list to achieve this Medal)\n\n{}", progressStr), "OK")->show();
	}
	else {
		if (medalType == "plus")
		{
			//get plus progress up to whatever plus medal is there
			auto progress = 0;
			auto totalLevels = 0;
			for (int i = 0; i <= PlusMedalRequirements[medalID]; i++) {
				auto pack = data["main"][i];
				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto total = pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault().size();
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				progress += listSave.progress;
				totalLevels += total;
			}

			if (progress >= totalLevels) {
				progress = clampf(progress, 0, totalLevels);
				FLAlertLayer::create("Medal Info", fmt::format("{}\n\n<cy>{}/{} to Medal</c>", PlusMedalDescriptions[medalID], progress, totalLevels), "OK")->show();
			}
			else {
				FLAlertLayer::create("Medal Info", fmt::format("{}\n\n{}/{} to Medal", PlusMedalDescriptions[medalID], progress, totalLevels), "OK")->show();
			}
		}
		else if (medalType == "normal")
		{
			//get plus progress up to whatever plus medal is there
			auto progress = 0;
			auto totalLevels = 0;
			for (int i = 0; i <= NormalMedalRequirements[medalID]; i++) {
				auto pack = data["main"][i];
				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto reqLevels = pack["reqLevels"].as<int>().unwrapOr(999);
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (listSave.hasRank) {
					progress += reqLevels;
				}
				else {
					progress += listSave.progress;
				}

				totalLevels += reqLevels;
			}

			if (progress >= totalLevels) {
				progress = clampf(progress, 0, totalLevels);
				FLAlertLayer::create("Medal Info", fmt::format("{}\n\n<cy>{}/{} to Medal</c>", NormalMedalDescriptions[medalID], progress, totalLevels), "OK")->show();
			}
			else {
				FLAlertLayer::create("Medal Info", fmt::format("{}\n\n{}/{} to Medal", NormalMedalDescriptions[medalID], progress, totalLevels), "OK")->show();
			}
		}
		else
		{
			FLAlertLayer::create("Oops!!", "Something went wrong, you should probably tell the dev about this.", "OK")->show();
		}
	}

	return;
}

void StatsPopup::rankInfoCallback(CCObject* sender) {

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto type = btn->getID();
	auto id = btn->getTag();

	auto saveID = data[type][id]["saveID"].asString().unwrapOr("null");
	auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

	if (type == "main")
	{
		if (listSave.hasRank && !listSave.completed)
		{
			FLAlertLayer::create("Rank Info", fmt::format("{} Demons\n\n<cy>{}/{} to Rank</c>\n{}/{} to Plus Rank", data["main"][id]["name"].asString().unwrapOr("null"), clampf(listSave.progress, 0, data["main"][id]["reqLevels"].asInt().unwrapOr(999)), data["main"][id]["reqLevels"].asInt().unwrapOr(999), listSave.progress, data["main"][id]["levelIDs"].asArray().unwrap().size()), "OK")->show();
		}
		else if (listSave.completed)
		{
			FLAlertLayer::create("Rank Info", fmt::format("{} Demons\n\n<cy>{}/{} to Rank</c>\n<cy>{}/{} to Plus Rank</c>", data["main"][id]["name"].asString().unwrapOr("null"), clampf(listSave.progress, 0, data["main"][id]["reqLevels"].asInt().unwrapOr(999)), data["main"][id]["reqLevels"].asInt().unwrapOr(999), listSave.progress, data["main"][id]["levelIDs"].asArray().unwrap().size()), "OK")->show();
		}
		else
		{
			FLAlertLayer::create("Rank Info", fmt::format("{} Demons\n\n{}/{} to Rank\n{}/{} to Plus Rank", data["main"][id]["name"].asString().unwrapOr("null"), clampf(listSave.progress, 0, data["main"][id]["reqLevels"].asInt().unwrapOr(999)), data["main"][id]["reqLevels"].asInt().unwrapOr(999), listSave.progress, data["main"][id]["levelIDs"].asArray().unwrap().size()), "OK")->show();
		}
	}
	else if (type == "legacy")
	{
		if (listSave.completed)
		{
			FLAlertLayer::create("Rank Info", fmt::format("{} Demons\n\n<cy>{}/{} to Completion</c>", data["legacy"][id]["name"].asString().unwrapOr("null"), listSave.progress, data["legacy"][id]["levelIDs"].asArray().unwrap().size()), "OK")->show();
		}
		else
		{
			FLAlertLayer::create("Rank Info", fmt::format("{} Demons\n\n{}/{} to Completion", data["legacy"][id]["name"].asString().unwrapOr("null"), listSave.progress, data["legacy"][id]["levelIDs"].asArray().unwrap().size()), "OK")->show();
		}
	}
	else if (type == "bonus")
	{
		if (listSave.completed)
		{
			FLAlertLayer::create("Rank Info", fmt::format("{}\n\n<cy>{}/{} to Completion</c>", data["bonus"][id]["name"].asString().unwrapOr("null"), listSave.progress, data["bonus"][id]["levelIDs"].asArray().unwrap().size()), "OK")->show();
		}
		else
		{
			FLAlertLayer::create("Rank Info", fmt::format("{}\n\n{}/{} to Completion", data["bonus"][id]["name"].asString().unwrapOr("null"), listSave.progress, data["bonus"][id]["levelIDs"].asArray().unwrap().size()), "OK")->show();
		}
	}
	else
	{
		FLAlertLayer::create("Oops!!", "Something went wrong, you should probably tell the dev about this.", "OK")->show();
	}

	return;
}

void StatsPopup::onTab(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto menuType = btn->getTag();

	auto mainbtn = m_tabs->getChildByID("main");
	auto ranksbtn = m_tabs->getChildByID("ranks");
	auto medalsbtn = m_tabs->getChildByID("medals");

	if (btn->isToggled()) {
		btn->toggle(false);
	}

	if (m_mainLayer) {
		m_mainLayer->removeAllChildrenWithCleanup(true);
	}

	switch(menuType) {
		case static_cast<int>(StatsTab::Main):
		{
			static_cast<CCMenuItemToggler*>(ranksbtn)->toggle(false);
			static_cast<CCMenuItemToggler*>(medalsbtn)->toggle(false);

			loadTab(menuType);
			break;
		}
		case static_cast<int>(StatsTab::Ranks):
		{
			static_cast<CCMenuItemToggler*>(mainbtn)->toggle(false);
			static_cast<CCMenuItemToggler*>(medalsbtn)->toggle(false);

			loadTab(menuType);
			break;
		}
		case static_cast<int>(StatsTab::Medals):
		{
			static_cast<CCMenuItemToggler*>(ranksbtn)->toggle(false);
			static_cast<CCMenuItemToggler*>(mainbtn)->toggle(false);

			loadTab(menuType);
			break;
		}
	}

	return;
}

int StatsPopup::getScore() {

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return 0; //NO WAY GEOMETRY DASH REFERENCE OOOOOOOOOOOOOOOO
	}

	auto score = 0;

	for (auto pack : data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {

		auto saveID = pack["saveID"].asString().unwrapOr("null");
		auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

		score += listSave.progress;
	}

	return score;
}

float StatsPopup::getPercentToRank(int rankID, bool isPlus) {

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return 0.f;
	}
	
	auto progress = 0;
	auto totalLvls = 0;

	if (rankID == 0) {
		auto saveID = data["main"][0]["saveID"].asString().unwrapOr("null");
		auto reqLevels = data["main"][0]["reqLevels"].asInt().unwrapOr(999);
		auto totalLevels = data["main"][0]["levelIDs"].asArray().unwrap().size();
		auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

		if (isPlus) {
			progress += listSave.progress;
			totalLvls += totalLevels;
		}
		else {
			if (listSave.hasRank) {
				progress += reqLevels;
			}
			else {
				progress += clampf(listSave.progress, 0, reqLevels);
			}
			totalLvls += reqLevels;
		}
	}
	else {
		for (int i = 0; i <= rankID; i++) {
			auto saveID = data["main"][i]["saveID"].asString().unwrapOr("null");
			auto reqLevels = data["main"][i]["reqLevels"].asInt().unwrapOr(999);
			auto totalLevels = data["main"][i]["levelIDs"].asArray().unwrap().size();
			auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

			if (isPlus) {
				progress += listSave.progress;
				totalLvls += totalLevels;
			}
			else {
				if (listSave.hasRank) {
					progress += reqLevels;
				}
				else {
					progress += clampf(listSave.progress, 0, reqLevels);
				}
				totalLvls += reqLevels;
			}
		}
	}

	auto percent = static_cast<float>(progress) / static_cast<float>(totalLvls);

	return percent;
}

StatsPopup* StatsPopup::create() {
	auto ret = new StatsPopup();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

StatsPopup::~StatsPopup() {
	this->removeAllChildrenWithCleanup(true);
}