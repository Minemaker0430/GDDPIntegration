#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class SupportPopup : public Popup {
protected:
	bool init() override;
	virtual ~SupportPopup();
public:
	static SupportPopup* create();

	void onSupport(CCObject*);
};