#pragma once
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include "../../CustomText.hpp"

//geode namespace
using namespace geode::prelude;

enum class ValueType {
    Color,
    Float,
    Bool,
    String,
    Enum
};

enum class GradientType {
    Default,
    Horizontal,
    Radial
};

class TextEffectEditorPopup : public Popup {
protected:
    bool init() override;
    void onValueChanged(CCObject*);
    void onSave(CCObject*);

    matjson::Value m_data;
    CustomText* m_sampleText;
    GradientType m_gradientType;

    geode::Function<void(matjson::Value const&)> m_callback;

    virtual ~TextEffectEditorPopup();
public:
    void setCallback(geode::Function<void(matjson::Value const&)>);

    static TextEffectEditorPopup* create(matjson::Value);
};

struct SetValueParameters : public CCObject {
    ValueType m_type;
    std::string m_key;

    SetValueParameters(ValueType type, std::string key) : m_type(type), m_key(key) {
        // Always remember to call autorelease on your classes!
        this->autorelease();
    }
};