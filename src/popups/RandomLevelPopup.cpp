// geode header
#include <Geode/Geode.hpp>

#include <Geode/ui/SliderNode.hpp>

#include "RandomLevelPopup.hpp"
#include "../DPUtils.hpp"

// geode namespace
using namespace geode::prelude;

bool RandomLevelPopup::init() {
    if (!Popup::init(420.f, 250.f)) return false;

    m_filter = Mod::get()->getSavedValue<matjson::Value>("rlp-filter");

    this->setTitle("Random Level Picker");

    auto toggleMenu = CCMenu::create();
    toggleMenu->setContentSize({ 0.f, 0.f });
    toggleMenu->setPosition(m_mainLayer->getContentSize() / 2.f);
    toggleMenu->setID("toggle-menu");
    m_mainLayer->addChild(toggleMenu);
    
    auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

    auto uncompletedToggleBtn = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(RandomLevelPopup::onToggle));
    uncompletedToggleBtn->setPositionY(65.f);
    uncompletedToggleBtn->toggle(m_filter["uncompleted"].asBool().unwrapOr(true));
    uncompletedToggleBtn->setID("uncompleted-toggle-btn");
    toggleMenu->addChild(uncompletedToggleBtn);

    auto uncompletedLabel = CCLabelBMFont::create("Uncompleted Levels", "bigFont.fnt");
    uncompletedLabel->setPositionY(65.f);
    uncompletedLabel->setScale(0.65f);
    uncompletedLabel->setID("uncompleted-label");
    toggleMenu->addChild(uncompletedLabel);

    auto completedToggleBtn = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(RandomLevelPopup::onToggle));
    completedToggleBtn->setPositionY(15.f);
    completedToggleBtn->setTag(1);
    completedToggleBtn->toggle(m_filter["completed"].asBool().unwrapOr(true));
    completedToggleBtn->setID("completed-toggle-btn");
    toggleMenu->addChild(completedToggleBtn);

    auto completedLabel = CCLabelBMFont::create("Completed Levels", "bigFont.fnt");
    completedLabel->setPositionY(15.f);
    completedLabel->setScale(0.65f);
    completedLabel->setID("completed-label");
    toggleMenu->addChild(completedLabel);

    uncompletedLabel->setPositionX((uncompletedToggleBtn->getContentWidth() / 2.f) + 5.f);
    uncompletedToggleBtn->setPositionX(-(uncompletedLabel->getScaledContentWidth() / 2.f) - 5.f);

    completedLabel->setPositionX((completedToggleBtn->getContentWidth() / 2.f) + 5.f);
    completedToggleBtn->setPositionX(-(completedLabel->getScaledContentWidth() / 2.f) - 5.f);

    auto textInputMenu = CCMenu::create();
    textInputMenu->setContentSize({ 0.f, 0.f });
    textInputMenu->setPosition(m_mainLayer->getContentSize() / 2.f);
    textInputMenu->setID("text-input-menu");
    m_mainLayer->addChild(textInputMenu);

    auto weightInput = TextInput::create(50.f, "...");
    weightInput->setPosition(-90.f, -60.f);
    weightInput->setID("weight-input");
    textInputMenu->addChild(weightInput);

    auto offsetInput = TextInput::create(50.f, "...");
    offsetInput->setPosition(90.f, -60.f);
    offsetInput->setID("offset-input");
    textInputMenu->addChild(offsetInput);

    auto sliderMenu = CCMenu::create();
    sliderMenu->setContentSize({ 0.f, 0.f });
    sliderMenu->setPosition(m_mainLayer->getContentSize() / 2.f);
    sliderMenu->setID("slider-menu");
    m_mainLayer->addChild(sliderMenu);

    auto weightSlider = SliderNode::create([&](SliderNode*, const float& value) {
        if (round(value) != m_filter["weight"].as<int>().unwrapOr(50)) {
            m_filter.set("weight", round(value));
            Mod::get()->setSavedValue<matjson::Value>("rlp-filter", m_filter);
        }
    });
    weightSlider->setValue(m_filter["weight"].as<int>().unwrapOr(50));
    weightSlider->setPosition(-90.f, -90.f);
    weightSlider->setContentWidth(150.f);
    weightSlider->linkTextInput(weightInput, 0);
    weightSlider->setID("weight-slider");
    sliderMenu->addChild(weightSlider);

    auto offsetSlider = SliderNode::create([&](SliderNode*, const float& value) {
        if (round(value) != m_filter["offset"].as<int>().unwrapOr(50)) {
            m_filter.set("offset", round(value));
            Mod::get()->setSavedValue<matjson::Value>("rlp-filter", m_filter);
        }
    });
    offsetSlider->setValue(m_filter["offset"].as<int>().unwrapOr(50));
    offsetSlider->setPosition(90.f, -90.f);
    offsetSlider->setContentWidth(150.f);
    offsetSlider->linkTextInput(offsetInput, 0);
    offsetSlider->setID("offset-slider");
    sliderMenu->addChild(offsetSlider);

    auto weightLabel = CCLabelBMFont::create("Weight", "goldFont.fnt");
    weightLabel->setScale(0.65f);
    weightLabel->setPosition({ (m_mainLayer->getContentWidth() / 2.f) - 90.f, (m_mainLayer->getContentHeight() / 2.f) - 30.f});
    weightLabel->setID("weight-label");
    m_mainLayer->addChild(weightLabel);

    auto offsetLabel = CCLabelBMFont::create("Offset", "goldFont.fnt");
    offsetLabel->setScale(0.65f);
    offsetLabel->setPosition({ (m_mainLayer->getContentWidth() / 2.f) + 90.f, (m_mainLayer->getContentHeight() / 2.f) - 30.f});
    offsetLabel->setID("offset-label");
    m_mainLayer->addChild(offsetLabel);

    auto infoMenu = CCMenu::create();
    infoMenu->setContentSize({ 0.f, 0.f });
    infoMenu->setPosition(m_mainLayer->getContentSize() / 2.f);
    infoMenu->setID("info-menu");
    m_mainLayer->addChild(infoMenu);

    auto infoButton = InfoAlertButton::create(
        "Random Level Picker Info", 
        "Picks a random level depending on your settings.\nUse <cy>Weight</c> and <cy>Offset</c> for more precise results.", 
        0.8f
    );
    infoButton->setPosition(190.f, 105.f);
    infoButton->setID("info-btn");
    infoMenu->addChild(infoButton);

    auto weightInfoButton = InfoAlertButton::create(
        "Weight Info", 
        "Changes the likeliness that you\'ll get a level from around <cy>Offset</c>.\n\n0 - <cr>Unlikely</c>\n50 - <cg>Equal Chances</c>\n100 - <cl>Very Likely</c>", 
        0.5f
    );
    weightInfoButton->setPosition((weightLabel->getPositionX() + (weightLabel->getScaledContentWidth() / 2.f) + 5.f) - (m_mainLayer->getContentWidth() / 2.f), weightLabel->getPositionY() - (m_mainLayer->getContentHeight() / 2.f));
    weightInfoButton->setID("weight-info-btn");
    infoMenu->addChild(weightInfoButton);

    auto offsetInfoButton = InfoAlertButton::create(
        "Offset Info", 
        "Changes which end of the spectrum of levels to choose from.\n(Note: This will not do anything if <cy>Weight</c> is <cg>50</c>.)\n\n0 - <cl>First</c>\n50 - <cg>Middle</c>\n100 - <cr>Last</c>", 
        0.5f
    );
    offsetInfoButton->setPosition((offsetLabel->getPositionX() + (offsetLabel->getScaledContentWidth() / 2.f) + 5.f) - (m_mainLayer->getContentWidth() / 2.f), offsetLabel->getPositionY() - (m_mainLayer->getContentHeight() / 2.f));
    offsetInfoButton->setID("offset-info-btn");
    infoMenu->addChild(offsetInfoButton);

    auto confirmMenu = CCMenu::create();
    confirmMenu->setContentSize({ 0.f, 0.f });
    confirmMenu->setPosition(m_mainLayer->getContentSize() / 2.f);
    confirmMenu->setID("confirm-menu");
    m_mainLayer->addChild(confirmMenu);

    auto confirmSpr = ButtonSprite::create("Confirm", "bigFont.fnt", "GJ_button_01.png", 0.65f);
    auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(RandomLevelPopup::onConfirm));
    confirmBtn->setPositionY(-(m_mainLayer->getContentHeight() / 2.f));
    confirmBtn->setID("confirm-btn");
    confirmMenu->addChild(confirmBtn);

    return true;
}

