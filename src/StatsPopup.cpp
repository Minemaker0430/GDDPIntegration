//geode header
#include <Geode/Geode.hpp>

//other headers
#include "DPLayer.hpp"
#include "StatsPopup.hpp"

//geode namespace
using namespace geode::prelude;

bool StatsPopup::setup() {
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	if (!data.contains("main")) {
		return true;
	}

	this->setTitle("Stats");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	auto scrollLayer = ScrollLayer::create({ 400.f, 200.f }, true, true);
	scrollLayer->setPosition({ 10.25f, 13.f });
	scrollLayer->setID("stats-layer");
	layer->addChild(scrollLayer);

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

	for (int i = 0; i < data["main"].as_array().size(); i++) {

		auto listID = data["main"][i]["listID"].as_int();
		auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(listID));

		auto sprName = fmt::format("{}.png", data["main"][i]["sprite"].as_string());
		auto plusSprName = fmt::format("{}.png", data["main"][i]["plusSprite"].as_string());

		auto rankSprite = CCSprite::createWithSpriteFrameName("DP_Beginner.png"_spr);

		if (sprName != "DP_Invisible.png") {
			rankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName.c_str()));
		}
		else {
			rankSprite->setVisible(false);
		}

		if (!listSave.hasRank && !listSave.completed) {
			rankSprite->setColor({ 0, 0, 0 });
		}
		else if (listSave.completed && sprName != "DP_Invisible.png") {
			rankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(plusSprName.c_str()));
		}

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

	for (int i = 0; i < data["legacy"].as_array().size(); i++) {

		auto listID = data["legacy"][i]["listID"].as_int();
		auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(listID));

		auto mainPack = data["legacy"][i]["mainPack"].as_int();

		auto sprName = fmt::format("{}.png", data["main"][mainPack]["sprite"].as_string());

		auto rankSprite = CCSprite::createWithSpriteFrameName("DP_Beginner.png"_spr);

		if (sprName != "DP_Invisible.png") {
			rankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName.c_str()));
		}
		else {
			rankSprite->setVisible(false);
		}

		if (!listSave.hasRank && !listSave.completed) {
			rankSprite->setColor({ 0, 0, 0 });
		}

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
	bonusRanksBG->setPositionY(-32.f);
	bonusRanksBG->setContentSize({ 400.f, 135.f });
	bonusRanksBG->setOpacity(128);
	bonusRanksBG->setID("bg");

	auto bonusRanksContainer = CCMenu::create();
	bonusRanksContainer->setPosition({ 0.f, -32.f });
	bonusRanksContainer->setContentSize({ 400.f, 142.25f });
	bonusRanksContainer->setID("container");

	bonusRanksContainer->setLayout(containerLayout, true);

	for (int i = 0; i < data["bonus"].as_array().size(); i++) {

		auto listID = data["bonus"][i]["listID"].as_int();
		auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(std::to_string(listID));

		auto sprName = fmt::format("{}.png", data["bonus"][i]["sprite"].as_string());

		auto rankSprite = CCSprite::createWithSpriteFrameName("DP_Beginner.png"_spr);

		if (sprName != "DP_Invisible.png") {
			rankSprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName.c_str()));
		}
		else {
			rankSprite->setVisible(false);
		}

		if (!listSave.hasRank && !listSave.completed) {
			rankSprite->setColor({ 0, 0, 0 });
		}

		bonusRanksContainer->addChild(rankSprite);
	}

	bonusRanksContainer->updateLayout(false);

	bonusRanksSection->addChild(bonusRanksBG);
	bonusRanksSection->addChild(bonusRanksHeader);
	bonusRanksSection->addChild(bonusRanksContainer);

	//Score
	auto scoreSection = CCNode::create();
	scoreSection->setPosition({ 200.f, -97.f });
	scoreSection->setID("score-section");

	auto scoreHeader = CCLabelBMFont::create("Score", "bigFont.fnt");
	scoreHeader->setPositionY(50.f);
	scoreHeader->setScale(0.65f);
	scoreHeader->setID("header");

	auto scoreValue = CCLabelBMFont::create("0", "bigFont.fnt");
	scoreValue->setPositionY(50.f);
	scoreValue->setScale(0.65f);
	scoreValue->setID("value");

	scoreValue->setCString(std::to_string(Mod::get()->setSavedValue<int>("demon-count-main", 0)).c_str());

	scoreSection->addChild(scoreHeader);
	scoreSection->addChild(scoreValue);

	//Add Sections
	auto contentLayer = CCLayer::create();
	contentLayer->setID("content-layer");

	contentLayer->addChild(ranksSection);
	contentLayer->addChild(legacyRanksSection);
	contentLayer->addChild(bonusRanksSection);
	contentLayer->addChild(scoreSection);

	contentLayer->setPositionY(200.f);
	scrollLayer->m_contentLayer->setPositionY(-200.f);
	scrollLayer->m_contentLayer->setContentHeight(400.f);

	scrollLayer->m_contentLayer->addChild(contentLayer);

	return true;
}

StatsPopup* StatsPopup::create() {
	auto ret = new StatsPopup();
	if (ret && ret->initAnchored(420.f, 250.f)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

StatsPopup::~StatsPopup() {
	this->removeAllChildrenWithCleanup(true);
}