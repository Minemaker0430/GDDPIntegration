//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/JsonValidation.hpp>

#include "../menus/DPLayer.hpp"
#include "../XPUtils.hpp"
#include "XPPopup.hpp"

//geode namespace
using namespace geode::prelude;

bool XPPopup::init() {
	if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Experience");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	//create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	//Get XP
	auto xp = Mod::get()->getSavedValue<std::vector<float>>("xp");
	auto levelProgress = Mod::get()->getSavedValue<std::vector<float>>("percent-to-level");
	auto level = Mod::get()->getSavedValue<std::vector<int>>("level");
	auto maxLevel = Mod::get()->getSavedValue<std::vector<int>>("max-levels");

	//Create Info Button
	auto infoMenu = CCMenu::create();
	auto infoButton = InfoAlertButton::create("XP Info", "<cy>XP</c> is gained by completing <cy>Main Tier</c> levels. Higher <cy>Tiers</c> give Higher <cy>XP</c>. Try to find levels that improve your <cr>Least Improved</c> skill!", 1.0f);
	infoMenu->setPosition({ 405.f, 235.f });
	infoMenu->setZOrder(2);
	infoMenu->addChild(infoButton);
	infoMenu->setID("info-menu");
	m_mainLayer->addChild(infoMenu);

	//Create Progress Bars

	auto alignment = CCNode::create();
	alignment->setPosition({ 210.f, 185.f });
	m_mainLayer->addChild(alignment);

	for (int i = 0; i < XPUtils::skillIDs.size(); i++) {
		auto skill = XPUtils::skillIDs[i];

		auto title = CCNode::create();
		title->setPosition(skillPositions[i]);
		title->setID(fmt::format("skill-{}", skill));
		title->setTag(i);

		auto titleHeader = CCLabelBMFont::create(skillNames[i].c_str(), "bigFont.fnt");
		titleHeader->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
		titleHeader->setPositionY(1.f);
		titleHeader->setScale(0.75f);
		titleHeader->setColor(skillColors[i]);
		titleHeader->setID("header");

		auto progressBack = CCSprite::create("GJ_progressBar_001.png");
		progressBack->setAnchorPoint({ 0, 0.5 });
		progressBack->setPosition({ -102.f, -15.f });
		progressBack->setColor({ 0, 0, 0 });
		progressBack->setOpacity(128);
		progressBack->setID("progress-bar");

		auto progressFront = CCSprite::create("GJ_progressBar_001.png");
		progressFront->setAnchorPoint({ 0, 0.5 });
		progressFront->setPosition({ 0.0f, 10.f });
		progressFront->setScaleX(0.98f);
		progressFront->setScaleY(0.75f);
		progressFront->setZOrder(1);
		progressFront->setColor(skillColors[i]);

		auto percent = levelProgress[i];
		if (xp[i] >= 1.f) {
			percent = 1.f;
		}

		auto clippingNode = CCClippingNode::create();
		auto stencil = CCScale9Sprite::create("square02_001.png");
		stencil->setAnchorPoint({ 0, 0.5f });
		stencil->setContentWidth(progressFront->getScaledContentSize().width);
		stencil->setScaleX(percent);
		stencil->setContentHeight(100);
		clippingNode->setStencil(stencil);
		clippingNode->setAnchorPoint({ 0, 0.5f });
		clippingNode->setPosition({ 3.25f, 10.5f });
		clippingNode->setContentWidth(progressFront->getContentWidth() - 2.f);
		clippingNode->setContentHeight(20);
		clippingNode->addChild(progressFront);
		progressBack->addChild(clippingNode);

		progressBack->setScaleX(0.6f);
		progressBack->setScaleY(0.65f);

		auto levelLabel = CCLabelBMFont::create(fmt::format("Level {}/{}", level[i], maxLevel[i]).c_str(), "bigFont.fnt");
		levelLabel->setPosition({ 10.f, 11.5f });
		levelLabel->setAnchorPoint({ 0.f, 0.5f });
		levelLabel->setScale(0.65f);
		levelLabel->setZOrder(2);
		levelLabel->setID("level-label");
		progressBack->addChild(levelLabel);

		auto progressLabel = CCLabelBMFont::create(fmt::format("{}%", clampf(floor(percent * 100), 0, 100)).c_str(), "bigFont.fnt");
		progressLabel->setPosition({ 330.f, 11.5f });
		progressLabel->setAnchorPoint({ 1.f, 0.5f });
		progressLabel->setScale(0.65f);
		progressLabel->setZOrder(2);
		progressLabel->setID("progress-label");
		progressBack->addChild(progressLabel);

		title->addChild(progressBack);
		title->addChild(titleHeader);

		alignment->addChild(title);
	}

	//Least Improved Skill
	auto leastImproved = 9;
	for (int i = 0; i < XPUtils::skillIDs.size(); i++) {
		if (xp[i] < xp[leastImproved]) {
			leastImproved = i;
		}
	}

	auto leastImprovedTitle = CCLabelBMFont::create("Least Improved", "bigFont.fnt");
	leastImprovedTitle->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	leastImprovedTitle->setPosition({ -100.f, -135.f });
	leastImprovedTitle->setScale(0.5f);
	leastImprovedTitle->setID("least-improved-header");
	alignment->addChild(leastImprovedTitle);

	auto leastImprovedValue = CCLabelBMFont::create(skillNames[leastImproved].c_str(), "bigFont.fnt");
	leastImprovedValue->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	leastImprovedValue->setPosition({ -100.f, -150.f });
	leastImprovedValue->setScale(0.7f);
	leastImprovedValue->setColor(skillColors[leastImproved]);
	leastImprovedValue->setID("least-improved-value");
	alignment->addChild(leastImprovedValue);

	//Most Improved Skill
	auto mostImproved = 0;
	for (int i = 0; i < XPUtils::skillIDs.size(); i++) {
		if (xp[i] > xp[mostImproved]) {
			mostImproved = i;
		}
	}

	auto mostImprovedTitle = CCLabelBMFont::create("Most Improved", "bigFont.fnt");
	mostImprovedTitle->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	mostImprovedTitle->setPosition({ 100.f, -135.f });
	mostImprovedTitle->setScale(0.5f);
	mostImprovedTitle->setID("most-improved-header");
	alignment->addChild(mostImprovedTitle);

	auto mostImprovedValue = CCLabelBMFont::create(skillNames[mostImproved].c_str(), "bigFont.fnt");
	mostImprovedValue->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	mostImprovedValue->setPosition({ 100.f, -150.f });
	mostImprovedValue->setScale(0.7f);
	mostImprovedValue->setColor(skillColors[mostImproved]);
	mostImprovedValue->setID("most-improved-value");
	alignment->addChild(mostImprovedValue);

	return true;
}

