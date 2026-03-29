#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class SearchPopup : public Popup {
protected:
	bool init() override;
	virtual ~SearchPopup();

	CCMenu* m_tabs;
	CCLayer* m_mainLayer;
	ListView* m_list;

	CCArrayExt<CCMenuItemToggler*> m_tabBtns;

	int m_currentTab = 0;

	matjson::Value m_filter = matjson::makeObject({
		{"completed", true},
		{"uncompleted", true},
		{"difficulties", matjson::Value()},
		{"packs", matjson::Value()},
		{"skills", matjson::Value()},
		{"xp", matjson::Value()}
	});

	/*std::vector<bool> m_difficulties = { true };
	std::vector<bool> m_packs = { true };
	std::vector<bool> m_skills = { true };
	std::vector<int> m_xp = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 0 = Min, 3 = Max
	std::vector<bool> m_xpToggle = { false, false, false, false, false, false, false, false, false, false };
	std::vector<int> m_xpMode = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }; //0 = GreaterEquals, 1 = Greater, 2 = Equals, 3 = Less, 4 = LessEquals
	bool m_completed = true;
	bool m_uncompleted = true;*/
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

struct SetToggleValue : public CCObject {
    std::string m_id;

    SetToggleValue(std::string id) : m_id(id) {
        // Always remember to call autorelease on your classes!
        this->autorelease();
    }
};

struct SetXPValue : public CCObject {
    std::string m_id;
	int m_value;

    SetXPValue(std::string id, int value) : m_id(id), m_value(value) {
        // Always remember to call autorelease on your classes!
        this->autorelease();
    }
};