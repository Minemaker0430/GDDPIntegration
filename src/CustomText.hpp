#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class CustomText : public CCNode {
public:
	//components
    CCLabelBMFont* m_label;
    CCLabelBMFont* m_outline;
    CCLabelBMFont* m_dropshadow;
    CCClippingNode* m_effects = CCClippingNode::create();

    static CustomText* create(std::string text);

    void setText(std::string text);
    std::string getText();

    void setTextColor(ccColor3B color);
    void setOutlineColor(ccColor3B color);
    void setDropShadowColor(ccColor3B color);

    void clearEffects();

    void addBevel();
    void addGradient(ccColor3B color, float opacity = 1.0, bool rotated = false);
    void addRadialGradient(ccColor3B color, float opacity = 1.0);
    void addStars(ccColor3B color, float opacity = 1.0);
    void addCrystals(ccColor3B color, float opacity = 1.0);
    void addSpiral(ccColor3B color, float opacity = 1.0);
    void addSpecial(std::string id);

    void addEffectsFromProperties(matjson::Value properties);
    void scaleToContentSize(CCNode* node, CCSize size);

protected:
    virtual bool init(std::string text);
    virtual ~CustomText();
};

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

gradientType (String) = "none", "horizontal", or "radial". Determines gradient style

gradient (Bool) = whether or not the text should have a gradient
crystal (Bool) = whether or not the text should have the crystal effect
stars (Bool) = whether or not the text should have the stars effect
bevel (Bool) = whether or not the text should have bevel

special (String) - custom effects

============================
*/

