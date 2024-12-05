//geode header
#include <Geode/Geode.hpp>
#include <Geode/utils/JsonValidation.hpp>

#include <random>

#include "RouletteUtils.hpp"
#include "Utils.hpp"

//geode namespace
using namespace geode::prelude;

std::string RouletteUtils::toFlags(std::vector<bool> settings) {
    std::string result = "";
    
    for (int i = 0; i < settings.size(); i++) {
        if (settings[i]) {
            result.push_back('1');
        }
        else {
            result.push_back('0');
        }
    }

    //log::info("to flags returned: {}", result);

    return result;
}

std::vector<bool> RouletteUtils::fromFlags(std::string flags) {

    //find the size of the settings by getting the number of main/legacy/bonus packs and adding 4
    auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
    auto mainPacks = data["main"].asArray().unwrap().size();
    auto legacyPacks = data["legacy"].asArray().unwrap().size();
    auto bonusPacks = data["bonus"].asArray().unwrap().size();
    auto size = (6 + mainPacks + legacyPacks + bonusPacks);

    //fill vector with false so we can work backwards
    std::vector<bool> result = {};
    for (int i = 0; i < size; i++) {
        result.push_back(false);
    }

    //iterate backwards
    for (int i = size - 1; i >= 0; i--) {
        if (flags.size() >= i && flags[i] == '1') {
            result[i] = true;
        }
    }

    //log::info("from flags returned: {}", result);

    return result;
}

std::vector<int> RouletteUtils::setupLevels(std::vector<matjson::Value> packs, std::string settings, int randomSeed) {
    
    auto settingsBools = fromFlags(settings);
    auto completedFlag = settingsBools[2];
    auto uncompletedFlag = settingsBools[3];

    auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

    //get levels
    std::vector<int> levels = {};
    for (auto pack : packs) {
        for (auto lvl : pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault()) {
            if (completedFlag != uncompletedFlag) {
                if (completedFlag && std::find(completedLvls.begin(), completedLvls.end(), lvl) != completedLvls.end()) {
                    levels.push_back(lvl);
                } 
                else if (uncompletedFlag && std::find(completedLvls.begin(), completedLvls.end(), lvl) == completedLvls.end()) {
                    levels.push_back(lvl);
                }
            }
            else {
                if (std::find(levels.begin(), levels.end(), lvl) == levels.end()) {
                    levels.push_back(lvl);
                }
            }
        }
    }

    //randomize order based on seed
    std::seed_seq seed{randomSeed};
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distribution(0, levels.size() - 1);

    std::vector<int> order = {};
    int i = 0;
    while (i < std::min(100, int(levels.size()))) {
        auto lvl = distribution(gen);
        if (std::find(order.begin(), order.end(), lvl) == order.end()) {
            order.push_back(lvl);
            i += 1;
        }
    }

    //trim levels so there's no decimal percents
    while ((100 / order.size()) != (100.f / static_cast<float>(order.size()))) {
        order.pop_back();
    }

    //set order
    std::vector<int> out = {};
    for (int place : order) {
        out.push_back(levels[place]);
    }

    return out;
}

void RouletteUtils::exportSettings(std::string settings, int seed) {
    auto str = fmt::format("GDDPSettingsFormat;{};{};END", settings, seed);
    auto encodedStr = ZipUtils::base64URLEncode(str);

    clipboard::write(encodedStr);
    //TextAlertPopup::create("Settings Copied to Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");

    return;
}

std::vector<std::string> RouletteUtils::importSettings(std::string str) {
    auto decodedStr = ZipUtils::base64URLDecode(str);
    auto values = Utils::substring(decodedStr, ";");

    //verify settings string
    if (values[0] != "GDDPSettingsFormat" || values.size() != 4 || values[3] != "END") { 
        FLAlertLayer::create(
			"ERROR",
			"Invalid Settings. Make sure you copied the correct text and try again.",
			"OK"
			)->show(); 
        return { "001100", "-1" };
    }

    std::string settings = values[1];
    std::string seed = values[2];

    std::vector<std::string> out = { settings, seed };
    return out;
}

void RouletteUtils::createSave(std::string name, std::vector<int> levels, std::string settings, int seed) {
    /*
    Save Format:
    Name
    Settings Flags
    Seed
    Levels
    Progress
    Skips
    Score
    */

   auto save = RouletteSaveFormat{ .name = name, .settings = settings, .seed = seed, .levels = levels };

   auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
   rouletteSaves.push_back(save);

   Mod::get()->setSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", rouletteSaves);

   return;
}

