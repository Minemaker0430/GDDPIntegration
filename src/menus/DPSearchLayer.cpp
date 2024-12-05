//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/JsonValidation.hpp>

#include "DPLayer.hpp"
#include "DPSearchLayer.hpp"
#include "../Utils.hpp"

//geode namespace
using namespace geode::prelude;

void DPSearchLayer::keyBackClicked() {
	m_loadingCancelled = true;

	CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
};

void DPSearchLayer::backButton(CCObject* sender) {
	keyBackClicked();
};

std::vector<int> DPSearchLayer::compareDifficulty(matjson::Value data, std::vector<int> IDs) {
	
	//assign difficulties to key
	std::vector<CompareDifficulty> out;

	for (int level : IDs) {
		out.push_back(CompareDifficulty(level, data["level-data"][std::to_string(level)]["difficulty"].as<int>().unwrapOr(0)));
	}

	//sort
	std::sort(out.begin(), out.end());

	//convert to int vector and return
	std::vector<int> result;
	for (CompareDifficulty value : out) {
		result.push_back(value.key);
	}

	return result;
}

std::vector<int> DPSearchLayer::compareName(matjson::Value data, std::vector<int> IDs) {
	//assign names to key
	std::vector<CompareName> out;

	for (int level : IDs) {
		std::string lvlName = data["level-data"][std::to_string(level)]["name"].asString().unwrapOr("-");
		std::transform(lvlName.begin(), lvlName.end(), lvlName.begin(), [](unsigned char c) { return std::tolower(c); }); 
		//level name NEEDS to be converted to lowercase otherwise levels with capital letters get priority and we don't want that
		out.push_back(CompareName(level, lvlName));
	}

	//sort
	std::sort(out.begin(), out.end());

	//convert to int vector and return
	std::vector<int> result;
	for (CompareName value : out) {
		result.push_back(value.key);
	}

	return result;
}

