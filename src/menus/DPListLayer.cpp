//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/loader/Event.hpp>

#include "DPLayer.hpp"
#include "DPListLayer.hpp"
#include "../DPUtils.hpp"
#include "../RecommendedUtils.hpp"
#include "../CustomText.hpp"

//geode namespace
using namespace geode::prelude;
using namespace cocos2d;

void DPListLayer::keyBackClicked() {
	updateSave();

	m_loadingCancelled = true;

	CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);

	return;
};

void DPListLayer::backButton(CCObject* sender) {
	keyBackClicked();

	return;
};

bool DPListLayer::init(const char* type, int id) {
	if (!CCLayer::init()) return false;

	m_type = type;
	m_id = id;

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

	//error text
	m_errorText = CCLabelBMFont::create("Something went wrong...", "bigFont.fnt");
	m_errorText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
	m_errorText->setPosition({ size.width / 2, size.height / 2 });
	m_errorText->setScale(0.6f);
	m_errorText->setZOrder(100);
	m_errorText->setVisible(false);
	m_errorText->setID("error-text");
	this->addChild(m_errorText);

	//back button
	auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backButton = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(DPListLayer::backButton));
	auto backMenu = CCMenu::create();
	backMenu->addChild(backButton);
	backMenu->setPosition({ 25, size.height - 25 });
	backMenu->setZOrder(2);
	backMenu->setID("back-menu");
	this->addChild(backMenu);

	//check for errors
	auto jsonCheck = checkJson(data, "");

	if (!jsonCheck.ok()) {
		log::info("Something went wrong validating the GDDP list data.");

		return true;
	}

	//info button
	auto infoMenu = CCMenu::create();
	auto infoButton = InfoAlertButton::create("Pack Info", data[m_type][m_id]["description"].asString().unwrapOr("erm that\'s awkward").c_str(), 1.0f);
	infoMenu->setPosition({ 25, 25 });
	infoMenu->setZOrder(2);
	infoMenu->addChild(infoButton);
	infoMenu->setID("info-menu");
	this->addChild(infoMenu);

	//reload menu
	auto reloadMenu = CCMenu::create();
	reloadMenu->setPosition({ size.width - 30, size.height - 30 });
	auto reloadBtnSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
	auto reloadBtn = CCMenuItemSpriteExtra::create(reloadBtnSprite, this, menu_selector(DPListLayer::reloadLevels));
	reloadBtn->setPosition({ 0, 0 });
	reloadMenu->addChild(reloadBtn);
	reloadMenu->setID("reload-menu");
	this->addChild(reloadMenu);

	//pages menu
	m_pagesMenu = CCMenu::create();
	m_pagesMenu->setPosition({ 0, 0 });
	m_pagesMenu->setID("pages-menu");

	m_left = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(DPListLayer::pageLeft));
	m_left->setPosition(24.f, size.height / 2);
	m_left->setVisible(false);
	m_pagesMenu->addChild(m_left);

	auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
	rightBtnSpr->setFlipX(true);
	m_right = CCMenuItemSpriteExtra::create(rightBtnSpr, this, menu_selector(DPListLayer::pageRight));
	m_right->setPosition(size.width - 24.0f, size.height / 2);
	m_right->setVisible(false);
	m_pagesMenu->addChild(m_right);

	this->addChild(m_pagesMenu);

	//progress bar
	m_progressBar = CCSprite::create("GJ_progressBar_001.png");
	m_progressBar->setAnchorPoint({ 0.5, 0.5 });
	m_progressBar->setPosition({ size.width / 2, 12.f });
	m_progressBar->setColor({ 0, 0, 0 });
	m_progressBar->setOpacity(128);
	m_progressBar->setScale(0.8f);
	m_progressBar->setZOrder(5);
	m_progressBar->setID("progress-bar");

	auto packProgressFront = CCSprite::create("GJ_progressBar_001.png");
	packProgressFront->setAnchorPoint({ 0, 0.5 });
	packProgressFront->setPosition({ 0.0f, 9.5f });
	packProgressFront->setScaleX(0.98f);
	packProgressFront->setScaleY(0.75f);
	packProgressFront->setZOrder(1);
	packProgressFront->setID("progress-bar-front");

	auto clippingNode = CCClippingNode::create();
	auto stencil = CCScale9Sprite::create("square02_001.png");
	stencil->setAnchorPoint({ 0, 0.5f });
	stencil->setContentWidth(packProgressFront->getScaledContentSize().width);
	stencil->setScaleX(1.f);
	stencil->setContentHeight(100);
	clippingNode->setStencil(stencil);
	clippingNode->setAnchorPoint({ 0, 0.5f });
	clippingNode->setPosition({ 3.25f, 10.5f });
	clippingNode->setContentWidth(packProgressFront->getContentWidth() - 2.f);
	clippingNode->setContentHeight(20);
	clippingNode->addChild(packProgressFront);
	clippingNode->setID("clipping-node");
	m_progressBar->addChild(clippingNode);

	auto progressText = CCLabelBMFont::create("0/0", "bigFont.fnt");
	progressText->setPosition({ m_progressBar->getContentWidth() / 2, m_progressBar->getContentHeight() / 2, });
	progressText->setScale(0.65f);
	progressText->setZOrder(5);
	progressText->setID("progress-text");
	m_progressBar->addChild(progressText);

	this->addChild(m_progressBar);
	m_progressBar->setVisible(false);

	m_list = GJListLayer::create(CustomListView::create(CCArray::create(), BoomListType::Level, 220.0f, 358.0f), "", {194, 114, 62, 255}, 358.0f, 220.0f, 0);
	m_list->setZOrder(2);
	m_list->setPosition(size / 2 - m_list->getContentSize() / 2);
	this->addChild(m_list);

	loadLevels(0);

	this->setKeyboardEnabled(true);
	this->setKeypadEnabled(true);

	return true;
}

