//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/JsonValidation.hpp>

#include "../menus/DPLayer.hpp"
#include "StatsPopup.hpp"
#include "../CustomText.hpp"

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

			//get total level count
			auto totalLevels = 0;
			for (auto pack : data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
				totalLevels += pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault().size();
			}

			auto milestone = floor(getScore() / 25);
			auto milestoneStr = fmt::format("Next Milestone at {}", (milestone + 1) * 25);
			if (milestone >= (floor(totalLevels / 25) - 1)) milestoneStr = "All Milestones Reached!";

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

			if (listSave.progress == 6) monthlySprite->addChild(epicSprite);
			else if (listSave.progress < 5) monthlySprite->setColor({ 0, 0, 0 });

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

			auto highestRankSprite = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);

			auto revData = data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
			std::reverse(revData.begin(), revData.end());

			for (auto pack : revData) {
				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (listSave.hasRank) {
					auto sprName = fmt::format("{}.png", pack["sprite"].asString().unwrapOr("DP_Unknown"));
					highestRankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
					break;
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

			auto highestPlusSprite = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);

			for (auto pack : revData) {
				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (listSave.completed) {
					auto sprName = fmt::format("{}.png", pack["plusSprite"].asString().unwrapOr("DP_Unknown"));
					highestPlusSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
					break;
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

			auto nextRankSprite = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);

			for (auto pack : data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {

				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (!listSave.hasRank) {
					auto sprName = fmt::format("{}.png", pack["sprite"].asString().unwrapOr("DP_Unknown"));
					nextRankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
					break;
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

			auto nextPlusSprite = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);

			for (auto pack : data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {

				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (!listSave.completed) {
					auto sprName = fmt::format("{}.png", pack["plusSprite"].asString().unwrapOr("DP_Unknown"));
					nextPlusSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
					break;
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

			auto normalMedals = data["medals"]["normal"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
			auto plusMedals = data["medals"]["plus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

			auto highestMedal = -1;
			for (int i = 0; i < normalMedals.size(); i++) if (getPercentToRank(normalMedals[i]["requirement"].as<int>().unwrapOr(0), false) >= 1.f) highestMedal = i;

			std::string medalStr = "None";
			ccColor3B medalColor = { 130, 130, 130 };
			if (highestMedal != -1) {
				medalStr = normalMedals[highestMedal]["name"].asString().unwrapOr("???");
				medalColor = normalMedals[highestMedal]["color"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255});
			}

			auto medalValue = CCLabelBMFont::create(medalStr.c_str(), "bigFont.fnt");
			medalValue->setColor(medalColor);
			medalValue->setPositionY(0.f);
			medalValue->setScale(0.7f);
			medalValue->setID("medal-value");

			if (Mod::get()->getSettingValue<bool>("fancy-medals")) {
				auto customText = CustomText::create(medalStr);
				customText->setAnchorPoint({ 0.5f, 0.5f });
				customText->setPositionY(0.f);
				customText->setScale(0.7f);
				customText->setID("medal-value");
				customText->setTextColor(medalColor);
				customText->addGradient((highestMedal == -1) ? ccColor3B{0, 0, 0} : ccColor3B{255, 255, 255}, 0.25f);
				if (highestMedal != -1) customText->addBevel();
				medalSection->addChild(customText);
			}
			else medalSection->addChild(medalValue);

			medalSection->addChild(medalHeader);

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
			for (int i = 0; i < plusMedals.size(); i++) if (getPercentToRank(plusMedals[i]["requirement"].as<int>().unwrapOr(0), true) >= 1.f) highestPlusMedal = i;

			//Absolute Perfection Progress
			auto progressPercent = getPercentToRank(data["main"].asArray().unwrap().size() - 1, true);

			auto bonusProgress = 0;
			auto bonusTotalLevels = 0;
			for (auto p : data["bonus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
				auto saveID = p["saveID"].asString().unwrapOr("null");
				auto totalLevels = p["levelIDs"].asArray().unwrap().size();
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				bonusProgress += listSave.progress;
				bonusTotalLevels += totalLevels;
			}

			auto bonusPercent = static_cast<float>(bonusProgress) / static_cast<float>(bonusTotalLevels);
			auto totalPercent = (progressPercent + bonusPercent) / 2.f;

			if (totalPercent >= 1.f) highestPlusMedal = -2;

			std::string plusMedalStr = "None";
			ccColor3B plusMedalColor = { 130, 130, 130 };
			if (highestPlusMedal > -1) {
				plusMedalStr = plusMedals[highestMedal]["name"].asString().unwrapOr("???");
				plusMedalColor = plusMedals[highestMedal]["color"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255});
			}
			else if (highestPlusMedal == -2) {
				plusMedalStr = "ABSOLUTE PERFECTION";
				plusMedalColor = { 255, 190, 255 };
			}

			auto plusMedalValue = CCLabelBMFont::create(plusMedalStr.c_str(), "bigFont.fnt");
			plusMedalValue->setColor(plusMedalColor);
			plusMedalValue->setPositionY(0.f);
			plusMedalValue->setScale(0.7f);
			if (highestPlusMedal == -2) plusMedalValue->setScale(0.425f);
			plusMedalValue->setID("plus-medal-value");

			if (Mod::get()->getSettingValue<bool>("fancy-medals")) {
				auto customText = CustomText::create(plusMedalStr);
				customText->setAnchorPoint({ 0.5f, 0.5f });
				customText->setPositionY(0.f);
				customText->setScale(0.7f);
				if (highestPlusMedal == -2) customText->setScale(0.425f);
				customText->setID("plus-medal-value");
				customText->setTextColor(plusMedalColor);
				customText->addGradient((highestPlusMedal == -1) ? ccColor3B{0, 0, 0} : ccColor3B{255, 255, 255}, 0.25f);
				if (highestPlusMedal != -1) {
					customText->setOutlineColor({255, 255, 255});
					customText->addCrystals({255, 255, 255}, 0.5f);
				}
				medalPlusSection->addChild(customText);
			}
			else medalPlusSection->addChild(plusMedalValue);

			medalPlusSection->addChild(medalPlusHeader);

			m_mainLayer->addChild(medalPlusSection);

			break;
		}
		case static_cast<int>(StatsTab::Ranks):
		{
			auto scrollLayer = ScrollLayer::create({ 400.f, 175.f }, true, true);
			scrollLayer->setPosition({ 10.25f, 10.f });
			scrollLayer->setID("scroll-layer");
			m_mainLayer->addChild(scrollLayer);

			auto layout = AxisLayout::create();
			layout->setGap(7.5f);
			layout->setAxisAlignment(AxisAlignment::Start);
			layout->setAxis(Axis::Column);
			layout->setAutoScale(false);
			layout->setAxisReverse(true);
			layout->ignoreInvisibleChildren(false);
			layout->setAutoGrowAxis(true);

			scrollLayer->m_contentLayer->setPosition({ 200.f, 0.f });
			scrollLayer->m_contentLayer->setLayout(layout);

			std::vector<std::string> names = {"Ranks", "Legacy Ranks", "Bonus Packs"};
			std::vector<std::string> indexes = {"main", "legacy", "bonus"};

			for (int id = 0; id < indexes.size(); id++) {
				auto section = CCNode::create();
				section->setAnchorPoint({ 0.f, 1.f });
				section->setID(fmt::format("{}-section", indexes[id]));

				auto container = CCMenu::create();
				container->setPosition({ 0.f, 10.f });
				container->setAnchorPoint({ 0.5f, 0.f });
				container->setContentSize({ 400.f, 95.5f });
				container->setID("container");

				auto containerLayout = AxisLayout::create();
				containerLayout->setGrowCrossAxis(true);
				containerLayout->setGap(-2.f);
				containerLayout->setAxisAlignment(AxisAlignment::Center);
				containerLayout->setCrossAxisAlignment(AxisAlignment::Center);
				containerLayout->setCrossAxisLineAlignment(AxisAlignment::Start);
				containerLayout->setAxis(Axis::Row);

				container->setLayout(containerLayout, true);

				for (int i = 0; i < data[indexes[id]].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {

					auto saveID = data[indexes[id]][i]["saveID"].asString().unwrapOr("null");
					auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

					auto sprName = fmt::format("{}.png", data[indexes[id]][i]["sprite"].asString().unwrapOr("DP_Unknown"));
					auto plusSprName = fmt::format("{}.png", data[indexes[id]][i]["plusSprite"].asString().unwrapOr("DP_Unknown"));

					auto sprite = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);

					if (sprName != "DP_Invisible.png") sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());

					if (!listSave.hasRank && !listSave.completed) sprite->setColor({ 0, 0, 0 });
					else if (listSave.completed && indexes[id] == "main") sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(plusSprName).data());
					if (sprName == "DP_Invisible.png") sprite->setOpacity(0);

					auto infoMenu = CCMenu::create();
					infoMenu->setPosition({ 10.f, 10.f });
					infoMenu->setContentSize({ 0.f, 0.f });
					infoMenu->setScale(0.6f);
					infoMenu->setZOrder(1);
					infoMenu->setID("info-menu");

					auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::rankInfoCallback));
					infoBtn->setID(indexes[id]);
					infoBtn->setTag(i);
					infoMenu->addChild(infoBtn);

					sprite->addChild(infoMenu);

					container->addChild(sprite);
				}

				container->updateLayout(false);

				auto bg = CCScale9Sprite::create("square02_001.png");
				bg->setAnchorPoint({ 0.5f, 0.f });
				bg->setContentSize({ 400.f, container->getContentHeight() + 20.f });
				bg->setOpacity(128);
				bg->setID("bg");

				auto header = CCLabelBMFont::create(names[id].c_str(), "bigFont.fnt");
				header->setPositionY(bg->getContentHeight() + 15.f);
				header->setScale(0.65f);
				header->setID("header");

				section->addChild(bg);
				section->addChild(header);
				section->addChild(container);

				section->setContentHeight(bg->getContentHeight() + header->getContentHeight());

				scrollLayer->m_contentLayer->addChild(section);
				scrollLayer->m_contentLayer->updateLayout(false);
			}

			scrollLayer->scrollToTop();
			break;
		}
		case static_cast<int>(StatsTab::Medals):
		{
			auto scrollLayer = ScrollLayer::create({ 400.f, 175.f }, true, true);
			scrollLayer->setPosition({ 10.25f, 10.f });
			scrollLayer->setID("scroll-layer");
			m_mainLayer->addChild(scrollLayer);

			auto layout = AxisLayout::create();
			layout->setGap(20.f);
			layout->setAxisAlignment(AxisAlignment::Start);
			layout->setAxis(Axis::Column);
			layout->setAutoScale(false);
			layout->setAxisReverse(true);
			layout->ignoreInvisibleChildren(false);
			layout->setAutoGrowAxis(true);

			scrollLayer->m_contentLayer->setPosition({ 200.f, 0.f });
			scrollLayer->m_contentLayer->setLayout(layout);

			for (std::string id : {"normal", "plus"}) {
				auto section = CCNode::create();
				section->setPosition({ 200.f, 138.f });
				section->setID(fmt::format("{}-section", id));
				
				auto sectionLayout = AxisLayout::create();
				sectionLayout->setGap(10.f);
				sectionLayout->setAxisAlignment(AxisAlignment::Start);
				sectionLayout->setAxis(Axis::Column);
				sectionLayout->setAutoScale(false);
				sectionLayout->setAxisReverse(true);
				sectionLayout->ignoreInvisibleChildren(true);
				sectionLayout->setAutoGrowAxis(true);
				section->setLayout(sectionLayout);

				auto header = CCLabelBMFont::create((id == "plus") ? "Plus Medals" : "Medals", "bigFont.fnt");
				header->setPositionY(30.f);
				header->setAnchorPoint({ 0.5f, 1.f });
				header->setScale(0.65f);
				header->setID(fmt::format("{}-header", id));

				auto headerNode = CCNode::create(); // gotta make the header a child of an empty node so it doesn't mess up the layout
				headerNode->setContentHeight(30.f);
				headerNode->setAnchorPoint({ 0.f, 1.f });
				headerNode->setID("header-parent");
				headerNode->addChild(header);
				section->addChild(headerNode);

				for (int i = 0; i < data["medals"][id].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
					auto medal = CCNode::create();
					medal->setAnchorPoint({ 0.f, 1.f });
					medal->setContentHeight(20.f);
					medal->setID(fmt::format("{}-medal-{}", id, (i + 1)));

					auto medalData = data["medals"][id][i];

					auto medalHeader = CCLabelBMFont::create(medalData["name"].asString().unwrapOr("???").c_str(), "bigFont.fnt");
					medalHeader->setPositionY(17.5f);
					medalHeader->setScale(0.75f);
					medalHeader->setColor(medalData["color"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}));
					medalHeader->setID("medal-header");

					if (Mod::get()->getSettingValue<bool>("fancy-medals")) {
						auto customHeader = CustomText::create(medalData["name"].asString().unwrapOr("???"));
						customHeader->setAnchorPoint({ 0.5f, 0.5f });
						customHeader->setPositionY(17.5f);
						customHeader->setScale(0.75f);
						customHeader->setTextColor(medalData["color"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}));
						customHeader->addGradient({255, 255, 255}, 0.25f);
						if (id == "plus") {
							customHeader->setOutlineColor({255, 255, 255});
							customHeader->addCrystals({255, 255, 255}, 0.5f);
						}
						else customHeader->addBevel();
						customHeader->setID("medal-header");
						medal->addChild(customHeader);
					}
					else medal->addChild(medalHeader);

					auto packProgressBack = CCSprite::create("GJ_progressBar_001.png");
					packProgressBack->setAnchorPoint({ 0.f, 0.5f });
					packProgressBack->setPosition({ -102.f, 0.f });
					packProgressBack->setColor((Mod::get()->getSettingValue<bool>("fancy-medals") && id == "plus") ? ccColor3B{ 255, 255, 255 } : ccColor3B{ 0, 0, 0 });
					packProgressBack->setOpacity(128);
					packProgressBack->setID("progress-bar");

					auto packProgressFront = CCSprite::create("GJ_progressBar_001.png");
					packProgressFront->setAnchorPoint({ 0.f, 0.5f });
					packProgressFront->setPosition({ 0.0f, 10.f });
					packProgressFront->setScaleX(0.98f);
					packProgressFront->setScaleY(0.75f);
					packProgressFront->setZOrder(1);
					packProgressFront->setColor(medalData["color"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}));

					auto progressPercent = getPercentToRank(medalData["requirement"].as<int>().unwrapOr(0), id == "plus");

					auto clippingNode = CCClippingNode::create();
					auto stencil = CCScale9Sprite::create("square02_001.png");
					stencil->setAnchorPoint({ 0, 0.5f });
					stencil->setContentWidth(packProgressFront->getScaledContentSize().width);
					stencil->setScaleX(progressPercent);
					stencil->setContentHeight(100);
					clippingNode->setStencil(stencil);
					clippingNode->setAnchorPoint({ 0.f, 0.5f });
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
					infoMenu->setPosition({ 110.f, 0.f });
					infoMenu->setContentSize({ 0.f, 0.f });
					infoMenu->setScale(0.6f);
					infoMenu->setZOrder(1);
					infoMenu->setID("info-menu");

					auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::medalInfoCallback));
					infoBtn->setID(id);
					infoBtn->setTag(i);
					infoMenu->addChild(infoBtn);

					medal->addChild(packProgressBack);
					medal->addChild(infoMenu);

					section->addChild(medal);
					section->updateLayout(false);
				}

				if (id == "plus") {
					//Absolute Perfection
					auto medal = CCNode::create();
					medal->setAnchorPoint({ 0.f, 1.f });
					medal->setContentHeight(20.f);
					medal->setID(fmt::format("plus-medal-{}", data["medals"][id].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size() + 1));
					medal->setTag(-1);

					auto medalHeader = CCLabelBMFont::create("ABSOLUTE PERFECTION", "bigFont.fnt");
					medalHeader->setPositionY(17.5f);
					medalHeader->setScale(0.75f);
					medalHeader->setColor({ 255, 190, 255 });
					medalHeader->setID("medal-header");

					if (Mod::get()->getSettingValue<bool>("fancy-medals")) {
						auto customHeader = CustomText::create("ABSOLUTE PERFECTION");
						customHeader->setAnchorPoint({ 0.5f, 0.5f });
						customHeader->setPositionY(17.5f);
						customHeader->setScale(0.75f);
						customHeader->setTextColor({ 255, 190, 255 });
						customHeader->addGradient({255, 255, 255}, 0.25f);
						if (id == "plus") {
							customHeader->setOutlineColor({255, 255, 255});
							customHeader->addCrystals({255, 255, 255}, 0.5f);
						}
						else customHeader->addBevel();
						customHeader->setID("medal-header");
						medal->addChild(customHeader);
					}
					else medal->addChild(medalHeader);

					auto packProgressBack = CCSprite::create("GJ_progressBar_001.png");
					packProgressBack->setAnchorPoint({ 0.f, 0.5f });
					packProgressBack->setPosition({ -102.f, 0.f });
					packProgressBack->setColor(Mod::get()->getSettingValue<bool>("fancy-medals") ? ccColor3B{ 255, 255, 255 } : ccColor3B{ 0, 0, 0 });
					packProgressBack->setOpacity(128);
					packProgressBack->setID("progress-bar");

					auto packProgressFront = CCSprite::create("GJ_progressBar_001.png");
					packProgressFront->setAnchorPoint({ 0.f, 0.5f });
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
					clippingNode->setAnchorPoint({ 0.f, 0.5f });
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
					infoMenu->setPosition({ 110.f, 0.f });
					infoMenu->setContentSize({ 0.f, 0.f });
					infoMenu->setScale(0.6f);
					infoMenu->setZOrder(1);
					infoMenu->setID("info-menu");

					auto infoBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(StatsPopup::medalInfoCallback));
					infoBtn->setID("plus");
					infoBtn->setTag(-1);
					infoMenu->addChild(infoBtn);

					medal->addChild(packProgressBack);
					medal->addChild(infoMenu);

					section->addChild(medal);

					auto margin = CCNode::create();
					margin->setID("margin-node");
					section->addChild(margin);

					section->updateLayout(false);
				}

				scrollLayer->m_contentLayer->addChild(section);
				scrollLayer->m_contentLayer->updateLayout(false);
			}

			scrollLayer->scrollToTop();
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

	auto normalMedals = data["medals"]["normal"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
	auto plusMedals = data["medals"]["plus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

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

		std::string progressStr = (
			((progress == totalLevels) ? "<cy>" : "") +
			fmt::format("{}/{} Main Levels", progress, totalLevels) +
			((progress == totalLevels) ? "</c>\n" : "\n") +
			((bonusProgress == bonusTotalLevels) ? "<cy>" : "") +
			fmt::format("{}/{} Bonus Levels", bonusProgress, bonusTotalLevels) +
			((bonusProgress == bonusTotalLevels) ? "</c>" : "")
		);

		FLAlertLayer::create("Medal Info", fmt::format("Nope, this is not happening, there is no way anyone will ever do this, absolutely no way. Wont happen.\n\n (Beat every Level in every Tier & Bonus Pack on the list to achieve this Medal)\n\n{}", progressStr), "OK")->show();
	}
	else {
		if (medalType == "plus") {
			//get plus progress up to whatever plus medal is there
			auto progress = 0;
			auto totalLevels = 0;
			for (int i = 0; i <= plusMedals[medalID]["requirement"].as<int>().unwrapOr(0); i++) {
				auto pack = data["main"][i];
				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto total = pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault().size();
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				progress += listSave.progress;
				totalLevels += total;
			}
			
			FLAlertLayer::create(
				"Medal Info", 
				fmt::format(
					"Achieve every rank from {} to {}+\n\n{}{}/{} to Medal{}", 
					data["main"][0]["name"].asString().unwrapOr("???"),
					data["main"][plusMedals[medalID]["requirement"].as<int>().unwrapOr(0)]["name"].asString().unwrapOr("???"),
					(progress == totalLevels) ? "<cy>" : "",
					clampf(progress, 0, totalLevels), 
					totalLevels,
					(progress == totalLevels) ? "</c>" : ""
				), 
				"OK"
			)->show();
		}
		else if (medalType == "normal") {
			//get plus progress up to whatever plus medal is there
			auto progress = 0;
			auto totalLevels = 0;
			for (int i = 0; i <= normalMedals[medalID]["requirement"].as<int>().unwrapOr(0); i++) {
				auto pack = data["main"][i];
				auto saveID = pack["saveID"].asString().unwrapOr("null");
				auto reqLevels = pack["reqLevels"].as<int>().unwrapOr(999);
				auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

				if (listSave.hasRank) progress += reqLevels;
				else progress += listSave.progress;

				totalLevels += reqLevels;
			}

			FLAlertLayer::create(
				"Medal Info", 
				fmt::format(
					"Get the normal ranks from {} to {}\n\n{}{}/{} to Medal{}", 
					data["main"][0]["name"].asString().unwrapOr("???"),
					data["main"][normalMedals[medalID]["requirement"].as<int>().unwrapOr(0)]["name"].asString().unwrapOr("???"),
					(progress == totalLevels) ? "<cy>" : "",
					clampf(progress, 0, totalLevels), 
					totalLevels,
					(progress == totalLevels) ? "</c>" : ""
				), 
				"OK"
			)->show();
		}
		else {
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

	FLAlertLayer::create(
		"Rank Info",
		(type == "main") ? fmt::format(
			"{} Demons\n\n{}{}/{} to Rank{}\n{}{}/{} to Plus Rank{}", 
			data[type][id]["name"].asString().unwrapOr("null"), 
			((listSave.hasRank) ? "<cy>" : ""),
			clampf(listSave.progress, 0, data[type][id]["reqLevels"].asInt().unwrapOr(999)), 
			data[type][id]["reqLevels"].asInt().unwrapOr(999),
			((listSave.hasRank) ? "</c>" : ""),
			((listSave.completed) ? "<cy>" : ""),
			listSave.progress, 
			data[type][id]["levelIDs"].as<std::vector<int>>().unwrapOrDefault().size(),
			((listSave.completed) ? "</c>" : "")
		) : fmt::format(
			"{}{}\n\n{}{}/{} to Completion{}", 
			data[type][id]["name"].asString().unwrapOr("null"), 
			((type == "legacy") ? " Demons" : ""),
			((listSave.completed) ? "<cy>" : ""),
			listSave.progress, 
			data[type][id]["levelIDs"].as<std::vector<int>>().unwrapOrDefault().size(),
			((listSave.completed) ? "</c>" : "")
		),
		"OK"
	)->show();

	return;
}

void StatsPopup::onTab(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto menuType = btn->getTag();

	CCArrayExt<CCMenuItemToggler*> tabs;
	tabs.push_back(static_cast<CCMenuItemToggler*>(m_tabs->getChildByID("main")));
	tabs.push_back(static_cast<CCMenuItemToggler*>(m_tabs->getChildByID("ranks")));
	tabs.push_back(static_cast<CCMenuItemToggler*>(m_tabs->getChildByID("medals")));

	for (auto b : tabs) if (b) static_cast<CCMenuItemToggler*>(b)->toggle(false);
	btn->m_toggled = false;

	if (m_mainLayer) m_mainLayer->removeAllChildrenWithCleanup(true);

	loadTab(menuType);

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
			if (listSave.hasRank) progress += reqLevels;
			else progress += clampf(listSave.progress, 0, reqLevels);
			totalLvls += reqLevels;
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