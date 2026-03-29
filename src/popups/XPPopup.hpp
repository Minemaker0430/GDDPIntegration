#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class XPPopup : public Popup {
protected:
	bool init() override;
	CCLayer* m_mainLayer;
	virtual ~XPPopup();
public:
	static XPPopup* create();
};

class DemonXPPopup : public Popup {
protected:
	bool init() override;
	
	CCLayer* m_mainLayer;

	void onToggle(CCObject*);
	void update();

	bool m_toggled = false;

	virtual ~DemonXPPopup();
public:
	int m_levelID = 0;
	static DemonXPPopup* create(int);
};