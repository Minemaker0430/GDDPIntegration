// geode header
#include <Geode/Geode.hpp>

#include "TextEffectEditorPopup.hpp"
#include "Geode/cocos/menu_nodes/CCMenu.h"

// other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include <matjson.hpp>

/*
============================

Properties:

textColor (Color) = base text color
outlineColor (Color) = outline color
shadowColor (Color) = drop shadow color

gradientColor (Color) = gradient color, not dependent on type
crystalColor (Color) = crystal color
starsColor (Color) = stars color

gradientOpacity (Float) = gradient opacity, not dependent on type
crystalOpacity (Float) = crystal opacity
starsOpacity (Float) = stars opacity

gradientType (Enum) = "none", "horizontal", or "radial". Determines gradient style

gradient (Bool) = whether or not the text should have a gradient
crystal (Bool) = whether or not the text should have the crystal effect
stars (Bool) = whether or not the text should have the stars effect
bevel (Bool) = whether or not the text should have bevel

special (String) - custom effects

============================
*/

// geode namespace
using namespace geode::prelude;

bool TextEffectEditorPopup::init()
{
    if (!Popup::init(420.f, 250.f)) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	this->setTitle("Edit Effects");

	auto layer = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));

    // sample text
    m_sampleText = CustomText::create("Sample Text");
    m_sampleText->setPosition(layer->getContentWidth() / 2.f, layer->getContentHeight() / 2.f);
    m_sampleText->setAnchorPoint({ 0.5f, 0.5f });
    m_sampleText->addEffectsFromProperties(m_data);
    layer->addChild(m_sampleText);

    // colors
    std::vector<std::string> colorKeys = {"textColor", "outlineColor", "shadowColor", "gradientColor", "crystalColor", "starsColor"};
    std::map<std::string, ccColor3B> defaultColors = {
        {"textColor", ccColor3B{255, 255, 255}}, 
        {"outlineColor", ccColor3B{0, 0, 0}}, 
        {"shadowColor", ccColor3B{0, 0, 0}}, 
        {"gradientColor", ccColor3B{255, 255, 255}}, 
        {"crystalColor", ccColor3B{255, 255, 255}}, 
        {"starsColor", ccColor3B{255, 255, 255}}
    };

    auto colorsMenu = CCMenu::create();
    colorsMenu->setPosition(layer->getContentWidth() / 2.f, 215.f);
    colorsMenu->setAnchorPoint({ 0.5f, 1.f });
    colorsMenu->setID("colors-menu");
    layer->addChild(colorsMenu);

    auto colorsLayout = AxisLayout::create();
    colorsLayout->setAxis(Axis::Row);
    colorsLayout->setAutoGrowAxis(true);
    colorsLayout->setGap(30.f);
    colorsLayout->setAutoScale(false);
    colorsMenu->setLayout(colorsLayout);

    for (auto key : colorKeys) {
        auto colorSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
		colorSpr->setScale(0.75f);
        auto colorLabel = CCLabelBMFont::create(key.c_str(), "bigFont.fnt");
        colorLabel->setPositionX(15.5f);
        colorLabel->setScale(0.3f);
        colorSpr->addChild(colorLabel);
		auto colorBtn = CCMenuItemSpriteExtra::create(colorSpr, this, menu_selector(TextEffectEditorPopup::onValueChanged));
		colorBtn->setID(fmt::format("{}-btn", key));
		colorBtn->setColor(m_data[key].as<ccColor3B>().unwrapOr(defaultColors[key]));
        colorBtn->setUserObject(new SetValueParameters(ValueType::Color, key));
		colorsMenu->addChild(colorBtn);
        colorsMenu->updateLayout();
    }

    // toggles
    std::vector<std::string> toggleKeys = {"gradient", "crystal", "stars", "bevel"};

    auto toggleMenu = CCMenu::create();
    toggleMenu->setPosition(layer->getContentWidth() / 2.f, 175.f);
    toggleMenu->setAnchorPoint({ 0.5f, 1.f });
    toggleMenu->setID("toggle-menu");
    layer->addChild(toggleMenu);

    auto toggleLayout = AxisLayout::create();
    toggleLayout->setAxis(Axis::Row);
    toggleLayout->setAutoGrowAxis(true);
    toggleLayout->setGap(30.f);
    toggleLayout->setAutoScale(false);
    toggleMenu->setLayout(toggleLayout);

    for (auto key : toggleKeys) {
        auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
		toggleOnSpr->setScale(0.75f);
        toggleOffSpr->setScale(0.75f);
        auto toggleLabel = CCLabelBMFont::create(key.c_str(), "bigFont.fnt");
        toggleLabel->setPositionX(15.5f);
        toggleLabel->setScale(0.3f);
        toggleOnSpr->addChild(toggleLabel);
        toggleOffSpr->addChild(toggleLabel);
		auto toggleBtn = CCMenuItemToggler::create(toggleOffSpr, toggleOnSpr, this, menu_selector(TextEffectEditorPopup::onValueChanged));
		toggleBtn->setID(fmt::format("{}-btn", key));
		toggleBtn->toggle(m_data[key].asBool().unwrapOr(false));
        toggleBtn->setUserObject(new SetValueParameters(ValueType::Bool, key));
		toggleMenu->addChild(toggleBtn);
        toggleMenu->updateLayout();
    }

    // opacity
    std::vector<std::string> opacityKeys = {"gradientOpacity", "crystalOpacity", "starsOpacity"};

    auto opacityMenu = CCMenu::create();
    opacityMenu->setPosition(layer->getContentWidth() / 2.f, 90.f);
    opacityMenu->setAnchorPoint({ 0.5f, 1.f });
    opacityMenu->setID("opacity-menu");
    layer->addChild(opacityMenu);

    auto opacityLayout = AxisLayout::create();
    opacityLayout->setAxis(Axis::Row);
    opacityLayout->setAutoGrowAxis(true);
    opacityLayout->setGap(30.f);
    opacityLayout->setAutoScale(false);
    opacityMenu->setLayout(opacityLayout);

    for (auto key : opacityKeys) {
        auto opacityField = TextInput::create(100.f, "1.00");
        opacityField->setCommonFilter(CommonFilter::Float);
		opacityField->setID(fmt::format("{}-field", key));
        opacityField->setLabel(key);
		opacityField->setString(std::to_string(m_data[key].as<float>().unwrapOr(1.00f)));
        opacityField->setUserObject(new SetValueParameters(ValueType::Float, key));
        opacityField->setCallback([this, opacityField](const std::string& text) {onValueChanged(static_cast<CCObject*>(opacityField));});
		opacityMenu->addChild(opacityField);
        opacityMenu->updateLayout();
    }

    // special
    auto stringMenu = CCMenu::create();
    stringMenu->setPosition(80.f, 30.f);
    stringMenu->setAnchorPoint({ 0.5f, 1.f });
    stringMenu->setID("string-menu");
    layer->addChild(stringMenu);

    /*auto opacityLayout = AxisLayout::create();
    opacityLayout->setAxis(Axis::Row);
    opacityLayout->setAutoGrowAxis(true);
    opacityLayout->setGap(30.f);
    opacityLayout->setAutoScale(false);
    opacityMenu->setLayout(opacityLayout);*/

    auto specialField = TextInput::create(100.f, "...");
    specialField->setCommonFilter(CommonFilter::Any);
	specialField->setID("special-field");
    specialField->setLabel("special");
	specialField->setString(m_data["special"].asString().unwrapOrDefault());
    specialField->setUserObject(new SetValueParameters(ValueType::String, "special"));
    specialField->setCallback([this, specialField](const std::string& text) {onValueChanged(static_cast<CCObject*>(specialField));});
	stringMenu->addChild(specialField);
    //opacityMenu->updateLayout();

    // gradient type enum
    auto enumMenu = CCMenu::create();
    enumMenu->setPosition(340.f, 30.f);
    enumMenu->setAnchorPoint({ 0.5f, 1.f });
    enumMenu->setID("enum-menu");
    layer->addChild(enumMenu);

    std::map<std::string, std::string> typeMap = {
        {"none", "Default"},
        {"horizontal", "Horizontal"},
        {"radial", "Radial"}
    };

    auto enumLabel = CCLabelBMFont::create(typeMap[m_data["gradientType"].asString().unwrapOr("none")].c_str(), "bigFont.fnt");
	enumLabel->setID("enum-label");
    enumLabel->setScale(0.55f);

    auto enumHeader = CCLabelBMFont::create("gradientType", "bigFont.fnt");
	enumHeader->setID("enum-header");
    enumHeader->setScale(0.4f);
    enumHeader->setPositionY(15.f);

    auto spr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    spr->setScale(0.5f);

    auto rightBtn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(TextEffectEditorPopup::onValueChanged));
    rightBtn->setUserObject(new SetValueParameters(ValueType::Enum, "right"));
    rightBtn->setPositionX(60.f);

    auto altSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    altSpr->setScale(0.5f);
    altSpr->setFlipX(true);
    auto leftBtn = CCMenuItemSpriteExtra::create(altSpr, this, menu_selector(TextEffectEditorPopup::onValueChanged));
    leftBtn->setUserObject(new SetValueParameters(ValueType::Enum, "left"));
    leftBtn->setPositionX(-60.f);

    enumMenu->addChild(enumHeader);
	enumMenu->addChild(enumLabel);
    enumMenu->addChild(rightBtn);
    enumMenu->addChild(leftBtn);

    auto saveMenu = CCMenu::create();
    saveMenu->setPosition(layer->getContentWidth() / 2.f, 30.f);
    saveMenu->setAnchorPoint({ 0.5f, 1.f });
    saveMenu->setID("save-menu");
    layer->addChild(saveMenu);

    auto saveSpr = ButtonSprite::create("Save");
    auto saveBtn = CCMenuItemSpriteExtra::create(saveSpr, this, menu_selector(TextEffectEditorPopup::onSave));
    saveBtn->setID("save-btn");
    saveMenu->addChild(saveBtn);

	return true;
}

