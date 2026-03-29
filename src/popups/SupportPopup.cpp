//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/utils/web.hpp>

#include "../menus/DPLayer.hpp"
#include "SupportPopup.hpp"

//geode namespace
using namespace geode::prelude;

bool SupportPopup::init() {
	if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Support Me!");

	//Add Text
	auto text = CCLabelBMFont::create("Hi there!\n\nThank you for playing the mod.\nThis mod takes me a lot of time and effort to make,\n and I'm still trying to constantly improve it wherever I can.\n\nPlease consider donating to help me out, even if it's just $1,\n it really helps. No pressure of course.\n\nDonation button is below.\n\nThanks! ~ Mocha", "bigFont.fnt");
	text->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	text->setScale(0.375f);
	text->setPosition({ 210.f, 215.f });
	text->setAnchorPoint({ 0.5f, 1.f });
	text->setID("support-text");
	m_mainLayer->addChild(text);

	//Support Buttons
	auto supportMenu = CCMenu::create();
	supportMenu->setPosition(m_mainLayer->getContentSize() / 2.f);
	supportMenu->setContentSize({ 0.f, 0.f });
	supportMenu->setID("support-menu");

	auto supportBtnSpr = ButtonSprite::create("Support", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto supportBtn = CCMenuItemSpriteExtra::create(supportBtnSpr, this, menu_selector(SupportPopup::onSupport));
	supportBtn->setPosition({0.f, -90.f });
	supportBtn->setID("support-button");
	supportMenu->addChild(supportBtn);

	m_mainLayer->addChild(supportMenu);

	return true;
}

void SupportPopup::onSupport(CCObject*) {
	web::openLinkInBrowser("https://ko-fi.com/itsmochatheotter");
	return;
}

SupportPopup* SupportPopup::create() {
	auto ret = new SupportPopup();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

SupportPopup::~SupportPopup() {
	this->removeAllChildrenWithCleanup(true);
}