void DPListLayer::updateProgressBar() {

	m_progressBar->setVisible(true);

	updateSave();

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	auto clippingNode = typeinfo_cast<CCClippingNode*>(m_progressBar->getChildByID("clipping-node"));
	auto front = typeinfo_cast<CCSprite*>(m_progressBar->getChildByID("clipping-node")->getChildByID("progress-bar-front"));
	auto progressText = typeinfo_cast<CCLabelBMFont*>(m_progressBar->getChildByID("progress-text"));

	int month = data[m_type][m_id]["month"].as<int>().unwrapOr(11);
	int year = data[m_type][m_id]["year"].as<int>().unwrapOr(1987);
	std::vector<int> levelIDs = data[m_type][m_id]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
	int reqLevels = data[m_type][m_id]["reqLevels"].as<int>().unwrapOr(-1);
	std::string saveID = (m_type == "monthly") ? fmt::format("{}-{}", month, year) : data[m_type][m_id]["saveID"].asString().unwrapOr("null");

	auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

	//update color
	if (listSave.completed) {
		front->setColor({ 255, 255, 0 });
	}
	else if ((m_type == "main" && !listSave.hasRank) || (m_type == "monthly" && listSave.progress < 5)) {
		front->setColor({ 255, 84, 50 });
	}
	else {
		front->setColor({ 80, 190, 255 });
	}

	//calculate percent
	auto progressPercent = (float)listSave.progress / ((m_type == "monthly") ? ((listSave.progress >= 5) ? 6.f : 5.f) : ((listSave.hasRank || m_type != "main") ? (float)levelIDs.size() : (float)reqLevels));

	//update bar
	auto stencil = CCScale9Sprite::create("square02_001.png");
	stencil->setAnchorPoint({ 0, 0.5f });
	stencil->setContentWidth(front->getScaledContentSize().width);
	stencil->setScaleX(progressPercent);
	stencil->setContentHeight(100.f);
	clippingNode->setStencil(stencil);

	//update label
	if (progressText->getParent() == m_progressBar) { progressText->removeFromParentAndCleanup(true); }
	progressText = CCLabelBMFont::create(fmt::format("{}/{}", listSave.progress, (m_type == "monthly") ? ((listSave.progress >= 5) ? 6 : 5) : ((listSave.hasRank || m_type != "main") ? levelIDs.size() : reqLevels)).c_str(), (listSave.completed) ? "goldFont.fnt" : "bigFont.fnt");
	progressText->setScale((listSave.completed) ? 0.85f : 0.65f);
	progressText->setPosition({ m_progressBar->getContentWidth() / 2, (m_progressBar->getContentHeight() / 2) + 1.5f });
	progressText->setZOrder(5);
	progressText->setID("progress-text");
	m_progressBar->addChild(progressText);

	return;
}

