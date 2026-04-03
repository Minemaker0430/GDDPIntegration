// geode header
#include <Geode/Geode.hpp>

#include "../../DPUtils.hpp"
#include "../../base64.h"
#include "../../XPUtils.hpp"
#include "SaveContentsPopup.hpp"
#include "VerificationPopup.hpp"

// other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/JsonValidation.hpp>

// geode namespace
using namespace geode::prelude;

bool SaveContentsPopup::init()
{
    if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	// create load circle
	m_loadcircle = LoadingCircle::create();
	m_loadcircle->m_parentLayer = this;
	m_loadcircle->show();

	// created load text
	m_loadText = CCLabelBMFont::create("Loading...", "chatFont.fnt");
	m_loadText->setPosition({m_mainLayer->getContentWidth() / 2, (m_mainLayer->getContentHeight() / 2) - 45.f});
	m_loadText->setScale(0.75f);
	m_loadText->setID("load-text");
	m_mainLayer->addChild(m_loadText);

	return true;
}

void SaveContentsPopup::getContents()
{
	log::info("Getting Database Contents...");

    // god help me this is agony

    //initialize listeners based on upload type
    if (m_uploadType == (int)UploadType::Dev) {
        m_loadText->setCString("Getting Dev List... (1/1)");

	    auto req = web::WebRequest();
	    req.userAgent("GDDP Mod Database");
	    req.header("Accept", "application/vnd.github+json");
	    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
	    req.header("X-GitHub-Api-Version", "2022-11-28");
	    m_listener.spawn(req.get(DEV_LIST), [&](web::WebResponse value){fetchDevList(value);});
    }
    else if (m_uploadType == (int)UploadType::Main || m_uploadType == (int)UploadType::Revert) {
        m_loadText->setCString("Getting Main List... (1/2)");

	    auto req = web::WebRequest();
	    req.userAgent("GDDP Mod Database");
	    req.header("Accept", "application/vnd.github+json");
	    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
	    req.header("X-GitHub-Api-Version", "2022-11-28");
	    m_listener.spawn(req.get(MAIN_LIST), [&](web::WebResponse value){fetchMainList(value);});
    }

	return;
}

void SaveContentsPopup::fetchMainList(web::WebResponse& value) {
    if (value.ok() && value.json().isOk() && !value.json().isErr()) {
        auto list = value.json().unwrapOrDefault();
        
        if (list.contains("content")) {
            if (m_uploadType == (int)UploadType::Revert) {
                m_dataNew = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
            } 
            else {
                m_dataOld = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                m_dataSha = list["sha"].asString().unwrapOr("");
            }
        }

        m_loadText->setCString("Getting Dev List... (2/2)");
    
        auto req = web::WebRequest();
        req.userAgent("GDDP Mod Database");
        req.header("Accept", "application/vnd.github+json");
        req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
        req.header("X-GitHub-Api-Version", "2022-11-28");
        m_listener2.spawn(req.get(DEV_LIST), [&](web::WebResponse value){fetchDevList(value);});
				
    }
    else {
		FLAlertLayer::create(
			"ERROR",
			fmt::format("Something went wrong...\nAt: Get Main List\nCode: {}", value.code()).c_str(),
			"OK"
		)->show();

		this->removeMeAndCleanup();
    }
    
    return;
}

void SaveContentsPopup::fetchDevList(web::WebResponse& value) {
    if (value.ok() && value.json().isOk() && !value.json().isErr()) {
        auto list = value.json().unwrapOrDefault();
        
        if (list.contains("content")) {
            if (m_uploadType == (int)UploadType::Main) {
                m_dataNew = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
            }
            else {
                m_dataOld = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                m_dataSha = list["sha"].asString().unwrapOr("");
            }
        }
    
        compareChanges();
            
        //stops here since all other data is defined by user
    }
    else {
		FLAlertLayer::create(
			"ERROR",
			fmt::format("Something went wrong...\nAt: Get Dev List\nCode: {}", value.code()).c_str(),
			"OK"
		)->show();

		this->removeMeAndCleanup();
    }
    
    return;
}

