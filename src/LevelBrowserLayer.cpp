#include <Geode/Geode.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include "ListManager.hpp"

using namespace geode::prelude;

class $modify(LevelBrowserLayer) {
    void loadLevelsFinished(CCArray *p0, char const* p1, int p2) {
        LevelBrowserLayer::loadLevelsFinished(p0, p1, p2);

        auto *listLayer = static_cast<GJListLayer *>(this->getChildByID("GJListLayer"));
        auto contentLayer = listLayer->m_listView->m_tableView->m_contentLayer;
        CCArray *levels = contentLayer->getChildren();
        int levelCount = contentLayer->getChildrenCount();

        for (int i = 0; i < levelCount; i++) {
            auto *levelCell = static_cast<LevelCell *>(levels->objectAtIndex(i));
            auto *level = levelCell->m_level;
            CCNode *diffContainer = levelCell->getChildByID("main-layer")->getChildByID("difficulty-container");
            auto *diffSpr = static_cast<GJDifficultySprite *>(diffContainer->getChildByID("difficulty-sprite"));
            if (Mod::get()->getSavedValue<bool>("in-gddp"))
                break;

            if (Mod::get()->getSettingValue<bool>("custom-difficulty-faces") && Mod::get()->getSettingValue<bool>("all-demons-rated")) {
                std::string sprite = ListManager::getSpriteName(level);
                if (sprite == "")
                    continue;

                std::string fullSpr = sprite + "SmallText.png";
                std::string fullPlusSpr = sprite + "PlusSmallText.png";

                auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr.c_str()));

                if (level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic")) {
                    customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr.c_str()));
                }

                if (customSpr) {
                    diffSpr->setOpacity(0);

                    customSpr->setID("gddp-difficulty");
                    customSpr->setAnchorPoint({ 0.5f, 1 });
                    customSpr->setPosition({ diffSpr->getPositionX(), diffSpr->getPositionY() + 30.75f });
                    customSpr->setZOrder(5);

                    diffContainer->addChild(customSpr);

                    if (this->getChildByID("grd-difficulty") && !Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
                        customSpr->setVisible(false);
                    }
                }
            }
        }
    }
};