void TextEffectEditorPopup::onValueChanged(CCObject* sender) {
    auto params = static_cast<SetValueParameters*>(static_cast<CCNode*>(sender)->getUserObject());

    switch (params->m_type) {
        case ValueType::Color: 
        {
            auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
            auto popup = ColorPickPopup::create(btn->getColor());
            popup->setCallback(
                [this, params, btn](const ccColor4B& color) {
                    m_data.set(params->m_key, to3B(color));
                    btn->setColor(to3B(color));

                    m_sampleText->clearEffects();
                    m_sampleText->addEffectsFromProperties(m_data);
                }
            );
            popup->show();
            break;
        }
        case ValueType::Float: 
        {
            auto btn = static_cast<TextInput*>(sender);
            m_data.set(params->m_key, clampf(numFromString<float>(btn->getString()).unwrapOr(1.f), 0.f, 1.f));

            m_sampleText->clearEffects();
            m_sampleText->addEffectsFromProperties(m_data);

            break;
        }
        case ValueType::Bool: 
        {
            auto btn = static_cast<CCMenuItemToggler*>(sender);
            m_data.set(params->m_key, !btn->isToggled());

            m_sampleText->clearEffects();
            m_sampleText->addEffectsFromProperties(m_data);

            break;
        }
        case ValueType::String: 
        {
            auto btn = static_cast<TextInput*>(sender);
            m_data.set(params->m_key, btn->getString());

            m_sampleText->clearEffects();
            m_sampleText->addEffectsFromProperties(m_data);

            break;
        }
        case ValueType::Enum:
        {
            auto type = (int)m_gradientType + ((params->m_key == "left") ? -1 : 1);
            m_gradientType = (type > (int)GradientType::Radial ? GradientType::Default : (type < (int)GradientType::Default ? GradientType::Radial : (GradientType)type));

            std::map<GradientType, std::string> typeMap = {
                {GradientType::Default, "Default"},
                {GradientType::Horizontal, "Horizontal"},
                {GradientType::Radial, "Radial"}
            };

            std::map<GradientType, std::string> keyMap = {
                {GradientType::Default, "none"},
                {GradientType::Horizontal, "horizontal"},
                {GradientType::Radial, "radial"}
            };

            static_cast<CCLabelBMFont*>(this->getChildByIDRecursive("enum-label"))->setCString(typeMap[m_gradientType].c_str());
            m_data.set("gradientType", keyMap[m_gradientType]);

            m_sampleText->clearEffects();
            m_sampleText->addEffectsFromProperties(m_data);

            break;
        }
    }
    
    return;
}

void TextEffectEditorPopup::onSave(CCObject*) {
    if (m_callback) m_callback(m_data);
	this->onClose(new CCObject());
}

void TextEffectEditorPopup::setCallback(geode::Function<void(matjson::Value const&)> cb) {
	m_callback = std::move(cb);
}

TextEffectEditorPopup* TextEffectEditorPopup::create(matjson::Value data) {
	auto ret = new TextEffectEditorPopup();
    ret->m_data = data;
	if (ret && ret->init())
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

TextEffectEditorPopup::~TextEffectEditorPopup() {
	this->removeAllChildrenWithCleanup(true);
}