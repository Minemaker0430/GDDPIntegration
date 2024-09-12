#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class SearchPopup : public Popup<> {
protected:
	bool setup() override;
	virtual ~SearchPopup();

	CCMenu* m_tabs;
	CCLayer* m_mainLayer;
	ListView* m_list;

	int m_currentTab = 0;
	float m_offset = 1.f;

	matjson::Array m_difficulties = { true };
	matjson::Array m_packs = { true };
	matjson::Array m_skills = { true };
	matjson::Array m_xp = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 0 = Min, 3 = Max
	matjson::Array m_xpToggle = { false, false, false, false, false, false, false, false, false, false };
	matjson::Array m_xpMode = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }; //0 = GreaterEquals, 1 = Greater, 2 = Equals, 3 = Less, 4 = LessEquals
	bool m_completed = true;
	bool m_uncompleted = true;
public:
	static SearchPopup* create();
	void loadTab(int);
	void restoreFilters();
	void saveFilters();

	void onTab(CCObject*);
	void onSearch(CCObject*); //Opens the search layer with the selected filters
	void onToggle(CCObject*); //Updates Filters
	void onXpValue(CCObject*); //When the XP Search Value is changed
	void onXpMode(CCObject*); //When XP Search Modes are changed
	void checkAll(CCObject*); //Checks all Boxes in the current tab
	void uncheckAll(CCObject*); //Unchecks all Boxes in the current tab
};

enum class SearchModes {
	Difficulty,
	Packs,
	Skills,
	XP
};