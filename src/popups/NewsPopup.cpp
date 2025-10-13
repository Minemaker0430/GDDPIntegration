//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/JsonValidation.hpp>

#include "../menus/DPLayer.hpp"
#include "NewsPopup.hpp"

//geode namespace
using namespace geode::prelude;

bool NewsPopup::setup() {
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("News");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	//create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	//Page Menu
	m_pagesMenu = CCMenu::create();
	m_pagesMenu->setPosition({ 210.f, 130.f });
	m_pagesMenu->setVisible(false);
	m_pagesMenu->setID("pages-menu");

	m_left = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(NewsPopup::pageLeft));
	m_left->setPositionX(-230.f);
	m_left->setVisible(false);
	m_left->setID("page-left");
	m_pagesMenu->addChild(m_left);

	auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
	rightBtnSpr->setFlipX(true);
	m_right = CCMenuItemSpriteExtra::create(rightBtnSpr, this, menu_selector(NewsPopup::pageRight));
	m_right->setPositionX(230.f);
	m_right->setVisible(false);
	m_right->setID("page-right");
	m_pagesMenu->addChild(m_right);

	m_first = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"), this, menu_selector(NewsPopup::firstPage));
	m_first->setPositionX(-265.f);
	m_first->setVisible(false);
	m_first->setID("page-first");
	m_pagesMenu->addChild(m_first);

	auto lastBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	lastBtnSpr->setFlipX(true);
	m_last = CCMenuItemSpriteExtra::create(lastBtnSpr, this, menu_selector(NewsPopup::lastPage));
	m_last->setPositionX(265.f);
	m_last->setVisible(false);
	m_last->setID("page-last");
	m_pagesMenu->addChild(m_last);

	m_mainLayer->addChild(m_pagesMenu);

	m_loadCircle = LoadingCircle::create();
	m_loadCircle->m_parentLayer = this;
	m_loadCircle->show();

	//Placeholder Text
	auto header = CCLabelBMFont::create("Header", "chatFont.fnt");
	header->setPosition({ 210.f, 200.f });
	header->setScale(1.25f);
	header->setID("header");
	m_mainLayer->addChild(header);

	auto text = CCLabelBMFont::create("Text", "chatFont.fnt");
	text->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	text->setPosition({ 210.f, 180.f });
	text->setAnchorPoint({ 0.5f, 1.f });
	text->setScale(0.8f);
	text->setID("text");
	m_mainLayer->addChild(text);

	//Load News
	m_listener.bind([&](web::WebTask::Event* e) {
		if (auto res = e->getValue()) {
			if (res->ok() && res->json().isOk()) {
				m_news = res->json().unwrapOrDefault();

				m_pagesMenu->setVisible(true);

				loadPage(0);

				m_loadCircle->fadeAndRemove();
			}
			else {
				auto alert = FLAlertLayer::create("ERROR", fmt::format("Something went wrong getting the News. ({}, {})", res->code(), res->json().isErr()), "OK");
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
	m_listener.setFilter(listReq.get("https://raw.githubusercontent.com/Minemaker0430/gddp-mod-database/main/news.json"));

	return true;
}

void NewsPopup::loadPage(int page) {
	m_right->setVisible(m_page < m_news["news"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size() - 1);
	m_left->setVisible(m_page > 0);
	m_last->setVisible(m_page < m_news["news"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size() - 1);
	m_first->setVisible(m_page > 0);

	typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("header"))->setCString(m_news["news"][page]["date"].asString().unwrapOr("0").c_str());
	typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("text"))->setCString(m_news["news"][page]["text"].asString().unwrapOr("erm that\'s awkward").c_str());
}

void NewsPopup::pageLeft(CCObject*) {
	m_page -= 1;

	loadPage(m_page);

	return;
}

void NewsPopup::pageRight(CCObject*) {
	m_page += 1;

	loadPage(m_page);

	return;
}

void NewsPopup::firstPage(CCObject*) {
	m_page = 0;

	loadPage(m_page);

	return;
}

void NewsPopup::lastPage(CCObject*) {
	m_page = m_news["news"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size() - 1;

	loadPage(m_page);

	return;
}

NewsPopup* NewsPopup::create() {
	auto ret = new NewsPopup();
	if (ret && ret->initAnchored(420.f, 250.f)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

NewsPopup::~NewsPopup() {
	this->removeAllChildrenWithCleanup(true);
}