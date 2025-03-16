// geode header
#include <Geode/Geode.hpp>

#include "../../Utils.hpp"
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

bool SaveContentsPopup::setup()
{
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

	m_listener.bind([&, this](web::WebTask::Event *e) {
		if (auto res = e->getValue()) {
			if (res->ok() && res->json().isOk() && !res->json().isErr()) {
                auto list = res->json().unwrapOrDefault();

                if (m_uploadType == (int)UploadType::Dev) { //gets dev list
                    if (list.contains("content")) {
                        m_dataOld = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                    }
    
                    if (list.contains("sha")) {
                        m_dataSha = list["sha"].asString().unwrapOr("");
                    }
    
                    m_loadText->setCString("Getting Dev Skillsets... (2/2)");
    
                    auto req = web::WebRequest();
                    req.userAgent("GDDP Mod Database");
                    req.header("Accept", "application/vnd.github+json");
                    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
                    req.header("X-GitHub-Api-Version", "2022-11-28");
                    m_listener2.setFilter(req.get(DEV_SKILLSET_LIST));
                }
                else { // gets main list
                    if (m_uploadType == (int)UploadType::Main) {
                        if (list.contains("content")) {
                            m_dataOld = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                        }
        
                        if (list.contains("sha")) {
                            m_dataSha = list["sha"].asString().unwrapOr("");
                        }
                    }
                    else if (m_uploadType == (int)UploadType::Revert) {
                        if (list.contains("content")) {
                            m_dataNew = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                        }
        
                        if (list.contains("sha")) {
                            m_dataSha = list["sha"].asString().unwrapOr("");
                        }
                    }
    
                    m_loadText->setCString("Getting Main Skillsets... (2/4)");
    
                    auto req = web::WebRequest();
                    req.userAgent("GDDP Mod Database");
                    req.header("Accept", "application/vnd.github+json");
                    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
                    req.header("X-GitHub-Api-Version", "2022-11-28");
                    m_listener2.setFilter(req.get(SKILLSET_LIST));
                }
				
            }
            else {
				FLAlertLayer::create(
					"ERROR",
					fmt::format("Something went wrong...\nCode: {}", res->code()).c_str(),
					"OK"
				)->show();

				this->removeMeAndCleanup();
            }
		}
		else if (e->isCancelled()) {
			log::info("Cancelled request.");
		} });

	m_listener2.bind([&, this](web::WebTask::Event *e) {
		if (auto res = e->getValue()) {
			if (res->ok() && res->json().isOk() && !res->json().isErr()) {
                auto list = res->json().unwrapOrDefault();

				if (m_uploadType == (int)UploadType::Dev) { //gets dev skillsets
                    if (list.contains("content")) {
                        m_skillsetsOld = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                    }
    
                    if (list.contains("sha")) {
                        m_skillsetsSha = list["sha"].asString().unwrapOr("");
                    }

                    compareChanges();
    
                    //stops here since all other data is defined by user
                }
                else { // gets main skillsets
                    if (m_uploadType == (int)UploadType::Main) {
                        if (list.contains("content")) {
                            m_skillsetsOld = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                        }
        
                        if (list.contains("sha")) {
                            m_skillsetsSha = list["sha"].asString().unwrapOr("");
                        }
                    }
                    else if (m_uploadType == (int)UploadType::Revert) {
                        if (list.contains("content")) {
                            m_skillsetsNew = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                        }
                    }
    
                    m_loadText->setCString("Getting Dev List... (3/4)");
    
                    auto req = web::WebRequest();
                    req.userAgent("GDDP Mod Database");
                    req.header("Accept", "application/vnd.github+json");
                    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
                    req.header("X-GitHub-Api-Version", "2022-11-28");
                    m_listener3.setFilter(req.get(DEV_LIST));
                }
            }
            else {
				FLAlertLayer::create(
					"ERROR",
					fmt::format("Something went wrong...\nCode: {}", res->code()).c_str(),
					"OK"
				)->show();

				this->removeMeAndCleanup();
            }
		}
		else if (e->isCancelled()) {
			log::info("Cancelled request.");
		} });

	m_listener3.bind([&, this](web::WebTask::Event *e) {
		if (auto res = e->getValue()) {
			if (res->ok() && res->json().isOk() && !res->json().isErr()) {
                auto list = res->json().unwrapOrDefault();

				if (m_uploadType == (int)UploadType::Main) {
                    if (list.contains("content")) {
                        m_dataNew = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                    }
                }
                else if (m_uploadType == (int)UploadType::Revert) {
                    if (list.contains("content")) {
                        m_dataOld = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                    }

                    if (list.contains("sha")) {
                        m_dataSha = list["sha"].asString().unwrapOr("");
                    }
                }

				m_loadText->setCString("Getting Dev Skillsets... (4/4)");

				auto req = web::WebRequest();
				req.userAgent("GDDP Mod Database");
				req.header("Accept", "application/vnd.github+json");
				req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
				req.header("X-GitHub-Api-Version", "2022-11-28");
				m_listener4.setFilter(req.get(DEV_SKILLSET_LIST));
            }
            else {
				FLAlertLayer::create(
					"ERROR",
					fmt::format("Something went wrong...\nCode: {}", res->code()).c_str(),
					"OK"
				)->show();

				this->removeMeAndCleanup();
            }
		}
		else if (e->isCancelled()) {
			log::info("Cancelled request.");
		} });

	m_listener4.bind([&, this](web::WebTask::Event *e) {
		if (auto res = e->getValue()) {
			if (res->ok() && res->json().isOk() && !res->json().isErr()) {
                auto list = res->json().unwrapOrDefault();

				if (m_uploadType == (int)UploadType::Main) {
                    if (list.contains("content")) {
                        m_skillsetsNew = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                    }
                }
                else if (m_uploadType == (int)UploadType::Revert) {
                    if (list.contains("content")) {
                        m_skillsetsOld = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
                    }

                    if (list.contains("sha")) {
                        m_skillsetsSha = list["sha"].asString().unwrapOr("");
                    }
                }

                compareChanges();
            }
            else {
				FLAlertLayer::create(
					"ERROR",
					fmt::format("Something went wrong...\nCode: {}", res->code()).c_str(),
					"OK"
				)->show();

				this->removeMeAndCleanup();
            }
		}
		else if (e->isCancelled()) {
			log::info("Cancelled request.");
		} });

    //initialize listeners based on upload type
    if (m_uploadType == (int)UploadType::Dev) {
        m_loadText->setCString("Getting Dev List... (1/2)");

	    auto req = web::WebRequest();
	    req.userAgent("GDDP Mod Database");
	    req.header("Accept", "application/vnd.github+json");
	    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
	    req.header("X-GitHub-Api-Version", "2022-11-28");
	    m_listener.setFilter(req.get(DEV_LIST));
    }
    else if (m_uploadType == (int)UploadType::Main || m_uploadType == (int)UploadType::Revert) {
        m_loadText->setCString("Getting Main List... (1/4)");

	    auto req = web::WebRequest();
	    req.userAgent("GDDP Mod Database");
	    req.header("Accept", "application/vnd.github+json");
	    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
	    req.header("X-GitHub-Api-Version", "2022-11-28");
	    m_listener.setFilter(req.get(MAIN_LIST));
    }

	return;
}

