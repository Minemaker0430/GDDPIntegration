#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class SupportPopup : public Popup {
protected:
	bool init() override;
	CCLayer* m_mainLayer;
	virtual ~SupportPopup();
public:
	static SupportPopup* create();

	void onSupport(CCObject*);
	void getSupporters(CCObject*);
};

class SupporterList : public Popup {
protected:
	bool init() override;
	CCLayer* m_mainLayer;
	LoadingCircle* m_loadCircle;
	virtual ~SupporterList();

	async::TaskHolder<web::WebResponse> m_listener;
public:
	static SupporterList* create();
};