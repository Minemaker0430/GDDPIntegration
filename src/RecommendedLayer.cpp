/*
How does the recommendations system work?
The recommendations system is pretty complicated. Since most people cannot understand code, the best way to explain it is with a high-level description. 
This aims to go over the specifics of how it works while glancing over minor edge cases or implementation details:

Definitions:
- highest rank: the highest tier that you have earned a non-plus rank (e.g. diamond rank = 4 completed diamond levels). If your highest rank is a plus rank, 
the recommendations system considers your highest rank as the rank above (e.g. if i have diamond+, my highest rank is onyx)
- strong skill: skills whose level divided by its max level is relatively higher than other skills
- weak skill: skills whose level divided by its max level is relatively lower than other skills
- unique recommendation: the level is recommended for only one skill
- partial rank: you completed [x-2, x) demons in a tier, where x is the required number of demons for obtaining the rank of that tier
- skill difficulty: gddp levels are rated based on the difficulty (relative to other levels in the same tier) of each skill present in that level. the difficulty can be any of the following:
not present
below average
average
above average

High-level Overview:
recommendations are deterministic, this means they are not based on probability ("rng") or AI
levels that are either below average difficulty or not present for a skill will never be recommended for that skill

if you have no ranks, the system defaults you to "having" beginner rank
this means users with no ranks will get beginner/bronze demon recommendations

for each of your skills, you will receive recommendations based on these conditions:
- strong skills: you will receive up to 4 unique recommendations for 4 of your strongest skills from any tier above your highest rank (starting with the tier above your highest rank). 
You will also receive up to 2 additional recommendations for your top 2 strongest skills from your highest partially completed rank. it will prioritize picking a level that is average 
difficulty for that skill, and if there is none it will pick a level with above average difficulty for that skill.
- weakest skill: you will receive up to 1 unique recommendation for your weakest skill from either (a) the tier below your highest rank or (b) the tier of your highest rank. In scenario (a),
it will prioritize picking a level that is above average difficulty for that skill, and if there is none it will pick a level with average difficulty for that skill. 
If you have no levels that can be chosen from scenario (a), then we pick a level from scenario (b). In scenario (b), it will prioritize picking a level that is rated equally as difficult 
as the hardest level you've beaten in that tier for the same skill, and if there is none it will pick either an easier or harder level.
- other skills: you will receive non-unique recommendations for the rest of your skills that are not classified as "strong skills" or "weakest skill". these recommendations will come from 
the tier of your highest rank. it will prioritize picking a level that is rated equally as difficult as the hardest level you've beaten in that tier for the same skill, and if there is 
none it will pick either an easier or harder level.

Other Notes:
it is possible that if you have completed all levels with at least average difficulty for a skill within the tier of your highest rank that you will receive a recommendation from any tier 
above for that skill. its very unlikely that anyone will encounter this scenario, but it (for the most part) ensures that you will receive recommendations for all of your skills
*/

//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

#include "DPLayer.hpp"
#include "StatsPopup.hpp"
#include "RecommendedLayer.hpp"
#include "Utils.hpp"

//geode namespace
using namespace geode::prelude;

void RecommendedLayer::keyBackClicked() {
	CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
};

void RecommendedLayer::backButton(CCObject* sender) {
	keyBackClicked();
};

bool RecommendedLayer::init() {
	if (!CCLayer::init()) return false;

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	auto bg = createLayerBG();
	if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
		bg->setColor({ 18, 18, 86 });
	}
	bg->setZOrder(-10);
	bg->setID("bg");
	this->addChild(bg);

	auto lCornerSprite = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
	lCornerSprite->setAnchorPoint({ 0, 0 });
	lCornerSprite->setID("left-corner-sprite");
	this->addChild(lCornerSprite);

	auto rCornerSprite = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
	rCornerSprite->setAnchorPoint({ 1, 0 });
	rCornerSprite->setPosition({ size.width, 0 });
	rCornerSprite->setFlipX(true);
	rCornerSprite->setID("right-corner-sprite");
	this->addChild(rCornerSprite);

	//back button
	auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backButton = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(RecommendedLayer::backButton));
	auto backMenu = CCMenu::create();
	backMenu->addChild(backButton);
	backMenu->setPosition({ 25, size.height - 25 });
	backMenu->setZOrder(2);
	backMenu->setID("back-menu");
	this->addChild(backMenu);

	//info button
	auto infoMenu = CCMenu::create();
	auto infoButton = InfoAlertButton::create("Recommendations", "", 1.0f);
	infoMenu->setPosition({ 25, 25 });
	infoMenu->setZOrder(2);
	infoMenu->addChild(infoButton);
	infoMenu->setID("info-menu");
	this->addChild(infoMenu);

	//reload menu
	auto reloadMenu = CCMenu::create();
	reloadMenu->setPosition({ size.width - 30, size.height - 30 });
	auto reloadBtnSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
	auto reloadBtn = CCMenuItemSpriteExtra::create(reloadBtnSprite, this, menu_selector(RecommendedLayer::reloadLevels));
	reloadBtn->setPosition({ 0, 0 });
	reloadMenu->addChild(reloadBtn);
	reloadMenu->setID("reload-menu");
	this->addChild(reloadMenu);

	m_list = GJListLayer::create(CustomListView::create(CCArray::create(), BoomListType::Level, 220.0f, 358.0f), "", { 194, 114, 62, 255 }, 358.0f, 220.0f, 0);
	m_list->setZOrder(2);
	m_list->setPosition(size / 2 - m_list->getContentSize() / 2);
	this->addChild(m_list);

	loadLevels();

	this->setKeyboardEnabled(true);
	this->setKeypadEnabled(true);

	return true;
}

