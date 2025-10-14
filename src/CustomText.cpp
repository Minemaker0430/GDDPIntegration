// geode header
#include <Geode/Geode.hpp>

#include "CustomText.hpp"

// geode namespace
using namespace geode::prelude;

bool CustomText::init(std::string text) {
    if (!CCNode::init()) return false;
    
    m_label = CCLabelBMFont::create(text.c_str(), "blankFont.fnt"_spr);
    m_outline = CCLabelBMFont::create(text.c_str(), "outlineFont.fnt"_spr);
    m_dropshadow = CCLabelBMFont::create(text.c_str(), "dropshadowFont.fnt"_spr);

    m_label->setAnchorPoint({ 0.f, 0.f });
    m_outline->setAnchorPoint({ 0.f, 0.f });
    m_dropshadow->setAnchorPoint({ 0.f, 0.f });
    m_effects->setAnchorPoint({ 0.f, 0.f });

    m_outline->setColor({0, 0, 0});
    m_dropshadow->setColor({0, 0, 0});

    m_dropshadow->setOpacity(125);

    m_label->setZOrder(2);
    m_outline->setZOrder(1);
    m_effects->setZOrder(3);

    m_effects->setContentSize(m_label->getContentSize());
    this->setContentSize(m_label->getContentSize());

    //convert text into sprites for stencil
    auto clippingText = CCLabelBMFont::create(text.c_str(), "blankFont.fnt"_spr);
    auto stencilParent = CCNode::create();

    for (int i = 0; i < clippingText->getChildrenCount(); i++) {
        CCSprite* spr = typeinfo_cast<CCSprite*>(clippingText->getChildren()->objectAtIndex(i));
        stencilParent->addChild(spr);
    }

    m_effects->setStencil(stencilParent);
    m_effects->setAlphaThreshold(0.5f);

    this->addChild(m_label);
    this->addChild(m_outline);
    this->addChild(m_dropshadow);
    this->addChild(m_effects);

    return true;
}

void CustomText::setText(std::string text) {
    m_label->setCString(text.c_str());
    m_outline->setCString(text.c_str());
    m_dropshadow->setCString(text.c_str());
    
    //convert text into sprites for stencil
    auto clippingText = CCLabelBMFont::create(text.c_str(), "blankFont.fnt"_spr);
    auto stencilParent = CCNode::create();

    for (int i = 0; i < clippingText->getChildrenCount(); i++) {
        CCSprite* spr = typeinfo_cast<CCSprite*>(clippingText->getChildren()->objectAtIndex(i));
        stencilParent->addChild(spr);
    }

    m_effects->setStencil(stencilParent);
    
    return;
}

std::string CustomText::getText() {
    return m_label->getString();
}

void CustomText::addBevel() {
    if (Mod::get()->getSettingValue<bool>("disable-bevel")) { return; }
    
    auto bevel = CCLabelBMFont::create(m_label->getString(), "bevelFont.fnt"_spr);
    bevel->setOpacity(125);
    bevel->setZOrder(5);
    bevel->setAnchorPoint({ 0.f, 0.f });
    bevel->setTag(100);

    m_effects->addChild(bevel);

    return;
}

void CustomText::addGradient(ccColor3B color, float opacity, bool rotated) {
    if (Mod::get()->getSettingValue<bool>("disable-text-gradient")) { return; }

    auto gradient = CCSprite::createWithSpriteFrameName("DP_Gradient.png"_spr);
    gradient->setColor(color);
    gradient->setOpacity(255 * opacity);
    gradient->setAnchorPoint({0.f, 0.f});
    scaleToContentSize(gradient, {m_label->getContentWidth(), m_label->getContentHeight() - 9.f});
    if (rotated) { 
        gradient->setRotation(90.f);
        scaleToContentSize(gradient, {m_label->getContentHeight() - 5.f, m_label->getContentWidth() - 4.5f});
        gradient->setAnchorPoint({1.f, 0.f});
    }

    //add a buffer for tall characters
    auto buffer = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
    buffer->setColor(color);
    buffer->setOpacity(255 * opacity);
    buffer->setAnchorPoint({0.f, 0.f});
    scaleToContentSize(buffer, {m_label->getContentWidth(), m_label->getContentHeight() - 9.f});
    buffer->setPositionY(buffer->getScaledContentHeight());

    if (!rotated) { m_effects->addChild(buffer); }
    m_effects->addChild(gradient);

    return;
}

void CustomText::addRadialGradient(ccColor3B color, float opacity) {
    if (Mod::get()->getSettingValue<bool>("disable-text-gradient")) { return; }
    
    auto gradient = CCSprite::createWithSpriteFrameName("DP_RadialGradient.png"_spr);
    gradient->setColor(color);
    gradient->setOpacity(255 * opacity);
    
    scaleToContentSize(gradient, {m_label->getContentWidth(), m_label->getContentHeight() - 5.f});

    gradient->setAnchorPoint({0.f, 0.f});

    m_effects->addChild(gradient);

    return;
}

