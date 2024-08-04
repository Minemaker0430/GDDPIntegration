#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class SupportPopup : public Popup<> {
protected:
	bool setup() override;
	CCLayer* m_mainLayer;
	virtual ~SupportPopup();
public:
	static SupportPopup* create();

	void onSupport(CCObject*);
	void getSupporters(CCObject*);
};

class SupporterList : public Popup<> {
protected:
	bool setup() override;
	CCLayer* m_mainLayer;
	LoadingCircle* m_loadCircle;
	virtual ~SupporterList();

	EventListener<web::WebTask> m_listener;
public:
	static SupporterList* create();
};