void RouletteUtils::exportSave(matjson::Value save, bool toFile) {

    RouletteSaveFormat rsfSave = save.as<RouletteSaveFormat>().unwrapOrDefault();

    auto settingsBools = fromFlags(rsfSave.settings);

    // format levels
    std::string lvlStr = "LVLS";
    for (auto lvl : rsfSave.levels) {
        lvlStr = fmt::format("{},{}", lvlStr.c_str(), lvl);
    }

    auto isOld = false; //are the packs selected in this file outdated?

    if (rsfSave.settings.size() < fromFlags("0").size()) { isOld = true; }

    std::string str = "ERR";
    if (isOld) {
        //if the imported settings are gonna be wrong anyways, might as well guarantee they're gonna have the hardcoded settings :v
        rsfSave.settings = toFlags({settingsBools[0], settingsBools[1], settingsBools[2], settingsBools[3], settingsBools[4], settingsBools[5]});
    }

    str = fmt::format("GDDPSaveFormat;{};{};{};{};{};{};{};END", rsfSave.name, rsfSave.settings, lvlStr, rsfSave.seed, rsfSave.progress, rsfSave.skips, rsfSave.score);
    auto encodedStr = ZipUtils::base64URLEncode(str);

    if (toFile) {
        // TODO
        log::info("TODO: Export save to file.");
    }
    else {
        clipboard::write(encodedStr);
    }

    return;
}

void RouletteUtils::importSave(std::string saveStr, bool fromFile) {
    if (fromFile) {
        //if fromFile, the save string is instead the file path used to import the save.

        //TODO
        log::info("TODO: Import save from file.");
    } else {
        auto decodedStr = ZipUtils::base64URLDecode(saveStr);
        auto values = Utils::substring(decodedStr, ";");

        //verify save string
        if (values[0] != "GDDPSaveFormat" || values.size() != 9 || values[8] != "END") { 
            FLAlertLayer::create(
			"ERROR",
			"Invalid Save File. Make sure you copied the correct text and try again.",
			"OK"
			)->show(); 
            return; 
        }

        //get packs and levels
        /*auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
        auto mainPacks = data["main"].asArray().unwrap();
        auto legacyPacks = data["legacy"].asArray().unwrap();
        auto bonusPacks = data["bonus"].asArray().unwrap();

        auto settingsBool = fromFlags(values[2]);

        std::vector<matjson::Value> packs = {};
        for (int i = 0; i < mainPacks.size(); i++) {
            if (settingsBool[6 + i]) {
                packs.push_back(mainPacks[i]);
            }
        }
        for (int i = 0; i < legacyPacks.size(); i++) {
            if (settingsBool[6 + mainPacks.size() + i]) {
                packs.push_back(legacyPacks[i]);
            }
        }
        for (int i = 0; i < bonusPacks.size(); i++) {
            if (settingsBool[6 + mainPacks.size() + legacyPacks.size() + i]) {
                packs.push_back(bonusPacks[i]);
            }
        }

        if (packs.empty()) { 
            FLAlertLayer::create(
			"ERROR",
			"Invalid Save File. Make sure you copied the correct text and try again.",
			"OK"
			)->show(); 
            return; 
        }

        std::vector<int> lvls = setupLevels(packs, values[2], std::stoi(values[3]));*/
        std::vector<int> lvls = {};
        auto lvlStrings = Utils::substring(values[3], ",");

        if (lvlStrings[0] != "LVLS") { 
            FLAlertLayer::create(
			"ERROR",
			"Invalid Save File. Make sure you copied the correct text and try again.",
			"OK"
			)->show(); 
            return; 
        }

        for (std::string str : lvlStrings) {
            if (str != "LVLS") {
                lvls.push_back(std::stoi(str));
            }
        }

        auto save = RouletteSaveFormat{
            .name = values[1],
            .settings = values[2],
            .seed = std::stoi(values[4]),
            .levels = lvls,
            .progress = std::stoi(values[5]),
            .skips = std::stoi(values[6]),
            .score = std::stoi(values[7])
        };

        auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
        rouletteSaves.push_back(save);

        Mod::get()->setSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", rouletteSaves);
    }

    return;
}

void RouletteUtils::deleteSaveAtPos(int pos) {
    auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
    
    auto at = rouletteSaves.begin() + pos;
	rouletteSaves.erase(at);
	Mod::get()->setSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", rouletteSaves);
    
    return;
}

void RouletteUtils::copySaveAtPos(int pos) {
    auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
    
    auto at = rouletteSaves.begin() + pos;
	rouletteSaves.insert(at, rouletteSaves[pos]);
	Mod::get()->setSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", rouletteSaves);
    
    return;
}