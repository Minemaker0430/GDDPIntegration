#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class NewsPopup : public Popup<> {
protected:
	bool setup() override;
	CCLayer* m_mainLayer;
	LoadingCircle* m_loadCircle;
	
	matjson::Value m_news;
	int m_page = 0;

	CCMenu* m_pagesMenu;
	CCMenuItemSpriteExtra* m_left;
	CCMenuItemSpriteExtra* m_right;
	CCMenuItemSpriteExtra* m_first;
	CCMenuItemSpriteExtra* m_last;

	virtual ~NewsPopup();

	EventListener<web::WebTask> m_listener;
public:
	static NewsPopup* create();

	void loadPage(int);

	void pageRight(CCObject*);
	void pageLeft(CCObject*);
	void firstPage(CCObject*);
	void lastPage(CCObject*);
};