bool DPSearchLayer::init(std::vector<int> IDs) {
	if (!CCLayer::init()) return false;

	m_searchList = IDs;
	m_filterPacks = IDs;

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	m_data = data;

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

	//error text
	m_errorText = CCLabelBMFont::create("heheheha", "bigFont.fnt");
	m_errorText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	m_errorText->setPosition({ size.width / 2, size.height / 2 });
	m_errorText->setScale(0.6f);
	m_errorText->setZOrder(100);
	m_errorText->setVisible(false);
	m_errorText->setID("error-text");
	this->addChild(m_errorText);

	//bottom text
	m_bottomText = CCLabelBMFont::create("Sorting by Packs (Ascending)", "goldFont.fnt");
	m_bottomText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	m_bottomText->setPosition({ size.width / 2, 25.f });
	m_bottomText->setScale(0.6f);
	m_bottomText->setZOrder(100);
	m_bottomText->setID("bottom-text");
	this->addChild(m_bottomText);

	//back button
	auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backButton = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(DPSearchLayer::backButton));
	auto backMenu = CCMenu::create();
	backMenu->addChild(backButton);
	backMenu->setPosition({ 25, size.height - 25 });
	backMenu->setZOrder(2);
	backMenu->setID("back-menu");
	this->addChild(backMenu);

	//reload menu
	auto reloadMenu = CCMenu::create();
	reloadMenu->setPosition({ size.width - 30, size.height - 30 });
	auto reloadBtnSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
	auto reloadBtn = CCMenuItemSpriteExtra::create(reloadBtnSprite, this, menu_selector(DPSearchLayer::reloadLevels));
	reloadBtn->setPosition({ 0, 0 });
	reloadMenu->addChild(reloadBtn);
	reloadMenu->setID("reload-menu");
	this->addChild(reloadMenu);

	//pages menu
	m_pagesMenu = CCMenu::create();
	m_pagesMenu->setPosition({ 0, 0 });
	m_pagesMenu->setID("pages-menu");

	auto leftBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
	m_left = CCMenuItemSpriteExtra::create(leftBtnSpr, this, menu_selector(DPSearchLayer::pageLeft));
	m_left->setPosition(24.f, size.height / 2);
	m_left->setVisible(false);
	m_pagesMenu->addChild(m_left);

	auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
	rightBtnSpr->setFlipX(true);
	m_right = CCMenuItemSpriteExtra::create(rightBtnSpr, this, menu_selector(DPSearchLayer::pageRight));
	m_right->setPosition(size.width - 24.f, size.height / 2);
	m_right->setVisible(false);
	m_pagesMenu->addChild(m_right);

	auto firstBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	auto firstBtnSpr2 = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	firstBtnSpr->setScale(0.5f);
	firstBtnSpr2->setAnchorPoint({0.5f, 0.f});
	firstBtnSpr2->setPositionX(-5.f);
	firstBtnSpr->addChild(firstBtnSpr2);
	m_first = CCMenuItemSpriteExtra::create(firstBtnSpr, this, menu_selector(DPSearchLayer::pageFirst));
	m_first->getChildByType<CCSprite>(0)->setPosition({14.5f, 10.f});
	m_first->setPosition(24.f, (size.height / 2) - 40.f);
	m_first->setVisible(false);
	m_pagesMenu->addChild(m_first);

	auto lastBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	auto lastBtnSpr2 = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
	lastBtnSpr->setFlipX(true);
	lastBtnSpr2->setFlipX(true);
	lastBtnSpr->setScale(0.5f);
	lastBtnSpr2->setAnchorPoint({0.5f, 0.f});
	lastBtnSpr2->setPositionX(-5.f);
	lastBtnSpr2->setZOrder(-1);
	lastBtnSpr->addChild(lastBtnSpr2);
	m_last = CCMenuItemSpriteExtra::create(lastBtnSpr, this, menu_selector(DPSearchLayer::pageLast));
	m_last->getChildByType<CCSprite>(0)->setPosition({14.5f, 10.f});
	m_last->setPosition(size.width - 24.f, (size.height / 2) - 40.f);
	m_last->setVisible(false);
	m_pagesMenu->addChild(m_last);

	auto selectBtnSpr = ButtonSprite::create("1", "bigFont.fnt", "GJ_button_02.png", 0.8f);
	selectBtnSpr->setScale(0.7f);
	selectBtnSpr->m_BGSprite->setContentSize({ 40.f, 40.f });
	m_select = CCMenuItemSpriteExtra::create(selectBtnSpr, this, menu_selector(DPSearchLayer::pageSelect));
	m_select->setPosition(size.width - 24.f, 24.f);
	m_select->setID("page-select-btn");
	m_pagesMenu->addChild(m_select);

	this->addChild(m_pagesMenu);

	//filters menu
	m_filterMenu = CCMenu::create();
	m_filterMenu->setPosition({ 70.f, 0.f });
	m_filterMenu->setAnchorPoint({0.f, 0.5f});
	m_filterMenu->setScale(0.75f);
	m_filterMenu->setID("filter-menu");

	auto reverseBtnSprOff = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_sortIcon_001.png"), 0, false, 0, "GJ_button_01.png", 1.f);
	auto reverseBtnSprOn = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_sortIcon_001.png"), 0, false, 0, "GJ_button_02.png", 1.f);
	m_reverseBtn = CCMenuItemToggler::create(reverseBtnSprOff, reverseBtnSprOn, this, menu_selector(DPSearchLayer::setFilter));
	m_reverseBtn->setPositionY((size.height / 2) + 80.f);
	m_reverseBtn->setID("reverse-btn");
	m_filterMenu->addChild(m_reverseBtn);

	auto packBtnSprOff = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png"), 0, false, 0, "GJ_button_01.png", 1.f);
	auto packBtnSprOn = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png"), 0, false, 0, "GJ_button_02.png", 1.f);
	m_packBtn = CCMenuItemToggler::create(packBtnSprOff, packBtnSprOn, this, menu_selector(DPSearchLayer::setFilter));
	m_packBtn->setPositionY((size.height / 2) + 40.f);
	m_packBtn->setTag(static_cast<int>(SearchFilter::Pack));
	m_packBtn->setID("pack-btn");
	m_packBtn->toggle(true);
	m_filterMenu->addChild(m_packBtn);

	auto difficultyBtnSprOff = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png"), 0, false, 0, "GJ_button_01.png", 1.f);
	auto difficultyBtnSprOn = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_demonIcon_001.png"), 0, false, 0, "GJ_button_02.png", 1.f);
	m_difficultyBtn = CCMenuItemToggler::create(difficultyBtnSprOff, difficultyBtnSprOn, this, menu_selector(DPSearchLayer::setFilter));
	m_difficultyBtn->setPositionY(size.height / 2);
	m_difficultyBtn->setTag(static_cast<int>(SearchFilter::Difficulty));
	m_difficultyBtn->setID("difficulty-btn");
	m_filterMenu->addChild(m_difficultyBtn);

	auto nameBtnSprOff = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_smallModeIcon_001.png"), 0, false, 0, "GJ_button_01.png", 1.f);
	auto nameBtnSprOn = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_smallModeIcon_001.png"), 0, false, 0,"GJ_button_02.png", 1.f);
	m_nameBtn = CCMenuItemToggler::create(nameBtnSprOff, nameBtnSprOn, this, menu_selector(DPSearchLayer::setFilter));
	m_nameBtn->setPositionY((size.height / 2) - 40.f);
	m_nameBtn->setTag(static_cast<int>(SearchFilter::Alphabetic));
	m_nameBtn->setID("name-btn");
	m_filterMenu->addChild(m_nameBtn);

	auto ageBtnSprOff = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png"), 0, false, 0, "GJ_button_01.png", 1.f);
	auto ageBtnSprOn = ButtonSprite::create(CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png"), 0, false, 0, "GJ_button_02.png", 1.f);
	m_ageBtn = CCMenuItemToggler::create(ageBtnSprOff, ageBtnSprOn, this, menu_selector(DPSearchLayer::setFilter));
	m_ageBtn->setPositionY((size.height / 2) - 80.f);
	m_ageBtn->setTag(static_cast<int>(SearchFilter::Age));
	m_ageBtn->setID("age-btn");
	m_filterMenu->addChild(m_ageBtn);

	this->addChild(m_filterMenu);

	//list
	m_list = GJListLayer::create(CustomListView::create(CCArray::create(), BoomListType::Level, 220.0f, 358.0f), "", { 194, 114, 62, 255 }, 358.0f, 220.0f, 0);
	m_list->setZOrder(2);
	m_list->setPosition(size / 2 - m_list->getContentSize() / 2);
	this->addChild(m_list);

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return true;
	}

	loadLevels(0);

	this->setKeyboardEnabled(true);
	this->setKeypadEnabled(true);

	return true;
}