void CustomText::addStars(ccColor3B color, float opacity) {
    if (Mod::get()->getSettingValue<bool>("disable-stars")) { return; }
    
    auto frontStars = CCSprite::createWithSpriteFrameName("DP_Stars.png"_spr);
    auto backStars = CCSprite::createWithSpriteFrameName("DP_Stars.png"_spr);

    frontStars->setColor(color);
    frontStars->setOpacity(255 * opacity);
    scaleToContentSize(frontStars, {m_label->getContentWidth(), m_label->getContentWidth()});

    backStars->setColor(color);
    backStars->setOpacity((255 * 0.5f) * opacity);
    backStars->setFlipX(true);
    backStars->setFlipY(true);
    scaleToContentSize(backStars, {m_label->getContentWidth(), m_label->getContentWidth()});

    frontStars->setAnchorPoint({0.f, 0.5f});
    backStars->setAnchorPoint({0.f, 0.5f});

    m_effects->addChild(frontStars);
    m_effects->addChild(backStars);

    return;
}

void CustomText::addCrystals(ccColor3B color, float opacity) {
    if (Mod::get()->getSettingValue<bool>("disable-crystals")) { return; }
    
    auto crystalParent = CCNode::create();
    
    for (int i = 0; i <= floor(m_label->getContentWidth() / 100.f); i++) {
        auto frontCrystal = CCSprite::createWithSpriteFrameName("DP_Crystal_001.png"_spr);
        auto midCrystal = CCSprite::createWithSpriteFrameName("DP_Crystal_002.png"_spr);
        auto backCrystal = CCSprite::createWithSpriteFrameName("DP_Crystal_003.png"_spr);

        frontCrystal->setColor(color);
        midCrystal->setColor(color);
        backCrystal->setColor(color);

        frontCrystal->setOpacity(255 * opacity);
        midCrystal->setOpacity((255 * 0.66f) * opacity);
        backCrystal->setOpacity((255 * 0.33f) * opacity);

        scaleToContentSize(frontCrystal, {100.f, m_label->getContentHeight() - 5.f});
        scaleToContentSize(midCrystal, {100.f, m_label->getContentHeight() - 5.f});
        scaleToContentSize(backCrystal, {100.f, m_label->getContentHeight() - 5.f});

        frontCrystal->setAnchorPoint({0.f, 0.f});
        midCrystal->setAnchorPoint({0.f, 0.f});
        backCrystal->setAnchorPoint({0.f, 0.f});

        frontCrystal->setPositionX(100.f * i);
        midCrystal->setPositionX(100.f * i);
        backCrystal->setPositionX(100.f * i);

        if (i % 2 == 1) {
            frontCrystal->setFlipX(true);
            midCrystal->setFlipX(true);
            backCrystal->setFlipX(true);

            frontCrystal->setFlipY(true);
            midCrystal->setFlipY(true);
            backCrystal->setFlipY(true);
        }

        crystalParent->addChild(frontCrystal);
        crystalParent->addChild(midCrystal);
        crystalParent->addChild(backCrystal);
    }
    
    m_effects->addChild(crystalParent);
    
    return;
}

void CustomText::addSpiral(ccColor3B color, float opacity) {
    return;
}

void CustomText::addSpecial(std::string id) {
    if (id == "invisible") {
        m_label->setVisible(false);
        m_outline->setOpacity(255 / 2);
    }
    else if (id == "rgb") {
        auto red = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
        auto green = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
        auto blue = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");

        red->setColor({255, 0, 0});
        green->setColor({0, 255, 0});
        blue->setColor({0, 0, 255});

        scaleToContentSize(red, {26.5f, m_label->getContentHeight()});
        scaleToContentSize(green, {26.5f, m_label->getContentHeight()});
        scaleToContentSize(blue, {26.5f, m_label->getContentHeight()});

        red->setAnchorPoint({0.f, 0.f});
        green->setAnchorPoint({0.f, 0.f});
        blue->setAnchorPoint({0.f, 0.f});

        green->setPositionX(22.5f);
        blue->setPositionX(45.f);

        m_effects->addChild(red);
        m_effects->addChild(green);
        m_effects->addChild(blue);
    }
    else if (id == "sonic-wave") {
        auto mid = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
        auto last = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");

        mid->setColor({ 0, 99, 192 });
        last->setColor({ 0, 132, 255 });

        scaleToContentSize(mid, {m_label->getContentWidth() / 3.f, m_label->getContentHeight()});
        scaleToContentSize(last, {m_label->getContentWidth() / 3.f, m_label->getContentHeight()});

        mid->setAnchorPoint({0.f, 0.f});
        last->setAnchorPoint({0.f, 0.f});

        mid->setPositionX(m_label->getContentWidth() * 0.33f);
        last->setPositionX(m_label->getContentWidth() * 0.66f);

        mid->setSkewX(10.f);
        last->setSkewX(10.f);

        m_effects->addChild(mid);
        m_effects->addChild(last);
    }
    else if (id == "nine-circles") {
        auto first = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
        auto mid = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
        auto last = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");

        first->setColor({ 200, 0, 0 });
        mid->setColor({ 200, 80, 0 });
        last->setColor({ 200, 200, 0 });

        scaleToContentSize(first, {m_label->getContentWidth() / 4.f, m_label->getContentHeight()});
        scaleToContentSize(mid, {m_label->getContentWidth() / 4.f, m_label->getContentHeight()});
        scaleToContentSize(last, {m_label->getContentWidth() / 4.f, m_label->getContentHeight()});

        first->setAnchorPoint({0.f, 0.f});
        mid->setAnchorPoint({0.f, 0.f});
        last->setAnchorPoint({0.f, 0.f});

        first->setPositionX(m_label->getContentWidth() * 0.25f);
        mid->setPositionX(m_label->getContentWidth() * 0.5f);
        last->setPositionX(m_label->getContentWidth() * 0.75f);

        first->setSkewX(10.f);
        mid->setSkewX(10.f);
        last->setSkewX(10.f);

        m_effects->addChild(first);
        m_effects->addChild(mid);
        m_effects->addChild(last);
    }
    else if (id == "2p-solo") {
        setTextColor(GameManager::get()->sharedState()->colorForIdx(GameManager::get()->sharedState()->getPlayerColor()));
        addGradient(GameManager::get()->sharedState()->colorForIdx(GameManager::get()->sharedState()->getPlayerColor2()), 1.f, true);
    }
    else if (id == "kria") {
        addGradient(ccColor3B{ 0, 140, 255 }, 1.f, true);
        addGradient(ccColor3B{ 255, 0, 255 }, 1.f, true);

        if (!Mod::get()->getSettingValue<bool>("disable-text-gradient")) {
            typeinfo_cast<CCSprite*>(m_effects->getChildren()->objectAtIndex(0))->setFlipY(true);
        }
    }
    else {
        log::info("Could not apply special effect \"{}\". (ID Not Found)", id);
    }
    
    return;
}

