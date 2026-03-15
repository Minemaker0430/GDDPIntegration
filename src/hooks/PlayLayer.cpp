//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/modify/PlayLayer.hpp>
#include "../DPUtils.hpp"

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
            this->m_isPracticeMode ||
            gauntletEnabled
        ) return;

        int percentage = this->getCurrentPercentInt();

        if ((perfectEnabled ? (percentage == rouletteGoal) : (percentage >= rouletteProgress)) && percentage > 0) {
            Mod::get()->setSavedValue<int>("roulette-progress", percentage);
            log::info("new best in roulette! {}%", percentage);
        }
        
        return;
    }

    void levelComplete() {
        PlayLayer::levelComplete();

        if (this->m_isPracticeMode) return; // ALWAYS return if in practice mode

        auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
        auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

        //mark level as completed
        if (data["level-data"].contains(std::to_string(this->m_level->m_levelID.value())) && std::find(completedLvls.begin(), completedLvls.end(), this->m_level->m_levelID.value()) == completedLvls.end()) {
            completedLvls.insert(completedLvls.begin(), this->m_level->m_levelID.value());
			Mod::get()->setSavedValue<std::vector<int>>("completed-levels", completedLvls);
        }

        //roulette stuff
        bool inRoulette = Mod::get()->getSavedValue<bool>("in-roulette", false);
        int currentLvlID = Mod::get()->getSavedValue<int>("roulette-lvl-id", -1);
        int rouletteProgress = Mod::get()->getSavedValue<int>("roulette-progress", 0);
        int rouletteGoal = Mod::get()->getSavedValue<int>("roulette-next-goal", 100);

        bool perfectEnabled = Mod::get()->getSavedValue<bool>("roulette-perfect", false);
        bool gauntletEnabled = Mod::get()->getSavedValue<bool>("roulette-gauntlet", false);

        //make sure you're playing the roulette
        if (!inRoulette || this->m_level->m_levelID.value() != currentLvlID) return;

        Mod::get()->setSavedValue<int>("roulette-progress", (gauntletEnabled) ? rouletteGoal : 100);

        return;
    }
};