void DPSearchLayer::reloadLevels(CCObject* sender) {
	m_errorText->setVisible(false);

	if (m_levelsLoaded) {
		loadLevels(m_page);
	}

	return;
}

void DPSearchLayer::pageLeft(CCObject* sender) {
	m_page -= 1;

	loadLevels(m_page);

	return;
}

void DPSearchLayer::pageRight(CCObject* sender) {
	m_page += 1;

	loadLevels(m_page);

	return;
}

void DPSearchLayer::pageFirst(CCObject* sender) {
	m_page = 0;

	loadLevels(m_page);

	return;
}

void DPSearchLayer::pageLast(CCObject* sender) {
	auto listSize = m_IDs.size();
	auto maxPage = (listSize % 10 == 0 ? listSize : listSize + (10 - (listSize % 10))) / 10 - 1;
	m_page = maxPage;

	loadLevels(m_page);

	return;
}
void DPSearchLayer::pageSelect(CCObject* sender) {
	PagePopup::create(m_page)->show();
	
	return;
}

void DPSearchLayer::setFilter(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemToggler*>(sender);
	auto id = btn->getID();
	auto tag = btn->getTag();

	if (btn->isToggled() && id != "reverse-btn") {
		btn->toggle(false);
	}

	if (id == "reverse-btn") {
		m_reversed = !m_reversed;
		loadLevels(m_page);
	}
	else if (tag == static_cast<int>(SearchFilter::Pack)) {

		//m_packBtn->toggle(false);
		m_difficultyBtn->toggle(false);
		m_nameBtn->toggle(false);
		m_ageBtn->toggle(false);

		m_filter = tag;
		
		m_searchList = m_filterPacks;
		loadLevelsAtPage(0);
	}
	else if (tag == static_cast<int>(SearchFilter::Difficulty)) {

		//sort by difficulty
		if (m_filterDifficulty.empty()) {
			m_filterDifficulty = compareDifficulty(m_data, m_filterPacks); //had to make a custom sort function because std::sort takes too long for these
		}

		m_packBtn->toggle(false);
		m_nameBtn->toggle(false);
		m_ageBtn->toggle(false);

		m_filter = tag;

		m_searchList = m_filterDifficulty;
		loadLevelsAtPage(0);
	}
	else if (tag == static_cast<int>(SearchFilter::Alphabetic)) {

		//sort by name
		if (m_filterAlphabetic.empty()) {
			m_filterAlphabetic = compareName(m_data, m_searchList); //had to make a custom sort function because std::sort takes too long for these
		}

		m_packBtn->toggle(false);
		m_difficultyBtn->toggle(false);
		m_ageBtn->toggle(false);

		m_filter = tag;

		m_searchList = m_filterAlphabetic;
		loadLevelsAtPage(0);
	}
	else if (tag == static_cast<int>(SearchFilter::Age)) {

		//sort by age (oldest id to newest)
		if (m_filterAge.empty()) {
			std::vector<int> temp = m_searchList;
			std::sort(temp.begin(), temp.end(), [](int a, int b) {
            	return a < b;
            });
			m_filterAge = temp;
		}

		m_packBtn->toggle(false);
		m_difficultyBtn->toggle(false);
		m_nameBtn->toggle(false);

		m_filter = tag;

		m_searchList = m_filterAge;
		loadLevelsAtPage(0);
	}

	std::vector<std::string> filterDesc = {"Packs", "Difficulty", "Name", "Age"};
	std::vector<std::string> filterMode = {"Ascending", "Descending"};
	auto filterModeText = filterMode[0];
	if (m_reversed) { filterModeText = filterMode[1]; }
	m_bottomText->setCString(fmt::format("Sorting by {} ({})", filterDesc[m_filter], filterModeText).c_str());

	return;
}