void SaveContentsPopup::compareChanges() {
    m_loadText->setCString("Trimming List...");

    if (m_uploadType == (int)UploadType::Dev) {
        m_dataNew.set("database-version", m_dataOld["database-version"].as<int>().unwrapOr(0) + 1);
    }
    else if (m_uploadType == (int)UploadType::Dev) {
        m_dataNew.set("database-version", m_dataOld["database-version"].as<int>().unwrapOr(0));
    }

    //find all levels and trim down unused ones
    std::vector<std::string> usedLevels = {};
    std::vector<std::string> indexes = {"main", "legacy", "bonus", "monthly"};
    for (auto index : indexes) {
        for (auto pack : m_dataNew[index].as<std::vector<matjson::Value>>().unwrapOrDefault()) {
            for (auto lvl : pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault()) {
                usedLevels.push_back(std::to_string(lvl));
            }
        }
    }

    for (auto [key, value] : m_dataNew["level-data"]) {
        auto exists = false;
        for (auto lvl : usedLevels) {
            if (key == lvl) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            m_dataNew["level-data"].erase(key);
        }
    }

    m_loadText->setCString("Comparing Changes...");

    std::vector<std::string> changesList = {};
    
    //start comparing changes
    if (m_dataOld != m_dataNew) { //if these are the same, skip
        changesList.push_back("--List Changes--");

        //check indexes first
        std::vector<std::string> indexes = {"main", "legacy", "bonus", "monthly"};
        for (auto index : indexes) {
            int sizeDiff = (m_dataOld[index].as<std::vector<matjson::Value>>().unwrapOrDefault().size() - m_dataNew[index].as<std::vector<matjson::Value>>().unwrapOrDefault().size());

            if (m_dataOld[index] != m_dataNew[index]) {
                changesList.push_back(fmt::format("{} List:", index));

                if (sizeDiff > 0) { //Packs were removed
                    changesList.push_back(fmt::format("- {} Packs were Removed.", sizeDiff));
                }
                if (sizeDiff < 0) { //Packs were added
                    changesList.push_back(fmt::format("- {} Packs were Added.", std::abs(sizeDiff)));
                }

                int packID = 0;
                for (auto pack : m_dataOld[index].as<std::vector<matjson::Value>>().unwrapOrDefault()) {
                    auto newPack = m_dataNew[index][packID].as<matjson::Value>().unwrapOrDefault();
                    
                    if (pack != newPack) {
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
                                    changesList.push_back(fmt::format("   {} Changed: {} -> {}", key, value.as<std::vector<int>>().unwrapOrDefault(), newValue));
                                }
                                else {
                                    auto newValue = newPack[key].as<std::string>().unwrapOr("?");
                                    changesList.push_back(fmt::format("   {} Changed: {} -> {}", key, value.as<std::string>().unwrapOr("?"), newValue));
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
            changesList.push_back("Levels:");

            int sizeDiff = (m_dataOld["level-data"].size() - m_dataNew["level-data"].size());
            if (sizeDiff > 0) { //Levels were removed
                changesList.push_back(fmt::format("- {} Levels were Removed.", sizeDiff));
            }
            if (sizeDiff < 0) { //Levels were added
                changesList.push_back(fmt::format("- {} Levels were Added.", std::abs(sizeDiff)));
            }

            for (auto [key, value] : m_dataOld["level-data"]) {
                if (!m_dataNew["level-data"].contains(key)) {
                    changesList.push_back(fmt::format("- {} Removed", key));
                }
                else if (value != m_dataNew["level-data"][key]) {
                    changesList.push_back(fmt::format("- {} Changed", key));
                    
                    for (auto [key2, value2] : value) {
                        if (value2 != m_dataNew["level-data"][key][key2]) {
                            if (key2 == "skillsets") {
                                auto newValue = m_dataNew["level-data"][key][key2].as<std::vector<std::string>>().unwrapOrDefault();
                                changesList.push_back(fmt::format("   {} Changed: {} -> {}", key2, value2.as<std::vector<std::string>>().unwrapOrDefault(), newValue));
                            }
                            else if (key2 == "xp") {
                                changesList.push_back(fmt::format("   {} Changed:", key2));
                                for (auto [key3, value3] : value2) {
                                    auto newValue = m_dataNew["level-data"][key][key2][key3].as<int>().unwrapOrDefault();
                                    changesList.push_back(fmt::format("     {} Changed: {} -> {}", key3, value3.as<int>().unwrapOrDefault(), newValue));
                                }
                            }
                            else {
                                auto newValue = m_dataNew["level-data"][key][key2].as<std::string>().unwrapOr("?");
                                changesList.push_back(fmt::format("   {} Changed: {} -> {}", key2, value2.as<std::string>().unwrapOr("?"), newValue));
                            }
                        }
                    }
                }
            }
        }
    }

    if (m_skillsetsOld != m_skillsetsNew) { //if these are the same, skip
        changesList.push_back("--Skillset Changes--");

        int sizeDiff = (m_skillsetsOld.size() - m_skillsetsNew.size());

        if (sizeDiff > 0) { //Packs were removed
            changesList.push_back(fmt::format("- {} Skillsets were Removed.", sizeDiff));
        }
        if (sizeDiff < 0) { //Packs were added
            changesList.push_back(fmt::format("- {} Skillsets were Added.", std::abs(sizeDiff)));
        }

        for (auto [key, value] : m_skillsetsOld) {
            if (!m_skillsetsNew.contains(key)) {
                changesList.push_back(fmt::format("- {} Removed", key));
            }
            else if (value != m_skillsetsNew[key]) {
                changesList.push_back(fmt::format("- {} Changed", key));
                    
                for (auto [key2, value2] : value) {
                    if (value2 != m_skillsetsNew[key][key2]) {
                        auto newValue = m_skillsetsNew[key][key2].as<std::string>().unwrapOr("?");
                        changesList.push_back(fmt::format("   {} Changed: {} -> {}", key2, value2.as<std::string>().unwrapOr("?"), newValue));
                    }
                }
            }
        }
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

	m_listener.bind([&, this](web::WebTask::Event *e) {
		if (auto res = e->getValue()) {
			if (res->ok()) {

                if (m_uploadType == (int)UploadType::Dev || m_uploadType == (int)UploadType::Revert) { //sets dev list
                    m_loadText->setCString("Setting Dev Skillsets... (2/2)");
    
                    auto contents = base64_encode(m_skillsetsNew.dump());

                    auto req = web::WebRequest();
                    req.userAgent("GDDP Mod Database");
                    req.header("Accept", "application/vnd.github+json");
                    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
                    req.header("X-GitHub-Api-Version", "2022-11-28");
                    
                    auto body = matjson::makeObject({
                        {"message", fmt::format("Database Update: {}", m_dataNew["database-version"].as<int>().unwrapOr(-1))},
                        {"content", contents},
                        {"sha", m_skillsetsSha}
                    });
                    req.bodyJSON(body);

                    m_listener2.setFilter(req.put(DEV_SKILLSET_LIST));
                }
                else { // sets main list
                    m_loadText->setCString("Setting Main Skillsets... (2/2)");
    
                    auto contents = base64_encode(m_skillsetsNew.dump());

                    auto req = web::WebRequest();
                    req.userAgent("GDDP Mod Database");
                    req.header("Accept", "application/vnd.github+json");
                    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
                    req.header("X-GitHub-Api-Version", "2022-11-28");
                    
                    auto body = matjson::makeObject({
                        {"message", fmt::format("Database Update: {}", m_dataNew["database-version"].as<int>().unwrapOr(-1))},
                        {"content", contents},
                        {"sha", m_skillsetsSha}
                    });
                    req.bodyJSON(body);

                    m_listener2.setFilter(req.put(SKILLSET_LIST));
                }
				
            }
            else {
				FLAlertLayer::create(
					"ERROR",
					fmt::format("Something went wrong...\nCode: {}", res->code()).c_str(),
					"OK"
				)->show();

				this->removeMeAndCleanup();
            }
		}
		else if (e->isCancelled()) {
			log::info("Cancelled request.");
		} });

	m_listener2.bind([&, this](web::WebTask::Event *e) {
		if (auto res = e->getValue()) {
			if (res->ok()) {

                VerificationPopup* popup = this->getParent()->getChildByType<VerificationPopup>(0);

                FLAlertLayer::create(
					"Success!",
					fmt::format("Your changes have been pushed successfully!", res->code()).c_str(),
					"OK"
				)->show();

                popup->removeMeAndCleanup();
				this->removeMeAndCleanup();
            }
            else {
				FLAlertLayer::create(
					"ERROR",
					fmt::format("Something went wrong...\nCode: {}", res->code()).c_str(),
					"OK"
				)->show();

				this->removeMeAndCleanup();
            }
		}
		else if (e->isCancelled()) {
			log::info("Cancelled request.");
		} });
    
    //initialize listeners based on upload type
    if (m_uploadType == (int)UploadType::Dev || m_uploadType == (int)UploadType::Revert) {
        m_loadText->setCString("Pushing Dev List... (1/2)");

        auto contents = base64_encode(m_dataNew.dump());

	    auto req = web::WebRequest();
	    req.userAgent("GDDP Mod Database");
	    req.header("Accept", "application/vnd.github+json");
	    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
	    req.header("X-GitHub-Api-Version", "2022-11-28");
        
        auto body = matjson::makeObject({
            {"message", fmt::format("Database Update: {}", m_dataNew["database-version"].as<int>().unwrapOr(-1))},
            {"content", contents},
            {"sha", m_dataSha}
        });
        req.bodyJSON(body);

	    m_listener.setFilter(req.put(DEV_LIST));
    }
    else if (m_uploadType == (int)UploadType::Main) {
        m_loadText->setCString("Pushing Main List... (1/2)");

        auto contents = base64_encode(m_dataNew.dump());

	    auto req = web::WebRequest();
	    req.userAgent("GDDP Mod Database");
	    req.header("Accept", "application/vnd.github+json");
	    req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
	    req.header("X-GitHub-Api-Version", "2022-11-28");

        auto body = matjson::makeObject({
            {"message", fmt::format("Database Update: {}", m_dataNew["database-version"].as<int>().unwrapOr(-1))},
            {"content", contents},
            {"sha", m_dataSha}
        });
        req.bodyJSON(body);

	    m_listener.setFilter(req.put(MAIN_LIST));
    }

	return;
}

void SaveContentsPopup::onClose(CCObject *sender)
{
	m_listener.getFilter().cancel();
	m_listener2.getFilter().cancel();
	m_listener3.getFilter().cancel();
	m_listener4.getFilter().cancel();

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
	if (ret && ret->initAnchored(420.f, 250.f))
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