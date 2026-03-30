#include <Geode/Geode.hpp>

#include "DPAchievementManager.hpp"
#include "RecommendedUtils.hpp"
#include "menus/DPLayer.hpp"
#include "popups/StatsPopup.hpp"
#include "DPUtils.hpp"

using namespace geode::prelude;

$on_game(Loaded) {
    DPUtils::verifyCompletedLevels();
    DPAchievementManager::get();
}

DPAchievementManager::DPAchievementManager() {
    CCScheduler::get()->scheduleUpdateForTarget(this, -1, false);
}

void DPAchievementManager::update(float dt) {
    if (m_completedLvls == Mod::get()->getSavedValue<std::vector<int>>("completed-levels")) return;
    m_completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

    auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
    auto achievements = Mod::get()->getSavedValue<matjson::Value>("achievements");
    auto achNotif = AchievementNotifier::sharedState();

    // update pack status
    for (std::string index : {"main", "legacy", "bonus", "monthly"}) {
        for (auto pack : data[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
            // setup important data
            std::string name = pack["name"].asString().unwrapOr("null");
            std::string sprite = pack["sprite"].asString().unwrapOr("DP_Unknown");
            std::string plusSprite = pack["plusSprite"].asString().unwrapOr("DP_Unknown");
            int reqLevels = pack["reqLevels"].as<int>().unwrapOr(-1); 
            std::vector<int> levelIDs = pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
            int month = pack["month"].as<int>().unwrapOr(-1); //Monthly Only
            int year = pack["year"].as<int>().unwrapOr(-1); //Monthly Only
            int mainPack = pack["mainPack"].as<int>().unwrapOr(-1); //Legacy Only
            std::string saveID = (index == "monthly") ? fmt::format("{}-{}", month, year) : pack["saveID"].asString().unwrapOr("null");

            // get pack save file
            auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

            // get completed levels
            auto progress = 0;
            for (auto level : levelIDs) if (DPUtils::containsInt(m_completedLvls, level)) progress += 1;

            // update status
            auto hasRank = listSave.hasRank || ((progress >= reqLevels) && (reqLevels > -1));
            auto completed = (progress == levelIDs.size());

            if (index == "monthly" && progress >= 5) {
                auto completedMonthlies = Mod::get()->getSavedValue<std::vector<std::string>>("monthly-completions");

                if (!DPUtils::containsString(completedMonthlies, saveID)) {
                    completedMonthlies.insert(completedMonthlies.begin(), saveID);
                    if (!Mod::get()->getSavedValue<bool>("dev-preview", false)) Mod::get()->setSavedValue<std::vector<std::string>>("monthly-completions", completedMonthlies);
                }
            }

            //continue if progress is the same
            if (listSave.progress == progress) continue;

            //save
            if (!Mod::get()->getSavedValue<bool>("dev-preview", false)) Mod::get()->setSavedValue<ListSaveFormat>(saveID, ListSaveFormat{ .progress = progress, .completed = completed, .hasRank = hasRank });
        }
    }

    // validate recommendations
    RecommendedUtils::validateLevels();

    // if achievements are disabled, just skip
    if (!Mod::get()->getSettingValue<bool>("achievement-popups")) return;

    // check main list achievements
    if (data.contains("main")) {
        for (auto pack : data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
            auto saveID = pack["saveID"].asString().unwrapOr("null");
            auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

            if (listSave.hasRank && !achievements[saveID].asBool().unwrapOr(false)) {
                AchievementBar* ach = AchievementBar::create(
                    fmt::format("{} Rank!", pack["name"].asString().unwrapOr("???")).c_str(), 
                    fmt::format("Beat {} levels in the {} Tier.", pack["reqLevels"].as<int>().unwrapOr(0), pack["name"].asString().unwrapOr("???")).c_str(), 
                    Mod::get()->expandSpriteName(fmt::format("{}.png", pack["sprite"].asString().unwrapOr("DP_Unknown"))).data(), 
                    true
                );
                ach->m_achievementSprite->setScale(1.f);
                achNotif->m_achievementBarArray->addObject(ach);
                achievements.set(saveID, true);
            }

            if (listSave.completed && !achievements[fmt::format("{}-plus", saveID)].asBool().unwrapOr(false)) {
                AchievementBar* ach = AchievementBar::create(
                    fmt::format("{}+ Complete!", pack["name"].asString().unwrapOr("???")).c_str(), 
                    fmt::format("Beat every level in the {} Tier.", pack["name"].asString().unwrapOr("???")).c_str(), 
                    Mod::get()->expandSpriteName(fmt::format("{}.png", pack["plusSprite"].asString().unwrapOr("DP_Unknown"))).data(), 
                    true
                );
                ach->m_achievementSprite->setScale(1.f);
                achNotif->m_achievementBarArray->addObject(ach);
                achievements.set(fmt::format("{}-plus", saveID), true);
            }
        }
    }

    // check legacy & bonus achievements
    if (!Mod::get()->getSettingValue<bool>("disable-pack-achievements")) {
        for (std::string index : {"legacy", "bonus"}) {
            if (!data.contains(index)) continue;
            for (auto pack : data[index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
                auto saveID = pack["saveID"].asString().unwrapOr("null");
                auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

                auto spriteName = pack["sprite"].asString().unwrapOr("DP_Unknown");
                auto trimmedName = pack["name"].asString().unwrapOr("???");

                if (index == "bonus") {
                    if (trimmedName.starts_with("The ")) trimmedName = trimmedName.substr(4);
                    if (trimmedName.ends_with(" Pack")) trimmedName = trimmedName.substr(0, trimmedName.length() - 5);
                }

                if (listSave.completed && !achievements[saveID].asBool().unwrapOr(false)) {
                    AchievementBar* ach = AchievementBar::create(
                        fmt::format("{} Complete!", pack["name"].asString().unwrapOr("???")).c_str(), 
                        fmt::format("Beat every level in the {} Pack.", trimmedName).c_str(), 
                        (spriteName != "DP_Invisible") ? Mod::get()->expandSpriteName(fmt::format("{}.png", spriteName)).data() : "DP_Unknown.png"_spr, 
                        true
                    );
                    ach->m_achievementSprite->setScale(1.f);
                    ach->m_achievementSprite->setVisible(spriteName != "DP_Invisible");
                    achNotif->m_achievementBarArray->addObject(ach);
                    achievements.set(saveID, true);
                }
            }
        }
    }

    // only look for this month's monthly pack
    if (!Mod::get()->getSettingValue<bool>("disable-monthly-achievements") && data.contains("monthly")) {
        auto monthlyPack = data["monthly"][0];
        auto month = monthlyPack["month"].as<int>().unwrapOr(1);
        auto year = monthlyPack["year"].as<int>().unwrapOr(1987);
        auto saveID = fmt::format("{}-{}", month, year);
        auto monthlySave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

        if (monthlySave.progress >= 5 && !(achievements["monthly"].asString().unwrapOr("") == saveID)) {
            auto trimmedName = monthlyPack["name"].asString().unwrapOr("???");

            if (trimmedName.starts_with("The ")) trimmedName = trimmedName.substr(4);
            if (trimmedName.ends_with(" Pack")) trimmedName = trimmedName.substr(0, trimmedName.length() - 5);
            
            AchievementBar* ach = AchievementBar::create(
                fmt::format("Monthly Complete! ({}/{})", month, year).c_str(), 
                fmt::format("Beat 5/6 levels in the {} Monthly Pack. (#{})", trimmedName, Mod::get()->getSavedValue<std::vector<std::string>>("monthly-completions").size()).c_str(), 
                Mod::get()->expandSpriteName(fmt::format("{}.png", monthlyPack["sprite"].asString().unwrapOr("DP_Unknown"))).data(), 
                true
            );
            ach->m_achievementSprite->setScale(1.f);
            achNotif->m_achievementBarArray->addObject(ach);
            achievements.set("monthly", saveID);
        }

        if (monthlySave.completed && !(achievements["monthly-plus"].asString().unwrapOr("") == saveID)) {
            auto trimmedName = monthlyPack["name"].asString().unwrapOr("???");

            if (trimmedName.starts_with("The ")) trimmedName = trimmedName.substr(4);
            if (trimmedName.ends_with(" Pack")) trimmedName = trimmedName.substr(0, trimmedName.length() - 5);

            auto epicSprite = CCSprite::createWithSpriteFrameName("GJ_epicCoin_001.png"); // for monthly sprites
            epicSprite->setPosition({ -2.f, -15.f });
            epicSprite->setAnchorPoint({ 0.f, 0.f });
            epicSprite->setZOrder(-1);
            
            AchievementBar* ach = AchievementBar::create(
                fmt::format("Monthly+ Complete! ({}/{})", month, year).c_str(), 
                fmt::format("Beat every level in the {} Monthly Pack.", trimmedName).c_str(), 
                Mod::get()->expandSpriteName(fmt::format("{}.png", monthlyPack["sprite"].asString().unwrapOr("DP_Unknown"))).data(), 
                true
            );
            ach->m_achievementSprite->setScale(1.f);
            ach->m_achievementSprite->addChild(epicSprite);
            achNotif->m_achievementBarArray->addObject(ach);
            achievements.set("monthly-plus", saveID);
        }
    }

    // check medals
    if (!Mod::get()->getSettingValue<bool>("disable-medal-achievements") && data.contains("medals")) {
        for (std::string index : {"normal", "plus"}) {
            for (int i = 0; i < data["medals"][index].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
                auto id = fmt::format("{}medal-{}", ((index == "plus") ? "plus-" : ""), i);
                auto name = data["medals"][index][i]["name"].asString().unwrapOr("");
                auto req = data["medals"][index][i]["requirement"].as<int>().unwrapOr(0);
                if (!achievements[id].asBool().unwrapOr(false) && (StatsPopup::getPercentToRank(req, (index == "plus")) >= 1.f)) {
                    AchievementBar* ach = AchievementBar::create(
                        (index == "plus") ? fmt::format("{} Plus Medal Obtained!", name).c_str() : fmt::format("{} Medal Obtained!", name).c_str(), 
                        (index == "plus") ? 
                        fmt::format("Achieve every rank from {} to {}+.", data["main"][0]["name"].asString().unwrapOr("???"), data["main"][req]["name"].asString().unwrapOr("???")).c_str() :
                        fmt::format("Get the normal ranks from {} to {}.", data["main"][0]["name"].asString().unwrapOr("???"), data["main"][req]["name"].asString().unwrapOr("???")).c_str(), 
                        "GJ_sStarsIcon_001.png", 
                        true
                    );
                    ach->m_achievementSprite->setScale(2.f);
                    achNotif->m_achievementBarArray->addObject(ach);
                    achievements.set(id, true);
                }
            }
        }

        // check absolute perfection
        auto progressPercent = StatsPopup::getPercentToRank(data["main"].asArray().unwrap().size() - 1, true);

        //Get All Bonus Progress
        auto bonusProgress = 0;
        auto bonusTotalLevels = 0;
        for (auto pack : data["bonus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>())) {
            auto saveID = pack["saveID"].asString().unwrapOr("null");
            auto totalLevels = pack["levelIDs"].as<std::vector<int>>().unwrapOrDefault().size();
            auto listSave = Mod::get()->getSavedValue<ListSaveFormat>(saveID);

            bonusProgress += listSave.progress;
            bonusTotalLevels += totalLevels;
        }

        auto bonusPercent = static_cast<float>(bonusProgress) / static_cast<float>(bonusTotalLevels);
        auto totalPercent = (progressPercent + bonusPercent) / 2.f;

        if (!achievements["medal-absolute-perfection"].asBool().unwrapOr(false) && (totalPercent >= 1.f)) {
            AchievementBar* ach = AchievementBar::create(
                "ABSOLUTE PERFECTION ACHIEVED!!",
                "Beat every Level in every Tier and Bonus Pack. GG.",
                "GJ_sStarsIcon_001.png", 
                true
            );
            ach->m_achievementSprite->setScale(2.f);
            achNotif->m_achievementBarArray->addObject(ach);
            achievements.set("medal-absolute-perfection", true);
        }
    }

    // activate achievements
    if (!achNotif->m_activeAchievementBar && achNotif->m_achievementBarArray->count() > 0) achNotif->showNextAchievement();

    // save
    Mod::get()->setSavedValue("achievements", achievements);
}