// Yes, all of these are hardcoded, they pretty much have to be to avoid any issues.
const matjson::Value DPTextEffects = matjson::parse(R"(
    {
        "beginner": {
            "textColor": [0, 134, 168],
            "gradientColor": [0, 255, 228],
            "gradient": true
        },
        "bronze": {
            "textColor": [121, 66, 0],
            "gradientColor": [255, 81, 0],
            "gradient": true,
            "bevel": true
        },
        "silver": {
            "textColor": [189, 189, 189],
            "gradientColor": [78, 78, 78],
            "gradient": true,
            "bevel": true
        },
        "gold": {
            "textColor": [255, 255, 0],
            "gradientColor": [255, 196, 0],
            "gradient": true,
            "bevel": true
        },
        "amber": {
            "textColor": [255, 123, 0],
            "gradientColor": [255, 212, 0],
            "gradient": true,
            "gradientType": "radial",
            "bevel": true
        },
        "platinum": {
            "textColor": [100, 64, 168],
            "gradientColor": [202, 156, 255],
            "gradient": true,
            "bevel": true
        },
        "sapphire": {
            "textColor": [0, 0, 255],
            "gradientColor": [0, 122, 255],
            "crystalColor": [110, 170, 255],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "jade": {
            "textColor": [0, 130, 100],
            "gradientColor": [0, 255, 200],
            "crystalColor": [180, 255, 200],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "emerald": {
            "textColor": [0, 150, 0],
            "gradientColor": [0, 255, 0],
            "crystalColor": [180, 255, 180],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "ruby": {
            "textColor": [150, 0, 0],
            "gradientColor": [255, 0, 0],
            "crystalColor": [255, 180, 180],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "diamond": {
            "textColor": [0, 50, 75],
            "gradientColor": [0, 255, 255],
            "crystalColor": [155, 255, 255],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "pearl": {
            "textColor": [255, 155, 255],
            "outlineColor": [255, 255, 255],
            "gradient": true,
            "crystal": true
        },
        "onyx": {
            "textColor": [50, 50, 50],
            "outlineColor": [255, 255, 255],
            "gradientColor": [150, 150, 150],
            "crystalColor": [255, 255, 230],
            "crystalOpacity": 0.5,
            "gradient": true,
            "crystal": true
        },
        "amethyst": {
            "textColor": [135, 0, 255],
            "outlineColor": [255, 255, 255],
            "gradientColor": [135, 255, 255],
            "crystalOpacity": 0.5,
            "starsOpacity": 0.5,
            "gradientType": "horizontal",
            "gradient": true,
            "crystal": true,
            "stars": true
        },
        "azurite": {
            "textColor": [0, 16, 92],
            "outlineColor": [255, 255, 255],
            "gradientColor": [0, 0, 0],
            "crystalColor": [0, 69, 160],
            "starsColor": [0, 0, 0],
            "starsOpacity": 1.0,
            "gradient": true,
            "crystal": true,
            "stars": true
        },
        "obsidian": {
            "textColor": [0, 0, 0],
            "outlineColor": [255, 255, 255],
            "gradientColor": [85, 85, 85],
            "starsColor": [0, 0, 0],
            "crystalOpacity": 0.25,
            "starsOpacity": 1.0,
            "gradient": true,
            "crystal": true,
            "stars": true
        },
        "legacy-beginner": {
            "textColor": [0, 134, 168],
            "gradientColor": [0, 255, 228],
            "gradient": true
        },
        "legacy-bronze": {
            "textColor": [121, 66, 0],
            "gradientColor": [255, 81, 0],
            "gradient": true,
            "bevel": true
        },
        "legacy-silver": {
            "textColor": [189, 189, 189],
            "gradientColor": [78, 78, 78],
            "gradient": true,
            "bevel": true
        },
        "legacy-gold": {
            "textColor": [255, 255, 0],
            "gradientColor": [255, 196, 0],
            "gradient": true,
            "bevel": true
        },
        "legacy-amber": {
            "textColor": [255, 123, 0],
            "gradientColor": [255, 212, 0],
            "gradient": true,
            "gradientType": "radial",
            "bevel": true
        },
        "legacy-platinum": {
            "textColor": [100, 64, 168],
            "gradientColor": [202, 156, 255],
            "gradient": true,
            "bevel": true
        },
        "legacy-sapphire": {
            "textColor": [0, 0, 255],
            "gradientColor": [0, 122, 255],
            "crystalColor": [110, 170, 255],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "legacy-jade": {
            "textColor": [0, 130, 100],
            "gradientColor": [0, 255, 200],
            "crystalColor": [180, 255, 200],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "legacy-emerald": {
            "textColor": [0, 150, 0],
            "gradientColor": [0, 255, 0],
            "crystalColor": [180, 255, 180],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "legacy-ruby": {
            "textColor": [150, 0, 0],
            "gradientColor": [255, 0, 0],
            "crystalColor": [255, 180, 180],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "legacy-diamond": {
            "textColor": [0, 50, 75],
            "gradientColor": [0, 255, 255],
            "crystalColor": [155, 255, 255],
            "gradient": true,
            "bevel": true,
            "crystal": true
        },
        "legacy-pearl": {
            "textColor": [255, 155, 255],
            "outlineColor": [255, 255, 255],
            "gradient": true,
            "crystal": true
        },
        "legacy-onyx": {
            "textColor": [50, 50, 50],
            "outlineColor": [255, 255, 255],
            "gradientColor": [150, 150, 150],
            "crystalColor": [255, 255, 230],
            "crystalOpacity": 0.5,
            "gradient": true,
            "crystal": true
        },
        "legacy-amethyst": {
            "textColor": [135, 0, 255],
            "outlineColor": [255, 255, 255],
            "gradientColor": [135, 255, 255],
            "crystalOpacity": 0.5,
            "starsOpacity": 0.5,
            "gradientType": "horizontal",
            "gradient": true,
            "crystal": true,
            "stars": true
        },
        "legacy-azurite": {
            "textColor": [0, 16, 92],
            "outlineColor": [255, 255, 255],
            "gradientColor": [0, 0, 0],
            "crystalColor": [0, 69, 160],
            "starsColor": [0, 0, 0],
            "starsOpacity": 1.0,
            "gradient": true,
            "crystal": true,
            "stars": true
        },
        "legacy-obsidian": {
            "textColor": [0, 0, 0],
            "outlineColor": [255, 255, 255],
            "gradientColor": [85, 85, 85],
            "starsColor": [0, 0, 0],
            "crystalOpacity": 0.25,
            "starsOpacity": 1.0,
            "gradient": true,
            "crystal": true,
            "stars": true
        },
        "12doc-2025": {
            "textColor": [255, 0, 0],
            "gradientColor": [0, 255, 0],
            "gradient": true
        },
        "12doc-2024": {
            "textColor": [255, 0, 0],
            "gradientColor": [0, 255, 0],
            "gradient": true
        },
        "12doc-2023": {
            "textColor": [255, 0, 0],
            "gradientColor": [0, 255, 0],
            "gradient": true
        },
        "12doc-2022": {
            "textColor": [255, 0, 0],
            "gradientColor": [0, 255, 0],
            "gradient": true
        },
        "gd10": {
            "textColor": [255, 255, 0],
            "gradientColor": [0, 255, 255],
            "gradient": true
        },
        "gddl-1": {
            "textColor": [255, 230, 0],
            "gradient": true
        },
        "gddl-2": {
            "textColor": [255, 230, 0],
            "gradient": true
        },
        "xxl": {
            "textColor": [0, 135, 200],
            "gradient": true
        },
        "rgb": {
            "special": "rgb",
            "bevel": true
        },
        "sonic-wave": {
            "special": "sonic-wave",
            "textColor": [0, 52, 100],
            "bevel": true
        },
        "invisible": {
            "outlineColor": [255, 255, 255],
            "gradient": true,
            "special": "invisible"
        },
        "1pt9-gdps": {
            "textColor": [0, 0, 0],
            "outlineColor": [255, 255, 255]
        },
        "temple-series": {
            "textColor": [0, 0, 0],
            "outlineColor": [255, 255, 255],
            "starsOpacity": 0.5,
            "stars": true
        },
        "apoc-trilogy": {
            "textColor": [0, 0, 0],
            "outlineColor": [255, 0, 0],
            "gradientColor": [255, 0, 0],
            "starsColor": [255, 0, 0],
            "stars": true,
            "gradient": true
        },
        "plasma": {
            "textColor": [0, 0, 0],
            "outlineColor": [255, 0, 255],
            "gradientColor": [0, 255, 255],
            "gradient": true
        },
        "sonic": {
            "textColor": [0, 135, 200],
            "gradientColor": [255, 0, 0],
            "gradient": true
        },
        "lightwave": {
            "outlineColor": [100, 135, 200],
            "gradient": true,
            "gradientType": "horizontal",
            "special": "invisible"
        },
        "coder": {
            "textColor": [0, 0, 0],
            "outlineColor": [0, 255, 0]
        },
        "aeonair": {
            "textColor": [255, 85, 0],
            "gradientColor": [0, 255, 255],
            "gradient": true
        },
        "trusta": {
            "textColor": [255, 0, 255],
            "gradientColor": [0, 255, 255],
            "gradient": true
        },
        "minigame": {
            "textColor": [50, 50, 50],
            "outlineColor": [255, 50, 50]
        },
        "nine-circles": {
            "textColor": [50, 50, 50],
            "special": "nine-circles"
        },
        "2p-solo": {
            "special": "2p-solo"
        },
        "speedrun": {
            "textColor": [0, 255, 0],
            "gradientType": "horizontal",
            "gradient": true
        },
        "kria": {
            "special": "kria"
        },
        "memory-novice": {
            "textColor": [100, 255, 100],
            "gradientColor": [100, 255, 255],
            "gradient": true
        },
        "memory-expert": {
            "textColor": [135, 0, 255],
            "gradientColor": [75, 0, 200],
            "gradient": true
        },
        "memory-master": {
            "textColor": [0, 0, 0],
            "outlineColor": [255, 255, 255],
            "crystal": true
        }
    }
)").unwrapOrDefault();