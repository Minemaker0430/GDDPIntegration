#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class DPAchievementManager : public CCObject {
protected:
    DPAchievementManager();

    std::vector<int> m_completedLvls;

    virtual void update(float dt) override;
public:
    static DPAchievementManager& get() {
        static DPAchievementManager instance;
        return instance;
    }
};