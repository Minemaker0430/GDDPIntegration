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

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	//create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

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
	supportMenu->setID("support-menu");

	auto supportBtnSpr = ButtonSprite::create("Support", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto supportBtn = CCMenuItemSpriteExtra::create(supportBtnSpr, this, menu_selector(SupportPopup::onSupport));
	//supportBtn->setPosition({-275.f, -125.f });
	//supportBtn->setAnchorPoint({ 0.f, 0.5f });
	supportBtn->setPosition({-75.f, -125.f });
	supportBtn->setID("support-button");
	supportMenu->addChild(supportBtn);

	auto supporterBtnSpr = ButtonSprite::create("Supporters", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto supporterBtn = CCMenuItemSpriteExtra::create(supporterBtnSpr, this, menu_selector(SupportPopup::getSupporters));
	supporterBtn->setPosition({ 125.f, -125.f });
	supporterBtn->setAnchorPoint({ 1.f, 0.5f });
	supporterBtn->setID("supporters-button");
	//supportMenu->addChild(supporterBtn);

	m_mainLayer->addChild(supportMenu);

	return true;
}

void SupportPopup::onSupport(CCObject*) {
	web::openLinkInBrowser("https://ko-fi.com/itsmochatheotter");
}
void SupportPopup::getSupporters(CCObject*) {
	SupporterList::create()->show();
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

//SUPPORTER LIST

bool SupporterList::init() {
	if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Supporters");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	//create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	//Create Loading circle
	m_loadCircle = LoadingCircle::create();
	m_loadCircle->m_parentLayer = m_mainLayer;
	m_loadCircle->show();

	//Get Data
	/*m_listener.bind([&](web::WebTask::Event* e) {
		if (auto res = e->getValue()) {
			if (res->ok() && res->json().isOk()) {
				auto list = res->json().unwrapOrDefault();
				
				auto fullList = CCNode::create();

				for (auto supporter : list.asArray().unwrap()) {
					fullList->addChild(CCLabelBMFont::create(supporter.asString().unwrapOr("???").c_str(), "chatFont.fnt"));
				}

				listText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
				listText->setPositionX(205.f);
				listText->setAnchorPoint({ 0.5f, 1.f });
				auto listScroll = ScrollLayer::create({ 410.f, 200.f }, true, true);
				listScroll->setPosition({ 5.f, 12.5f });
				listScroll->m_contentLayer->setContentHeight(std::max(listScroll->getContentHeight(), fullList->getContentHeight()));
				listText->setPositionY(listScroll->m_contentLayer->getContentHeight());
				listScroll->m_contentLayer->addChild(listText);
				listScroll->scrollToTop();

				//m_mainLayer->addChild(listScroll);

				m_loadCircle->fadeAndRemove();
			}
			else {
				auto alert = FLAlertLayer::create("ERROR", fmt::format("Something went wrong getting the Supporter List. ({}, {})", res->code(), res->json().isErr()), "OK");
				alert->m_scene = this;
				alert->show();

				m_loadCircle->fadeAndRemove();
			}
		}
		else if (e->isCancelled()) {
			log::info("Cancelled request.");
		}
	});

	auto listReq = web::WebRequest();
	m_listener.setFilter(listReq.get("https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/kofi-supporters.json"));*/

	return true;
}

SupporterList* SupporterList::create() {
	auto ret = new SupporterList();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

SupporterList::~SupporterList() {
	this->removeAllChildrenWithCleanup(true);
}