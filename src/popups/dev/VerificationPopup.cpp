// geode header
#include <Geode/Geode.hpp>

#include "VerificationPopup.hpp"
#include "../../DPUtils.hpp"
#include "../../base64.h"
#include "../../XPUtils.hpp"
#include "Geode/ui/TextInput.hpp"
#include "SaveContentsPopup.hpp"
#include "TextEffectEditorPopup.hpp"

// other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/JsonValidation.hpp>

// geode namespace
using namespace geode::prelude;

bool VerificationPopup::init() {
	if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	// add docs button
	auto docsMenu = CCMenu::create();
	docsMenu->setID("docs-menu");
	docsMenu->setContentSize({ 0.f, 0.f });
	docsMenu->setPosition({210.f, 260.f });

	auto docsBtnSpr = ButtonSprite::create("Documentation", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto docsBtn = CCMenuItemSpriteExtra::create(docsBtnSpr, this, menu_selector(VerificationPopup::onDocs));
	docsBtn->setID("docs-button");
	docsMenu->addChild(docsBtn);
	layer->addChild(docsMenu);

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

	// find access token
	m_accessCodePath = Mod::get()->getConfigDir() += std::filesystem::path("\\access_token.txt");
	auto clientIDPath = Mod::get()->getConfigDir() += std::filesystem::path("\\client_id.txt");
	auto fileCheck = file::readDirectory(Mod::get()->getConfigDir()).unwrapOrDefault();
	// log::info("{}, {}", fileCheck, m_accessCodePath);

	if (std::find(fileCheck.begin(), fileCheck.end(), clientIDPath) != fileCheck.end()) {
		log::info("Found Client ID File.");
		m_clientID = file::readString(clientIDPath).unwrapOr("");
	}
	else {
		FLAlertLayer::create(
			"ERROR",
			"Can't find client ID.",
			"OK")
			->show();

		this->removeMeAndCleanup();

		return true;
	}

	if (std::find(fileCheck.begin(), fileCheck.end(), m_accessCodePath) != fileCheck.end()) {
		log::info("Found Access Token File.");
		m_accessCode = file::readString(m_accessCodePath).unwrapOr("");
	}
	else {
		FLAlertLayer::create(
			"ERROR",
			"Can't find access token.",
			"OK")
			->show();

		this->removeMeAndCleanup();

		return true;
	}

	//log::info("client id: {}, access token: {}", m_clientID, m_accessCode);

	if (m_clientID != "" && m_accessCode == "") reqCode();
	else if (m_accessCode == "") {
		FLAlertLayer::create(
			"ERROR",
			"Access files are blank. Nice try.",
			"OK")
			->show();

		this->removeMeAndCleanup();
	}
	else getContents();

	return true;
}

void VerificationPopup::onDocs(CCObject*) {
	web::openLinkInBrowser("https://github.com/Minemaker0430/gddp-mod-dev-data/blob/main/README.md");
	
	return;
}

void VerificationPopup::reqCode()
{
	auto req = web::WebRequest();

	m_listener.spawn(
		req.post(fmt::format("https://github.com/login/device/code?client_id={}", m_clientID)),
		[&](web::WebResponse value) {
			if (value.ok() && value.string().isOk() && !value.string().isErr()) {
				log::info("Got data: {}", value.string().unwrapOr(""));
				m_loadcircle->fadeAndRemove();
				m_loadText->setVisible(false);

				//parse response
				parseResponse(value.string().unwrapOr(""));

				//setup device code
				m_verificationCode = m_responseValues["user_code"].asString().unwrapOr("uhhhh");
				log::info("code: {}", m_verificationCode);

				auto instructionText = CCLabelBMFont::create("Go to https://github.com/login/device and enter this code:", "bigFont.fnt");
				auto codeText = CCLabelBMFont::create(m_verificationCode.c_str(), "goldFont.fnt");
				auto refreshText = CCLabelBMFont::create("This popup will check for your code every 10 seconds.", "bigFont.fnt");

				instructionText->setPosition({ m_mainLayer->getContentWidth() / 2, 160.f });
				codeText->setPosition({ m_mainLayer->getContentWidth() / 2, m_mainLayer->getContentHeight() / 2 });
				refreshText->setPosition({ m_mainLayer->getContentWidth() / 2, 90.f });

				instructionText->setScale(0.4f);
				codeText->setScale(1.75f);
				refreshText->setScale(0.425f);

				m_mainLayer->addChild(instructionText);
				m_mainLayer->addChild(codeText);
				m_mainLayer->addChild(refreshText);

				//setup polling
				m_deviceCode = m_responseValues["device_code"].asString().unwrapOr("uhhhh");
				this->schedule(schedule_selector(VerificationPopup::poll), 10.f);
			}
			else {
				log::info("Something went wrong... {}", value.code());
				FLAlertLayer::create(
					"ERROR",
					fmt::format("Something went wrong...\nCode: {}", value.code()).c_str(),
					"OK"
				)->show();

				m_listener.cancel();
			}
		}
	);

	return;
}

void VerificationPopup::poll(float dt)
{
	log::info("Checking for code...");

	auto req = web::WebRequest();

	m_listener.spawn(
		req.post(fmt::format(
		"https://github.com/login/oauth/access_token?client_id={}&device_code={}&grant_type=urn:ietf:params:oauth:grant-type:device_code",
		m_clientID, m_deviceCode)),
		[&](web::WebResponse value) {
			if (value.ok() && value.string().isOk() && !value.string().isErr()) {
                log::info("Got data: {}", value.string().unwrapOr(""));

				//parse response
				parseResponse(value.string().unwrapOr(""));

				//interpret response
				if (m_responseValues.contains("error")) { //if the response contains an error, it wasn't successful.
					std::map<std::string, std::string> errorCodes = {
						{"incorrect_client_credentials", "Incorrect client credentials. You\'re most likely trying to access something you aren\'nt supposed to."},
						{"access_denied", "You likely cancelled the authorization."},
						{"device_flow_disabled", "Device Flow verification is disabled. That\'s my bad, oops."},
						{"unsupported_grant_type", "Grant type is invalid. I don\'t know how you managed to accomplish this. It\'s literally hardcoded in the mod."},
						{"incorrect_device_code", "Incorrect device code. Not sure how you managed this but oh well."},
						{"slow_down", "Made too many requests too quickly. This probably isn\'t your fault. If it keeps happening, tell Mocha he messed up."},
						{"expired_token", "Your device token has expired. Try authenticating again."}
					};
					
					if (m_responseValues["error"].asString().unwrapOr("") == "authorization_pending") log::info("Authorization still pending, trying again in 10 seconds...");
					else {
						if (errorCodes.contains(m_responseValues["error"].asString().unwrapOr(""))) {
							FLAlertLayer::create(
								"ERROR",
								errorCodes[m_responseValues["error"].asString().unwrapOr("")],
								"OK"
							)->show();
						}
						else {
							FLAlertLayer::create(
								"ERROR",
								"An Unknown Error occurred, please tell Mocha about this.",
								"OK"
							)->show();
						}

						m_mainLayer->removeAllChildrenWithCleanup(true);
						this->unschedule(schedule_selector(VerificationPopup::poll));
						m_listener.cancel();

						this->removeMeAndCleanup();
					}
				}
				else if (m_responseValues.contains("access_token")) {
					log::info("Successfully obtained token!");
					file::writeStringSafe(m_accessCodePath, m_responseValues["access_token"].asString().unwrapOr(""));

					FLAlertLayer::create(
						"Success!",
						"You are now authenticated! Reopen the popup to access your panel.",
						"OK"
					)->show();

					m_mainLayer->removeAllChildrenWithCleanup(true);
					this->unschedule(schedule_selector(VerificationPopup::poll));
					m_listener.cancel();

					this->removeMeAndCleanup();
				}
				else {
					FLAlertLayer::create(
						"ERROR",
						"Something went wrong, somehow.",
						"OK"
					)->show();

					m_mainLayer->removeAllChildrenWithCleanup(true);
					this->unschedule(schedule_selector(VerificationPopup::poll));
					m_listener.cancel();

					this->removeMeAndCleanup();
				}
            }
            else {
                log::info("Something went wrong... {}", value.code());
				FLAlertLayer::create(
					"ERROR",
					fmt::format("Something went wrong...\nCode: {}", value.code()).c_str(),
					"OK"
				)->show();

				m_mainLayer->removeAllChildrenWithCleanup(true);
				this->unschedule(schedule_selector(VerificationPopup::poll));
				m_listener.cancel();

				this->removeMeAndCleanup();
            }
		}
	);

	return;
}

void VerificationPopup::parseResponse(std::string res)
{
	m_responseValues.clear();

	auto params = DPUtils::substring(res, "&");
	std::vector<std::string> keys;
	std::vector<std::string> values;
	for (auto key : params) {
		auto result = DPUtils::substring(key, "=");
		keys.push_back(result[0]);
		values.push_back(result[1]);
	}

	for (int i = 0; i < keys.size(); i++) m_responseValues.set(keys[i], values[i]);
}

void VerificationPopup::getContents()
{
	log::info("Getting Database Contents...");

	// with geode v5 this is about to be nesting HELL 

	m_loadText->setCString("Getting Main List... (1/2)");

	auto req = web::WebRequest();
	req.userAgent("GDDP Mod Database");
	req.header("Accept", "application/vnd.github+json");
	req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
	req.header("X-GitHub-Api-Version", "2022-11-28");
	m_listener.spawn(
		req.get("https://api.github.com/repos/Minemaker0430/gddp-mod-database/contents/main-list.json"),
		[&](web::WebResponse value) {
			if (value.ok() && value.json().isOk() && !value.json().isErr()) {
                auto list = value.json().unwrapOrDefault();

				if (list.contains("content")) m_dataMain = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();

				if (list.contains("sha")) m_dataMainSha = list["sha"].asString().unwrapOr("");

				m_loadText->setCString("Getting Dev List... (2/2)");

				auto req = web::WebRequest();
				req.userAgent("GDDP Mod Database");
				req.header("Accept", "application/vnd.github+json");
				req.header("Authorization", fmt::format("Bearer {}", m_accessCode));
				req.header("X-GitHub-Api-Version", "2022-11-28");
				m_listener2.spawn(
					req.get("https://api.github.com/repos/Minemaker0430/gddp-mod-dev-data/contents/list.json"),
					[&](web::WebResponse value) {
						if (value.ok() && value.json().isOk() && !value.json().isErr()) {
							auto list = value.json().unwrapOrDefault();

							if (list.contains("content")) {
								m_dataDev = matjson::parse(base64_decode(list["content"].asString().unwrapOr(""), true)).unwrapOrDefault();
							}

							if (list.contains("sha")) {
								m_dataDevSha = list["sha"].asString().unwrapOr("");
							}

							m_loadText->setVisible(false);
							m_loadcircle->fadeAndRemove();

							log::info("All Data Loaded Successfully!");

							// check if autosave is populated (indicates crash)
							auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\dev_autosave.json");
							if (file::readString(path).unwrapOrDefault().size() > 1) {
								m_dataDev = file::readJson(path).unwrapOrDefault();

								FLAlertLayer::create(
									"Autosave Restored",
									fmt::format("Your last session seems to have crashed, an autosave has been restored.", value.code()).c_str(),
									"OK"
								)->show();
							}

							loadMain(0);
						}
						else {
							log::info("Something went wrong... {}", value.code());
							FLAlertLayer::create(
								"ERROR",
								fmt::format("Something went wrong getting the Dev List...\nCode: {}", value.code()).c_str(),
								"OK"
							)->show();

							this->removeMeAndCleanup();
						}
					}
				);
			}
			else {
				log::info("Something went wrong... {}", value.code());
				FLAlertLayer::create(
					"ERROR",
					fmt::format("Something went wrong getting the Main List...\nCode: {}", value.code()).c_str(),
					"OK"
				)->show();

				this->removeMeAndCleanup();
			}
		}
	);

	return;
}

void VerificationPopup::onPushChanges(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto id = btn->getID();
	
	auto popup = SaveContentsPopup::create();

	//set values
	if (id == "save-dev-btn") {
		popup->m_dataNew = m_dataDev;
		popup->m_uploadType = (int)UploadType::Dev;
	}
	if (id == "save-main-btn") popup->m_uploadType = (int)UploadType::Main;
	if (id == "revert-btn") popup->m_uploadType = (int)UploadType::Revert;

	popup->m_accessCode = m_accessCode;

	popup->show();
	popup->getContents();
	
	return;
}

void VerificationPopup::onUpdateSearch(std::string text) {
	//cells
	auto cells = CCArray::create();

	switch(m_currentTab) {
		case 0: // packs
		{
			std::vector<std::string> packIndexes = {"main", "legacy", "bonus", "monthly"};
			auto data = m_dataDev;

			for (std::string index : packIndexes) {
				//header
				auto header = CCNode::create();
				auto headerText = CCLabelBMFont::create(index.c_str(), "goldFont.fnt");
				headerText->setScale(0.4f);
				headerText->setPosition({5.f, 25.f / 2.f});
				headerText->setAnchorPoint({ 0.f, 0.5f });
				header->addChild(headerText);
				cells->addObject(header);

				//packs
				for (int i = 0; i < data[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
					auto packNode = CCNode::create();
					packNode->setID(fmt::format("{}-pack-{}", index, i));
					packNode->setScale(0.75f);

					auto packData = data[index][i];

					auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr);

					// sprite
					auto spriteName = fmt::format("{}.png", packData["sprite"].asString().unwrapOr("DP_Unknown"));
					CCSprite* sprite = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(spriteName).data(), unkSpr);
					if (spriteName == "DP_Invisible.png") sprite->setVisible(false);
					sprite->setID("sprite");
					sprite->setScale(0.75f);
					sprite->setAnchorPoint({0.f, 0.5f});
					sprite->setPosition({5.f, 15.5f});

					// label
					auto label = CCLabelBMFont::create(fmt::format("{}", packData["name"].asString().unwrapOr("null")).c_str(), "bigFont.fnt");
					label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
					label->setID("label");
					label->setScale(0.5f);
					label->setAnchorPoint({0.f, 0.5f});
					label->setPosition({40.f, 17.5f});

					// edit menu
					auto editMenu = CCMenu::create();
					editMenu->setScale(0.75f);
					editMenu->setPosition({330.f, -25.f});
					editMenu->setID("edit-menu");

					auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
					editSpr->setScale(0.5f);
					auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(VerificationPopup::onEdit));
					editBtn->setTag(i);
					editBtn->setID(fmt::format("{}", index));
					editMenu->addChild(editBtn);

					auto moveSpr = CCSprite::createWithSpriteFrameName("GJ_editModeBtn_001.png");
					moveSpr->setScale(0.85f);
					auto moveBtn = CCMenuItemSpriteExtra::create(moveSpr, this, menu_selector(VerificationPopup::onMove));
					moveBtn->setPositionX(-40.f);
					moveBtn->setTag(i);
					moveBtn->setID(fmt::format("{}", index));
					editMenu->addChild(moveBtn);

					auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
					deleteSpr->setScale(0.85f);
					auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(VerificationPopup::onDelete));
					deleteBtn->setPositionX(-80.f);
					deleteBtn->setTag(i);
					deleteBtn->setID(fmt::format("{}", index));
					editMenu->addChild(deleteBtn);

					// add children
					packNode->addChild(label);
					packNode->addChild(editMenu);
					packNode->addChild(sprite);

					// search function
					std::string searchLower = text;
    				std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
					
					std::string nameLower = packData["name"].asString().unwrapOr("null");
					std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

					if (searchLower.size() > 0) {
						if (nameLower.find(searchLower) == std::string::npos &&
							std::to_string(i).find(searchLower) == std::string::npos || (
							(index == "monthly" && packData["month"].asString().unwrapOr("1").find(searchLower) == std::string::npos) &&
							(index == "monthly" && packData["year"].asString().unwrapOr("1987").find(searchLower) == std::string::npos))) {
							continue;
						}
					}

					cells->addObject(packNode);
				}
			}

			break;
		}
		case 1: // skillsets
		{
			std::vector<std::string> skillsetTypes = {"none", "gamemode", "misc", "platformer", "special"};
			auto skillsets = m_dataDev["skillset-data"];

			for (auto type : skillsetTypes)
			{
				// header
				auto header = CCNode::create();
				auto headerText = CCLabelBMFont::create(type.c_str(), "goldFont.fnt");
				headerText->setScale(0.4f);
				headerText->setPosition({5.f, 25.f / 2.f});
				headerText->setAnchorPoint({ 0.f, 0.5f });
				header->addChild(headerText);
				cells->addObject(header);

				// skillsets
				for (auto [key, value] : skillsets) {
					auto skillNode = CCNode::create();
					skillNode->setID(fmt::format("skill-{}", key));
					skillNode->setScale(0.75f);
	
					//log::info("i: {}", i);
					//log::info("key: {}", key);
					//log::info("value: {}", value);
	
					auto skillData = value;

					if (skillData["type"].asString().unwrapOr("none") != type) continue;

					auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Skill_Unknown.png"_spr);
	
					//sprite
					auto spriteName = fmt::format("{}.png", skillData["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
					CCSprite* sprite = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(spriteName).data(), unkSpr);
					sprite->setID("sprite");
					sprite->setScale(0.75f);
					sprite->setAnchorPoint({ 0.f, 0.5f });
					sprite->setPosition({ 5.f, 17.5f });
	
					//label
					auto label = CCLabelBMFont::create(skillData["display-name"].asString().unwrapOr("null").c_str(), "bigFont.fnt");
					label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
					label->setID("label");
					label->setScale(0.5f);
					label->setAnchorPoint({ 0.f, 0.5f });
					label->setPosition({ 30.f, 17.5f });
	
					// edit menu
					auto editMenu = CCMenu::create();
					editMenu->setScale(0.75f);
					editMenu->setPosition({330.f, -25.f});
					editMenu->setID("edit-menu");

					auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
					editSpr->setScale(0.5f);
					auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(VerificationPopup::onEdit));
					editBtn->setID(key);
					editMenu->addChild(editBtn);

					auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
					deleteSpr->setScale(0.85f);
					auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(VerificationPopup::onDelete));
					deleteBtn->setPositionX(-40.f);
					deleteBtn->setID(key);
					editMenu->addChild(deleteBtn);
	
					//add children
					skillNode->addChild(sprite);
					skillNode->addChild(label);
					skillNode->addChild(editMenu);

					// search function
					std::string searchLower = text;
    				std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
					
					std::string nameLower = skillData["display-name"].asString().unwrapOr("null");
					std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
					std::string keyLower = key;
					std::transform(keyLower.begin(), keyLower.end(), keyLower.begin(), ::tolower);

					if (searchLower.size() > 0) {
						if (nameLower.find(searchLower) == std::string::npos &&
							keyLower.find(searchLower) == std::string::npos) {
							continue;
						}
					}
	
					cells->addObject(skillNode);
				}
			}

			break;
		}
		case 2: // levels
		{
			// get every level, registered or unregistered
			std::vector<int> unregisteredLvls = {};
			std::vector<int> registeredLvls = {};
			std::vector<int> warnedLvls = {};

			for (auto index : {"main", "legacy", "bonus", "monthly"}) {
				for (auto pack : m_dataDev[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
					for (auto lvl : pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault()) {
						if (DPUtils::containsInt(unregisteredLvls, lvl) || DPUtils::containsInt(registeredLvls, lvl) || DPUtils::containsInt(warnedLvls, lvl)) continue;
						if (m_dataDev["level-data"].contains(std::to_string(lvl))) registeredLvls.push_back(lvl);
						else unregisteredLvls.push_back(lvl);
					}
				}
			}

			// check if any registered levels aren't in packs
			for (auto [key, value] : m_dataDev["level-data"]) {
				if (!(
					DPUtils::containsInt(unregisteredLvls, numFromString<int>(key).unwrapOrDefault()) || 
					DPUtils::containsInt(registeredLvls, numFromString<int>(key).unwrapOrDefault())
				)) warnedLvls.push_back(numFromString<int>(key).unwrapOrDefault());
			}

			std::sort(unregisteredLvls.begin(), unregisteredLvls.end(), [](int a, int b) { return a < b; });
			std::sort(warnedLvls.begin(), warnedLvls.end(), [](int a, int b) { return a < b; });
			std::sort(registeredLvls.begin(), registeredLvls.end(), [](int a, int b) { return a < b; });

			std::vector<std::string> sections = {"Unregistered", "Not In A Pack", "Registered"};
			for (int s = 0; s < sections.size(); s++) {
				std::vector<int> list;
				switch(s) {
					case 0:
					{
						list = unregisteredLvls;
						break;
					}
					case 1:
					{
						list = warnedLvls;
						break;
					}
					case 2:
					{
						list = registeredLvls;
						break;
					}
				}

				if (list.empty()) continue;

				// header
				auto header = CCNode::create();
				auto headerText = CCLabelBMFont::create(sections[s].c_str(), "goldFont.fnt");
				headerText->setScale(0.4f);
				headerText->setPosition({5.f, 25.f / 2.f});
				headerText->setAnchorPoint({ 0.f, 0.5f });
				header->addChild(headerText);
				cells->addObject(header);

				std::vector<ccColor3B> colors = {
					ccColor3B{255, 0, 0},
					ccColor3B{255, 255, 0},
					ccColor3B{255, 255, 255}
				};

				for (auto lvl : list) {
					auto idNode = CCNode::create();
					idNode->setID(fmt::format("level-{}", lvl));
					idNode->setScale(0.75f);
					
					std::string levelName = "???";
					if (m_dataDev["level-data"].contains(std::to_string(lvl))) levelName = m_dataDev["level-data"][std::to_string(lvl)]["name"].asString().unwrapOr("???");
					if (lvl == 0) levelName = "PLACEHOLDER"; //realistically should not happen

					// label
					auto label = CCLabelBMFont::create(fmt::format("{} ({})", levelName, lvl).c_str(), "bigFont.fnt");
					label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
					label->setID("label");
					label->setScale(0.5f);
					label->setAnchorPoint({0.f, 0.5f});
					label->setPosition({5.f, 17.5f});
					label->setColor(colors[s]);

					// edit menu
					auto editMenu = CCMenu::create();
					editMenu->setScale(0.75f);
					editMenu->setPosition({330.f, -25.f});
					editMenu->setID("edit-menu");

					auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
					editSpr->setScale(0.5f);
					auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(VerificationPopup::onEdit));
					editBtn->setID(std::to_string(lvl));
					editBtn->setTag(-300);
					if (lvl > 0) editMenu->addChild(editBtn); //realistically should not not happen

					auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
					deleteSpr->setScale(0.85f);
					auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(VerificationPopup::onDelete));
					deleteBtn->setPositionX(-40.f);
					deleteBtn->setTag(100);
					deleteBtn->setID(std::to_string(lvl));
					if (m_dataDev["level-data"].contains(std::to_string(lvl))) editMenu->addChild(deleteBtn);

					// add children
					idNode->addChild(label);
					idNode->addChild(editMenu);

					// search function
					std::string searchLower = text;
    				std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
					
					std::string nameLower = levelName;
					std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

					if (searchLower.size() > 0) {
						if (nameLower.find(searchLower) == std::string::npos &&
							std::to_string(lvl).find(searchLower) == std::string::npos) {
							continue;
						}
					}

					cells->addObject(idNode);
				}
			}

			break;
		}
		case 3: // medals
		{
			// sort medals by requirement
			for (auto type : {"normal", "plus"}) {
				auto medals = m_dataDev["medals"][type].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
				
				//assign names to key
				std::vector<SortMedals> out;

				for (int i = 0; i < medals.size(); i++) {
					int req = medals[i]["requirement"].as<int>().unwrapOr(0);
					out.push_back(SortMedals(i, req));
				}

				//sort
				std::sort(out.begin(), out.end());

				//convert to int vector and return
				std::vector<matjson::Value> result;
				for (SortMedals value : out) result.push_back(medals[value.index]);
				m_dataDev["medals"].set(type, result);
			}

			std::vector<std::string> medalTypes = {"normal", "plus"};

			for (auto type : medalTypes)
			{
				// header
				auto header = CCNode::create();
				auto headerText = CCLabelBMFont::create(type.c_str(), "goldFont.fnt");
				headerText->setScale(0.4f);
				headerText->setPosition({5.f, 25.f / 2.f});
				headerText->setAnchorPoint({ 0.f, 0.5f });
				header->addChild(headerText);
				cells->addObject(header);

				// medals
				int i = 0;
				for (auto medal : m_dataDev["medals"][type].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
					auto medalNode = CCNode::create();
					medalNode->setID(fmt::format("{}-medal-{}", type, i));
					medalNode->setScale(0.75f);
	
					//label
					auto label = CCLabelBMFont::create(medal["name"].asString().unwrapOr("null").c_str(), "bigFont.fnt");
					label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
					label->setID("label");
					label->setScale(0.5f);
					label->setAnchorPoint({ 0.f, 0.5f });
					label->setPosition({ 5.f, 17.5f });
					label->setColor(medal["color"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}));
	
					// edit menu
					auto editMenu = CCMenu::create();
					editMenu->setScale(0.75f);
					editMenu->setPosition({330.f, -25.f});
					editMenu->setID("edit-menu");

					auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
					editSpr->setScale(0.5f);
					auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(VerificationPopup::onEdit));
					editBtn->setID(type);
					editBtn->setTag(i);
					editMenu->addChild(editBtn);

					auto diffSprName = fmt::format("{}.png", m_dataDev["main"][medal["requirement"].as<int>().unwrapOr(0)]["sprite"].asString().unwrapOr("DP_Unknown"));
					auto reqSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(diffSprName).data());
					reqSpr->setScale(0.75f);
					auto reqBtn = CCMenuItemSpriteExtra::create(reqSpr, this, menu_selector(VerificationPopup::onDifficulty));
					reqBtn->setPositionX(-40.f);
					reqBtn->setID(type);
					reqBtn->setTag(i);
					editMenu->addChild(reqBtn);

					auto colorSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
					colorSpr->setScale(0.75f);
					auto colorBtn = CCMenuItemSpriteExtra::create(colorSpr, this, menu_selector(VerificationPopup::onColor));
					colorBtn->setPositionX(-80.f);
					colorBtn->setID(type);
					colorBtn->setTag(i);
					colorBtn->setColor(medal["color"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}));
					editMenu->addChild(colorBtn);

					auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
					deleteSpr->setScale(0.85f);
					auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(VerificationPopup::onDelete));
					deleteBtn->setPositionX(-120.f);
					deleteBtn->setID(type);
					deleteBtn->setTag(i);
					editMenu->addChild(deleteBtn);
	
					//add children
					medalNode->addChild(label);
					medalNode->addChild(editMenu);

					i++;

					// search function
					std::string searchLower = text;
    				std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
					
					std::string nameLower = medal["name"].asString().unwrapOr("null");
					std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

					if (searchLower.size() > 0) {
						if (nameLower.find(searchLower) == std::string::npos &&
							medal["requirement"].asString().unwrapOr("0").find(searchLower) == std::string::npos) {
							continue;
						}
					}
	
					cells->addObject(medalNode);
				}
			}

			break;
		}
	}

	if (m_list) m_list->removeAllChildrenWithCleanup(true);

	//list
	m_list = ListView::create(cells, 25.f, 315.f, 155.f);
	m_list->setPosition({52.5f, 10.f});
	m_mainLayer->addChild(m_list);

	return;
}

