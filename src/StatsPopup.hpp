#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class StatsPopup : public Popup<> {
protected:
	bool setup() override;
	virtual ~StatsPopup();
public:
	static StatsPopup* create();
};