//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/JsonValidation.hpp>

#include "../menus/DPLayer.hpp"
#include "../XPUtils.hpp"
#include "../DPUtils.hpp"
#include "XPPopup.hpp"
#include "../CustomText.hpp"

//geode namespace
using namespace geode::prelude;

bool XPPopup::init() {
	if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Experience");

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

	for (auto [key, value] : XPUtils::skills) {
		//Get XP
		auto xp = Mod::get()->getSavedValue<matjson::Value>("xp")[key].as<float>().unwrapOr(0.f);
		auto levelProgress = Mod::get()->getSavedValue<matjson::Value>("percent-to-level")[key].as<float>().unwrapOr(0.f);
		auto level = Mod::get()->getSavedValue<matjson::Value>("level")[key].as<int>().unwrapOr(0);
		auto maxLevel = Mod::get()->getSavedValue<matjson::Value>("max-levels")[key].as<int>().unwrapOr(0);

		auto title = CCNode::create();
		title->setPosition({ value["position"][0].as<float>().unwrapOr(0.f), value["position"][1].as<float>().unwrapOr(0.f) });
		title->setID(fmt::format("skill-{}", key));

		auto titleHeader = CCLabelBMFont::create(value["name"].asString().unwrapOr("???").c_str(), "bigFont.fnt");
		titleHeader->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
		titleHeader->setPositionY(1.f);
		titleHeader->setScale(0.75f);
		titleHeader->setColor(value["color"].as<ccColor3B>().unwrapOrDefault());
		titleHeader->setID("header");
		title->addChild(titleHeader);

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
		progressFront->setColor(value["color"].as<ccColor3B>().unwrapOrDefault());

		auto percent = levelProgress;
		if (xp >= 1.f) percent = 1.f;

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

		auto levelLabel = CCLabelBMFont::create(fmt::format("Level {}/{}", level, maxLevel).c_str(), "bigFont.fnt");
		levelLabel->setPosition({ 10.f, 11.5f });
		levelLabel->setAnchorPoint({ 0.f, 0.5f });
		levelLabel->setScale(0.65f);
		levelLabel->setZOrder(2);
		levelLabel->setID("level-label");
		progressBack->addChild(levelLabel);

		auto totalXPLabel = CCLabelBMFont::create(fmt::format("({}%)", clampf(floor(((float)level / (float)maxLevel) * 10000) / 100.f, 0, 100)).c_str(), "bigFont.fnt");
		totalXPLabel->setPosition({ 10.f + levelLabel->getScaledContentWidth(), 11.5f });
		totalXPLabel->setAnchorPoint({ 0.f, 0.5f });
		totalXPLabel->setScale(0.5f);
		totalXPLabel->setZOrder(2);
		totalXPLabel->setOpacity(150);
		totalXPLabel->setID("total-xp-label");
		progressBack->addChild(totalXPLabel);

		auto progressLabel = CCLabelBMFont::create(fmt::format("{}%", clampf(floor(percent * 100), 0, 100)).c_str(), "bigFont.fnt");
		progressLabel->setPosition({ 330.f, 11.5f });
		progressLabel->setAnchorPoint({ 1.f, 0.5f });
		progressLabel->setScale(0.65f);
		progressLabel->setZOrder(2);
		progressLabel->setID("progress-label");
		progressBack->addChild(progressLabel);

		title->addChild(progressBack);

		alignment->addChild(title);
	}

	//Least Improved Skill
	std::string leastImproved = "null";
	auto skillRev = XPUtils::skills;
	std::reverse(skillRev.begin(), skillRev.end());
	for (auto [key, value] : skillRev) {
		auto xp = Mod::get()->getSavedValue<matjson::Value>("xp")[key].as<float>().unwrapOr(0.f);
		auto liXP = Mod::get()->getSavedValue<matjson::Value>("xp")[leastImproved].as<float>().unwrapOr(1.f);
		if (xp < liXP) leastImproved = key;
	}

	auto leastImprovedTitle = CCLabelBMFont::create("Least Improved", "bigFont.fnt");
	leastImprovedTitle->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	leastImprovedTitle->setPosition({ -100.f, -135.f });
	leastImprovedTitle->setScale(0.5f);
	leastImprovedTitle->setID("least-improved-header");
	alignment->addChild(leastImprovedTitle);

	auto leastImprovedValue = CCLabelBMFont::create(XPUtils::skills[leastImproved]["name"].asString().unwrapOr("???").c_str(), "bigFont.fnt");
	leastImprovedValue->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	leastImprovedValue->setPosition({ -100.f, -150.f });
	leastImprovedValue->setScale(0.7f);
	leastImprovedValue->setColor(XPUtils::skills[leastImproved]["color"].as<ccColor3B>().unwrapOrDefault());
	leastImprovedValue->setID("least-improved-value");
	alignment->addChild(leastImprovedValue);

	//Most Improved Skill
	std::string mostImproved = "null";
	for (auto [key, value] : XPUtils::skills) {
		auto xp = Mod::get()->getSavedValue<matjson::Value>("xp")[key].as<float>().unwrapOr(0.f);
		auto miXP = Mod::get()->getSavedValue<matjson::Value>("xp")[mostImproved].as<float>().unwrapOr(0.f);
		if (xp > miXP) mostImproved = key;
	}

	auto mostImprovedTitle = CCLabelBMFont::create("Most Improved", "bigFont.fnt");
	mostImprovedTitle->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	mostImprovedTitle->setPosition({ 100.f, -135.f });
	mostImprovedTitle->setScale(0.5f);
	mostImprovedTitle->setID("most-improved-header");
	alignment->addChild(mostImprovedTitle);

	auto mostImprovedValue = CCLabelBMFont::create(XPUtils::skills[mostImproved]["name"].asString().unwrapOr("???").c_str(), "bigFont.fnt");
	mostImprovedValue->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	mostImprovedValue->setPosition({ 100.f, -150.f });
	mostImprovedValue->setScale(0.7f);
	mostImprovedValue->setColor(XPUtils::skills[mostImproved]["color"].as<ccColor3B>().unwrapOrDefault());
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

	//Get data and completed levels
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

	//Create Info Button
	auto infoMenu = CCMenu::create();
	auto infoButton = InfoAlertButton::create("Demon XP Info", "The <cy>XP</c> gained from beating <cy>Demons</c> depends on how full the bars are. Empty gives no <cy>XP</c>, full gives a lot.", 1.0f);
	infoMenu->setPosition({ 405.f, 235.f });
	infoMenu->setZOrder(2);
	infoMenu->addChild(infoButton);
	infoMenu->setID("info-menu");
	m_mainLayer->addChild(infoMenu);

	//Create Projection Toggle
	auto toggleMenu = CCMenu::create();
	toggleMenu->setPosition(m_mainLayer->getContentSize() / 2.f);
	toggleMenu->setContentSize({ 0.f, 0.f });
	toggleMenu->setID("toggle-menu");

	auto offSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
	auto onSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

	offSpr->setScale(0.5f);
	onSpr->setScale(0.5f);
	
	auto toggleBtn = CCMenuItemToggler::create(
		offSpr,
		onSpr,
		this,
		menu_selector(DemonXPPopup::onToggle)
	);
	toggleBtn->setPosition({ -190.f, -105.f });
	
	auto toggleLabel = CCLabelBMFont::create("Show Projected XP", "bigFont.fnt");
	toggleLabel->setScale(0.4f);
	toggleLabel->setAnchorPoint({ 0.f, 0.5f });
	toggleLabel->setPosition({ -180.f, -105.f });

	toggleMenu->addChild(toggleBtn);
	toggleMenu->addChild(toggleLabel);
	if (!DPUtils::containsInt(completedLvls, m_levelID)) m_mainLayer->addChild(toggleMenu);

	//Create Progress Bars

	auto alignment = CCNode::create();
	alignment->setPosition({ 210.f, 170.f });
	m_mainLayer->addChild(alignment);

	for (auto [key, value] : XPUtils::skills) {
		auto levelID = std::to_string(m_levelID);
		//log::info("id: {}, skill: {}", levelID, skill);
		auto skillValue = data["level-data"][levelID]["xp"][key].as<int>().unwrapOr(0);

		auto title = CCNode::create();
		title->setPosition({ value["position"][0].as<float>().unwrapOr(0.f), value["position"][1].as<float>().unwrapOr(0.f) });
		title->setID(fmt::format("skill-{}", key));

		auto titleHeader = CCLabelBMFont::create(value["name"].asString().unwrapOr("???").c_str(), "bigFont.fnt");
		titleHeader->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
		titleHeader->setPositionY(1.f);
		titleHeader->setScale(0.75f);
		titleHeader->setColor(value["color"].as<ccColor3B>().unwrapOrDefault());
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
		progressFront->setColor(value["color"].as<ccColor3B>().unwrapOrDefault());
		progressFront->setID("progress-bar-front");

		auto clippingNode = CCClippingNode::create();
		clippingNode->setStencil(progressFront);
		clippingNode->setAnchorPoint({ 0, 0.5f });
		clippingNode->setPosition({ 3.25f, 10.5f });
		clippingNode->setContentWidth(progressFront->getContentWidth() - 2.f);
		clippingNode->setContentHeight(20);
		clippingNode->setID("progress-bar-node");
		clippingNode->addChild(progressFront);
		progressBack->addChild(clippingNode);

		progressBack->setScaleX(0.6f);
		progressBack->setScaleY(0.65f);

		auto projProgressFront = CCSprite::create("GJ_progressBar_001.png");
		projProgressFront->setAnchorPoint({ 0, 0.5 });
		projProgressFront->setPosition({ 0.0f, 10.f });
		projProgressFront->setScaleX(0.98f);
		projProgressFront->setScaleY(0.75f);
		projProgressFront->setZOrder(1);
		projProgressFront->setColor(value["color"].as<ccColor3B>().unwrapOrDefault());
		projProgressFront->setOpacity(150);
		projProgressFront->setID("projected-progress-bar-front");

		auto projectedClippingNode = CCClippingNode::create();
		projectedClippingNode->setStencil(progressFront);
		projectedClippingNode->setAnchorPoint({ 0, 0.5f });
		projectedClippingNode->setPosition({ 3.25f, 10.5f });
		projectedClippingNode->setContentWidth(progressFront->getContentWidth() - 2.f);
		projectedClippingNode->setContentHeight(20);
		projectedClippingNode->setID("projected-progress-bar-node");
		projectedClippingNode->addChild(projProgressFront);
		progressBack->addChild(projectedClippingNode);

		auto projectedChangeLabel = CCLabelBMFont::create("No Change", "bigFont.fnt");
		projectedChangeLabel->setPosition({ 10.f, 11.5f });
		projectedChangeLabel->setAnchorPoint({ 0.f, 0.5f });
		projectedChangeLabel->setScale(0.65f);
		projectedChangeLabel->setZOrder(2);
		projectedChangeLabel->setOpacity(150);
		projectedChangeLabel->setID("projected-change-label");
		progressBack->addChild(projectedChangeLabel);

		title->addChild(progressBack);
		title->addChild(titleHeader);

		alignment->addChild(title);
	}

	update();

	return true;
}