void DPSearchLayer::loadLevelsAtPage(int page) {

	m_page = page;
	loadLevels(m_page);

	return;
}

void DPSearchLayer::loadLevels(int page) {

	m_pagesMenu->setVisible(false);

	m_levelsLoaded = false;

	m_loadCircle = LoadingCircle::create();
	m_loadCircle->m_parentLayer = this;
	m_loadCircle->show();

	m_list->m_listView->setVisible(false);

	m_IDs.clear();

	auto searchResults = m_searchList;
	std::vector<int> reversedResults;
	for (int i = m_searchList.size() - 1; i >= 0; i--) {
		reversedResults.push_back(m_searchList[i]);
	}

	if (m_reversed) {
		searchResults = reversedResults;
	}

	for (auto const& level : searchResults) {
		m_IDs.push_back(std::to_string(level));
	}

	log::info("{}", m_IDs);

	m_right->setVisible(m_IDs.size() > 10);

	auto listSize = m_IDs.size();
	auto maxPage = (listSize % 10 == 0 ? listSize : listSize + (10 - (listSize % 10))) / 10 - 1;
	if (page > maxPage) { 
		page = maxPage;
		m_page = page;
	}
	
	m_select->getChildByType<ButtonSprite>(0)->setString(std::to_string(page + 1).c_str());
	m_select->getChildByType<ButtonSprite>(0)->m_BGSprite->setContentSize({ 40.f, 40.f });

	//borrowed some stuff from integrated demon list
	auto glm = GameLevelManager::sharedState();
	glm->m_levelManagerDelegate = this;
	auto results = std::vector<std::string>(m_IDs.begin() + m_page * 10,
		m_IDs.begin() + std::min(static_cast<int>(m_IDs.size()), (m_page + 1) * 10));
	auto searchObject = GJSearchObject::create(SearchType::Type19, string::join(results, ","));
	auto storedLevels = glm->getStoredOnlineLevels(searchObject->getKey());

	if (storedLevels) {
		loadLevelsFinished(storedLevels, "");
	}
	else
	{
		glm->getOnlineLevels(searchObject);
	}

	return;
}

void DPSearchLayer::loadLevelsFinished(CCArray* levels, const char*) {

	if (m_loadingCancelled) { return; }

	auto listSize = m_IDs.size();
	auto maxPage = (listSize % 10 == 0 ? listSize : listSize + (10 - (listSize % 10))) / 10 - 1;
	m_left->setVisible(m_page > 0);
	m_right->setVisible(m_page < maxPage);
	m_first->setVisible(m_left->isVisible());
	m_last->setVisible(m_right->isVisible());
	m_pagesMenu->setVisible(true);

	m_levelsLoaded = true;
	m_list->m_listView->setVisible(true);

	m_loadCircle->fadeAndRemove();

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	if (m_list->getParent() == this) { this->removeChild(m_list); }

	m_list = GJListLayer::create(CustomListView::create(levels, BoomListType::Level, 220.0f, 358.0f), "Search Results", { 194, 114, 62, 255 }, 358.0f, 220.0f, 0);
	m_list->setZOrder(2);
	m_list->setPosition(size / 2 - m_list->getContentSize() / 2);
	this->addChild(m_list);

	return;
}