void SaveContentsPopup::compareChanges() {
    m_loadText->setCString("Trimming List...");

    if (m_uploadType == (int)UploadType::Dev) m_dataNew.set("database-version", m_dataOld["database-version"].as<int>().unwrapOr(0) + 1);
    else m_dataNew.set("database-version", m_dataNew["database-version"].as<int>().unwrapOr(0));

    //find all levels and trim down unused ones
    std::vector<std::string> usedLevels = {};
    std::vector<std::string> indexes = {"main", "legacy", "bonus", "monthly"};
    for (auto index : indexes) {
        for (auto pack : m_dataNew[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
            for (auto lvl : pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault()) {
                usedLevels.push_back(std::to_string(lvl));
            }
        }
    }

    //remove xp on all non-main levels
    for (auto lvl : usedLevels) {
        auto isMain = false;

        if (!m_dataNew["level-data"][lvl].contains("xp")) continue;
        
        for (auto pack : m_dataNew["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
            for (auto id : pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault()) {
                if (std::to_string(id) == lvl) {
                    isMain = true;
                    break;
                }
            }

            if (isMain) break;
        }
        
        if (!isMain) m_dataNew["level-data"][lvl].erase("xp");
    }

    for (auto [key, value] : m_dataNew["level-data"]) {
        auto exists = false;
        for (auto lvl : usedLevels) {
            if (key == lvl) {
                exists = true;
                break;
            }
        }

        if (!exists) m_dataNew["level-data"].erase(key);
    }

    // erase level data for id 0 since that's an issue now apparently
    if (m_dataNew["level-data"].contains("0")) m_dataNew["level-data"].erase("0");

    // erase default values for text effects
    for (auto index : {"main", "legacy", "bonus", "monthly"}) {
        for (auto pack : m_dataNew[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
            if (!pack.contains("textEffects")) continue;

            if (pack["textEffects"]["textColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}) == ccColor3B{255, 255, 255}) pack["textEffects"].erase("textColor");
            if (pack["textEffects"]["outlineColor"].as<ccColor3B>().unwrapOr(ccColor3B{0, 0, 0}) == ccColor3B{0, 0, 0}) pack["textEffects"].erase("outlineColor");
            if (pack["textEffects"]["shadowColor"].as<ccColor3B>().unwrapOr(ccColor3B{0, 0, 0}) == ccColor3B{0, 0, 0}) pack["textEffects"].erase("shadowColor");

            if (pack["textEffects"]["gradientColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}) == ccColor3B{255, 255, 255}) pack["textEffects"].erase("gradientColor");
            if (pack["textEffects"]["crystalColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}) == ccColor3B{255, 255, 255}) pack["textEffects"].erase("crystalColor");
            if (pack["textEffects"]["starsColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}) == ccColor3B{255, 255, 255}) pack["textEffects"].erase("starsColor");

            if (pack["textEffects"]["gradientOpacity"].as<float>().unwrapOr(1.f) == 1.f) pack["textEffects"].erase("gradientOpacity");
            if (pack["textEffects"]["crystalOpacity"].as<float>().unwrapOr(1.f) == 1.f) pack["textEffects"].erase("crystalOpacity");
            if (pack["textEffects"]["starsOpacity"].as<float>().unwrapOr(1.f) == 1.f) pack["textEffects"].erase("starsOpacity");

            if (pack["textEffects"]["gradientType"].asString().unwrapOr("none") == "none") pack["textEffects"].erase("gradientType");
            if (pack["textEffects"]["special"].asString().unwrapOr(std::string()).empty()) pack["textEffects"].erase("special");

            if (!pack["textEffects"]["gradient"].asBool().unwrapOrDefault()) pack["textEffects"].erase("gradient");
            if (!pack["textEffects"]["crystal"].asBool().unwrapOrDefault()) pack["textEffects"].erase("crystal");
            if (!pack["textEffects"]["stars"].asBool().unwrapOrDefault()) pack["textEffects"].erase("stars");
            if (!pack["textEffects"]["bevel"].asBool().unwrapOrDefault()) pack["textEffects"].erase("bevel");
        }
    }

    m_loadText->setCString("Comparing Changes...");

    std::vector<std::string> changesList = {};
    
    //start comparing changes
    if (m_dataOld != m_dataNew) { //if these are the same, skip
        changesList.push_back("--List Changes--");

        //database version change
        if (m_dataOld["database-version"] != m_dataNew["database-version"]) {
            changesList.push_back(fmt::format("Database Version Changed: {} -> {}", m_dataOld["database-version"].as<int>().unwrapOr(0), m_dataNew["database-version"].as<int>().unwrapOr(0)));
        }

        //check indexes first
        std::vector<std::string> indexes = {"main", "legacy", "bonus", "monthly"};
        for (auto index : indexes) {
            int sizeDiff = (m_dataOld[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size() - m_dataNew[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size());

            if (m_dataOld[index] != m_dataNew[index]) {
                changesList.push_back(fmt::format("{} List:", index));

                if (sizeDiff > 0) { //Packs were removed
                    changesList.push_back(fmt::format("- {} Packs were Removed.", sizeDiff));
                }
                if (sizeDiff < 0) { //Packs were added
                    changesList.push_back(fmt::format("- {} Packs were Added.", std::abs(sizeDiff)));
                }

                int packID = 0;
                for (auto pack : m_dataOld[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
                    auto newPack = m_dataNew[index][packID];
                    
                    if (pack != newPack) {
                        if (!DPUtils::containsJson(m_dataOld[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()), newPack) && sizeDiff < 0) {
                            changesList.push_back(fmt::format("- Pack {}-{} Added", index, packID));
                            packID += 1;
                        }
                        else if (!DPUtils::containsJson(m_dataNew[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()), pack) && sizeDiff > 0) {
                            changesList.push_back(fmt::format("- Pack {}-{} Removed", index, packID));
                            packID -= 1;
                        }
                        else if (DPUtils::containsJson(m_dataNew[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()), pack)) {
                            changesList.push_back(fmt::format("- Pack {}-{} Moved", index, packID));
                        }
                        else {
                            changesList.push_back(fmt::format("- Pack {}-{} Changed", index, packID));

                            for (auto [key, value] : pack) {
                                if (value != newPack[key]) {
                                    if (key == "reqLevels" ||
                                        key == "mainPack" ||
                                        key == "month" ||
                                        key == "year"
                                    ) {
                                        auto newValue = newPack[key].as<int>().unwrapOr(-999);
                                        changesList.push_back(fmt::format("   {} Changed: {} -> {}", key, value.as<int>().unwrapOr(-999), newValue));
                                    }
                                    else if (key == "levelIDs") {
                                        auto newValue = newPack[key].as<std::vector<int>>().unwrapOrDefault();
                                        auto oldValue = value.as<std::vector<int>>().unwrapOrDefault();
                                        changesList.push_back(fmt::format("   {} Changed:", key));
                                        auto pos = 0;
                                        int sizeDiff = newValue.size() - oldValue.size();
                                        for (auto id : newValue) {
                                            if (sizeDiff > 0 && !DPUtils::containsInt(oldValue, id)) {
                                                changesList.push_back(fmt::format("     - {} Added.", id));
                                                pos += 1;
                                            }
                                            else if (sizeDiff < 0 && DPUtils::containsInt(oldValue, id)) {
                                                changesList.push_back(fmt::format("     - {} Removed.", id));
                                                pos -= 1;
                                            }
                                            else if (pos < oldValue.size() && oldValue.at(pos) != id) {
                                                changesList.push_back(fmt::format("     - {} -> {}", oldValue.at(pos), id));
                                            }
                                            else {
                                                changesList.push_back(fmt::format("     - {}", id));
                                            }

                                            pos += 1;
                                        }
                                    }
                                    else if (key == "textEffects") {
                                        auto newValue = newPack[key];
                                        changesList.push_back(fmt::format("   {} Changed:", key));
                                        for (auto [key2, value2] : value) {
                                            if (newValue[key2] == value[key2]) continue;
                                            if (!newValue.contains(key2)) changesList.push_back(fmt::format("     {} Erased", key2));
                                            else changesList.push_back(fmt::format("     {} Changed: {} -> {}", key2, value2.dump(), newValue[key2].dump()));
                                        }
                                        for (auto [key2, value2] : newValue) {
                                            if (!value.contains(key2) && !(value2.dump() == "{}")) changesList.push_back(fmt::format("     {} Changed: {}", key2, value2.dump()));
                                        }
                                    }
                                    else {
                                        auto newValue = newPack[key].as<std::string>().unwrapOr("?");
                                        changesList.push_back(fmt::format("   {} Changed: {} -> {}", key, value.as<std::string>().unwrapOr("?"), newValue));
                                    }
                                }
                            }
                        }
                    }

                    packID += 1;
                }
            }    
        }
        
        //check levels
        if (m_dataOld["level-data"] != m_dataNew["level-data"]) {
            changesList.push_back("--Level Changes--");

            int sizeDiff = (m_dataOld["level-data"].size() - m_dataNew["level-data"].size());
            if (sizeDiff > 0) { //Levels were removed
                changesList.push_back(fmt::format("- {} Levels were Removed.", sizeDiff));
            }
            if (sizeDiff < 0) { //Levels were added
                changesList.push_back(fmt::format("- {} Levels were Added.", std::abs(sizeDiff)));
            }

            for (auto [key, value] : m_dataOld["level-data"]) {
                if (!m_dataOld["level-data"].contains(key)) {
                    changesList.push_back(fmt::format("- {} Added", key));
                }
                else if (!m_dataNew["level-data"].contains(key)) {
                    changesList.push_back(fmt::format("- {} Removed", key));
                }
                else if (value != m_dataNew["level-data"][key]) {
                    changesList.push_back(fmt::format("- {} Changed", key));

                    if (value.contains("xp") && !m_dataNew["level-data"][key].contains("xp")) {
                        changesList.push_back(fmt::format("- XP Removed", key));
                    }
                    
                    for (auto [key2, value2] : value) {
                        if (!m_dataNew["level-data"][key].contains(key2)) {
                            continue;
                        }

                        if (value2 != m_dataNew["level-data"][key][key2]) {
                            if (key2 == "difficulty" || key2 == "startpos-copy") {
                                auto newValue = m_dataNew["level-data"][key][key2].as<int>().unwrapOrDefault();
                                changesList.push_back(fmt::format("   {} Changed: {} -> {}", key2, value2.as<int>().unwrapOrDefault(), newValue));
                            }
                            else if (key2 == "skillsets") {
                                auto newValue = m_dataNew["level-data"][key][key2].as<std::vector<std::string>>().unwrapOrDefault();
                                changesList.push_back(fmt::format("   {} Changed: {} -> {}", key2, value2.as<std::vector<std::string>>().unwrapOrDefault(), newValue));
                            }
                            else if (key2 == "xp") {
                                changesList.push_back(fmt::format("   {} Changed:", key2));
                                for (auto [key3, value3] : value2) {
                                    auto newValue = m_dataNew["level-data"][key][key2][key3].as<int>().unwrapOrDefault();
                                    if (value3.as<int>().unwrapOrDefault() == newValue) continue;
                                    changesList.push_back(fmt::format("     {} Changed: {} -> {}", key3, value3.as<int>().unwrapOrDefault(), newValue));
                                }
                            }
                            else {
                                auto newValue = m_dataNew["level-data"][key][key2].dump();
                                changesList.push_back(fmt::format("   {} Changed: {} -> {}", key2, value2.dump(), newValue));
                            }
                        }
                    }
                }
            }
        }
    }

    if (m_dataOld["skillset-data"] != m_dataNew["skillset-data"]) { //if these are the same, skip
        changesList.push_back("--Skillset Changes--");

        int sizeDiff = (m_dataOld["skillset-data"].size() - m_dataNew["skillset-data"].size());

        if (sizeDiff > 0) { //Packs were removed
            changesList.push_back(fmt::format("- {} Skillsets were Removed.", sizeDiff));
        }
        if (sizeDiff < 0) { //Packs were added
            changesList.push_back(fmt::format("- {} Skillsets were Added.", std::abs(sizeDiff)));
        }

        for (auto [key, value] : m_dataOld["skillset-data"]) {
            if (!m_dataNew["skillset-data"].contains(key)) {
                changesList.push_back(fmt::format("- {} Removed", key));
            }
            else if (value != m_dataNew["skillset-data"][key]) {
                changesList.push_back(fmt::format("- {} Changed", key));
                    
                for (auto [key2, value2] : value) {
                    if (value2 != m_dataNew["skillset-data"][key][key2]) {
                        auto newValue = m_dataNew["skillset-data"][key][key2].as<std::string>().unwrapOr("?");
                        changesList.push_back(fmt::format("   {} Changed: {} -> {}", key2, value2.as<std::string>().unwrapOr("?"), newValue));
                    }
                }
            }
        }
    }

    if (m_dataOld["medals"] != m_dataNew["medals"]) { //if these are the same, skip
        changesList.push_back("--Medal Changes--");
    }

    this->setTitle("Review Changes");

    m_loadText->setVisible(false);
	m_loadcircle->fadeAndRemove();

    //setup list
    auto cells = CCArray::create();

    for (auto text : changesList) {
		auto textNode = CCNode::create();
		auto textLabel = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
		textLabel->setScale(0.4f);
		textLabel->setPosition({5.f, 5.f});
		textLabel->setAnchorPoint({ 0.f, 0.5f });
		textNode->addChild(textLabel);
		cells->addObject(textNode);
    }

    m_list = ListView::create(cells, 10.f, 315.f, 180.f);
	m_list->setPosition({52.5f, 25.f});
	m_mainLayer->addChild(m_list);

    //confirm button
    auto confirmMenu = CCMenu::create();
	confirmMenu->setID("confirm-menu");
	confirmMenu->setPosition({ 210.f, 0.f });
	confirmMenu->setZOrder(10);
	m_mainLayer->addChild(confirmMenu);

    auto confirmSpr = ButtonSprite::create("Confirm", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(SaveContentsPopup::onConfirm));
	confirmMenu->addChild(confirmBtn);

    return;
}

void SaveContentsPopup::onConfirm(CCObject* sender) {
    auto confirmBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
    
    geode::createQuickPopup(
        "Push Contents",																					// title
        "Are you sure you want to push all pending changes?\n<cr>This cannot be undone. (Unless you get Mocha to revert it manually)</c>", // content
        "No", "Yes",																					// buttons
        [this, confirmBtn](auto, bool btn2)
        {
            if (btn2)
            {
                this->setTitle("");

                m_list->setVisible(false);
                m_loadText->setVisible(true);
	            
                m_loadcircle = LoadingCircle::create();
	            m_loadcircle->m_parentLayer = this;
	            m_loadcircle->show();

                setContents();

                confirmBtn->removeMeAndCleanup();
            }
        });
    
    return;
}

void SaveContentsPopup::setContents() {
    log::info("Pushing Changes...");
    
    //initialize listeners based on upload type
    if (m_uploadType == (int)UploadType::Dev || m_uploadType == (int)UploadType::Revert) {
        m_loadText->setCString("Pushing Dev List... (1/1)");

        auto contents = base64_encode(m_dataNew.dump());

	    auto req = web::WebRequest();
	    req.userAgent("GDDP Mod Database");
	    req.header("Accept", "application/vnd.github+json");
	    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
	    req.header("X-GitHub-Api-Version", "2022-11-28");
        
        auto body = matjson::makeObject({
            {"message", fmt::format("Database Update: {} - {}", m_dataNew["database-version"].as<int>().unwrapOr(-1), GameManager::sharedState()->m_playerName)},
            {"content", contents},
            {"sha", m_dataSha}
        });
        req.bodyJSON(body);

	    m_listener.spawn(req.put(DEV_LIST), [&](web::WebResponse value){finalizePush(value);});
    }
    else if (m_uploadType == (int)UploadType::Main) {
        m_loadText->setCString("Pushing Main List... (1/1)");

        auto contents = base64_encode(m_dataNew.dump());

	    auto req = web::WebRequest();
	    req.userAgent("GDDP Mod Database");
	    req.header("Accept", "application/vnd.github+json");
	    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
	    req.header("X-GitHub-Api-Version", "2022-11-28");

        auto body = matjson::makeObject({
            {"message", fmt::format("Database Update: {} - {}", m_dataNew["database-version"].as<int>().unwrapOr(-1), GameManager::sharedState()->m_playerName)},
            {"content", contents},
            {"sha", m_dataSha}
        });
        req.bodyJSON(body);

	    m_listener.spawn(req.put(MAIN_LIST), [&](web::WebResponse value){finalizePush(value);});
    }

	return;
}

void SaveContentsPopup::finalizePush(web::WebResponse& value) {
    if (value.ok()) {

        VerificationPopup* popup = this->getParent()->getChildByType<VerificationPopup>(0);

        FLAlertLayer::create(
			"Success!",
			"Your changes have been pushed successfully!",
			"OK"
		)->show();

        // wipe autosave
		auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\dev_autosave.json");
		file::writeStringSafe(path, "");

        popup->removeMeAndCleanup();
		this->removeMeAndCleanup();
    }
    else {
		FLAlertLayer::create(
			"ERROR",
			fmt::format("Something went wrong...\nAt: Finalizing\nCode: {}", value.code()).c_str(),
			"OK"
		)->show();

		this->removeMeAndCleanup();
    }
    
    return;
}

void SaveContentsPopup::onClose(CCObject *sender)
{
	m_listener.cancel();
	m_listener2.cancel();

	// normal closing stuff
	// CloseEvent(this).post();
	this->setKeypadEnabled(false);
	this->setTouchEnabled(false);
	this->removeFromParentAndCleanup(true);

    FLAlertLayer::create(
        "Failed",
        "Update was Cancelled.",
        "OK"
    )->show();

	return;
}

SaveContentsPopup* SaveContentsPopup::create() {
	auto ret = new SaveContentsPopup();
	if (ret && ret->init())
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

SaveContentsPopup::~SaveContentsPopup() {
	this->removeAllChildrenWithCleanup(true);
}