void DemonXPPopup::onToggle(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);

	m_toggled = !btn->isToggled();

	update();
	
	return;
}

void DemonXPPopup::update() {

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto projected = (m_toggled) ? XPUtils::getProjectedXP(m_levelID) : matjson::Value();
	
	for (auto [key, value] : XPUtils::skills) {

		if (m_toggled) { // show projected xp values
			auto levelProgress = Mod::get()->getSavedValue<matjson::Value>("percent-to-level")[key].as<float>().unwrapOr(0.f);
			auto level = Mod::get()->getSavedValue<matjson::Value>("level")[key].as<int>().unwrapOr(0);

			//get nodes
			auto parent = m_mainLayer->getChildByIDRecursive(fmt::format("skill-{}", key));
			auto clippingNode = static_cast<CCClippingNode*>(parent->getChildByIDRecursive("progress-bar-node"));
			auto projectedClippingNode = static_cast<CCClippingNode*>(parent->getChildByIDRecursive("projected-progress-bar-node"));
			auto projectedChangeLabel = static_cast<CCLabelBMFont*>(parent->getChildByIDRecursive("projected-change-label"));
			auto progressFront = static_cast<CCSprite*>(clippingNode->getChildByID("progress-bar-front"));

			auto stencil = CCScale9Sprite::create("square02_001.png");
			stencil->setAnchorPoint({ 0, 0.5f });
			stencil->setContentWidth(progressFront->getScaledContentSize().width);
			stencil->setScaleX(levelProgress);
			stencil->setContentHeight(100);
			clippingNode->setStencil(stencil);

			auto pXP = projected["xp"][key].as<float>().unwrapOr(0.f);
			auto pLevel = projected["level"][key].as<int>().unwrapOr(1);
			auto pNext = projected["next"][key].as<float>().unwrapOr(0.f);

			auto projectedStencil = CCScale9Sprite::create("square02_001.png");
			projectedStencil->setAnchorPoint({ 0, 0.5f });
			projectedStencil->setContentWidth(progressFront->getScaledContentSize().width);
			projectedStencil->setScaleX((pLevel > level) ? 1.f : pNext);
			projectedStencil->setContentHeight(100);
			projectedClippingNode->setStencil(projectedStencil);

			auto diff = (pNext - levelProgress);
			if (diff > 0.f) projectedChangeLabel->setCString(fmt::format("+{}%", floorf(diff * 100)).c_str());
			if (pLevel > level) projectedChangeLabel->setCString(((pLevel - level) > 1) ? fmt::format("+Level Up x{}", (pLevel - level)).c_str() : "+Level Up");

			// show nodes
			parent->getChildByIDRecursive("projected-progress-bar-node")->setVisible(true);
			parent->getChildByIDRecursive("projected-change-label")->setVisible(true);
		}
		else { // load bars as normal
			auto skillValue = data["level-data"][std::to_string(m_levelID)]["xp"][key].as<int>().unwrapOr(0);
			auto percent = static_cast<float>(skillValue) / 3.f;

			//get nodes
			auto parent = m_mainLayer->getChildByIDRecursive(fmt::format("skill-{}", key));
			auto clippingNode = static_cast<CCClippingNode*>(parent->getChildByIDRecursive("progress-bar-node"));
			auto progressFront = static_cast<CCSprite*>(clippingNode->getChildByID("progress-bar-front"));

			auto stencil = CCScale9Sprite::create("square02_001.png");
			stencil->setAnchorPoint({ 0, 0.5f });
			stencil->setContentWidth(progressFront->getScaledContentSize().width);
			stencil->setScaleX(percent);
			stencil->setContentHeight(100);
			clippingNode->setStencil(stencil);

			// hide unneeded nodes
			parent->getChildByIDRecursive("projected-progress-bar-node")->setVisible(false);
			parent->getChildByIDRecursive("projected-change-label")->setVisible(false);
		}
	}
	
	return;
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