void RandomLevelPopup::onToggle(CCObject* target) {
    auto btn = static_cast<CCMenuItemToggler*>(target);
    if (btn->getTag() == 1)  m_filter.set("completed", !btn->isToggled());
    else m_filter.set("uncompleted", !btn->isToggled());
    Mod::get()->setSavedValue<matjson::Value>("rlp-filter", m_filter);
    
    return;
}

void RandomLevelPopup::onConfirm(CCObject*) {
    /*
    Formula:
    f(x) = -(x - o)(xw - ow) + 1
    -0.5 <= w <= 0.5
    -0.5 <= o <= 0.5
    
    ALGEBRA JUMPSCARE

    Ok so basically:
    w = (W / 100) - 0.5
    o = (O / 100) - 0.5
    ^^^ This gives us numbers that are better to work with.

    Then iterate through the list
    
    First we take the position of the iterator + 1, and divide it by the list size to get a percentage of where it's at
    We then subtract 0.5 to keep it in the middle where we want
    p = ((i + 1) / list.size()) - 0.5

    Then we plug p into the formula, and multiply the result by 100 to give us cleaner numbers, then subtract by 50 (the lowest value you can possibly get)
    P = floor(f(p) * 100) - 50

    So now all of the levels should be at a value of at least 0
    We then subtract every value of P by the lowest P in the list, then add 1 so every value is at least 1
    **We want the levels with the lowest values to be the rarest possible picks.**
    
    We then add each level P times into the random number picker, and roll for what level we get
    */

    auto completed = m_filter["completed"].asBool().unwrapOr(true);
    auto uncompleted = m_filter["uncompleted"].asBool().unwrapOr(true);

    std::vector<int> levelList;

    if (completed != uncompleted) {
        DPUtils::verifyCompletedLevels();
        auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
        for (auto lvl : m_levels) {
            if ((DPUtils::containsInt(completedLvls, lvl) && completed) || (!DPUtils::containsInt(completedLvls, lvl) && uncompleted)) levelList.push_back(lvl);
        }
    }
    else  levelList = m_levels;

    if (levelList.empty()) {
        auto alert = FLAlertLayer::create("Whoops!", "You don't have eny levels that match your <cy>Completed</c>/<cy>Uncompleted</c> Filter!", "OK");
        alert->setParent(this);
        alert->show();
        return;
    }

    auto weight = (m_filter["weight"].as<float>().unwrapOr(50.f) / 100.f) - 0.5f;
    auto offset = (m_filter["offset"].as<float>().unwrapOr(50.f) / 100.f) - 0.5f;

    auto values = std::vector<int>(levelList.size(), 0);

    for (int i = 0; i < values.size(); i++) {
        auto x = ((i + 1.f) / (float)values.size()) - 0.5;
        values[i] = floor((-((x - offset) * ((x * weight) - (offset * weight))) + 1.f) * 100) - 50;
    }

    //log::info("distribution: {}", values);

    auto lowest = 100;
    for (auto i : values) if (i < lowest) lowest = i;
    for (int i = 0; i < values.size(); i++) values[i] = (values[i] - lowest);

    //log::info("distribution (after subtraction): {}", values);

    std::vector<int> weightedLevels;
    for (int i = 0; i < levelList.size(); i++) for (int j = 0; j <= values[i]; j++) weightedLevels.push_back(levelList[i]);

    std::seed_seq seed{rand()};
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distribution(0, weightedLevels.size() - 1);

    auto res = weightedLevels[distribution(gen)];
    //log::info("level result: {}", res);

    // put up loading screen and get result

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
	auto searchObject = GJSearchObject::create(SearchType::Type19, std::to_string(res));
	auto storedLevels = glm->getStoredOnlineLevels(searchObject->getKey());

	if (storedLevels) {
		loadLevelsFinished(storedLevels, "");

        m_loadCircle->fadeAndRemove();
        this->removeAllChildrenWithCleanup(true);
        this->removeMeAndCleanup();
	}
	else glm->getOnlineLevels(searchObject);

    return;
}

void RandomLevelPopup::loadLevelsFinished(CCArray* levels, const char*) {

	auto lvl = typeinfo_cast<GJGameLevel*>(levels->objectAtIndex(0));

    auto scene = CCScene::create();								// creates the scene
	auto layer = LevelInfoLayer::create(lvl, false); // creates the layer

	scene->addChild(layer);

	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene)); // push transition

    m_loadCircle->fadeAndRemove();
    this->getChildByID("loading-bg"_spr)->removeMeAndCleanup();

	return;
}

void RandomLevelPopup::loadLevelsFailed(const char*) {

	auto alert = FLAlertLayer::create("ERROR", "Failed to pick a random level.", "OK");
	alert->setParent(this);
	alert->show();

    m_loadCircle->fadeAndRemove();
    this->removeMeAndCleanup();

	return;
}

RandomLevelPopup* RandomLevelPopup::create(std::vector<int> levels) {
	auto ret = new RandomLevelPopup();
    ret->m_levels = levels;
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

RandomLevelPopup::~RandomLevelPopup() {
	auto glm = GameLevelManager::sharedState();
	if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
	this->removeAllChildrenWithCleanup(true);
}