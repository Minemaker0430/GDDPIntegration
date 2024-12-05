//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/modify/PlayLayer.hpp>

//geode namespace
using namespace geode::prelude;

class $modify(PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        PlayLayer::destroyPlayer(player, obj);

        bool inRoulette = Mod::get()->getSavedValue<bool>("in-roulette", false);
        int currentLvlID = Mod::get()->getSavedValue<int>("roulette-lvl-id", -1);
        int rouletteProgress = Mod::get()->getSavedValue<int>("roulette-progress", 0);
        int rouletteGoal = Mod::get()->getSavedValue<int>("roulette-next-goal", 100);

        bool perfectEnabled = Mod::get()->getSavedValue<bool>("roulette-perfect", false);
        bool gauntletEnabled = Mod::get()->getSavedValue<bool>("roulette-gauntlet", false);

        //make sure you're playing the roulette
        if (
            !inRoulette ||
            this->m_level->m_levelID.value() != currentLvlID ||
            this->m_isPracticeMode
        ) { return; }

        const int percentage = this->getCurrentPercentInt();
        if (percentage > 0) {
            //log::info("died at {}%", percentage);
        }

        if (perfectEnabled && percentage == rouletteGoal) {
            Mod::get()->setSavedValue<int>("roulette-progress", percentage);
            log::info("reached goal of {}% in perfect mode", percentage);
        }
        else if (!perfectEnabled && percentage > rouletteProgress) {
            Mod::get()->setSavedValue<int>("roulette-progress", percentage);
            log::info("new best in roulette! {}%", percentage);
        }
        
        return;
    }

    void levelComplete() {
        PlayLayer::levelComplete();

        bool inRoulette = Mod::get()->getSavedValue<bool>("in-roulette", false);
        int currentLvlID = Mod::get()->getSavedValue<int>("roulette-lvl-id", -1);
        int rouletteProgress = Mod::get()->getSavedValue<int>("roulette-progress", 0);
        int rouletteGoal = Mod::get()->getSavedValue<int>("roulette-next-goal", 100);

        bool perfectEnabled = Mod::get()->getSavedValue<bool>("roulette-perfect", false);
        bool gauntletEnabled = Mod::get()->getSavedValue<bool>("roulette-gauntlet", false);

        //make sure you're playing the roulette
        if (
            !inRoulette ||
            this->m_level->m_levelID.value() != currentLvlID ||
            this->m_isPracticeMode
        ) { return; }

        Mod::get()->setSavedValue<int>("roulette-progress", 100);

        return;
    }
};