void RecommendedLayer::reloadLevels(CCObject* sender) {
	if (m_levelsLoaded) {
		loadLevels();
	}

	return;
}

void RecommendedLayer::loadLevels() {

	m_levelsLoaded = false;

	m_loadCircle = LoadingCircle::create();
	m_loadCircle->m_parentLayer = this;
	m_loadCircle->show();

	m_list->m_listView->setVisible(false);

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	//get highest rank
	auto highestRank = 0;
	for (int i = data["main"].as_array().size(); i > 0; i--) {
		if (StatsPopup::getPercentToRank(i, false) > 1.f) { //check what the highest rank is
			highestRank = i;
			break;
		}
		else if (StatsPopup::getPercentToRank(i - 1, true) > 1.f && i > 0) { //if you don't have the highest rank at i, check if you have the previous plus rank
			highestRank = i;
			break;
		}
	}

	auto skillsets = Mod::get()->getSavedValue<matjson::Value>("skillset-info");
	auto completedLvls = Mod::get()->getSavedValue<matjson::Value>("completed-levels");

	//sort skills
	/*std::vector<std::string, int> skills = {}; //lowest to highest
	int lowestValue = 0;
	for (int j = 0; j < completedLvls.as_array().size(); j++) {

		auto lvlData = data["level-data"][std::to_string(completedLvls[j].as_int())];
		for (int k = 0; k < lvlData["skillsets"].as_array().size(); k++) {
			
			if (!(lvlData["skillsets"][k].as_string() == "unknown" || lvlData["skillsets"][k].as_string() == "overall" || string::startsWith(lvlData["skillsets"][k].as_string(), "sp-"))) { //skip first two + special

				//create a new entry if it doesn't exist
				if (std::find(skills.begin(), skills.end(), lvlData["skillsets"][k].as_string()) == skills.end()) {
					skills.push_back({ lvlData["skillsets"][k].as_string(), 1 });
				}
				else { //continue as normal
					//skills[lvlData["skillsets"][k].as_string(), 0]
				}

			}

		}
	}*/

	//borrowed some stuff from integrated demon list
	auto glm = GameLevelManager::sharedState();
	glm->m_levelManagerDelegate = this;
	auto results = std::vector<std::string>();
	auto searchObject = GJSearchObject::create(SearchType::Type19, string::join(results, ","));
	auto storedLevels = glm->getStoredOnlineLevels(searchObject->getKey());

	log::info("{}", searchObject);
	log::info("{}", searchObject->getKey());
	log::info("{}", storedLevels);

	if (storedLevels) {
		loadLevelsFinished(storedLevels, "");
	}
	else
	{
		glm->getOnlineLevels(searchObject);
	}

	return;
}

void RecommendedLayer::loadLevelsFinished(CCArray* levels, const char*) {

	m_levelsLoaded = true;
	m_list->m_listView->setVisible(true);

	m_loadCircle->fadeAndRemove();

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	if (m_list->getParent() == this) { this->removeChild(m_list); }

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	m_list = GJListLayer::create(CustomListView::create(levels, BoomListType::Level, 220.0f, 358.0f), "Recommendations", {194, 114, 62, 255}, 358.0f, 220.0f, 0);
	m_list->setZOrder(2);
	m_list->setPosition(size / 2 - m_list->getContentSize() / 2);
	this->addChild(m_list);

	return;
}

void RecommendedLayer::loadLevelsFailed(const char*) {
	m_levelsLoaded = true;

	m_loadCircle->fadeAndRemove();

	auto alert = FLAlertLayer::create("ERROR", "Failed to load levels. Please try again later.", "OK");
	alert->setParent(this);
	alert->show();

	return;
}

RecommendedLayer* RecommendedLayer::create() {
	auto pRet = new RecommendedLayer();
	if (pRet && pRet->init()) {
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet); //don't crash if it fails
	return nullptr;
}

RecommendedLayer::~RecommendedLayer() {
	this->removeAllChildrenWithCleanup(true);
}