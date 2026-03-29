#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class StatsPopup : public Popup {
protected:
	bool init() override;
	CCMenu* m_tabs;
	CCLayer* m_mainLayer;
	virtual ~StatsPopup();
public:
	static StatsPopup* create();
	void loadTab(int);
	void onTab(CCObject*);
	int getScore();
	static float getPercentToRank(int, bool);

	void medalInfoCallback(CCObject*); //on medal info
	void rankInfoCallback(CCObject*); //on rank info
	void onScoreInfo(CCObject*); //on score info
	void onMonthlyInfo(CCObject*); //on monthly info
};

enum class StatsTab {
	Main,
	Ranks,
	Medals,
};