XPPopup* XPPopup::create() {
	auto ret = new XPPopup();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

XPPopup::~XPPopup() {
	this->removeAllChildrenWithCleanup(true);
}

//DEMON XP

bool DemonXPPopup::init() {
	if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Demon XP");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	//create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	//Get XP
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	//Create Info Button
	auto infoMenu = CCMenu::create();
	auto infoButton = InfoAlertButton::create("Demon XP Info", "The <cy>XP</c> gained from beating <cy>Demons</c> depends on how full the bars are. Empty gives no <cy>XP</c>, full gives a lot.", 1.0f);
	infoMenu->setPosition({ 405.f, 235.f });
	infoMenu->setZOrder(2);
	infoMenu->addChild(infoButton);
	infoMenu->setID("info-menu");
	m_mainLayer->addChild(infoMenu);

	//Create Progress Bars

	auto alignment = CCNode::create();
	alignment->setPosition({ 210.f, 170.f });
	m_mainLayer->addChild(alignment);

	for (int i = 0; i < XPUtils::skillIDs.size(); i++) {
		auto skill = XPUtils::skillIDs[i];
		auto levelID = std::to_string(m_levelID);
		//log::info("id: {}, skill: {}", levelID, skill);
		auto skillValue = 0;
		if (data["level-data"][levelID]["xp"][skill].isNumber()) {
			skillValue = data["level-data"][levelID]["xp"][skill].as<int>().unwrapOr(0);
		}

		auto title = CCNode::create();
		title->setPosition(skillPositions[i]);
		title->setID(fmt::format("skill-{}", skill));
		title->setTag(i);

		auto titleHeader = CCLabelBMFont::create(skillNames[i].c_str(), "bigFont.fnt");
		titleHeader->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
		titleHeader->setPositionY(1.f);
		titleHeader->setScale(0.75f);
		titleHeader->setColor(skillColors[i]);
		titleHeader->setID("header");

		auto progressBack = CCSprite::create("GJ_progressBar_001.png");
		progressBack->setAnchorPoint({ 0, 0.5 });
		progressBack->setPosition({ -102.f, -15.f });
		progressBack->setColor({ 0, 0, 0 });
		progressBack->setOpacity(128);
		progressBack->setID("progress-bar");

		auto progressFront = CCSprite::create("GJ_progressBar_001.png");
		progressFront->setAnchorPoint({ 0, 0.5 });
		progressFront->setPosition({ 0.0f, 10.f });
		progressFront->setScaleX(0.98f);
		progressFront->setScaleY(0.75f);
		progressFront->setZOrder(1);
		progressFront->setColor(skillColors[i]);

		auto percent = static_cast<float>(skillValue) / 3.f;

		auto clippingNode = CCClippingNode::create();
		auto stencil = CCScale9Sprite::create("square02_001.png");
		stencil->setAnchorPoint({ 0, 0.5f });
		stencil->setContentWidth(progressFront->getScaledContentSize().width);
		stencil->setScaleX(percent);
		stencil->setContentHeight(100);
		clippingNode->setStencil(stencil);
		clippingNode->setAnchorPoint({ 0, 0.5f });
		clippingNode->setPosition({ 3.25f, 10.5f });
		clippingNode->setContentWidth(progressFront->getContentWidth() - 2.f);
		clippingNode->setContentHeight(20);
		clippingNode->addChild(progressFront);
		progressBack->addChild(clippingNode);

		progressBack->setScaleX(0.6f);
		progressBack->setScaleY(0.65f);

		title->addChild(progressBack);
		title->addChild(titleHeader);

		alignment->addChild(title);
	}

	return true;
}

DemonXPPopup* DemonXPPopup::create(int levelID) {
	auto ret = new DemonXPPopup();
	ret->m_levelID = levelID;
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

DemonXPPopup::~DemonXPPopup() {
	this->removeAllChildrenWithCleanup(true);
}