void VerificationPopup::loadMain(int tab)
{
	m_currentTab = tab;
	m_practiceToggle = false;

	this->setTitle("Mod Panel");
	m_mainLayer->removeAllChildrenWithCleanup(true);

	// setup tabs
	m_tabs = CCMenu::create();
	m_tabs->setPosition({210.f, 205.f});
	m_tabs->setID("data-tabs");
	m_mainLayer->addChild(m_tabs);

	auto packsBtnSprOff = ButtonSprite::create("Packs", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	packsBtnSprOff->m_BGSprite->setContentSize({90.f, 25.f});

	auto packsBtnSprOn = ButtonSprite::create("Packs", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	packsBtnSprOn->m_BGSprite->setContentSize({90.f, 25.f});

	auto packsBtn = CCMenuItemToggler::create(packsBtnSprOff, packsBtnSprOn, this, menu_selector(VerificationPopup::onToggle));
	packsBtn->setContentSize({175.f, 25.f});
	packsBtn->setPosition({-145.f, 0.f});
	packsBtn->setID("packs");
	packsBtn->setTag(0);
	packsBtn->toggle(tab == 0);
	m_tabs->addChild(packsBtn);

	auto skillsBtnSprOff = ButtonSprite::create("Skills", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	skillsBtnSprOff->m_BGSprite->setContentSize({90.f, 25.f});

	auto skillsBtnSprOn = ButtonSprite::create("Skills", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	skillsBtnSprOn->m_BGSprite->setContentSize({90.f, 25.f});

	auto skillsBtn = CCMenuItemToggler::create(skillsBtnSprOff, skillsBtnSprOn, this, menu_selector(VerificationPopup::onToggle));
	skillsBtn->setContentSize({175.f, 25.f});
	skillsBtn->setPosition({-47.5f, 0.f});
	skillsBtn->setID("skills");
	skillsBtn->setTag(1);
	skillsBtn->toggle(tab == 1);
	m_tabs->addChild(skillsBtn);

	auto lvlsBtnSprOff = ButtonSprite::create("Levels", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	lvlsBtnSprOff->m_BGSprite->setContentSize({90.f, 25.f});

	auto lvlsBtnSprOn = ButtonSprite::create("Levels", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	lvlsBtnSprOn->m_BGSprite->setContentSize({90.f, 25.f});

	auto lvlsBtn = CCMenuItemToggler::create(lvlsBtnSprOff, lvlsBtnSprOn, this, menu_selector(VerificationPopup::onToggle));
	lvlsBtn->setContentSize({175.f, 25.f});
	lvlsBtn->setPosition({47.5f, 0.f});
	lvlsBtn->setID("lvls");
	lvlsBtn->setTag(2);
	lvlsBtn->toggle(tab == 2);
	m_tabs->addChild(lvlsBtn);

	auto medalsBtnSprOff = ButtonSprite::create("Medals", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	medalsBtnSprOff->m_BGSprite->setContentSize({90.f, 25.f});

	auto medalsBtnSprOn = ButtonSprite::create("Medals", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	medalsBtnSprOn->m_BGSprite->setContentSize({90.f, 25.f});

	auto medalsBtn = CCMenuItemToggler::create(medalsBtnSprOff, medalsBtnSprOn, this, menu_selector(VerificationPopup::onToggle));
	medalsBtn->setContentSize({175.f, 25.f});
	medalsBtn->setPosition({145.f, 0.f});
	medalsBtn->setID("medals");
	medalsBtn->setTag(3);
	medalsBtn->toggle(tab == 3);
	m_tabs->addChild(medalsBtn);

	//save menu
	auto saveMenu = CCMenu::create();
	saveMenu->setPosition({210.f, 0.f});
	saveMenu->setZOrder(5);
	saveMenu->setID("save-menu");

	auto saveDevSpr = ButtonSprite::create("Save to Dev", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto saveMainSpr = ButtonSprite::create("Push to Main", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto revertSpr = ButtonSprite::create("Revert from Main", "bigFont.fnt", "GJ_button_02.png", 0.65f);

	saveDevSpr->setScale(0.625f);
	saveMainSpr->setScale(0.625f);
	revertSpr->setScale(0.625f);

	auto saveDevBtn = CCMenuItemSpriteExtra::create(saveDevSpr, this, menu_selector(VerificationPopup::onPushChanges));
	saveDevBtn->setID("save-dev-btn");
	saveDevBtn->setPositionX(-141.f);

	auto saveMainBtn = CCMenuItemSpriteExtra::create(saveMainSpr, this, menu_selector(VerificationPopup::onPushChanges));
	saveMainBtn->setID("save-main-btn");
	saveMainBtn->setPositionX(-22.f);

	auto revertBtn = CCMenuItemSpriteExtra::create(revertSpr, this, menu_selector(VerificationPopup::onPushChanges));
	revertBtn->setID("revert-btn");
	revertBtn->setPositionX(119.f);

	saveMenu->addChild(saveDevBtn);
	saveMenu->addChild(saveMainBtn);
	saveMenu->addChild(revertBtn);
	m_mainLayer->addChild(saveMenu);

	//create search menu
    auto searchMenu = CCMenu::create();
    searchMenu->setPosition(m_mainLayer->getContentWidth() / 2.f, 180.f);
    searchMenu->setID("search-menu");

	m_searchInput = TextInput::create(300.f, "Search...", "bigFont.fnt");
	m_searchInput->setCallback([this](const std::string& text) {this->onUpdateSearch(text);});
	m_searchInput->setCommonFilter(CommonFilter::Any);
	m_searchInput->setID("search-input");

    searchMenu->addChild(m_searchInput);
    m_mainLayer->addChild(searchMenu);

	// new btn
	auto newMenu = CCMenu::create();
	newMenu->setPosition({322.5f, -13.f});
	newMenu->setScale(0.75f);
	newMenu->setID("new-menu");

	auto newBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png"), this, menu_selector(VerificationPopup::onNew));
	newBtn->setID("new-btn");
	newBtn->setTag(tab);

	newMenu->addChild(newBtn);
	m_mainLayer->addChild(newMenu);

	onUpdateSearch("");

	return;
}

void VerificationPopup::onDifficulty(CCObject* sender) {
	auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
	auto type = btn->getID();
	auto id = btn->getTag();
	
	auto popup = DifficultyPopup::create(m_dataDev);
	if (id == -200) {
		popup->setCallback(
			[this](const int& ret) {
				m_dataDev[m_index][m_packID].set("mainPack", ret);
				auto offs = m_list->m_tableView->m_contentLayer->getPositionY();
				loadPack(m_index, m_packID);
				m_list->m_tableView->m_contentLayer->setPositionY(offs);
			}
		);
	}
	else if (id == -100) {
		popup->setCallback(
			[this](const int& ret) {
				m_levelData.set("difficulty", ret);
				auto offs = m_list->m_tableView->m_contentLayer->getPositionY();
				refreshLevel();
				m_list->m_tableView->m_contentLayer->setPositionY(offs);
			}
		);
	}
	else {
		popup->setCallback(
			[this, type, id](const int& ret) {
				m_dataDev["medals"][type][id].set("requirement", ret);
				auto offs = m_list->m_tableView->m_contentLayer->getPositionY();
				onUpdateSearch(m_searchInput->getString());
				m_list->m_tableView->m_contentLayer->setPositionY(offs);
			}
		);
	}
	popup->show();
	
	return;
}

void VerificationPopup::onColor(CCObject* sender) {
	auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
	auto type = btn->getID();
	auto id = btn->getTag();
	
	auto popup = ColorPickPopup::create(btn->getColor());
	popup->setCallback(
		[this, type, id](const ccColor4B& color) {
			m_dataDev["medals"][type][id].set("color", to3B(color));
			auto offs = m_list->m_tableView->m_contentLayer->getPositionY();
			onUpdateSearch(m_searchInput->getString());
			m_list->m_tableView->m_contentLayer->setPositionY(offs);
		}
	);
	popup->show();
	
	return;
}

void VerificationPopup::onEdit(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto tag = btn->getTag();
	auto id = btn->getID();

	if (m_currentTab == 3) {
		log::info("Editing Medal: {}-{}", id, tag);

		auto popup = RenamePopup::create(m_dataDev["medals"][id][tag]["name"].asString().unwrapOrDefault());
		popup->setCallback(
			[this, id, tag](const std::string& ret) {
				m_dataDev["medals"][id][tag].set("name", ret);
				auto offs = m_list->m_tableView->m_contentLayer->getPositionY();
				onUpdateSearch(m_searchInput->getString());
				m_list->m_tableView->m_contentLayer->setPositionY(offs);
			}
		);
		popup->show();
	}
	else if (id == "edit-practice-id") {
		log::info("Editing Startpos ID at: {}", tag);

		auto popup = AddLevelPopup::create();
		popup->m_practiceIndex = tag;
		popup->show();
	}
	else if (tag == -400) {
		auto popup = TextEffectEditorPopup::create(m_currentData["textEffects"]);
		popup->setCallback(
			[this](const matjson::Value& ret) {
				m_currentData.set("textEffects", ret);
			}
		);
		popup->show();
	}
	else if (tag == -300) {
		log::info("Editing Level: {}", id);

		m_index = "main";
		loadLevel(numFromString<int>(id).unwrapOrDefault());
	}
	else if (tag == -200) EditDescriptionPopup::create(m_currentData["description"].asString().unwrapOr("???"))->show();
	else if (tag == -100) {
		log::info("Editing Level: {}", id);

		loadLevel(numFromString<int>(id).unwrapOrDefault());
	}
	else if (m_dataDev["skillset-data"].contains(id)) {
		log::info("Editing Skillset: {}", id);

		loadSkill(id);
	}
	else {
		log::info("Editing Pack: {}-{}", id, tag);

		loadPack(id, tag);
	}
	
	return;
}

void VerificationPopup::onMove(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto tag = btn->getTag();
	auto id = btn->getID();

	if (tag <= -100) {
		log::info("Moving Level: {}", id);

		MovePopup::create("level", id, std::abs(tag + 100))->show();
	}
	else {
		log::info("Moving Pack: {}-{}", id, tag);

		MovePopup::create("pack", id, tag)->show();
	}

	// backup save
	auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\dev_autosave.json");
	file::writeToJson(path, m_dataDev);
	
	return;
}

void VerificationPopup::onDelete(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto tag = btn->getTag();
	auto id = btn->getID();

	if (tag <= -100) {
		log::info("Removing Level: {}", id);

		geode::createQuickPopup(
			"Remove Level",																					// title
			"Are you sure you want to <cr>remove</c> this level from the list?\n<cy>The level data will be saved if it is still in a pack.</c>", // content
			"No", "Yes",																					// buttons
			[this, id, tag](auto, bool btn2) {
				if (btn2) {
					if (m_index == "main") {
						geode::createQuickPopup(
							"Move to Legacy",																					// title
							"Would you like to move this <cy>Main Pack Level</c> to the <cy>Legacy List</c>?\n(If none for this tier exists, one will be created)", // content
							"No", "Yes",																					// buttons
							[this, id, tag](auto, bool btn2b) {
								log::info("{}", id);
								if (btn2b) {
									moveToLegacy(id);
									auto pos = std::abs(tag + 100);
									removeObject("level", id, pos);
									log::info("Level moved to legacy.");
								}
								else {
									auto pos = std::abs(tag + 100);
									removeObject("level", id, pos);
									log::info("Level removed.");
								}
							});
					}
					else {
						auto pos = std::abs(tag + 100);
						removeObject("level", id, pos);
						log::info("Level removed.");
					}
				}
			});
	}
	else if (tag == 100 && m_dataDev["level-data"].contains(id)) {
		log::info("Removing Level: {}", id);

		geode::createQuickPopup(
			"Remove Level",																					// title
			"Are you sure you want to <cr>remove</c> this level from the database?\n<cr>This cannot be undone.</c>", // content
			"No", "Yes",																					// buttons
			[this, id](auto, bool btn2) {
				if (btn2) {
					removeObject("level-data", id);
					log::info("Level removed.");
				}
			});
	}
	else if (m_dataDev["skillset-data"].contains(id)) {
		log::info("Removing Skillset: {}", id);

		geode::createQuickPopup(
			"Remove Skillset",																					// title
			"Are you sure you want to <cr>delete</c> this skillset?\n<cr>This cannot be undone.</c>", // content
			"No", "Yes",																					// buttons
			[this, id](auto, bool btn2) {
				if (btn2) {
					removeObject("skillset", id, 0);
					log::info("Skillset removed.");
				}
			});
	}
	else if (m_currentTab == 3) {
		log::info("Removing Medal: {}-{}", id, tag);

		geode::createQuickPopup(
			"Remove Medal",																					// title
			"Are you sure you want to <cr>delete</c> this medal?\n<cr>This cannot be undone.</c>", // content
			"No", "Yes",																					// buttons
			[this, id, tag](auto, bool btn2) {
				if (btn2) {
					removeObject("medal", id, tag);
					log::info("Medal removed.");
				}
			});
	}
	else {
		log::info("Removing Pack: {}-{}", id, tag);

		geode::createQuickPopup(
			"Remove Pack",																					// title
			"Are you sure you want to <cr>delete</c> this pack?\n<cr>This cannot be undone.</c>", // content
			"No", "Yes",																					// buttons
			[this, id, tag](auto, bool btn2) {
				if (btn2) {
					removeObject("pack", id, tag);
					log::info("Pack removed.");
				}
			});
	}

	// backup save
	auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\dev_autosave.json");
	file::writeToJson(path, m_dataDev);
	
	return;
}

void VerificationPopup::moveToLegacy(std::string id) {
	if (!m_dataDev["level-data"].contains(id)) return;

	// get difficulty
	auto diff = m_dataDev["level-data"][id]["difficulty"].as<int>().unwrapOr(0);

	// iterate through legacy packs to find the correct one
	for (int i = 0; i < m_dataDev["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
		if (m_dataDev["legacy"][i]["mainPack"].as<int>().unwrapOr(-1) == diff) {
			auto lvls = m_dataDev["legacy"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
			lvls.insert(lvls.begin(), numFromString<int>(id).unwrapOr(0));
			m_dataDev["legacy"][i].set("levelIDs", lvls);
			
			// backup save
			auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\dev_autosave.json");
			file::writeToJson(path, m_dataDev);
			
			return;
		}
	}

	// if no packs match, automatically make a new one
	auto legacyPacks = m_dataDev["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
	auto mainPackData = m_dataDev["main"][diff];

	auto newLegacy = GDDPLegacyPackFormat{
		.name = fmt::format("Legacy {}", mainPackData["name"].asString().unwrapOr("???")),
		.description = fmt::format("Levels that used to be in the {} Tier.", mainPackData["name"].asString().unwrapOr("???")),
		.sprite = mainPackData["sprite"].asString().unwrapOr("DP_Unknown"),
		.plusSprite = mainPackData["plusSprite"].asString().unwrapOr("DP_Unknown"),
		.saveID = fmt::format("legacy-{}", mainPackData["saveID"].asString().unwrapOr("null")),
		.levelIDs = {numFromString<int>(id).unwrapOr(0)},
		.mainPack = diff
	};

	legacyPacks.insert(std::min(legacyPacks.begin() + diff, legacyPacks.end()), newLegacy);
	m_dataDev.set("legacy", legacyPacks);

	// backup save
	auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\dev_autosave.json");
	file::writeToJson(path, m_dataDev);
	
	return;
}

void VerificationPopup::removeObject(std::string type, std::string id, int pos) {
	
	if (type == "pack") {
		std::vector<matjson::Value> packs = m_dataDev[id].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
		packs.erase(packs.begin() + pos);

		//if id (index) is main, subtract everything pointing to pos by 1 
		if (id == "main") {
			auto levels = m_dataDev["level-data"];
			auto legacyPacks = m_dataDev["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

			for (auto [key, value] : levels) if (value["difficulty"].as<int>().unwrapOr(0) > pos) levels[key].set("difficulty", value["difficulty"].as<int>().unwrapOr(0) - 1);

			auto packPos = 0;
			for (auto pack : legacyPacks) {
				if (pack["mainPack"].as<int>().unwrapOr(0) > pos) legacyPacks.at(packPos).set("mainPack", pack["mainPack"].as<int>().unwrapOr(0) - 1);
				packPos += 1;
			}

			m_dataDev.set("level-data", levels);
			m_dataDev.set("legacy", legacyPacks);
		}
		
		m_dataDev.set(id, packs);

		auto offs = m_list->m_tableView->m_contentLayer->getPositionY();

		onUpdateSearch(m_searchInput->getString());
		m_list->m_tableView->m_contentLayer->setPositionY(offs);
	} 
	else if (type == "skillset") {
		m_dataDev["skillset-data"].erase(id);
		auto offs = m_list->m_tableView->m_contentLayer->getPositionY();
		onUpdateSearch(m_searchInput->getString());
		m_list->m_tableView->m_contentLayer->setPositionY(offs);
	}
	else if (type == "level") {
		std::vector<int> lvlList = m_currentData["levelIDs"].as<std::vector<int>>().unwrapOrDefault();

		lvlList.erase(lvlList.begin() + pos);
		m_currentData.set("levelIDs", lvlList);

		auto offs = m_levelList->m_tableView->m_contentLayer->getPositionY();

		loadPack(m_index, m_packID, true);
		m_levelList->m_tableView->m_contentLayer->setPositionY(offs);
	}
	else if (type == "level-data") {
		m_dataDev["level-data"].erase(id);

		auto offs = m_list->m_tableView->m_contentLayer->getPositionY();
		onUpdateSearch(m_searchInput->getString());
		m_list->m_tableView->m_contentLayer->setPositionY(offs);
	}
	else if (type == "medal") {
		std::vector<matjson::Value> medalList = m_dataDev["medals"][id].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

		medalList.erase(medalList.begin() + pos);
		m_dataDev["medals"].set(id, medalList);

		auto offs = m_list->m_tableView->m_contentLayer->getPositionY();

		onUpdateSearch(m_searchInput->getString());
		m_list->m_tableView->m_contentLayer->setPositionY(offs);
	}

	// backup save
	auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\dev_autosave.json");
	file::writeToJson(path, m_dataDev);
	
	return;
}

void VerificationPopup::onSave(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto tag = btn->getTag();

	switch(tag) {
		case 0: //Save Pack
		{
			log::info("Saving Pack...");

			//convert text inputs
			auto name = typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-name")->getChildByID("value-menu")->getChildByID("value-input"))->getString();
			auto sprite = typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-sprite")->getChildByID("value-menu")->getChildByID("value-input"))->getString();
			std::string plusSprite = (m_index == "main" || m_index == "legacy") ? typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-plusSprite")->getChildByID("value-menu")->getChildByID("value-input"))->getString() : "DP_Unknown";
			std::string saveID = (m_index != "monthly") ? typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-saveID")->getChildByID("value-menu")->getChildByID("value-input"))->getString() : "null";
			int reqLevels = (m_index == "main") ? numFromString<int>(typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-reqLevels")->getChildByID("value-menu")->getChildByID("value-input"))->getString()).unwrapOrDefault() : 999;
			int month = (m_index == "monthly") ? numFromString<int>(typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-month")->getChildByID("value-menu")->getChildByID("value-input"))->getString()).unwrapOrDefault() : 1;
			int year = (m_index == "monthly") ? numFromString<int>(typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-year")->getChildByID("value-menu")->getChildByID("value-input"))->getString()).unwrapOrDefault() : 1987;

			matjson::Value packData;
			
			if (m_index == "main") {
				packData = GDDPMainPackFormat{
					.name = name,
					.description = m_currentData["description"].as<std::string>().unwrapOr("???"),
					.sprite = sprite,
					.plusSprite = plusSprite,
					.saveID = saveID,
					.levelIDs = m_currentData["levelIDs"].as<std::vector<int>>().unwrapOrDefault(),
					.reqLevels = reqLevels,
					.textEffects = m_currentData["textEffects"]
				};
			}
			if (m_index == "legacy") {
				packData = GDDPLegacyPackFormat{
					.name = name,
					.description = m_currentData["description"].as<std::string>().unwrapOr("???"),
					.sprite = sprite,
					.plusSprite = plusSprite,
					.saveID = saveID,
					.levelIDs = m_currentData["levelIDs"].as<std::vector<int>>().unwrapOrDefault(),
					.mainPack = m_currentData["mainPack"].as<int>().unwrapOrDefault()
				};
			}
			if (m_index == "bonus") {
				packData = GDDPBonusPackFormat{
					.name = name,
					.description = m_currentData["description"].as<std::string>().unwrapOr("???"),
					.sprite = sprite,
					.saveID = saveID,
					.levelIDs = m_currentData["levelIDs"].as<std::vector<int>>().unwrapOrDefault(),
					.textEffects = m_currentData["textEffects"]
				};
			}
			if (m_index == "monthly") {
				packData = GDDPMonthlyPackFormat{
					.name = name,
					.description = m_currentData["description"].as<std::string>().unwrapOr("???"),
					.sprite = sprite,
					.month = month,
					.year = year,
					.levelIDs = m_currentData["levelIDs"].as<std::vector<int>>().unwrapOrDefault(),
					.textEffects = m_currentData["textEffects"]
				};
			}

			//save pack in index
			auto data = m_dataDev[m_index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
			data.at(m_packID) = packData;

			m_dataDev.set(m_index, data);

			log::info("Pack {}-{} saved!", m_index, m_packID);
			loadMain(0);

			break;
		}
		case 1: //Save Level
		{
			log::info("Saving Level...");

			auto levelID = typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-level-id")->getChildByID("value-menu")->getChildByID("value-input"))->getString();
			auto idChanged = false;
			if (numFromString<int>(levelID).unwrapOrDefault() != m_levelID) idChanged = true;

			//convert text inputs
			auto name = typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-name")->getChildByID("value-menu")->getChildByID("value-input"))->getString();
			matjson::Value xp;
			if (m_list->getChildByIDRecursive("property-xp")) { //we only need to check for this since the rest can't exist without it
				for (auto [key, value] : XPUtils::skills) {
					xp.set(key, numFromString<int>(typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive(fmt::format("property-xp-{}", key))->getChildByID("value-menu")->getChildByID("value-input"))->getString()).unwrapOrDefault());
				}
			}

			if (idChanged) {
				//remove old level data and change level IDs in level lists
				m_dataDev["level-data"].erase(std::to_string(m_levelID));

				std::vector<std::string> packIndexes = {"main", "legacy", "bonus", "monthly"};
				for (auto index : packIndexes) {
					auto data = m_dataDev[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

					auto packPos = 0;
					for (auto pack : data) {
						auto lvlList = pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
						
						auto pos = 0;
						for (auto lvl : lvlList) {
							if (lvl == m_levelID) {
								lvlList.at(pos) = numFromString<int>(levelID).unwrapOrDefault();
								data.at(packPos).set("levelIDs", lvlList);
							}
							pos += 1;
						}
						packPos += 1;
					}
					m_dataDev.set(index, data);
				}
			}

			auto lvlData = GDDPLevelFormat{
				.name = name,
				.difficulty = m_levelData["difficulty"].as<int>().unwrapOrDefault(),
				.skillsets = m_levelData["skillsets"].as<std::vector<std::string>>().unwrapOrDefault(),
				.xp = xp,
				.startposCopy =	m_levelData["startpos-copy"].as<int>().unwrapOrDefault()
			};

			//save level in level data
			auto data = m_dataDev["level-data"];
			data.set(levelID, lvlData);

			if (xp.size() < 1) { 
				auto newLvlData = data[levelID];
				newLvlData.erase("xp");
				data.set(levelID, newLvlData);
			}

			m_dataDev.set("level-data", data);

			log::info("Level {} saved!", levelID);
			if (m_currentTab == 2) loadMain(2);
			else loadPack(m_index, m_packID, true);

			break;
		}
		case 2: //Save Skillset
		{
			log::info("Saving Skillset...");

			auto id = typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-key")->getChildByID("value-menu")->getChildByID("value-input"))->getString();
			auto idChanged = (m_index != id);

			//convert text inputs
			auto name = typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-display-name")->getChildByID("value-menu")->getChildByID("value-input"))->getString();
			auto type = typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-type")->getChildByID("value-menu")->getChildByID("value-input"))->getString();
			auto sprite = typeinfo_cast<TextInput*>(m_list->getChildByIDRecursive("property-sprite")->getChildByID("value-menu")->getChildByID("value-input"))->getString();

			if (!(type == "none" ||
				type == "gamemode" ||
				type == "misc" ||
				type == "platformer" ||
				type == "special")) type = "none";

			if (idChanged) {
				//remove old skill data and change skills in levels
				m_dataDev["skillset-data"].erase(m_index);

				auto data = m_dataDev["level-data"];
				for (auto lvl : data) {
					auto skills = lvl["skillsets"].as<std::vector<std::string>>().unwrapOrDefault();

					auto pos = 0;
					for (auto skillID : skills) {
						
						if (skillID == m_index) {
							skills.at(pos) = id;
						}

						pos += 1;
					}
					lvl.set("skillsets", skills);
					data.set(lvl.getKey().value(), lvl);
				}
				m_dataDev.set("level-data", data);
			}

			auto skillData = GDDPSkillsetFormat{
				.displayName = name,
				.type = type,
				.description = m_currentData["description"].as<std::string>().unwrapOr("???"),
				.sprite = sprite
			};

			//save skillset in skillset data
			m_dataDev["skillset-data"].set(id, skillData);

			log::info("Skillset {} saved!", id);
			loadMain(1);

			break;
		}
	}

	// backup save
	auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\dev_autosave.json");
	file::writeToJson(path, m_dataDev);
	
	return;
}

void VerificationPopup::onDBSync(CCObject*) {
	auto input = Mod::get()->getConfigDir() += std::filesystem::path("\\xpsync.json");
	matjson::Value sData = file::readJson(input).unwrapOrDefault();
	for (auto p : sData["list"]["collections"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
		if (p["type"].asString().unwrapOrDefault() != "tiers") continue;
		log::info("converting {}", p["name"].asString().unwrapOr("???"));
		for (auto l : p["demons"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
			auto id = l["id"].asString().unwrapOr("0");
			if (m_dataDev["level-data"].contains(id)) {
				for (auto [key, value] : l["skillRatings"]) {
					m_dataDev["level-data"][id]["xp"].set(key, value);
				}
			}
		}
	}
	auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\conversion.json");
	file::writeToJson(path, m_dataDev);
	log::info("Finished Sync");
}

void VerificationPopup::loadPack(std::string index, int id, bool fromLvl) {

	this->setTitle("Edit Pack");
	m_mainLayer->removeAllChildrenWithCleanup(true);

	// back button
	auto backMenu = CCMenu::create();
	backMenu->setPosition({0.f, 0.f});
	backMenu->setID("back-menu");
	backMenu->setZOrder(10);
	m_mainLayer->addChild(backMenu);

	auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backBtn = CCMenuItemSpriteExtra::create(backSpr, this, menu_selector(VerificationPopup::onBack));
	backBtn->setPosition(this->m_closeBtn->getPositionX(), this->m_closeBtn->getPositionY() - 40.f);
	backBtn->setID("back-btn");
	backBtn->setTag(0);
	backMenu->addChild(backBtn);

	//save button
	auto saveMenu = CCMenu::create();
	saveMenu->setID("search-menu");
	saveMenu->setPosition({ 210.f, 0.f });
	saveMenu->setZOrder(10);
	m_mainLayer->addChild(saveMenu);

	auto saveSpr = ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto saveBtn = CCMenuItemSpriteExtra::create(saveSpr, this, menu_selector(VerificationPopup::onSave));
	saveBtn->setTag(0);
	saveMenu->addChild(saveBtn);

	// new btn
	auto newMenu = CCMenu::create();
	newMenu->setPosition({322.5f, 168.f});
	newMenu->setScale(0.75f);
	newMenu->setID("new-menu");

	auto newBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png"), this, menu_selector(VerificationPopup::onNew));
	newBtn->setID("new-btn");
	newBtn->setTag(2);

	newMenu->addChild(newBtn);
	m_mainLayer->addChild(newMenu);

	//startpos toggle
	auto toggleMenu = CCMenu::create();
	toggleMenu->setPosition({187.5f, 168.f});
	toggleMenu->setScale(0.75f);
	toggleMenu->setID("toggle-menu");

	auto toggleBtnSprOff = ButtonSprite::create("Startpos", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto toggleBtnSprOn = ButtonSprite::create("Startpos", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto toggleBtn = CCMenuItemToggler::create(toggleBtnSprOff, toggleBtnSprOn, this, menu_selector(VerificationPopup::onToggle));
	toggleBtn->setID("startpos-toggle");
	toggleBtn->toggle(m_practiceToggle);
	toggleMenu->addChild(toggleBtn);
	m_mainLayer->addChild(toggleMenu);

	//set up data
	m_index = index;
	m_packID = id;
	auto data = m_dataDev[index][id];
	if (fromLvl) {
		data = m_currentData;
		m_levelID = -1;
	}
	else m_currentData = data;

	//log::info("{}", m_currentData["description"].asString().unwrapOr("uhhh"));

	//set up edit menu
	//level IDs will be on the right side, base pack options on the left

	//property cells
	auto cells = CCArray::create();

	for (auto [key, value] : data) {
		if (key != "levelIDs" && key != "description" && key != "textEffects" && key != "mainPack") {
			auto propertyNode = CCNode::create();
			propertyNode->setID(fmt::format("property-{}", key));
			propertyNode->setScale(0.75f);

			// label
			auto label = CCLabelBMFont::create(key.c_str(), "bigFont.fnt");
			label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
			label->setID("label");
			label->setScale(0.5f);
			label->setAnchorPoint({0.f, 0.5f});
			label->setPosition({5.f, 17.5f});

			// edit menu
			auto editMenu = CCMenu::create();
			editMenu->setScale(0.75f);
			editMenu->setPosition({42.f, -25.f});
			editMenu->setID("value-menu");

			auto valueInput = TextInput::create(190.f, "...", "bigFont.fnt");
			if (value.isNumber()) {
				valueInput->setString(std::to_string(value.as<int>().unwrapOr(0)));
				valueInput->setCommonFilter(CommonFilter::Int);
			}
			else {
				valueInput->setString(value.asString().unwrapOr(""));
				valueInput->setCommonFilter(CommonFilter::Any);
			}
			valueInput->setPosition({ 180.f, 0.f });
			valueInput->setAnchorPoint({ 1.f, 0.5f });
			valueInput->setID("value-input");
			editMenu->addChild(valueInput);

			// add children
			propertyNode->addChild(label);
			propertyNode->addChild(editMenu);

			cells->addObject(propertyNode);
		}
		else if (key == "description") {
			auto propertyNode = CCNode::create();
			propertyNode->setID(fmt::format("property-{}", key));
			propertyNode->setScale(0.75f);

			// label
			auto label = CCLabelBMFont::create(key.c_str(), "bigFont.fnt");
			label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
			label->setID("label");
			label->setScale(0.5f);
			label->setAnchorPoint({0.f, 0.5f});
			label->setPosition({5.f, 17.5f});

			// edit menu
			auto editMenu = CCMenu::create();
			editMenu->setScale(0.75f);
			editMenu->setPosition({165.f, -25.f});
			editMenu->setID("edit-menu");

			auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
			editSpr->setScale(0.5f);
			auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(VerificationPopup::onEdit));
			editBtn->setID("edit-button");
			editBtn->setTag(-200);
			editMenu->addChild(editBtn);

			// add children
			propertyNode->addChild(label);
			propertyNode->addChild(editMenu);

			cells->addObject(propertyNode);
		}
		else if (key == "textEffects") {
			auto propertyNode = CCNode::create();
			propertyNode->setID(fmt::format("property-{}", key));
			propertyNode->setScale(0.75f);

			// label
			auto label = CCLabelBMFont::create(key.c_str(), "bigFont.fnt");
			label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
			label->setID("label");
			label->setScale(0.5f);
			label->setAnchorPoint({0.f, 0.5f});
			label->setPosition({5.f, 17.5f});

			// edit menu
			auto editMenu = CCMenu::create();
			editMenu->setScale(0.75f);
			editMenu->setPosition({165.f, -25.f});
			editMenu->setID("edit-menu");

			auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
			editSpr->setScale(0.5f);
			auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(VerificationPopup::onEdit));
			editBtn->setID("edit-button");
			editBtn->setTag(-400);
			editMenu->addChild(editBtn);

			// add children
			propertyNode->addChild(label);
			propertyNode->addChild(editMenu);

			cells->addObject(propertyNode);
		}
		else if (key == "mainPack") {
			auto propertyNode = CCNode::create();
			propertyNode->setID(fmt::format("property-{}", key));
			propertyNode->setScale(0.75f);

			// label
			auto label = CCLabelBMFont::create(key.c_str(), "bigFont.fnt");
			label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
			label->setID("label");
			label->setScale(0.5f);
			label->setAnchorPoint({0.f, 0.5f});
			label->setPosition({5.f, 17.5f});

			// edit menu
			auto editMenu = CCMenu::create();
			editMenu->setScale(0.75f);
			editMenu->setPosition({165.f, -25.f});
			editMenu->setID("edit-menu");

			auto sprName = fmt::format("{}.png", m_dataDev["main"][value.as<int>().unwrapOr(0)]["sprite"].asString().unwrapOr("DP_Unknown"));
			auto diffSpr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(sprName).data(), CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr));
			diffSpr->setScale(0.75f);

			auto diffBtn = CCMenuItemSpriteExtra::create(diffSpr, this, menu_selector(VerificationPopup::onDifficulty));
			diffBtn->setID("difficulty-btn");
			diffBtn->setTag(-200);
			editMenu->addChild(diffBtn);

			// add children
			propertyNode->addChild(label);
			propertyNode->addChild(editMenu);

			cells->addObject(propertyNode);
		}
	}

	//list
	m_list = ListView::create(cells, 25.f, 190.f, 180.f);
	m_list->setPosition({15.5f, 10.f});
	m_mainLayer->addChild(m_list);

	//levelID cells

	cells = CCArray::create();

	if (m_practiceToggle) {
		for (auto id : data["levelIDs"].as<std::vector<int>>().unwrapOrDefault()) {
			auto practiceID = m_dataDev["level-data"][std::to_string(id)]["startpos-copy"].as<int>().unwrapOr(0);
			
			auto idNode = CCNode::create();
			idNode->setID(fmt::format("level-sp-{}", id));
			idNode->setScale(0.75f);

			std::string levelName = "???";
			if (m_dataDev["level-data"].contains(std::to_string(id))) levelName = m_dataDev["level-data"][std::to_string(id)]["name"].asString().unwrapOr("???");

			// label
			auto label = CCLabelBMFont::create(fmt::format("{}\n({}) [SP]", levelName, practiceID).c_str(), "bigFont.fnt");
			label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
			label->setID("label");
			label->setScale(0.5f);
			label->setAnchorPoint({0.f, 0.5f});
			label->setPosition({5.f, 17.5f});
			if (practiceID == 0) { label->setColor({ 255, 0, 0 }); }

			// edit menu
			auto editMenu = CCMenu::create();
			editMenu->setScale(0.75f);
			editMenu->setPosition({165.f, -25.f});
			editMenu->setID("edit-menu");

			auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
			editSpr->setScale(0.5f);
			auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(VerificationPopup::onEdit));
			editBtn->setID("edit-practice-id");
			editBtn->setTag(id);
			editMenu->addChild(editBtn);

			// add children
			idNode->addChild(label);
			idNode->addChild(editMenu);

			cells->addObject(idNode);
		}
	} else {
		auto pos = 0;
		for (auto id : data["levelIDs"].as<std::vector<int>>().unwrapOr(std::vector<int>(1, 0))) {
			auto idNode = CCNode::create();
			idNode->setID(fmt::format("level-{}", id));
			idNode->setScale(0.75f);

			std::string levelName = "???";
			if (m_dataDev["level-data"].contains(std::to_string(id))) levelName = m_dataDev["level-data"][std::to_string(id)]["name"].asString().unwrapOr("???");
			if (id == 0) levelName = "PLACEHOLDER";

			// label
			auto label = CCLabelBMFont::create(fmt::format("{}. {}\n({})", (pos + 1), levelName, id).c_str(), "bigFont.fnt");
			label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
			label->setID("label");
			label->setScale(0.5f);
			label->setAnchorPoint({0.f, 0.5f});
			label->setPosition({5.f, 17.5f});
			if (levelName == "???") label->setColor({ 255, 0, 0 });
			if (levelName == "PLACEHOLDER") label->setColor({ 0, 255, 0 });

			// edit menu
			auto editMenu = CCMenu::create();
			editMenu->setScale(0.75f);
			editMenu->setPosition({165.f, -25.f});
			editMenu->setID("edit-menu");

			auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
			editSpr->setScale(0.5f);
			auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(VerificationPopup::onEdit));
			editBtn->setID(std::to_string(id));
			editBtn->setTag(-100);
			if (id > 0) editMenu->addChild(editBtn);

			auto moveSpr = CCSprite::createWithSpriteFrameName("GJ_editModeBtn_001.png");
			moveSpr->setScale(0.85f);
			auto moveBtn = CCMenuItemSpriteExtra::create(moveSpr, this, menu_selector(VerificationPopup::onMove));
			moveBtn->setPositionX(-40.f);
			moveBtn->setTag((-100 - pos));
			moveBtn->setID(std::to_string(id));
			editMenu->addChild(moveBtn);

			auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
			deleteSpr->setScale(0.85f);
			auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(VerificationPopup::onDelete));
			deleteBtn->setPositionX(-80.f);
			deleteBtn->setTag((-100 - pos));
			deleteBtn->setID(std::to_string(id));
			editMenu->addChild(deleteBtn);

			// add children
			idNode->addChild(label);
			idNode->addChild(editMenu);

			cells->addObject(idNode);
			pos += 1;
		}
	}

	//list
	m_levelList = ListView::create(cells, 25.f, 190.f, 180.f);
	m_levelList->setPosition({215.5f, 10.f});
	m_mainLayer->addChild(m_levelList);

	return;
}

void VerificationPopup::loadLevel(int id) {
	this->setTitle("Edit Level");
	m_mainLayer->removeAllChildrenWithCleanup(true);

	m_levelID = id;

	// back button
	auto backMenu = CCMenu::create();
	backMenu->setPosition({0.f, 0.f});
	backMenu->setID("back-menu");
	backMenu->setZOrder(10);
	m_mainLayer->addChild(backMenu);

	auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backBtn = CCMenuItemSpriteExtra::create(backSpr, this, menu_selector(VerificationPopup::onBack));
	backBtn->setPosition(this->m_closeBtn->getPositionX(), this->m_closeBtn->getPositionY() - 40.f);
	backBtn->setID("back-btn");
	backBtn->setTag(1);
	backMenu->addChild(backBtn);

	//save button
	auto saveMenu = CCMenu::create();
	saveMenu->setID("search-menu");
	saveMenu->setPosition({ 210.f, 0.f });
	saveMenu->setZOrder(10);
	m_mainLayer->addChild(saveMenu);

	auto saveSpr = ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto saveBtn = CCMenuItemSpriteExtra::create(saveSpr, this, menu_selector(VerificationPopup::onSave));
	saveBtn->setTag(1);
	saveMenu->addChild(saveBtn);

	//setup data
	matjson::Value data = GDDPLevelFormat{};
	if (m_dataDev["level-data"].contains(std::to_string(id))) data = m_dataDev["level-data"][std::to_string(id)];
	m_levelData = data;

	if (m_index == "main" && !data.contains("xp")) data.set("xp", GDDPLevelFormat{}.xp);

	//cells
	auto cells = CCArray::create();

	//create id input
	auto propertyNode = CCNode::create();
	propertyNode->setID(fmt::format("property-level-id"));
	propertyNode->setScale(0.75f);

	// label
	auto label = CCLabelBMFont::create("levelID", "bigFont.fnt");
	label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
	label->setID("label");
	label->setScale(0.5f);
	label->setAnchorPoint({0.f, 0.5f});
	label->setPosition({5.f, 17.5f});

	// edit menu
	auto editMenu = CCMenu::create();
	editMenu->setScale(0.75f);
	editMenu->setPosition({205.f, -25.f});
	editMenu->setID("value-menu");

	auto valueInput = TextInput::create(190.f, "Level ID (Required)", "bigFont.fnt");
	valueInput->setString(std::to_string(id));
	valueInput->setCommonFilter(CommonFilter::ID);
	valueInput->setPosition({180.f, 0.f});
	valueInput->setAnchorPoint({1.f, 0.5f});
	valueInput->setID("value-input");
	editMenu->addChild(valueInput);

	// add children
	propertyNode->addChild(label);
	propertyNode->addChild(editMenu);

	cells->addObject(propertyNode);

	for (auto [key, value] : data) {
		auto propertyNode = CCNode::create();
		propertyNode->setID(fmt::format("property-{}", key));
		propertyNode->setScale(0.75f);

		// label
		auto label = CCLabelBMFont::create(key.c_str(), "bigFont.fnt");
		label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
		label->setID("label");
		label->setScale(0.5f);
		label->setAnchorPoint({0.f, 0.5f});
		label->setPosition({5.f, 17.5f});

		// edit menu
		auto editMenu = CCMenu::create();
		editMenu->setScale(0.75f);
		editMenu->setPosition({205.f, -25.f});
		editMenu->setID("value-menu");

		if (key == "startpos-copy") continue;
		else if (key == "xp") {
			if (m_index != "main") continue;

			// add children
			propertyNode->addChild(label);

			cells->addObject(propertyNode);

			for (auto [key2, value2] : XPUtils::skills) {
				auto XPpropertyNode = CCNode::create();
				XPpropertyNode->setID(fmt::format("property-xp-{}", key2));
				XPpropertyNode->setScale(0.75f);

				// label
				auto XPlabel = CCLabelBMFont::create(value2["name"].asString().unwrapOr("???").c_str(), "bigFont.fnt");
				XPlabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
				XPlabel->setID("label");
				XPlabel->setScale(0.5f);
				XPlabel->setAnchorPoint({0.f, 0.5f});
				XPlabel->setPosition({20.f, 17.5f});

				// edit menu
				auto xpEditMenu = CCMenu::create();
				xpEditMenu->setScale(0.75f);
				xpEditMenu->setPosition({205.f, -25.f});
				xpEditMenu->setID("value-menu");

				auto valueInput = TextInput::create(190.f, "...", "bigFont.fnt");
				valueInput->setString(std::to_string(value[key2].as<int>().unwrapOr(0)));
				valueInput->setCommonFilter(CommonFilter::Int);
				valueInput->setPosition({ 180.f, 0.f });
				valueInput->setAnchorPoint({ 1.f, 0.5f });
				valueInput->setID("value-input");
				xpEditMenu->addChild(valueInput);

				XPpropertyNode->addChild(XPlabel);
				XPpropertyNode->addChild(xpEditMenu);

				cells->addObject(XPpropertyNode);
			}

			continue;
		}
		else if (key == "skillsets") {
			auto skillsetsNode = CCNode::create();
			skillsetsNode->setID("skillsets-node");
			skillsetsNode->setScale(0.75f);

			propertyNode->addChild(skillsetsNode);
		}
		else if (key == "difficulty") {
			auto difficultyNode = CCNode::create();
			difficultyNode->setID("difficulty-node");
			difficultyNode->setScale(0.75f);

			propertyNode->addChild(difficultyNode);
		}
		else {
			auto valueInput = TextInput::create(190.f, "...", "bigFont.fnt");
			if (value.isNumber()) {
				valueInput->setString(std::to_string(value.as<int>().unwrapOr(0)));
				valueInput->setCommonFilter(CommonFilter::Int);
			}
			else {
				valueInput->setString(value.asString().unwrapOr(""));
				valueInput->setCommonFilter(CommonFilter::Any);
			}
			valueInput->setPosition({ 180.f, 0.f });
			valueInput->setAnchorPoint({ 1.f, 0.5f });
			valueInput->setID("value-input");
			editMenu->addChild(valueInput);
		}

		// add children
		propertyNode->addChild(label);
		propertyNode->addChild(editMenu);

		cells->addObject(propertyNode);
	}

	//list
	m_list = ListView::create(cells, 25.f, 315.f, 180.f);
	m_list->setPosition({52.5f, 10.f});
	m_mainLayer->addChild(m_list);

	refreshLevel();

	return;
}

void VerificationPopup::refreshLevel() {
	auto skillsetsNode = this->getChildByIDRecursive("skillsets-node");
	auto difficultyNode = this->getChildByIDRecursive("difficulty-node");
	if (skillsetsNode) skillsetsNode->removeAllChildrenWithCleanup(true);
	if (difficultyNode) difficultyNode->removeAllChildrenWithCleanup(true);
	
	auto skillsets = m_levelData["skillsets"].as<std::vector<std::string>>().unwrapOrDefault();

	//create menu (with layout)
	auto skillMenu = CCMenu::create();
	skillMenu->setID("skillsets-menu");
	skillMenu->setPosition({ 555.f, 20.f });
	skillMenu->setAnchorPoint({ 1.f, 0.5f });
	skillMenu->setContentSize({ 150.f, 30.f });
	skillMenu->setScale(1.25f);
	skillsetsNode->addChild(skillMenu);

	auto skillLayout = AxisLayout::create();
	skillLayout->setAxisAlignment(AxisAlignment::End);

	skillMenu->setLayout(skillLayout);

	auto pos = 0;
	for (auto id : skillsets) {
		auto skillData = m_dataDev["skillset-data"][id];

		auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Skill_Unknown.png"_spr);
	
		//sprite
		auto spriteName = fmt::format("{}.png", skillData["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
		CCSprite* sprite = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(spriteName).data(), unkSpr);
		sprite->setID("sprite");
		sprite->setScale(0.75f);

		//button
		auto skillBtn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(VerificationPopup::onChangeLevelSkill));
		skillBtn->setID(fmt::format("skill-btn-{}", id));
		skillBtn->setTag(pos);
		skillMenu->addChild(skillBtn);

		pos += 1;
	}

	//always add 'new' button last
	auto newSkillSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
	newSkillSpr->setScale(0.55f);

	auto newSkillBtn = CCMenuItemSpriteExtra::create(newSkillSpr, this, menu_selector(VerificationPopup::onChangeLevelSkill));
	newSkillBtn->setID("skill-new-btn");
	newSkillBtn->setTag(-1);
	skillMenu->addChild(newSkillBtn);

	skillMenu->updateLayout();

	// difficulty menu
	auto diffMenu = CCMenu::create();
	diffMenu->setID("difficulty-menu");
	diffMenu->setPosition({ 555.f, 20.f });
	diffMenu->setAnchorPoint({ 1.f, 0.5f });
	diffMenu->setContentSize({ 150.f, 0.f });
	diffMenu->setScale(1.25f);
	difficultyNode->addChild(diffMenu);

	//difficulty button
	auto sprName = fmt::format("{}.png", m_dataDev["main"][m_levelData["difficulty"].as<int>().unwrapOrDefault()]["sprite"].asString().unwrapOr("DP_Unknown"));
	auto diffSpr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(sprName).data(), CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr));
	diffSpr->setScale(0.75f);

	auto diffBtn = CCMenuItemSpriteExtra::create(diffSpr, this, menu_selector(VerificationPopup::onDifficulty));
	diffBtn->setID("difficulty-btn");
	diffBtn->setTag(-100);
	diffMenu->addChild(diffBtn);

	diffMenu->updateLayout();

	return;
}

void VerificationPopup::onChangeLevelSkill(CCObject* sender) {
	auto pos = sender->getTag();
	auto isNew = false;

	if (sender->getTag() == -1) isNew = true;

	SkillsetSelectionPopup::create(pos, m_dataDev["skillset-data"], isNew)->show();

	return;
}

void VerificationPopup::loadSkill(std::string key) {
	this->setTitle("Edit Skillset");
	m_mainLayer->removeAllChildrenWithCleanup(true);

	m_index = key;

	// back button
	auto backMenu = CCMenu::create();
	backMenu->setPosition({0.f, 0.f});
	backMenu->setID("back-menu");
	backMenu->setZOrder(10);
	m_mainLayer->addChild(backMenu);

	auto backSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backBtn = CCMenuItemSpriteExtra::create(backSpr, this, menu_selector(VerificationPopup::onBack));
	backBtn->setPosition(this->m_closeBtn->getPositionX(), this->m_closeBtn->getPositionY() - 40.f);
	backBtn->setID("back-btn");
	backBtn->setTag(0);
	backMenu->addChild(backBtn);

	//save button
	auto saveMenu = CCMenu::create();
	saveMenu->setID("search-menu");
	saveMenu->setPosition({ 210.f, 0.f });
	saveMenu->setZOrder(10);
	m_mainLayer->addChild(saveMenu);

	auto saveSpr = ButtonSprite::create("Save", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto saveBtn = CCMenuItemSpriteExtra::create(saveSpr, this, menu_selector(VerificationPopup::onSave));
	saveBtn->setTag(2);
	saveMenu->addChild(saveBtn);

	//setup data
	matjson::Value skillData;
	if (!m_dataDev["skillset-data"].contains(key)) skillData = GDDPSkillsetFormat{};
	else skillData = m_dataDev["skillset-data"][key];
	
	m_currentData = skillData;

	//cells
	auto cells = CCArray::create();

	//create key input
	auto propertyNode = CCNode::create();
	propertyNode->setID(fmt::format("property-key"));
	propertyNode->setScale(0.75f);

	// label
	auto label = CCLabelBMFont::create("key", "bigFont.fnt");
	label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
	label->setID("label");
	label->setScale(0.5f);
	label->setAnchorPoint({0.f, 0.5f});
	label->setPosition({5.f, 17.5f});

	// edit menu
	auto editMenu = CCMenu::create();
	editMenu->setScale(0.75f);
	editMenu->setPosition({205.f, -25.f});
	editMenu->setID("value-menu");

	auto valueInput = TextInput::create(190.f, "Key (Required)", "bigFont.fnt");
	valueInput->setString(key);
	valueInput->setCommonFilter(CommonFilter::Any);
	valueInput->setPosition({180.f, 0.f});
	valueInput->setAnchorPoint({1.f, 0.5f});
	valueInput->setID("value-input");
	editMenu->addChild(valueInput);

	// add children
	propertyNode->addChild(label);
	propertyNode->addChild(editMenu);

	cells->addObject(propertyNode);

	for (auto [key, value] : skillData) {
		auto propertyNode = CCNode::create();
		propertyNode->setID(fmt::format("property-{}", key));
		propertyNode->setScale(0.75f);

		// label
		auto label = CCLabelBMFont::create(key.c_str(), "bigFont.fnt");
		label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
		label->setID("label");
		label->setScale(0.5f);
		label->setAnchorPoint({0.f, 0.5f});
		label->setPosition({5.f, 17.5f});

		// edit menu
		auto editMenu = CCMenu::create();
		editMenu->setScale(0.75f);
		editMenu->setPosition({205.f, -25.f});
		editMenu->setID("value-menu");

		if (key == "description") {
			auto propertyNode = CCNode::create();
			propertyNode->setID(fmt::format("property-{}", key));
			propertyNode->setScale(0.75f);

			// label
			auto label = CCLabelBMFont::create(key.c_str(), "bigFont.fnt");
			label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
			label->setID("label");
			label->setScale(0.5f);
			label->setAnchorPoint({0.f, 0.5f});
			label->setPosition({5.f, 17.5f});

			// edit menu
			auto editMenu = CCMenu::create();
			editMenu->setScale(0.75f);
			editMenu->setPosition({325.f, -25.f});
			editMenu->setID("edit-menu");

			auto editSpr = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
			editSpr->setScale(0.5f);
			auto editBtn = CCMenuItemSpriteExtra::create(editSpr, this, menu_selector(VerificationPopup::onEdit));
			editBtn->setID("edit-button");
			editBtn->setTag(-200);
			editMenu->addChild(editBtn);

			// add children
			propertyNode->addChild(label);
			propertyNode->addChild(editMenu);

			cells->addObject(propertyNode);
		}
		else {
			auto valueInput = TextInput::create(190.f, "...", "bigFont.fnt");
			valueInput->setString(value.asString().unwrapOr(""));
			valueInput->setCommonFilter(CommonFilter::Any);
			valueInput->setPosition({ 180.f, 0.f });
			valueInput->setAnchorPoint({ 1.f, 0.5f });
			valueInput->setID("value-input");
			editMenu->addChild(valueInput);

			// add children
			propertyNode->addChild(label);
			propertyNode->addChild(editMenu);

			cells->addObject(propertyNode);
		}
	}

	//list
	m_list = ListView::create(cells, 25.f, 315.f, 180.f);
	m_list->setPosition({52.5f, 10.f});
	m_mainLayer->addChild(m_list);

	return;
}

void VerificationPopup::onToggle(CCObject* sender)
{
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto id = btn->getID();
	auto menuType = btn->getTag();

	if (id == "startpos-toggle") {
		m_practiceToggle = !m_practiceToggle;

		auto offs = m_levelList->m_tableView->m_contentLayer->getPositionY(); // save list offset
		loadPack(m_index, m_packID, true);
		m_levelList->m_tableView->m_contentLayer->setPositionY(offs);
	}
	else {
		CCArrayExt<CCMenuItemToggler*> btns;
		btns.push_back(static_cast<CCMenuItemToggler*>(m_tabs->getChildByID("packs")));
		btns.push_back(static_cast<CCMenuItemToggler*>(m_tabs->getChildByID("skills")));
		btns.push_back(static_cast<CCMenuItemToggler*>(m_tabs->getChildByID("lvls")));
		btns.push_back(static_cast<CCMenuItemToggler*>(m_tabs->getChildByID("medals")));

		for (auto b : btns) if (b) static_cast<CCMenuItemToggler*>(b)->toggle(false);
		btn->m_toggled = false;

		loadMain(menuType);
	}

	return;
}

void VerificationPopup::onNew(CCObject *sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto tag = btn->getTag();

	switch(tag) {
		case 0: //New Pack
		{
			NewPackPopup::create()->show();
			break;
		}
		case 1: //New Skillset
		{
			loadSkill("new-skill");
			break;
		}
		case 2: //New Level
		{
			AddLevelPopup::create()->show();
			break;
		}
		case 3: //New Medal
		{
			NewMedalPopup::create()->show();
			break;
		}
	}

	return;
}

void VerificationPopup::onClose(CCObject *sender) {

	geode::createQuickPopup(
		"Close Panel",																					// title
		"Are you sure you want to <cr>close</c> the panel?\n<cr>Anything unsaved will be deleted.</c>", // content
		"No", "Yes",																					// buttons
		[&](auto, bool btn2)
		{
			if (btn2) {
				this->unschedule(schedule_selector(VerificationPopup::poll));
				m_listener.cancel();
				m_listener2.cancel();

				// wipe autosave
				auto path = Mod::get()->getConfigDir() += std::filesystem::path("\\dev_autosave.json");
				file::writeStringSafe(path, "");

				// normal closing stuff
				// CloseEvent(this).post();
				this->setKeypadEnabled(false);
				this->setTouchEnabled(false);
				this->removeFromParentAndCleanup(true);
			}
		});

	return;
}

void VerificationPopup::onBack(CCObject* sender) {
	auto btn = static_cast<CCMenuItemToggler*>(sender);
	auto tag = btn->getTag();

	switch(tag) {
		case 0: // Pack/Skillset Menu
		{
			geode::createQuickPopup(
				"Cancel",																					// title
				"Are you sure you want to <cr>cancel</c> your changes?\n<cr>Anything unsaved will be deleted.</c>", // content
				"No", "Yes",																					// buttons
				[&](auto, bool btn2) {
					if (btn2) loadMain(m_currentTab);
				});

			break;
		}
		case 1: // Level Menu
		{
			geode::createQuickPopup(
				"Cancel",																					// title
				"Are you sure you want to <cr>cancel</c> your changes?\n<cr>Anything unsaved will be deleted.</c>", // content
				"No", "Yes",																					// buttons
				[&](auto, bool btn2) {
					if (btn2) {
						if (m_currentTab == 2) loadMain(2);
						else loadPack(m_index, m_packID, true);
					}
				});
			
			break;
		}
	
	}

	return;
}

VerificationPopup *VerificationPopup::create() {
	auto ret = new VerificationPopup();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

VerificationPopup::~VerificationPopup() {
	this->removeAllChildrenWithCleanup(true);
}

//===================
//  DifficultyPopup
//===================

bool DifficultyPopup::init() {
	if (!Popup::init(420.f, 125.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Select Difficulty");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	// add buttons
	auto menu = CCMenu::create();
	menu->setPosition({210.f, 55.f});
	menu->setID("button-menu");
	menu->setContentWidth(420.f);
	m_mainLayer->addChild(menu);

	auto containerLayout = AxisLayout::create();
	containerLayout->setGrowCrossAxis(true);
	containerLayout->setGap(-2.f);
	containerLayout->setAxisAlignment(AxisAlignment::Center);
	containerLayout->setCrossAxisAlignment(AxisAlignment::Center);
	containerLayout->setCrossAxisLineAlignment(AxisAlignment::Start);
	containerLayout->setAxis(Axis::Row);
	menu->setLayout(containerLayout);

	for (int i = 0; i < m_data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
		auto spr = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", m_data["main"][i]["sprite"].asString().unwrapOr("DP_Unknown"))).data(), CCSprite::createWithSpriteFrameName("DP_Unknown.png"_spr));
		auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(DifficultyPopup::onSelect));
		btn->setID(fmt::format("difficulty-btn-{}", i));
		btn->setTag(i);
		menu->addChild(btn);
		menu->updateLayout(false);
	}

	return true;
}

void DifficultyPopup::onSelect(CCObject* sender) {
	auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
	auto id = btn->getTag();

	if (m_callback) m_callback(id);

	this->onClose(sender);

	return;
}

void DifficultyPopup::setCallback(geode::Function<void(int const&)> cb) {
	m_callback = std::move(cb);
}

DifficultyPopup* DifficultyPopup::create(matjson::Value data) {
	auto ret = new DifficultyPopup();
	ret->m_data = data;
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

DifficultyPopup::~DifficultyPopup() {
	this->removeAllChildrenWithCleanup(true);
}

//===================
//  AddLevelPopup
//===================

bool AddLevelPopup::init() {
	if (!Popup::init(420.f, 125.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Enter Level ID");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	m_value = TextInput::create(400.f, "Enter Level ID", "bigFont.fnt");
	m_value->setString("0");
	m_value->setCommonFilter(CommonFilter::Int);
	m_value->setMaxCharCount(0);
	m_value->setPosition({210.f, 75.f});
	m_value->setID("value");
	m_mainLayer->addChild(m_value);

	// add confirm button
	auto confirmMenu = CCMenu::create();
	confirmMenu->setPosition({0.f, 0.f});
	confirmMenu->setID("confirm-menu");
	m_mainLayer->addChild(confirmMenu);

	auto confirmSpr = ButtonSprite::create("OK", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(AddLevelPopup::onAddLevel));
	confirmBtn->setPosition(210.f, 30.f);
	confirmBtn->setID("confirm-btn");
	confirmMenu->addChild(confirmBtn);

	// add paste button
	auto pasteMenu = CCMenu::create();
	pasteMenu->setPosition({0.f, 0.f});
	pasteMenu->setID("paste-menu");
	m_mainLayer->addChild(pasteMenu);

	auto pasteSpr = ButtonSprite::create("Paste", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto pasteBtn = CCMenuItemSpriteExtra::create(pasteSpr, this, menu_selector(AddLevelPopup::onPaste));
	pasteBtn->setPosition(370.f, 30.f);
	pasteBtn->setID("paste-btn");
	pasteMenu->addChild(pasteBtn);

	return true;
}

void AddLevelPopup::onPaste(CCObject*) {
	m_value->setString(clipboard::read());

	auto tap = TextAlertPopup::create("Pasted From Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");
	this->addChild(tap);

	return;
}

void AddLevelPopup::onAddLevel(CCObject*) {

	int lvlID = numFromString<int>(m_value->getString()).unwrapOrDefault();

	VerificationPopup* popup = this->getParent()->getChildByType<VerificationPopup>(0);
	
	if (popup->m_currentTab == 2) {
		popup->m_index = "main";
		popup->loadLevel(lvlID);
	}
	else {
		auto lvlList = popup->m_currentData["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
		if (m_practiceIndex > -1) popup->m_dataDev["level-data"][std::to_string(m_practiceIndex)].set("startpos-copy", lvlID);
		else {
			lvlList.insert(lvlList.begin(), lvlID);
			popup->m_currentData.set("levelIDs", lvlList);
		}

		auto offs = popup->m_levelList->m_tableView->m_contentLayer->getPositionY(); // save list offset
		popup->loadPack(popup->m_index, popup->m_packID, true);
		if (lvlList.size() > 7) popup->m_levelList->m_tableView->m_contentLayer->setPositionY(offs);
	}

	this->onClose(new CCObject());

	return;
}

AddLevelPopup* AddLevelPopup::create() {
	auto ret = new AddLevelPopup();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

AddLevelPopup::~AddLevelPopup() {
	this->removeAllChildrenWithCleanup(true);
}

//===================
//  RenamePopup
//===================

bool RenamePopup::init() {
	if (!Popup::init(420.f, 125.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Rename");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	m_value = TextInput::create(400.f, "Rename", "bigFont.fnt");
	m_value->setString(m_string);
	m_value->setCommonFilter(CommonFilter::Any);
	m_value->setMaxCharCount(0);
	m_value->setPosition({210.f, 75.f});
	m_value->setID("value");
	m_mainLayer->addChild(m_value);

	// add confirm button
	auto confirmMenu = CCMenu::create();
	confirmMenu->setPosition({0.f, 0.f});
	confirmMenu->setID("confirm-menu");
	m_mainLayer->addChild(confirmMenu);

	auto confirmSpr = ButtonSprite::create("OK", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(RenamePopup::onConfirm));
	confirmBtn->setPosition(210.f, 30.f);
	confirmBtn->setID("confirm-btn");
	confirmMenu->addChild(confirmBtn);

	// add paste button
	auto pasteMenu = CCMenu::create();
	pasteMenu->setPosition({0.f, 0.f});
	pasteMenu->setID("paste-menu");
	m_mainLayer->addChild(pasteMenu);

	auto pasteSpr = ButtonSprite::create("Paste", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto pasteBtn = CCMenuItemSpriteExtra::create(pasteSpr, this, menu_selector(RenamePopup::onPaste));
	pasteBtn->setPosition(370.f, 30.f);
	pasteBtn->setID("paste-btn");
	pasteMenu->addChild(pasteBtn);

	return true;
}

void RenamePopup::onPaste(CCObject*) {
	m_value->setString(clipboard::read());

	auto tap = TextAlertPopup::create("Pasted From Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");
	this->addChild(tap);

	return;
}

void RenamePopup::onConfirm(CCObject*) {
	if (m_callback) m_callback(m_value->getString());
	this->onClose(new CCObject());

	return;
}

void RenamePopup::setCallback(geode::Function<void(std::string const&)> cb) {
	m_callback = std::move(cb);
}

RenamePopup* RenamePopup::create(std::string str) {
	auto ret = new RenamePopup();
	ret->m_string = str;
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

RenamePopup::~RenamePopup() {
	this->removeAllChildrenWithCleanup(true);
}

//===================
//  EditDescriptionPopup
//===================

bool EditDescriptionPopup::init()
{
	if (!Popup::init(540.f, 125.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Set Description");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	m_value = TextInput::create(530.f, "Enter Description", "chatFont.fnt");
	//m_value->setContentHeight(120.f);
	m_value->setString(m_description.c_str());
	m_value->setPosition({270.f, 75.f});
	m_value->setID("value");
	m_value->setCommonFilter(CommonFilter::Any);
	m_mainLayer->addChild(m_value);

	// add confirm button
	auto confirmMenu = CCMenu::create();
	confirmMenu->setPosition({0.f, 0.f});
	confirmMenu->setID("confirm-menu");
	m_mainLayer->addChild(confirmMenu);

	auto confirmSpr = ButtonSprite::create("OK", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(EditDescriptionPopup::onConfirm));
	confirmBtn->setPosition(270.f, 30.f);
	confirmBtn->setID("confirm-btn");
	confirmMenu->addChild(confirmBtn);

	// add paste button
	auto pasteMenu = CCMenu::create();
	pasteMenu->setPosition({0.f, 0.f});
	pasteMenu->setID("paste-menu");
	m_mainLayer->addChild(pasteMenu);

	auto pasteSpr = ButtonSprite::create("Paste", "bigFont.fnt", "GJ_button_01.png", 0.65f);
	auto pasteBtn = CCMenuItemSpriteExtra::create(pasteSpr, this, menu_selector(EditDescriptionPopup::onPaste));
	pasteBtn->setPosition(490.f, 30.f);
	pasteBtn->setID("paste-btn");
	pasteMenu->addChild(pasteBtn);

	return true;
}

void EditDescriptionPopup::onPaste(CCObject*) {
	m_value->setString(clipboard::read());

	auto tap = TextAlertPopup::create("Pasted From Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");
	this->addChild(tap);

	return;
}

void EditDescriptionPopup::onConfirm(CCObject*) {

	auto popup = this->getParent()->getChildByType<VerificationPopup*>(0);
	popup->m_currentData.set("description", m_value->getString().c_str());

	this->removeMeAndCleanup();

	return;
}

EditDescriptionPopup* EditDescriptionPopup::create(std::string description) {
	auto ret = new EditDescriptionPopup();
	ret->m_description = description;
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

EditDescriptionPopup::~EditDescriptionPopup() {
	this->removeAllChildrenWithCleanup(true);
}

//===================
//  MovePopup
//===================

bool MovePopup::init()
{
	if (!Popup::init(220.f, 150.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	if (m_type == "pack") this->setTitle("Pack Order");
	else if (m_type == "level") this->setTitle("Level Order");

	m_newPos = m_pos;

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	m_value = TextInput::create(50.f, "...", "bigFont.fnt");
	m_value->setString(std::to_string(m_pos + 1).c_str());
	m_value->setCommonFilter(CommonFilter::Int);
	m_value->setPosition({ m_mainLayer->getContentWidth() / 2.f, m_mainLayer->getContentHeight() / 2.f });
	m_value->setID("value");
	m_mainLayer->addChild(m_value);

	// add confirm button
	auto confirmMenu = CCMenu::create();
	confirmMenu->setPosition({0.f, 0.f});
	confirmMenu->setID("confirm-menu");
	m_mainLayer->addChild(confirmMenu);

	auto confirmSpr = ButtonSprite::create("OK", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(MovePopup::onConfirm));
	confirmBtn->setPosition({ m_mainLayer->getContentWidth() / 2.f, 30.f });
	confirmBtn->setID("confirm-btn");
	confirmMenu->addChild(confirmBtn);

	// add move buttons
	auto moveMenu = CCMenu::create();
	moveMenu->setPosition({m_mainLayer->getContentWidth() / 2.f, 0.f});
	moveMenu->setID("move-menu");
	m_mainLayer->addChild(moveMenu);

	auto upSpr = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
	upSpr->setRotation(90.f);
	upSpr->setScale(1.1f);
	auto upBtn = CCMenuItemSpriteExtra::create(upSpr, this, menu_selector(MovePopup::onUp));
	upBtn->setPosition({ -50.f, m_mainLayer->getContentHeight() / 2.f });
	upBtn->setID("up-btn");
	moveMenu->addChild(upBtn);

	auto downSpr = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
	downSpr->setRotation(90.f);
	downSpr->setScale(1.1f);
	auto downBtn = CCMenuItemSpriteExtra::create(downSpr, this, menu_selector(MovePopup::onDown));
	downBtn->setPosition({ 50.f, m_mainLayer->getContentHeight() / 2.f });
	downBtn->setID("down-btn");
	moveMenu->addChild(downBtn);

	auto topSpr = CCSprite::createWithSpriteFrameName("edit_leftBtn2_001.png");
	topSpr->setRotation(90.f);
	topSpr->setColor({ 0, 255, 0 });
	topSpr->setScale(1.1f);
	auto topBtn = CCMenuItemSpriteExtra::create(topSpr, this, menu_selector(MovePopup::onTop));
	topBtn->setPosition({ -85.f, m_mainLayer->getContentHeight() / 2.f });
	topBtn->setID("top-btn");
	moveMenu->addChild(topBtn);

	auto bottomSpr = CCSprite::createWithSpriteFrameName("edit_rightBtn2_001.png");
	bottomSpr->setRotation(90.f);
	bottomSpr->setScale(1.1f);
	bottomSpr->setColor({ 255, 127, 0 });
	auto bottomBtn = CCMenuItemSpriteExtra::create(bottomSpr, this, menu_selector(MovePopup::onBottom));
	bottomBtn->setPosition({ 85.f, m_mainLayer->getContentHeight() / 2.f });
	bottomBtn->setID("bottom-btn");
	moveMenu->addChild(bottomBtn);

	return true;
}

void MovePopup::onUp(CCObject*) {
	m_newPos -= 1;
	m_newPos = std::max(m_newPos, 0);
	m_value->setString(std::to_string(m_newPos + 1));
	
	return;
}

void MovePopup::onDown(CCObject*) {

	m_newPos += 1;

	auto popup = this->getParent()->getChildByType<VerificationPopup*>(0); //this is needed because we gotta get the total array size

	if (m_type == "pack") { //pack
		auto index = m_ID;
		auto data = popup->m_dataDev[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

		m_newPos = std::min(m_newPos, int(data.size() - 1));
		
	}
	else if (m_type == "level") { //level
		auto levelList = popup->m_currentData["levelIDs"].as<std::vector<int>>().unwrapOrDefault();

		m_newPos = std::min(m_newPos, int(levelList.size() - 1));
	}

	m_value->setString(std::to_string(m_newPos + 1));
	
	return;
}

void MovePopup::onTop(CCObject*) {
	m_value->setString("1");
	m_newPos = 0;
	
	return;
}

void MovePopup::onBottom(CCObject*) {
	auto popup = this->getParent()->getChildByType<VerificationPopup*>(0); //this is needed because we gotta get the total array size

	if (m_type == "pack") { //pack
		auto index = m_ID;
		auto data = popup->m_dataDev[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

		m_value->setString(std::to_string(data.size()).c_str());
		m_newPos = data.size() - 1;
		
	}
	else if (m_type == "level") { //level
		auto levelList = popup->m_currentData["levelIDs"].as<std::vector<int>>().unwrapOrDefault();

		m_value->setString(std::to_string(levelList.size()).c_str());
		m_newPos = levelList.size() - 1;
	}

	return;
}

void MovePopup::onConfirm(CCObject*) {

	auto popup = this->getParent()->getChildByType<VerificationPopup*>(0);
	
	if (m_type == "pack") { //pack
		auto index = m_ID;
		auto originalData = popup->m_dataDev[index][m_pos];
		auto data = popup->m_dataDev[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
		int newPos = numFromString<int>(m_value->getString()).unwrapOrDefault() - 1;
		log::info("old pos: {}", m_pos);
		log::info("new pos: {}", newPos);

		//remove old pack at position since we have the placement stored now
		data.erase(data.begin() + m_pos);

		//place the pack at the new position
		data.insert(data.begin() + newPos, originalData);

		//if index is main, update difficulties and mainPack values in legacy packs
		if (index == "main") {
			auto levels = popup->m_dataDev["level-data"];
			auto legacyPacks = popup->m_dataDev["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
			//m_pos = old position, newPos = new position

			//first set all levels and legacy packs pointing to old pos to -1 temporarily
			for (auto [key, value] : levels) {
				if (value["difficulty"].as<int>().unwrapOr(0) == m_pos) {
					levels[key].set("difficulty", -1);
				} 
			}

			auto pos = 0;
			for (auto pack : legacyPacks) {
				if (pack["mainPack"].as<int>().unwrapOr(0) == m_pos) {
					legacyPacks.at(pos).set("mainPack", -1);
				}
				pos += 1;
			}

			/*
			evaluation time!
			the range that should be changed is dependent on the difference between new pos and old pos
			example:
			1-2-3-4-5-6-7-8-9
			if 3 is moved to 4...
			1-2-3->4-5-6-7-8-9
			4 is the only one affected
			new pos - old pos = 1
			positive = shift x values to the right, left (subtract)
			negative = shift x values to the left, right (add)
			a) set the original value to -1: 3 -> -1
			1-2-(-1)-4-5-6-7-8-9
			b) one value is affected to the right, so shift 4 down by one
			1-2-3-(-1)-5-6-7-8-9
			c) set -1 to new pos (4)
			1-2-3-4-5-6-7-8-9
			*/

			auto diff = newPos - m_pos;
			if (diff > 0) {
				for (auto [key, value] : levels) {
					auto difficulty = value["difficulty"].as<int>().unwrapOr(0);
					if (difficulty > m_pos && difficulty <= m_pos + diff) {
						levels[key].set("difficulty", difficulty - 1);
					} 
				}
		
				pos = 0;
				for (auto pack : legacyPacks) {
					auto mainPack = pack["mainPack"].as<int>().unwrapOr(0);
					if (mainPack > m_pos && mainPack <= m_pos + diff) {
						legacyPacks.at(pos).set("mainPack", mainPack - 1);
					}
					pos += 1;
				}

			}
			else if (diff < 0) {
				for (auto [key, value] : levels) {
					auto difficulty = value["difficulty"].as<int>().unwrapOr(0);
					if (difficulty < m_pos && difficulty >= m_pos + diff) {
						levels[key].set("difficulty", difficulty + 1);
					} 
				}
		
				pos = 0;
				for (auto pack : legacyPacks) {
					auto mainPack = pack["mainPack"].as<int>().unwrapOr(0);
					if (mainPack < m_pos && mainPack >= m_pos + diff) {
						legacyPacks.at(pos).set("mainPack", mainPack + 1);
					}
					pos += 1;
				}
			}

			//finally, set all values of -1 to newPos
			for (auto [key, value] : levels) {
				if (value["difficulty"].as<int>().unwrapOr(0) == -1) {
					levels[key].set("difficulty", newPos);
				} 
			}

			pos = 0;
			for (auto pack : legacyPacks) {
				if (pack["mainPack"].as<int>().unwrapOr(0) == -1) {
					legacyPacks.at(pos).set("mainPack", newPos);
				}
				pos += 1;
			}

			popup->m_dataDev.set("legacy", legacyPacks);
			popup->m_dataDev.set("level-data", levels);
		}

		//store data
		popup->m_dataDev.set(index, data);

		log::info("Pack successfully moved.");

		auto offs = popup->m_list->m_tableView->m_contentLayer->getPositionY(); // save list offset

		popup->loadMain(0);
		popup->m_list->m_tableView->m_contentLayer->setPositionY(offs);
	}
	else if (m_type == "level") { //level
		auto levelList = popup->m_currentData["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
		int lvlID = numFromString<int>(m_ID).unwrapOrDefault();
		int newPos = numFromString<int>(m_value->getString()).unwrapOrDefault() - 1;

		//remove old level at position since we have the placement stored now
		levelList.erase(levelList.begin() + m_pos);

		//place the level at the new position
		levelList.insert(levelList.begin() + newPos, lvlID);

		//store data
		popup->m_currentData.set("levelIDs", levelList);

		log::info("Level successfully moved.");

		auto offs = popup->m_levelList->m_tableView->m_contentLayer->getPositionY(); // save list offset

		popup->loadPack(popup->m_index, popup->m_packID, true);
		popup->m_levelList->m_tableView->m_contentLayer->setPositionY(offs);
	}

	this->removeMeAndCleanup();

	return;
}

MovePopup* MovePopup::create(std::string type, std::string id, int pos) {
	auto ret = new MovePopup();
	ret->m_type = type;
	ret->m_ID = id;
	ret->m_pos = pos;
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

MovePopup::~MovePopup() {
	this->removeAllChildrenWithCleanup(true);
}

//===================
//  SkillsetSelectionPopup
//===================

bool SkillsetSelectionPopup::init()
{
	if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Add Skillset");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	//delete menu
	auto deleteMenu = CCMenu::create();
	deleteMenu->setPosition({250.f, 145.f});
	deleteMenu->setAnchorPoint({ 1.f, 1.f });
	deleteMenu->setScale(0.75f);
	deleteMenu->setID("delete-menu");

	auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
	auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(SkillsetSelectionPopup::onRemove));
	deleteBtn->setID("delete-btn");
	deleteMenu->addChild(deleteBtn);

	if (!m_isNew) m_mainLayer->addChild(deleteMenu);

	//list
	auto cells = CCArray::create();

	std::vector<std::string> skillsetTypes = {"none", "gamemode", "misc", "platformer", "special"};
	for (auto type : skillsetTypes) {
		// header
		auto header = CCNode::create();
		auto headerText = CCLabelBMFont::create(type.c_str(), "bigFont.fnt");
		headerText->setScale(0.4f);
		headerText->setPosition({5.f, 25.f / 2.f});
		headerText->setAnchorPoint({ 0.f, 0.5f });
		header->addChild(headerText);
		cells->addObject(header);

		// skillsets
		for (auto [key, value] : m_data) {
			auto skillNode = CCNode::create();
			skillNode->setID(fmt::format("skill-{}", key));
			skillNode->setScale(0.75f);
	
			auto skillData = value;

			if (skillData["type"].asString().unwrapOr("none") != type) continue;
	
			auto unkSpr = CCSprite::createWithSpriteFrameName("DP_Skill_Unknown.png"_spr);

			//sprite
			auto spriteName = fmt::format("{}.png", skillData["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
			CCSprite* sprite = DPUtils::safeSpriteWithFrameName(Mod::get()->expandSpriteName(spriteName).data(), unkSpr);
			sprite->setID("sprite");
			sprite->setScale(0.75f);
			sprite->setAnchorPoint({ 0.f, 0.5f });
			sprite->setPosition({ 5.f, 17.5f });
	
			//label
			auto label = CCLabelBMFont::create(skillData["display-name"].asString().unwrapOr("null").c_str(), "bigFont.fnt");
			label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
			label->setID("label");
			label->setScale(0.5f);
			label->setAnchorPoint({ 0.f, 0.5f });
			label->setPosition({ 30.f, 17.5f });
	
			// confirm menu
			auto confirmMenu = CCMenu::create();
			confirmMenu->setScale(0.75f);
			confirmMenu->setPosition({330.f, -25.f});
			confirmMenu->setID("confirm-menu");

			auto confirmSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
			confirmSpr->setFlipX(true);
			auto confirmBtn = CCMenuItemSpriteExtra::create(confirmSpr, this, menu_selector(SkillsetSelectionPopup::onSelection));
			confirmBtn->setID(key);
			confirmMenu->addChild(confirmBtn);
	
			//add children
			skillNode->addChild(sprite);
			skillNode->addChild(label);
			skillNode->addChild(confirmMenu);
	
			cells->addObject(skillNode);
		}
	}

	//list
	m_list = ListView::create(cells, 25.f, 315.f, 180.f);
	m_list->setPosition({52.5f, 10.f});
	m_mainLayer->addChild(m_list);

	return true;
}

void SkillsetSelectionPopup::onSelection(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto id = btn->getID();

	auto popup = this->getParent()->getChildByType<VerificationPopup*>(0);
	auto skillsets = popup->m_levelData["skillsets"].as<std::vector<std::string>>().unwrapOrDefault();
	log::info("{}", skillsets);
	
	if (m_isNew) skillsets.push_back(id);
	else skillsets.at(m_index) = id;

	popup->m_levelData.set("skillsets", skillsets);
	popup->refreshLevel();

	this->removeMeAndCleanup();

	return;
}

void SkillsetSelectionPopup::onRemove(CCObject*) {
	auto popup = this->getParent()->getChildByType<VerificationPopup*>(0);
	auto skillsets = popup->m_levelData["skillsets"].as<std::vector<std::string>>().unwrapOrDefault();

	skillsets.erase(skillsets.begin() + m_index);

	popup->m_levelData.set("skillsets", skillsets);
	popup->refreshLevel();

	this->removeMeAndCleanup();
	
	return;
}

SkillsetSelectionPopup* SkillsetSelectionPopup::create(int placeAt, matjson::Value data, bool isNew) {
	auto ret = new SkillsetSelectionPopup();
	ret->m_index = placeAt;
	ret->m_data = data;
	ret->m_isNew = isNew;
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

SkillsetSelectionPopup::~SkillsetSelectionPopup() {
	this->removeAllChildrenWithCleanup(true);
}

//===================
//  NewPackPopup
//===================

bool NewPackPopup::init()
{
	if (!Popup::init(220.f, 150.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("New Pack");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	// add buttons
	auto menu = CCMenu::create();
	menu->setPosition({m_mainLayer->getContentWidth() / 2.f, m_mainLayer->getContentHeight() / 2.f});
	menu->setID("move-menu");
	m_mainLayer->addChild(menu);

	auto mainSpr = ButtonSprite::create("Main", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto mainBtn = CCMenuItemSpriteExtra::create(mainSpr, this, menu_selector(NewPackPopup::onConfirm));
	mainBtn->setPosition({ -50.f, 15.f });
	mainBtn->setID("main-btn");
	menu->addChild(mainBtn);

	auto legacySpr = ButtonSprite::create("Legacy", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto legacyBtn = CCMenuItemSpriteExtra::create(legacySpr, this, menu_selector(NewPackPopup::onConfirm));
	legacyBtn->setPosition({ 50.f, 15.f });
	legacyBtn->setID("legacy-btn");
	menu->addChild(legacyBtn);

	auto bonusSpr = ButtonSprite::create("Bonus", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto bonusBtn = CCMenuItemSpriteExtra::create(bonusSpr, this, menu_selector(NewPackPopup::onConfirm));
	bonusBtn->setPosition({ -50.f, -30.f });
	bonusBtn->setID("bonus-btn");
	menu->addChild(bonusBtn);

	auto monthlySpr = ButtonSprite::create("Monthly", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto monthlyBtn = CCMenuItemSpriteExtra::create(monthlySpr, this, menu_selector(NewPackPopup::onConfirm));
	monthlyBtn->setPosition({ 50.f, -30.f });
	monthlyBtn->setID("monthly-btn");
	menu->addChild(monthlyBtn);

	return true;
}

void NewPackPopup::onConfirm(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto id = btn->getID();
	auto popup = this->getParent()->getChildByType<VerificationPopup*>(0);

	//modify data based on id
	if (id == "main-btn") {
		auto data = popup->m_dataDev["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
		data.insert(data.begin(), GDDPMainPackFormat{});

		auto levels = popup->m_dataDev["level-data"];
		auto legacyPacks = popup->m_dataDev["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());

		//move every main pack pointer value up by one
		for (auto [key, value] : levels) {
			levels[key].set("difficulty", value["difficulty"].as<int>().unwrapOr(0) + 1);
		}

		auto pos = 0;
		for (auto pack : legacyPacks) {
			legacyPacks.at(pos).set("mainPack", pack["mainPack"].as<int>().unwrapOr(0) + 1);
			pos += 1;
		}

		popup->m_dataDev.set("level-data", levels);
		popup->m_dataDev.set("legacy", legacyPacks);
		popup->m_dataDev.set("main", data);
	}
	if (id == "legacy-btn") {
		auto data = popup->m_dataDev["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
		data.insert(data.begin(), GDDPLegacyPackFormat{});
		popup->m_dataDev.set("legacy", data);
	}
	if (id == "bonus-btn") {
		auto data = popup->m_dataDev["bonus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
		data.insert(data.begin(), GDDPBonusPackFormat{});
		popup->m_dataDev.set("bonus", data);
	}
	if (id == "monthly-btn") {
		auto data = popup->m_dataDev["monthly"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
		data.insert(data.begin(), GDDPMonthlyPackFormat{});
		popup->m_dataDev.set("monthly", data);
	}
	
	
	auto offs = popup->m_list->m_tableView->m_contentLayer->getPositionY();
	popup->onUpdateSearch(popup->m_searchInput->getString());
	popup->m_list->m_tableView->m_contentLayer->setPositionY(offs);

	this->removeMeAndCleanup();

	return;
}

NewPackPopup* NewPackPopup::create() {
	auto ret = new NewPackPopup();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

NewPackPopup::~NewPackPopup() {
	this->removeAllChildrenWithCleanup(true);
}

//===================
//  NewMedalPopup
//===================

bool NewMedalPopup::init()
{
	if (!Popup::init(220.f, 150.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("New Medal");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

	// create main layer
	auto mainLayer = CCLayer::create();
	mainLayer->setID("main-layer");
	mainLayer->setContentSize(layer->getContentSize());

	layer->addChild(mainLayer);
	m_mainLayer = mainLayer;

	// add buttons
	auto menu = CCMenu::create();
	menu->setPosition({m_mainLayer->getContentWidth() / 2.f, m_mainLayer->getContentHeight() / 2.f});
	menu->setID("main-menu");
	m_mainLayer->addChild(menu);

	auto mainSpr = ButtonSprite::create("Normal", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto mainBtn = CCMenuItemSpriteExtra::create(mainSpr, this, menu_selector(NewMedalPopup::onConfirm));
	mainBtn->setPosition({ -50.f, 15.f });
	mainBtn->setID("normal-btn");
	menu->addChild(mainBtn);

	auto legacySpr = ButtonSprite::create("Plus", "bigFont.fnt", "GJ_button_02.png", 0.65f);
	auto legacyBtn = CCMenuItemSpriteExtra::create(legacySpr, this, menu_selector(NewMedalPopup::onConfirm));
	legacyBtn->setPosition({ 50.f, 15.f });
	legacyBtn->setID("plus-btn");
	menu->addChild(legacyBtn);

	return true;
}

void NewMedalPopup::onConfirm(CCObject* sender) {
	auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender);
	auto id = btn->getID();
	auto popup = this->getParent()->getChildByType<VerificationPopup*>(0);

	//modify data based on id
	if (id == "normal-btn") {
		auto data = popup->m_dataDev["medals"]["normal"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
		data.insert(data.begin(), GDDPMedalFormat{});
		popup->m_dataDev["medals"].set("normal", data);
	}
	if (id == "plus-btn") {
		auto data = popup->m_dataDev["medals"]["plus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>());
		data.insert(data.begin(), GDDPMedalFormat{});
		popup->m_dataDev["medals"].set("plus", data);
	}
	
	auto offs = popup->m_list->m_tableView->m_contentLayer->getPositionY();
	popup->onUpdateSearch(popup->m_searchInput->getString());
	popup->m_list->m_tableView->m_contentLayer->setPositionY(offs);

	this->removeMeAndCleanup();

	return;
}

NewMedalPopup* NewMedalPopup::create() {
	auto ret = new NewMedalPopup();
	if (ret && ret->init()) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

NewMedalPopup::~NewMedalPopup() {
	this->removeAllChildrenWithCleanup(true);
}