void DPListLayer::updateSave() {

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	
	int month = data[m_type][m_id]["month"].as<int>().unwrapOr(11);
	int year = data[m_type][m_id]["year"].as<int>().unwrapOr(1987);
	std::vector<int> levelIDs = data[m_type][m_id]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
	int reqLevels = data[m_type][m_id]["reqLevels"].as<int>().unwrapOr(-1);
	std::string saveID = (m_type == "monthly") ? fmt::format("{}-{}", month, year) : data[m_type][m_id]["saveID"].asString().unwrapOr("null");

	auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

	//get completed levels
	auto progress = 0;
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
	for (auto const& level : levelIDs) {
		if (std::find(completedLvls.begin(), completedLvls.end(), level) != completedLvls.end()) {
			progress += 1;
		}
	}

	if (m_type == "main" && !listSave.hasRank && progress < listSave.progress) { //If you don't have the rank, any progress you have will be maintained even if a level is moved to legacy
		progress = listSave.progress;
	}

	//update status
 	auto hasRank = listSave.hasRank || ((progress >= reqLevels) && (reqLevels > -1));
	auto completed = (progress == levelIDs.size());

	if (m_type == "monthly" && progress >= 5) {
		auto completedMonthlies = Mod::get()->getSavedValue<std::vector<std::string>>("monthly-completions");

		if (std::find(completedMonthlies.begin(), completedMonthlies.end(), saveID) == completedMonthlies.end()) {
			completedMonthlies.insert(completedMonthlies.begin(), saveID);
			Mod::get()->setSavedValue<std::vector<std::string>>("monthly-completions", completedMonthlies);
		}
	}

	//update recommendations
	if (listSave.progress != progress && progress >= reqLevels - 2 && listSave.progress < reqLevels) {
		RecommendedUtils::generateRecommendations();
	}

	//save
	Mod::get()->setSavedValue<ListSaveFormat>(saveID, ListSaveFormat{ .progress = progress, .completed = completed, .hasRank = hasRank });
	
	return;
}

void DPListLayer::reloadLevels(CCObject* sender) {
	m_errorText->setVisible(false);

	if (m_levelsLoaded) {
		loadLevels(m_page);
	}

	return;
}

void DPListLayer::pageLeft(CCObject* sender) {
	m_page -= 1;

	loadLevels(m_page);

	return;
}

void DPListLayer::pageRight(CCObject* sender) {
	m_page += 1;

	loadLevels(m_page);

	return;
}

void DPListLayer::loadLevels(int page) {

	m_pagesMenu->setVisible(false);

	m_levelsLoaded = false;

	m_loadCircle = LoadingCircle::create();
	m_loadCircle->m_parentLayer = this;
	m_loadCircle->show();

	m_list->m_listView->setVisible(false);
	
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	std::vector<int> levelIDs = {0};
	if (!data[m_type][m_id]["levelIDs"].isNull()) levelIDs = data[m_type][m_id]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();

	m_IDs.clear();

	for (auto const& level : levelIDs) {
		if (level > 0) m_IDs.push_back(std::to_string(level));
	}

	log::info("{}", m_IDs);

	m_right->setVisible(m_IDs.size() > 10);

	//borrowed some stuff from integrated demon list
	auto glm = GameLevelManager::sharedState();
	glm->m_levelManagerDelegate = this;
	auto results = std::vector<std::string>(m_IDs.begin() + m_page * 10,
		m_IDs.begin() + std::min(static_cast<int>(m_IDs.size()), (m_page + 1) * 10));
	auto searchObject = GJSearchObject::create(SearchType::Type19, string::join(results, ","));
	auto storedLevels = glm->getStoredOnlineLevels(searchObject->getKey());

	//log::info("{}", searchObject);
	//log::info("{}", searchObject->getKey());
	//log::info("{}", storedLevels);
	
	if (storedLevels) {
		loadLevelsFinished(storedLevels, "");
	}
	else
	{
		glm->getOnlineLevels(searchObject);
	}

	return;
}

