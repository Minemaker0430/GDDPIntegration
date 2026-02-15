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

const std::vector<std::string> NormalMedals = { //String is the Medal Name, int is the Rank Requirement
	"ADEPT",
	"PROFESSIONAL",
	"MASTER",
	"GRANDMASTER",
	"GODMASTER"
};

const std::vector<int> NormalMedalRequirements = {
	6,
	8,
	10,
	12,
	14
};

const std::vector<ccColor3B> NormalMedalColors = {
	{ 107, 130, 255 },
	{ 158, 97, 255 },
	{ 255, 137, 107 },
	{ 255, 207, 119 },
	{ 0, 255, 255 }
};

const std::vector<std::string> NormalMedalDescriptions = {
	"Get the normal ranks from Beginner to Sapphire",
	"Get the normal ranks from Beginner to Emerald",
	"Get the normal ranks from Beginner to Diamond",
	"Get the normal ranks from Beginner to Onyx",
	"Get the normal ranks from Beginner to Azurite"
};

const std::vector<std::string> PlusMedals = { //String is the Medal Name, int is the Rank Requirement
	"EXPERIENCED",
	"EXEMPLARY",
	"EXCELLENT",
	"EXCEPTIONAL",
	"EXTRAORDINARY",
	"EXPERT",
	"EXCESSIVE",
	"EXTREME",
	"EXALTED",
	"PERFECTION" //ABSOLUTE PERFECTION is a special case, thus it is exempt.
};

const std::vector<int> PlusMedalRequirements = {
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13
};

const std::vector<ccColor3B> PlusMedalColors = {
	{ 158, 255, 96 },
	{ 116, 255, 156 },
	{ 89, 236, 255 },
	{ 73, 172, 255 },
	{ 63, 122, 255 },
	{ 118, 92, 255 },
	{ 98, 36, 255 },
	{ 115, 0, 255 },
	{ 181, 67, 255 },
	{ 255, 90, 255 }
};

const std::vector<std::string> PlusMedalDescriptions = {
	"Achieve every rank from Beginner to Amber+",
	"Achieve every rank from Beginner to Platinum+",
	"Achieve every rank from Beginner to Sapphire+",
	"Achieve every rank from Beginner to Jade+",
	"Achieve every rank from Beginner to Emerald+",
	"Achieve every rank from Beginner to Ruby+",
	"Achieve every rank from Beginner to Diamond+",
	"Achieve every rank from Beginner to Pearl+",
	"Achieve every rank from Beginner to Onyx+",
	"Achieve every rank from Beginner to Amethyst+"
};