void DPSearchLayer::loadLevelsFailed(const char*) {
	
	if (m_loadingCancelled) { return; }
	
	m_levelsLoaded = true;

	m_loadCircle->fadeAndRemove();

	if (m_searchList.empty()) {
		m_errorText->setCString("No Results Found");
	}
	else {
		m_errorText->setCString("Something Went Wrong...");
	}

	m_errorText->setVisible(true);

	return;
}

DPSearchLayer* DPSearchLayer::create(std::vector<int> IDs) {
	auto pRet = new DPSearchLayer();
	if (pRet && pRet->init(IDs)) {
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet); //don't crash if it fails
	return nullptr;
}

DPSearchLayer::~DPSearchLayer() {
	this->removeAllChildrenWithCleanup(true);
}

bool PagePopup::setup() {
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Go to Page");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));
	layer->setPosition(284.5f, 220.f);

	//create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	//create main menu
	m_pageSelectMenu = CCMenu::create();
	m_pageSelectMenu->setPosition(111.5f, 24.f);
	m_pageSelectMenu->setID("page-select-menu");

	auto confirmBtnSpr = ButtonSprite::create("Go", "goldFont.fnt", "GJ_button_01.png", 0.8f);
	m_confirm = CCMenuItemSpriteExtra::create(confirmBtnSpr, this, menu_selector(PagePopup::confirmPage));
	m_confirm->setID("confirm-btn");
	m_pageSelectMenu->addChild(m_confirm);

	auto resetBtnSpr = CCSprite::createWithSpriteFrameName("GJ_resetBtn_001.png");
	m_reset = CCMenuItemSpriteExtra::create(resetBtnSpr, this, menu_selector(PagePopup::resetPage));
	m_reset->setPosition(90.f, 106.f);
	m_reset->setID("reset-btn");
	m_pageSelectMenu->addChild(m_reset);

	auto leftBtnSpr = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
	m_left = CCMenuItemSpriteExtra::create(leftBtnSpr, this, menu_selector(PagePopup::pageLeft));
	m_left->setPosition(-50.f, 56.f);
	m_left->setID("left-btn");
	m_pageSelectMenu->addChild(m_left);

	auto rightBtnSpr = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
	m_right = CCMenuItemSpriteExtra::create(rightBtnSpr, this, menu_selector(PagePopup::pageRight));
	m_right->setPosition(50.f, 56.f);
	m_right->setID("right-btn");
	m_pageSelectMenu->addChild(m_right);

	m_value = TextInput::create(50.f, "1", "bigFont.fnt");
	m_value->setString(std::to_string(m_page + 1).c_str());
	m_value->setCommonFilter(CommonFilter::Int);
	m_value->setMaxCharCount(3);
	m_value->setPositionY(56.f);
	m_value->setID("page-value");
	m_pageSelectMenu->addChild(m_value);

	m_mainLayer->addChild(m_pageSelectMenu);

	return true;
}

void PagePopup::confirmPage(CCObject* sender) {

	DPSearchLayer* searchLayer = this->getParent()->getChildByType<DPSearchLayer>(0);
	searchLayer->loadLevelsAtPage(std::stoi(m_value->getString()) - 1);

	this->removeMeAndCleanup();

	return;
}

void PagePopup::resetPage(CCObject* sender) {
	m_value->setString("1"); //yep, it's that shrimple

	return;
}

void PagePopup::pageLeft(CCObject* sender) {
	auto value = std::stoi(m_value->getString());

	value -= 1;
	value = std::max(value, 1);
	m_value->setString(std::to_string(value));
	
	return;
}

void PagePopup::pageRight(CCObject* sender) {
	auto value = std::stoi(m_value->getString());

	value += 1;
	value = std::min(value, 999);
	m_value->setString(std::to_string(value));
	
	return;
}

PagePopup* PagePopup::create(int page) {
	auto ret = new PagePopup();
	ret->m_page = page;
	if (ret && ret->initAnchored(220.f, 150.f)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

PagePopup::~PagePopup() {
	this->removeAllChildrenWithCleanup(true);
}