void CustomText::setTextColor(ccColor3B color) {
    m_label->setColor(color);
    return;
}

void CustomText::setOutlineColor(ccColor3B color) {
    m_outline->setColor(color);
    return;
}

void CustomText::setDropShadowColor(ccColor3B color) {
    m_dropshadow->setColor(color);
    return;
}

void CustomText::scaleToContentSize(CCNode* node, CCSize size) {
    node->setScale(0.f);

    while (node->getScaledContentWidth() < size.width) {
        node->setScaleX(node->getScaleX() + 0.01f);
    }
    
    while (node->getScaledContentHeight() < size.height) {
        node->setScaleY(node->getScaleY() + 0.01f);
    }

    return;
}

void CustomText::addEffectsFromProperties(matjson::Value properties) {
    bool reducedEffects = (Mod::get()->getSettingValue<bool>("reduce-demon-label-effects") && this->getScale() <= 0.35f);
    
    m_label->setColor(properties["textColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}));
    if (reducedEffects) { 
        m_outline->setColor(ccColor3B{ 135, 135, 135 });
    }
    else {
        m_outline->setColor(properties["outlineColor"].as<ccColor3B>().unwrapOr(ccColor3B{0, 0, 0}));
    }

    m_dropshadow->setColor(properties["shadowColor"].as<ccColor3B>().unwrapOr(ccColor3B{0, 0, 0}));

    bool gradient = properties["gradient"].as<bool>().unwrapOr(false);
    bool crystal = properties["crystal"].as<bool>().unwrapOr(false);
    bool stars = properties["stars"].as<bool>().unwrapOr(false);
    bool bevel = properties["bevel"].as<bool>().unwrapOr(false);

    if (gradient) {
        auto gradientType = properties["gradientType"].as<std::string>().unwrapOr("none");
        
        if (gradientType == "radial") {
            addRadialGradient(
                properties["gradientColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}),
                properties["gradientOpacity"].as<float>().unwrapOr(1.0f)
            );
        }
        else if (gradientType == "horizontal") {
            addGradient(
                properties["gradientColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}),
                properties["gradientOpacity"].as<float>().unwrapOr(1.0f),
                true
            );
        }
        else {
            addGradient(
                properties["gradientColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}),
                properties["gradientOpacity"].as<float>().unwrapOr(1.0f),
                false
            );
        }
    }

    if (crystal && !reducedEffects) {
        addCrystals(
            properties["crystalColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}),
            properties["crystalOpacity"].as<float>().unwrapOr(1.0f)
        );
    }

    if (stars && !reducedEffects) {
        addStars(
            properties["starsColor"].as<ccColor3B>().unwrapOr(ccColor3B{255, 255, 255}),
            properties["starsOpacity"].as<float>().unwrapOr(1.0f)
        );
    }

    if (bevel && !reducedEffects) {
        addBevel();
    }

    if (properties.contains("special")) {
        addSpecial(properties["special"].as<std::string>().unwrapOrDefault());
    }

    return;
}

CustomText* CustomText::create(std::string text) {
	auto pRet = new CustomText();
	if (pRet && pRet->init(text)) {
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet); //don't crash if it fails
	return nullptr;
}

CustomText::~CustomText() {
	this->removeAllChildrenWithCleanup(true);
}