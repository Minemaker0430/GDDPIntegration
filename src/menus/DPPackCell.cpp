#include <Geode/Geode.hpp>

#include "DPPackCell.hpp"
#include "DPLayer.hpp"
#include "../CustomText.hpp"
#include "../DPUtils.hpp"

//geode namespace
using namespace geode::prelude;

DPPackCell *DPPackCell::create(matjson::Value data, std::string index, int id) {
	auto ret = new DPPackCell();

    ret->m_pack = data;
    ret->m_index = index;
	ret->m_id = id;

	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool DPPackCell::init() {
    if (!CCLayer::init()) return false;

    const auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

    // setup important data
    std::string name = m_pack["name"].asString().unwrapOr("null");
    std::string sprite = m_pack["sprite"].asString().unwrapOr("DP_Unknown");
    std::string plusSprite = m_pack["plusSprite"].asString().unwrapOr("DP_Unknown");
    int reqLevels = m_pack["reqLevels"].as<int>().unwrapOr(-1); 
    std::vector<int> levelIDs = m_pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
    int month = m_pack["month"].as<int>().unwrapOr(-1); //Monthly Only
    int year = m_pack["year"].as<int>().unwrapOr(-1); //Monthly Only
	int mainPack = m_pack["mainPack"].as<int>().unwrapOr(-1); //Legacy Only
    std::string saveID = (m_index == "monthly") ? fmt::format("{}-{}", month, year) : m_pack["saveID"].asString().unwrapOr("null");

    // get pack save file
    auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

	// get completed levels
	auto progress = 0;
	auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");
	for (auto level : levelIDs) {
		if (std::find(completedLvls.begin(), completedLvls.end(), level) != completedLvls.end()) {
			progress += 1;
		}
	}

    // update status
    auto hasRank = listSave.hasRank || ((progress >= reqLevels) && (reqLevels > -1));
	auto completed = (progress == levelIDs.size());

	if (m_index == "monthly" && progress >= 5) {
		auto completedMonthlies = Mod::get()->getSavedValue<std::vector<std::string>>("monthly-completions");

		if (std::find(completedMonthlies.begin(), completedMonthlies.end(), saveID) == completedMonthlies.end()) {
			completedMonthlies.insert(completedMonthlies.begin(), saveID);
			Mod::get()->setSavedValue<std::vector<std::string>>("monthly-completions", completedMonthlies);
		}
	}

	//save
	Mod::get()->setSavedValue<ListSaveFormat>(saveID, ListSaveFormat{ .progress = progress, .completed = completed, .hasRank = hasRank });

    // get full name
    std::string fullName = name + 
    ((m_index == "main" && hasRank) ? "+ " :" ") + 
    ((m_index == "main" || m_index == "legacy") ? "Demons" : "") +
    ((m_index == "monthly" && listSave.progress > 5) ? " +" : "");

    // start making the actual contents

    CCLabelBMFont* packText = CCLabelBMFont::create(fullName.c_str(), "bigFont.fnt");
	packText->setScale(0.65f);
	if (fullName.length() > 18) { packText->setScale(0.50f); }
	if (fullName.length() > 25) { packText->setScale(0.425f); }
	packText->setAnchorPoint({ 0.f, 1.f });
	packText->setPosition({ 53.f, 49.f });
	packText->setID("pack-text");

    //custom pack text
	auto customPackText = CustomText::create(fullName);
	if (Mod::get()->getSettingValue<bool>("custom-pack-text") && DPTextEffects.contains(saveID)) {
		customPackText->addEffectsFromProperties(DPTextEffects[saveID].as<matjson::Value>().unwrapOrDefault());
		customPackText->setScale(0.65f);
		if (fullName.length() > 18) { customPackText->setScale(0.50f); }
		if (fullName.length() > 25) { customPackText->setScale(0.425f); }
		customPackText->setAnchorPoint({ 0, 1 });
		customPackText->setPosition({ 53, 49 });
		customPackText->setID("custom-pack-text");

		packText->setVisible(false);
	}
	else {
		customPackText->setVisible(false);
	}

	if (m_index == "bonus" && Mod::get()->getSettingValue<bool>("disable-fancy-bonus-text")) {
		packText->setVisible(true);
		customPackText->setVisible(false);	
	}

	if (completed) {
		packText->setFntFile("goldFont.fnt");
		packText->setScale(0.85f);
		if (fullName.length() > 18) { packText->setScale(0.65f); }
		if (fullName.length() > 25) { packText->setScale(0.55f); }

		if (!Mod::get()->getSettingValue<bool>("override-gold-text")) {
			customPackText->setVisible(false);
			packText->setVisible(true);
		}
	}

	auto spr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", sprite)).data());
    auto packSpr = (spr) ? spr : CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);
	packSpr->setScale(1.0f);
	packSpr->setAnchorPoint({ 0.5f, 0.5f });
	packSpr->setPosition({ 28.5f, 25.f });
	packSpr->setVisible(sprite != "DP_Invisible");
	packSpr->setID("pack-sprite");

	auto plusSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", plusSprite)).data());
	auto packPlusSpr = (plusSpr) ? plusSpr : CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);
	packPlusSpr->setScale(1.0f);
	packPlusSpr->setAnchorPoint({ 0.5f, 0.5f });
	packPlusSpr->setPosition({ 28.5f, 25.f });
	packPlusSpr->setVisible(sprite != "DP_Invisible" && ((m_index == "main" && hasRank) || (m_index == "legacy" && completed)));
	packPlusSpr->setID("pack-plus-sprite");

    auto packProgressBack = CCSprite::create("GJ_progressBar_001.png");
	packProgressBack->setAnchorPoint({ 0.f, 0.5f });
	packProgressBack->setPosition({ 53.f, 15.f });
	packProgressBack->setColor({ 0, 0, 0 });
	packProgressBack->setOpacity(128);
	packProgressBack->setID("progress-bar");

	auto packProgressFront = CCSprite::create("GJ_progressBar_001.png");
	packProgressFront->setAnchorPoint({ 0.f, 0.5f });
	packProgressFront->setPosition({ 0.0f, 10.f });
	packProgressFront->setScaleX(0.98f);
	packProgressFront->setScaleY(0.75f);
	packProgressFront->setZOrder(1);

	if (completed) {
		packProgressFront->setColor({ 255, 255, 0 });
	}
	else if ((m_index == "main" && !hasRank) || (m_index == "monthly" && progress < 5)) {
		packProgressFront->setColor({ 255, 84, 50 });
	}
	else {
		packProgressFront->setColor({ 80, 190, 255 });
	}

	auto progressPercent = (float)progress / ((m_index == "monthly") ? ((progress >= 5) ? 6.f : 5.f) : ((hasRank || m_index != "main") ? (float)levelIDs.size() : (float)reqLevels));

    auto clippingNode = CCClippingNode::create();
	auto stencil = CCScale9Sprite::create("square02_001.png");
	stencil->setAnchorPoint({ 0, 0.5f });
	stencil->setContentWidth(packProgressFront->getScaledContentSize().width);
	stencil->setScaleX(progressPercent);
	stencil->setContentHeight(100);
	clippingNode->setStencil(stencil);
	clippingNode->setAnchorPoint({ 0, 0.5f });
	clippingNode->setPosition({ 3.25f, 10.5f });
	clippingNode->setContentWidth(packProgressFront->getContentWidth() - 2.f);
	clippingNode->setContentHeight(20);
	clippingNode->addChild(packProgressFront);
	packProgressBack->addChild(clippingNode);

	packProgressBack->setScaleX(0.6f);
	packProgressBack->setScaleY(0.65f);

	std::string progStr = "...";
	CCLabelBMFont* progText = CCLabelBMFont::create("...", "bigFont.fnt");

    auto packs = data[m_index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

	// check if locked
	if (
		((m_id > 0) && (m_index == "main") && !Mod::get()->getSettingValue<bool>("unlock-all-tiers")) ||
		((m_index == "legacy") && !Mod::get()->getSettingValue<bool>("unlock-all-legacy"))
	) {
		matjson::Value prevPackData = (m_index == "main") ? data["main"][fmax(0, m_id - 1)] : data["main"][mainPack];

		//get rank of required pack and finish the cell
		if (!Mod::get()->getSavedValue<ListSaveFormat>(prevPackData["saveID"].asString().unwrapOr("null")).hasRank) {
			auto lockIcon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
			lockIcon->setPosition({ 180, 20 });
			lockIcon->setAnchorPoint({ 0.5f, 0 });
			lockIcon->setID("lock-icon");

			auto lockText = CCLabelBMFont::create(fmt::format("Get the {} Rank to unlock!", prevPackData["name"].asString().unwrapOr("???")).c_str(), "bigFont.fnt");
			lockText->setPosition({ 180.f, 5.f });
			lockText->setAnchorPoint({ 0.5f, 0.f });
			lockText->setScale(0.5f);
			lockText->setID("lock-text");

			this->addChild(lockIcon);
			this->addChild(lockText);

			return true;
		}
	}

	// if there's no lock, just continue as normal :)

	auto epicSprite = CCSprite::createWithSpriteFrameName("GJ_epicCoin_001.png"); // for monthly sprites
	epicSprite->setPosition({ -2.f, -15.f });
    epicSprite->setAnchorPoint({ 0.f, 0.f });
	epicSprite->setZOrder(-1);

	if (!listSave.hasRank && m_index == "main") {
		progStr = fmt::format("{}/{} to {} Tier", progress, reqLevels, data[m_index][m_id + 1]["name"].asString().unwrapOr("???"));
	}
	else if (listSave.completed && progressPercent == 1.0f) {
		progStr = "100% Complete!";
		progText->setFntFile("goldFont.fnt");
		packPlusSpr->setVisible(m_index == "main" || m_index == "legacy");
		if (m_index == "monthly") packSpr->addChild(epicSprite);
	}
	else if (m_index == "monthly") {
		if (progress < 5) {
			progStr = fmt::format("{}/5 to Partial Completion", progress);
		}
		else {
			progStr = fmt::format("{}/6 to Completion", progress);
			packSpr->addChild(epicSprite);
		}
	}
	else {
		progStr = fmt::format("{}/{} to Completion", progress, levelIDs.size());
		if (m_index == "main") packPlusSpr->setVisible(true);
	}

	progText->setCString(progStr.c_str());
	progText->setPosition({ 155.f, 16.f });
	progText->setAnchorPoint({ 0.5f, 0.5f });
	progText->setScale(0.35f);
	if (completed) { progText->setScale(0.45f); }
	progText->setID("progress-text");

	//extra monthly things
	if (m_index == "monthly") {
		std::string months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
		ccColor3B monthColor[12] = { {255, 0, 0}, {255, 128, 0}, {255, 255, 0}, {128, 255, 0}, {0, 255, 0}, {0, 255, 128}, {0, 255, 255}, {0, 128, 255}, {0, 0, 255}, {128, 0, 255}, {255, 0, 255}, {255, 0, 128} };
		std::string monthlyDisp = fmt::format("{} {}", months[month - 1], year);

		auto yearBg = CCLayerColor::create({monthColor[year % 13].r, monthColor[year % 13].g, monthColor[year % 13].b, 75});
		yearBg->setContentHeight(50.f);
		yearBg->setZOrder(-2);
		yearBg->setID("year-bg");
		if (!Mod::get()->getSettingValue<bool>("disable-year-color")) this->addChild(yearBg);

		auto monthlyText = CCLabelBMFont::create(monthlyDisp.c_str(), "bigFont.fnt");
		monthlyText->setColor(monthColor[month - 1]);
		monthlyText->setScale(0.35f);
		monthlyText->setAnchorPoint({ 0.f, 1.f });
		monthlyText->setPosition({ 53.f, 32.f });
		monthlyText->setZOrder(1);
		monthlyText->setID("monthly-text");
		this->addChild(monthlyText);

		// add placeholder levels if it's empty or missing some
		if (levelIDs.empty()) { levelIDs = std::vector<int>(6, 0); }
		for (int i = levelIDs.size(); i <= 6; i++) { levelIDs.push_back(0); }
		if (levelIDs != m_pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault()) m_pack.set("levelIDs", levelIDs);
	}

	// o7 <3
	if (saveID == "temple-series") {
		auto michiSpikes = CCSprite::createWithSpriteFrameName("communityIcon_03_001.png");
		auto michiHeart = CCSprite::createWithSpriteFrameName("d_heart01_001.png");

		michiSpikes->setScale(0.5f);
		michiSpikes->setPositionX(packText->getContentWidth() - 52.f);
		michiSpikes->setPositionY(35.f);
		michiSpikes->setID("michigun-spikes");

		michiHeart->setScale(0.35f);
		michiHeart->setPositionX(packText->getContentWidth() - 52.f);
		michiHeart->setPositionY(44.f);
		michiHeart->setColor({ 0, 0, 0 });
		michiHeart->setID("michigun-heart");

		this->addChild(michiSpikes);
		this->addChild(michiHeart);
	}

	//now for the actual menu stuff
	auto cellMenu = CCMenu::create();
	cellMenu->setID("view-menu");
	cellMenu->setPosition({ 0.f, 0.f });

	auto viewSpr = ButtonSprite::create("View", "bigFont.fnt", "GJ_button_01.png", 0.6f);
	viewSpr->m_BGSprite->setContentSize({ 66.f, 30.f });
		
	auto viewBtn = CCMenuItemSpriteExtra::create(viewSpr, this, menu_selector(DPLayer::openList));
	viewBtn->setPosition({ 320.f, 25.f });
	viewBtn->setID("view-btn");
	viewBtn->setUserObject(new ListParameters(m_index, m_id));
	cellMenu->addChild(viewBtn);

	if (m_id == 0 && m_index == "monthly") {
		auto goldBG = CCLayerColor::create({ 255, 200, 0, 255 });
		//if (Loader::get()->isModLoaded("alphalaneous.transparent_lists")) { goldBG->setOpacity(50); }
		goldBG->setID("gold-bg");
		goldBG->setContentHeight(50.f);
		goldBG->setZOrder(-1);
		this->addChild(goldBG);
	}

	this->addChild(cellMenu);
	this->addChild(packText);
	this->addChild(customPackText);
	this->addChild(packSpr);
	this->addChild(packPlusSpr);
	this->addChild(packProgressBack);
	this->addChild(progText);

    return true;
}

DPPackCell::~DPPackCell() {
	this->removeAllChildrenWithCleanup(true);
}