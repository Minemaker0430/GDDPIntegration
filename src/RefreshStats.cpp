#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

class RefreshStatsSetting : public SettingV3 {
public:
    // All settings must provide a `parse` function for parsing the setting 
    // from its definition in `mod.json`. The signature must match this exactly, 
    // although the return type should point to a `SettingV3`-derivative rather 
    // than `std::shared_ptr<SettingV3>` directly

    // Once again implement the parse function
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
        auto res = std::make_shared<RefreshStatsSetting>();
        auto root = checkJson(json, "RefreshStatsSetting");

        // `parseBaseProperties` parses all base properties, including 
        // `requires-restart` - which does not really make sense in our use case, 
        // as there's nothing in this setting that could require a restart.
        // So, we instead parse the base properties that actually apply to this 
        // setting manually:
        res->init(key, modID, root);
        res->parseNameAndDescription(root);
        res->parseEnableIf(root);
        
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }

    // Since there's no data to save or load, these can just return true
    // Although you could use these for example to store how many times the 
    // button has been clicked 
    bool load(matjson::Value const& json) override {
        return true;
    }
    bool save(matjson::Value& json) const override {
        return true;
    }

    // This setting can't ever have anything but the default value, as it has 
    // no value
    bool isDefaultValue() const override {
        return true;
    }
    void reset() override {}

    // Once again defined out-of-line
    SettingNodeV3* createNode(float width) override;
};

// The second most important part of the setting is defining the node for the 
// UI. Again, here we use a helper class, which automatically defines a bunch 
// of the required virtual functions for us. Note that using 
// `SettingValueNodeV3` requires the setting type to implement 
// `SettingBaseValueV3`!
class RefreshStatsSettingNode : public SettingNodeV3 {
protected:
    // If we use `SettingValueNodeV3`, we once again don't need to track the 
    // current value ourselves - we just use the `getValue` and `setValue` 
    // functions!
    CCSprite* m_buttonSprite;
    CCMenuItemSpriteExtra* m_button;

    bool init(std::shared_ptr<RefreshStatsSetting> setting, float width) {
        if (!SettingNodeV3::init(setting, width))
            return false;
        
        // Create button
        m_buttonSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
        m_buttonSprite->setScale(0.5f);
        m_button = CCMenuItemSpriteExtra::create(
            m_buttonSprite, this, menu_selector(RefreshStatsSettingNode::onBtn)
        );
            
        this->getButtonMenu()->addChildAtPosition(m_button, Anchor::Center);
        this->getButtonMenu()->setContentWidth(20);
        this->getButtonMenu()->updateLayout();

        // Remember to always call `updateState` at the end of your init!
        this->updateState(nullptr);
        
        return true;
    }
    
    void updateState(CCNode* invoker) override {
        // Remember to always call the base class's `updateState`!
        SettingNodeV3::updateState(invoker);

        // If your setting supports "enable-if" schemes, you should make sure 
        // that the UI is updated to reflect that. If `shouldEnable` returns 
        // false, all toggles and buttons should be grayed out and disabled!
        auto shouldEnable = this->getSetting()->shouldEnable();
        m_button->setEnabled(shouldEnable);
        m_buttonSprite->setCascadeColorEnabled(true);
        m_buttonSprite->setCascadeOpacityEnabled(true);
        m_buttonSprite->setOpacity(shouldEnable ? 255 : 155);
        m_buttonSprite->setColor(shouldEnable ? ccWHITE : ccGRAY);
    }

    void onBtn(CCObject* sender) {
        geode::createQuickPopup(
		"Refresh Stats?",										 // title
		"Are you sure you want to refresh your stats? This will clear the list of levels that the mod has marked as completed.\n\n<cr>Ranks and your base save file will NOT be affected.</c>", // content
		"No", "Yes",										 // buttons
		[&](auto, bool btn2)
		{
			if (btn2)
			{
				Mod::get()->setSavedValue<std::vector<int>>("completed-levels", std::vector<int>());
				FLAlertLayer::create(
			        "Done!",
			        "Stats have successfully been refreshed.",
			        "OK"
		        )->show();

			}
		});
    }

    // Both of these can just be no-ops, since they make no sense for our 
    // setting as it's just a button
    void onCommit() override {}
    void onResetToDefault() override {}

public:
    static RefreshStatsSettingNode* create(std::shared_ptr<RefreshStatsSetting> setting, float width) {
        auto ret = new RefreshStatsSettingNode();
        if (ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    // Both of these can just return false, since they make no sense for our 
    // setting as it's just a button
    bool hasUncommittedChanges() const override {
        return false;
    }
    bool hasNonDefaultValue() const override {
        return false;
    }

    std::shared_ptr<RefreshStatsSetting> getSetting() const {
        return std::static_pointer_cast<RefreshStatsSetting>(SettingNodeV3::getSetting());
    }
};

// Define the node creation function
SettingNodeV3* RefreshStatsSetting::createNode(float width) {
    return RefreshStatsSettingNode::create(
        std::static_pointer_cast<RefreshStatsSetting>(shared_from_this()),
        width
    );
}

// When the mod is loaded, you need to register your setting type
$execute {
    // You can also handle the errors, but if this fails, the game is probably about to crash anyway
    (void)Mod::get()->registerCustomSettingType("refresh-stats", &RefreshStatsSetting::parse);
}