void DPListLayer::loadLevelsFinished(CCArray* levels, const char*) {

	if (m_loadingCancelled) { return; }

	auto listSize = m_IDs.size();
	auto maxPage = (listSize % 10 == 0 ? listSize : listSize + (10 - (listSize % 10))) / 10 - 1;
	m_left->setVisible(m_page > 0);
	m_right->setVisible(m_page < maxPage);
	m_pagesMenu->setVisible(true);

	m_levelsLoaded = true;
	m_list->m_listView->setVisible(true);
	
	m_loadCircle->fadeAndRemove();

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	if (m_list->getParent() == this) { this->removeChild(m_list); }

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

	std::string fullName = data[m_type][m_id]["name"].asString().unwrapOr("null") + 
    ((m_type == "main" || m_type == "legacy") ? " Demons" : "");

	m_list = GJListLayer::create(CustomListView::create(levels, BoomListType::Level, 220.0f, 358.0f), fullName.c_str(), { 194, 114, 62, 255 }, 358.0f, 220.0f, 0);
	m_list->setZOrder(2);
	m_list->setPosition(size / 2 - m_list->getContentSize() / 2);
	this->addChild(m_list);

	//custom pack label
	if ((Mod::get()->getSettingValue<bool>("custom-pack-text") && DPTextEffects.contains(data[m_type][m_id]["saveID"].asString().unwrapOr("null")))
	&& !(m_type == "bonus" && Mod::get()->getSettingValue<bool>("disable-fancy-bonus-text"))) {
		auto label = typeinfo_cast<CCLabelBMFont*>(m_list->getChildByID("title"));

		auto customText = CustomText::create(label->getString());
		customText->addEffectsFromProperties(DPTextEffects[data[m_type][m_id]["saveID"].asString().unwrapOr("null")].as<matjson::Value>().unwrapOrDefault());
		customText->setPosition(label->getPosition());
		customText->setAnchorPoint(label->getAnchorPoint());
		customText->setScale(label->getScale());
		customText->setZOrder(12);
		customText->setID("custom-pack-title"_spr);

		label->setOpacity(0);
		label->setVisible(false);
				
		m_list->addChild(customText);
	}

	//add sprite
	/*auto listTitle = typeinfo_cast<CCLabelBMFont*>(m_list->getChildByID("title"));
	listTitle->setAnchorPoint({ 0.f, 0.5f });
	listTitle->setPositionX(109.75f);

	std::string saveID = "beginner";
	auto month = 0;
	auto year = 0;

	if (m_type == "monthly") { month = data[m_type][m_id]["month"].as_int(); }
	if (m_type == "monthly") { year = data[m_type][m_id]["year"].as_int(); }
	if (m_type == "monthly") {
		saveID = fmt::format("{}-{}", month, year);
	}
	else {
		saveID = data[m_type][m_id]["saveID"].as_string();
	}

	auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

	std::string sprName = "DP_Beginner";

	if (m_type == "main") {
		if (listSave.hasRank) {
			sprName = fmt::format("{}.png", data[m_type][m_id]["plusSprite"].as_string());
		}
		else {
			sprName = fmt::format("{}.png", data[m_type][m_id]["sprite"].as_string());
		}
	}
	else {
		sprName = fmt::format("{}.png", data[m_type][m_id]["sprite"].as_string());
	}

	//fallbacks
	if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data()) == nullptr) {
		sprName = "DP_Invisible.png";
	}

	if (sprName != "DP_Invisible.png") {
		auto dpIcon = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(sprName).data());
		dpIcon->setAnchorPoint({ 1.f, 0.5f });
		dpIcon->setScale(0.85f);
		dpIcon->setPosition({ 214.75f, 286.f });
		dpIcon->setZOrder(listTitle->getZOrder());
		dpIcon->setID("gddp-difficulty");

		if (m_type == "monthly" && listSave.progress >= 5) {
			auto epicSprite = CCSprite::createWithSpriteFrameName("GJ_epicCoin_001.png");
			epicSprite->setPosition({ 21.75f, 18.f });
			epicSprite->setZOrder(-1);
			dpIcon->addChild(epicSprite);
		}

		if (this->getChildByID("gddp-difficulty")) { this->getChildByID("gddp-difficulty")->removeMeAndCleanup(); }
		this->addChild(dpIcon);
	}*/

	updateProgressBar();

	return;
}

void DPListLayer::loadLevelsFailed(const char*) {
	if (m_loadingCancelled) { return; }
	
	m_levelsLoaded = true;

	m_loadCircle->fadeAndRemove();

	/*auto alert = FLAlertLayer::create("ERROR", "Failed to load levels. Please try again later.", "OK");
	alert->setParent(this);
	alert->show();*/

	m_errorText->setVisible(true);

	return;
}

DPListLayer* DPListLayer::create(const char* type, int id) {
	auto pRet = new DPListLayer();
	if (pRet && pRet->init(type, id)) {
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet); //don't crash if it fails
	return nullptr;
}

DPListLayer::~DPListLayer() {
    this->removeAllChildrenWithCleanup(true);
}