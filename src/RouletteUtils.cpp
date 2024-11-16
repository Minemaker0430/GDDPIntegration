//geode header
#include <Geode/Geode.hpp>
#include <Geode/utils/JsonValidation.hpp>

#include <random>

#include "RouletteUtils.hpp"
#include "Utils.hpp"

//geode namespace
using namespace geode::prelude;

int RouletteUtils::toFlags(std::vector<bool> settings) {
    auto result = 0;
    
    for (int i = 0; i < settings.size(); i++) {
        if (settings[i]) {
            result += std::max(1, i * 2);
        }
    }

    return result;
}

std::vector<bool> RouletteUtils::fromFlags(int flags) {

    //find the size of the settings
    auto size = 0;
    auto flagCheck = 0;
    while (flagCheck < flags) {
        flagCheck += std::max(1, size * 2);
        size += 1;
    }
    

    //fill vector with false so we can work backwards
    std::vector<bool> result = {};
    for (int i = 0; i < size; i++) {
        result.push_back(false);
    }

    //iterate backwards
    //if (flags - flag value < 0), it's false
    for (int i = size - 1; i >= 0; i--) {
        if (flags - std::max(1, i * 2) >= 0) {
            result[i] = true;
            flags -= std::max(1, i * 2);
        }
    }

    return result;
}

std::vector<int> RouletteUtils::setupLevels(std::vector<matjson::Value> packs, int randomSeed) {
    
    //get levels
    std::vector<int> levels = {};
    for (auto pack : packs) {
        for (auto lvl : pack["levelIDs"].as<std::vector<int>>().unwrap()) {
            levels.push_back(lvl);
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

    //set order
    std::vector<int> out = {};
    for (int place : order) {
        out.push_back(levels[place]);
    }

    return out;
}

void RouletteUtils::exportSettings(std::vector<bool> settings, int seed) {
    auto settingFlags = toFlags(settings);
    auto str = fmt::format("GDDPSettingsFormat;{};{}", settingFlags, seed);
    auto encodedStr = ZipUtils::base64URLEncode(str);

    clipboard::write(encodedStr);
    //TextAlertPopup::create("Settings Copied to Clipboard", 2.f, .6f, 0x96, "bigFont.fnt");

    return;
}

std::vector<int> RouletteUtils::importSettings(std::string str) {
    auto decodedStr = ZipUtils::base64URLDecode(str);
    auto values = Utils::substring(decodedStr, ";");

    //verify settings string
    if (values[0] != "GDDPSettingsFormat") { 
        FLAlertLayer::create(
			"ERROR",
			"Invalid Settings. Make sure you pasted the correct code and try again.",
			"OK"
			)->show(); 
        return { 0, rand() };
    }

    auto settingFlags = std::stoi(values[1]);
    auto seed = std::stoi(values[2]);

    std::vector<int> out = { settingFlags, seed };
    return out;
}

void RouletteUtils::createSave(std::string name, std::vector<int> levels, int settingsFlags, int seed) {
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

   auto save = RouletteSaveFormat{ .name = name, .settings = settingsFlags, .seed = seed, .levels = levels };

   auto rouletteSaves = Mod::get()->getSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", {});
   rouletteSaves.push_back(save);

   Mod::get()->setSavedValue<std::vector<RouletteSaveFormat>>("roulette-saves", rouletteSaves);

   return;
}

void RouletteUtils::exportSave(matjson::Value save, bool toFile) {

    RouletteSaveFormat rsfSave = save.as<RouletteSaveFormat>().unwrap();

    //format levels
    std::string lvlStr = "LVLS";
    for (auto lvl : rsfSave.levels) {
        lvlStr = fmt::format("{},{}", lvlStr.c_str(), lvl);
    }

    auto str = fmt::format("GDDPSaveFormat;{};{};{};{};{};{};{}", rsfSave.name, rsfSave.settings, rsfSave.seed, lvlStr, rsfSave.progress, rsfSave.skips, rsfSave.score);
    auto encodedStr = ZipUtils::base64URLEncode(str);

    if (toFile) {
        //TODO
        log::info("TODO: Export save to file.");
    } else {
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
        if (values[0] != "GDDPSaveFormat") { 
            FLAlertLayer::create(
			"ERROR",
			"Invalid Save File. Make sure you pasted the correct code and try again.",
			"OK"
			)->show(); 
            return; 
        }

        //convert levels to a proper readable list
        std::vector<int> lvls = {};
        auto lvlStrings = Utils::substring(values[4], ",");
        for (std::string str : lvlStrings) {
            if (str != "LVLS") {
                lvls.push_back(std::stoi(str));
            }
        }

        auto save = RouletteSaveFormat{
            .name = values[1],
            .settings = std::stoi(values[2]),
            .seed = std::stoi(values[3]),
            .levels = lvls,
            .progress = std::stoi(values[5]),
            .skips = std::stoi(values[6]),
            .score = std::stoi(values[7])
        };
    }

    return;
}