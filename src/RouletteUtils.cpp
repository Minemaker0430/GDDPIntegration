//geode header
#include <Geode/Geode.hpp>
#include <Geode/utils/JsonValidation.hpp>

#include <random>
#include <string>

#include "RouletteUtils.hpp"
#include "DPUtils.hpp"

//geode namespace
using namespace geode::prelude;

std::string RouletteUtils::toFlags(std::vector<bool> settings) {
    std::string result = "";
    int flag = 0b000000;
    
    for (int i = 0; i < settings.size(); i++) {
        if (settings[i]) {
            switch(i % 6) {
                case 0:
                    flag |= 0b000001;
                    break;
                case 1:
                    flag |= 0b000010;
                    break;
                case 2:
                    flag |= 0b000100;
                    break;
                case 3:
                    flag |= 0b001000;
                    break;
                case 4:
                    flag |= 0b010000;
                    break;
                case 5:
                    flag |= 0b100000;
                    break;
            }
        }

        if ((i % 6 == 5) || (i == settings.size() - 1)) { //convert flag to string and push
            std::string f = std::to_string(flag) + ',';
            if (flag < 10) f.insert(f.begin(), '0');
            result += f;

            flag = 0b000000;
        }
    }

    return result;
}

std::vector<bool> RouletteUtils::fromFlags(std::string flags) {

    //find the size of the settings by getting the number of main/legacy/bonus packs and adding 4
    auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
    auto mainPacks = data["main"].asArray().unwrap().size();
    auto legacyPacks = data["legacy"].asArray().unwrap().size();
    auto bonusPacks = data["bonus"].asArray().unwrap().size();
    auto size = (6 + mainPacks + legacyPacks + bonusPacks);

    std::vector<bool> result = {};
    std::vector<std::string> flagArray = DPUtils::substring(flags, ",");

    if (flagArray.size() <= 1) { // if using the old settings system, convert to bools this way 
        for (auto c : flags) {
            result.push_back(c == '1');
        }

        for (int i = result.size(); i < size; i++) {
            result.push_back(false); // fill in remaining settings
        }

        return result;
    }

    auto f = numFromString<int>(flagArray[0]).unwrapOr(0b000000);
    int fIndex = 0;
    for (int i = 0; i < size; i++) {
        switch(i % 6) {
            case 0:
                result.push_back((f & 0b000001) > 0);
                break;
            case 1:
                result.push_back((f & 0b000010) > 0);
                break;
            case 2:
                result.push_back((f & 0b000100) > 0);
                break;
            case 3:
                result.push_back((f & 0b001000) > 0);
                break;
            case 4:
                result.push_back((f & 0b010000) > 0);
                break;
            case 5:
                result.push_back((f & 0b100000) > 0);
                break;
        }

        if (i % 6 == 5) {
            fIndex += 1;
            if (fIndex >= flagArray.size()) break;
            f = numFromString<int>(flagArray[fIndex]).unwrapOr(0b000000);
        }
    }

    for (int i = result.size(); i < size; i++) {
        result.push_back(false); // fill in remaining settings
    }

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

    if (levels.empty()) return levels;

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
    auto str = fmt::format("GDDPSettingsFormatV2;{};{};END", settings, seed);
    auto encodedStr = ZipUtils::base64URLEncode(str);

    clipboard::write(encodedStr);
    //TextAlertPopup::create("Settings Copied to Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");

    return;
}

std::vector<std::string> RouletteUtils::importSettings(std::string str) {
    auto decodedStr = ZipUtils::base64URLDecode(str);
    auto values = DPUtils::substring(decodedStr, ";");

    //verify settings string
    if (values[0] == "GDDPSettingsFormat" && values.size() == 4 && values[3] == "END") {
        // convert old settings string to the new system
        auto f = fromFlags(values[1]);
        values[1] = toFlags(f);
        values[0] = "GDDPSettingsFormatV2";
    }

    if (values[0] != "GDDPSettingsFormatV2" || values.size() != 4 || values[3] != "END") { 
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

    // format levels
    std::string lvlStr = "LVLS";
    for (auto lvl : rsfSave.levels) {
        lvlStr = fmt::format("{},{}", lvlStr.c_str(), lvl);
    }

    std::string str = fmt::format("GDDPSaveFormatV2;{};{};{};{};{};{};{};END", rsfSave.name, rsfSave.settings, lvlStr, rsfSave.seed, rsfSave.progress, rsfSave.skips, rsfSave.score);
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
        auto values = DPUtils::substring(decodedStr, ";");

        //verify save string
        if (values[0] == "GDDPSaveFormat" && values.size() == 9 && values[8] == "END") {
            // convert old settings string to the new system
            auto f = fromFlags(values[2]);
            values[2] = toFlags(f);
            values[0] = "GDDPSaveFormatV2";
        }

        if (values[0] != "GDDPSaveFormatV2" || values.size() != 9 || values[8] != "END") { 
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

        std::vector<int> lvls = setupLevels(packs, values[2], Utils::safe_stoi(values[3]));*/
        std::vector<int> lvls = {};
        auto lvlStrings = DPUtils::substring(values[3], ",");

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
                lvls.push_back(numFromString<int>(str).unwrapOrDefault());
            }
        }

        auto save = RouletteSaveFormat{
            .name = values[1],
            .settings = values[2],
            .seed = numFromString<int>(values[4]).unwrapOr(-1),
            .levels = lvls,
            .progress = numFromString<int>(values[5]).unwrapOrDefault(),
            .skips = numFromString<int>(values[6]).unwrapOrDefault(),
            .score = numFromString<int>(values[7]).unwrapOrDefault()
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