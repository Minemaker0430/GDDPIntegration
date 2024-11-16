#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class RoulettePopup : public Popup<> {
protected:
	bool setup() override;
	virtual ~RoulettePopup();

	CCLayer* m_mainLayer;
	ListView* m_list;

	void loadSaveMenu();
	void placeholder(CCObject*);
	void onNewSave(CCObject*);
	void onDeleteSave(CCObject*);
	void onCopy(CCObject*);
	void onExport(CCObject*);
public:
	static RoulettePopup* create();
};