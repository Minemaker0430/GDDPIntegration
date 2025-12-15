//geode header
#include <Geode/Geode.hpp>

//other headers
#include "OpenStartposPopup.hpp"

bool OpenStartposPopup::init() {
    if (!CCLayer::init()) return false;

    auto size = CCDirector::sharedDirector()->getWinSize();

    auto dimBG = CCLayerColor::create({ 0, 0, 0, 135 });
    dimBG->setContentSize(size);
    dimBG->setID("loading-bg"_spr);
    this->addChild(dimBG);

    m_loadCircle = LoadingCircle::create();
    m_loadCircle->setZOrder(101);
    m_loadCircle->setID("loading-circle"_spr);
    m_loadCircle->m_parentLayer = this;
    m_loadCircle->show();

    auto glm = GameLevelManager::sharedState();
	glm->m_levelManagerDelegate = this;
	auto searchObject = GJSearchObject::create(SearchType::Type19, std::to_string(m_levelID));
	auto storedLevels = glm->getStoredOnlineLevels(searchObject->getKey());

	if (storedLevels)
	{
		loadLevelsFinished(storedLevels, "");

        m_loadCircle->fadeAndRemove();
        this->removeAllChildrenWithCleanup(true);
        this->removeMeAndCleanup();
	}
	else
	{
		glm->getOnlineLevels(searchObject);
	}

    return true;
}

void OpenStartposPopup::loadLevelsFinished(CCArray* levels, const char*) {

	auto lvl = typeinfo_cast<GJGameLevel*>(levels->objectAtIndex(0));

    auto scene = CCScene::create();								// creates the scene
	auto layer = LevelInfoLayer::create(lvl, false); // creates the layer

	scene->addChild(layer);

	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition

    m_loadCircle->fadeAndRemove();
    this->removeMeAndCleanup();

	return;
}

void OpenStartposPopup::loadLevelsFailed(const char*) {

	auto alert = FLAlertLayer::create("ERROR", fmt::format("Failed to load StartPos copy with ID {}. Please try again later.", m_levelID), "OK");
	alert->setParent(this);
	alert->show();

    m_loadCircle->fadeAndRemove();
    this->removeMeAndCleanup();

	return;
}

OpenStartposPopup::~OpenStartposPopup() {
	this->removeAllChildrenWithCleanup(true);
}

OpenStartposPopup* OpenStartposPopup::create(int lvlID) {
	auto pRet = new OpenStartposPopup();
    pRet->m_levelID = lvlID;
	if (pRet && pRet->init()) {
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet); //don't crash if it fails
	return nullptr;
}