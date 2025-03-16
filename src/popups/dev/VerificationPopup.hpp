#pragma once
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/JsonValidation.hpp>

//geode namespace
using namespace geode::prelude;

class VerificationPopup : public Popup<> {
    protected:
        bool setup() override;
        void onClose(CCObject*) override;
        virtual ~VerificationPopup();
    
        CCLayer* m_mainLayer;
        LoadingCircle* m_loadcircle;
        CCLabelBMFont* m_loadText;
        CCMenu* m_tabs;
        ListView* m_list;
        ListView* m_levelList; //only used in pack settings

        int m_currentTab = 0;
        
        std::string m_deviceCode;
        std::string m_verificationCode = "XXXX-XXXX";
        matjson::Value m_responseValues;

        std::filesystem::path m_accessCodePath;
        std::string m_accessCode;

        matjson::Value m_dataMain;
        matjson::Value m_dataTemp;
        std::string m_dataMainSha;
        std::string m_dataDevSha;
        matjson::Value m_skillsetsMain;
        matjson::Value m_skillsetsDev;
        matjson::Value m_skillsetsTemp;
        std::string m_skillsetsMainSha;
        std::string m_skillsetsDevSha;

        EventListener<web::WebTask> m_listener;
        EventListener<web::WebTask> m_listener2;
        EventListener<web::WebTask> m_listener3;
        EventListener<web::WebTask> m_listener4;
    
        void parseResponse(std::string res);

        //Auth Stuff
        void reqCode();
        void poll(float dt);

        void getContents();

        void onToggleTab(CCObject*);
        void onBack(CCObject*);
        void onNew(CCObject*);
        void onEdit(CCObject*);
        void onSave(CCObject*);
        void onMove(CCObject*);
        void onDelete(CCObject*);
        void onChangeLevelSkill(CCObject*);
        void onPushChanges(CCObject*);
        void onDocs(CCObject*);
    public:
        std::string m_clientID;

        matjson::Value m_dataDev;

        std::string m_index;
        int m_packID;
        int m_levelID = -1;

        matjson::Value m_currentData; //Use for packs and skillsets
        matjson::Value m_levelData; //ONLY used in packs as there's no need for levels to be used with skillsets

        void removeObject(std::string type, std::string id, int pos);
        void refreshLevelSkillsets();

        //GUI
        void loadMain(int tab);
        void loadPack(std::string index, int id, bool fromLvl = false);
        void loadSkill(std::string key);
        void loadLevel(int id);

        static VerificationPopup* create();
    };

//===================
//  AddLevelPopup
//===================

class AddLevelPopup : public Popup<> {
    protected:
        bool setup() override;
        virtual ~AddLevelPopup();

        TextInput* m_value;

        void onAddLevel(CCObject*);
        void onPaste(CCObject*);
    public:
        static AddLevelPopup* create();
    };

//===================
//  EditDescriptionPopup
//===================

class EditDescriptionPopup : public Popup<> {
    protected:
        bool setup() override;
        virtual ~EditDescriptionPopup();

        TextInput* m_value;
        std::string m_description;

        void onConfirm(CCObject*);
        void onPaste(CCObject*);
    public:
        static EditDescriptionPopup* create(std::string description);
    };

//===================
//  SkillsetSelectionPopup
//===================

class SkillsetSelectionPopup : public Popup<> {
    protected:
        bool setup() override;
        virtual ~SkillsetSelectionPopup();

        ListView* m_list;
        int m_index = 0;
        matjson::Value m_data;
        bool m_isNew;

        void onSelection(CCObject*);
        void onRemove(CCObject*);
    public:
        static SkillsetSelectionPopup* create(int placeAt, matjson::Value data, bool isNew = false);
    };

//===================
//  NewPackPopup
//===================

class NewPackPopup : public Popup<> {
    protected:
        bool setup() override;
        virtual ~NewPackPopup();

        void onConfirm(CCObject*);
    public:
        static NewPackPopup* create();
    };

//===================
//  MovePopup
//===================

class MovePopup : public Popup<> {
    protected:
        bool setup() override;
        virtual ~MovePopup();

        std::string m_type;
        std::string m_ID;
        int m_pos = 0;

        int m_newPos = 0;

        TextInput* m_value;

        void onUp(CCObject*);
        void onDown(CCObject*);
        void onTop(CCObject*);
        void onBottom(CCObject*);
        void onConfirm(CCObject*);
    public:
        static MovePopup* create(std::string type, std::string id, int pos);
    };

//===================
//  Main Pack Format
//===================

    struct GDDPMainPackFormat
    {
        std::string name = "Pack Name";
        std::string description = "Pack Description";
        std::string sprite = "DP_Unknown";
        std::string plusSprite = "DP_Unknown";
        std::string saveID = "null";
        std::vector<int> levelIDs = {0};
        int reqLevels = 999;
    };

    template <>
    struct matjson::Serialize<GDDPMainPackFormat>
    {
        static Result<GDDPMainPackFormat> fromJson(matjson::Value const &value)
        {
            auto f = GDDPMainPackFormat{
                .name = value["name"].as<std::string>().unwrapOr("Name"),
                .description = value["description"].as<std::string>().unwrapOr("Description"),
                .sprite = value["sprite"].as<std::string>().unwrapOr("DP_Unknown"),
                .plusSprite = value["plusSprite"].as<std::string>().unwrapOr("DP_Unknown"),
                .saveID = value["saveID"].as<std::string>().unwrapOr("null"),
                .levelIDs = value["levelIDs"].as<std::vector<int>>().unwrapOr(std::vector<int>(1, 0)),
                .reqLevels = value["reqLevels"].as<int>().unwrapOr(999)};

            return Ok(f);
        }

        static matjson::Value toJson(GDDPMainPackFormat const &value)
        {
            auto obj = matjson::makeObject({{"name", value.name},
                                            {"description", value.description},
                                            {"sprite", value.sprite},
                                            {"plusSprite", value.plusSprite},
                                            {"saveID", value.saveID},
                                            {"levelIDs", value.levelIDs},
                                            {"reqLevels", value.reqLevels}});
            return obj;
        }
    };

//===================
//  Legacy Pack Format
//===================

struct GDDPLegacyPackFormat
{
    std::string name = "Pack Name";
    std::string description = "Levels that used to be in the [TIER] tier.";
    std::string sprite = "DP_Unknown";
    std::string plusSprite = "DP_Unknown";
    std::string saveID = "null";
    std::vector<int> levelIDs = {0};
    int mainPack = 0;
};

template <>
struct matjson::Serialize<GDDPLegacyPackFormat>
{
    static Result<GDDPLegacyPackFormat> fromJson(matjson::Value const &value)
    {
        auto f = GDDPLegacyPackFormat{
            .name = value["name"].as<std::string>().unwrapOr("Name"),
            .description = value["description"].as<std::string>().unwrapOr("Description"),
            .sprite = value["sprite"].as<std::string>().unwrapOr("DP_Unknown"),
            .plusSprite = value["plusSprite"].as<std::string>().unwrapOr("DP_Unknown"),
            .saveID = value["saveID"].as<std::string>().unwrapOr("null"),
            .levelIDs = value["levelIDs"].as<std::vector<int>>().unwrapOr(std::vector<int>(1, 0)),
            .mainPack = value["mainPack"].as<int>().unwrapOr(0)};

        return Ok(f);
    }

    static matjson::Value toJson(GDDPLegacyPackFormat const &value)
    {
        auto obj = matjson::makeObject({{"name", value.name},
                                        {"description", value.description},
                                        {"sprite", value.sprite},
                                        {"plusSprite", value.plusSprite},
                                        {"saveID", value.saveID},
                                        {"levelIDs", value.levelIDs},
                                        {"mainPack", value.mainPack}});
        return obj;
    }
};

//===================
//  Bonus Pack Format
//===================

struct GDDPBonusPackFormat
{
    std::string name = "Pack Name";
    std::string description = "Pack Description";
    std::string sprite = "DP_Unknown";
    std::string saveID = "null";
    std::vector<int> levelIDs = {0};
};

template <>
struct matjson::Serialize<GDDPBonusPackFormat>
{
    static Result<GDDPBonusPackFormat> fromJson(matjson::Value const &value)
    {
        auto f = GDDPBonusPackFormat{
            .name = value["name"].as<std::string>().unwrapOr("Name"),
            .description = value["description"].as<std::string>().unwrapOr("Description"),
            .sprite = value["sprite"].as<std::string>().unwrapOr("DP_Unknown"),
            .saveID = value["saveID"].as<std::string>().unwrapOr("null"),
            .levelIDs = value["levelIDs"].as<std::vector<int>>().unwrapOr(std::vector<int>(1, 0))};

        return Ok(f);
    }

    static matjson::Value toJson(GDDPBonusPackFormat const &value)
    {
        auto obj = matjson::makeObject({{"name", value.name},
                                        {"description", value.description},
                                        {"sprite", value.sprite},
                                        {"saveID", value.saveID},
                                        {"levelIDs", value.levelIDs}});
        return obj;
    }
};

//===================
//  Monthly Pack Format
//===================

struct GDDPMonthlyPackFormat
{
    std::string name = "Pack Name";
    std::string description = "Monthly Pack Description (GDDP Monthly Pack for [MONTH] [YEAR])";
    std::string sprite = "DP_Unknown";
    int month = 1;
    int year = 1987;
    std::vector<int> levelIDs = {0};
};

template <>
struct matjson::Serialize<GDDPMonthlyPackFormat>
{
    static Result<GDDPMonthlyPackFormat> fromJson(matjson::Value const &value)
    {
        auto f = GDDPMonthlyPackFormat{
            .name = value["name"].as<std::string>().unwrapOr("Name"),
            .description = value["description"].as<std::string>().unwrapOr("Description"),
            .sprite = value["sprite"].as<std::string>().unwrapOr("DP_Unknown"),
            .month = value["month"].as<int>().unwrapOr(11),
            .year = value["year"].as<int>().unwrapOr(1987),
            .levelIDs = value["levelIDs"].as<std::vector<int>>().unwrapOr(std::vector<int>(1, 0))};

        return Ok(f);
    }

    static matjson::Value toJson(GDDPMonthlyPackFormat const &value)
    {
        auto obj = matjson::makeObject({{"name", value.name},
                                        {"description", value.description},
                                        {"sprite", value.sprite},
                                        {"month", value.month},
                                        {"year", value.year},
                                        {"levelIDs", value.levelIDs}});
        return obj;
    }
};

//===================
//  Level Format
//===================

struct GDDPLevelFormat
{
    std::string name = "Level Name";
    int difficulty = 0;
    std::vector<std::string> skillsets;
    matjson::Value xp; //Will always initialize blank since not all levels will have XP.
};

template <>
struct matjson::Serialize<GDDPLevelFormat>
{
    static Result<GDDPLevelFormat> fromJson(matjson::Value const &value)
    {
        auto f = GDDPLevelFormat{
            .name = value["name"].as<std::string>().unwrapOr("Name"),
            .difficulty = value["difficulty"].as<int>().unwrapOr(0),
            .skillsets = value["skillsets"].as<std::vector<std::string>>().unwrapOrDefault(),
            .xp = value["xp"].as<matjson::Value>().unwrapOrDefault()};
        return Ok(f);
    }

    static matjson::Value toJson(GDDPLevelFormat const &value)
    {
        auto obj = matjson::makeObject({{"name", value.name},
                                        {"difficulty", value.difficulty},
                                        {"skillsets", value.skillsets},
                                        {"xp", value.xp}});
        return obj;
    }
};

//===================
//  Skillset Format
//===================

struct GDDPSkillsetFormat
{
    std::string displayName = "New Skillset";
    std::string type = "none";
    std::string description = "Skillset Description";
    std::string sprite = "DP_Skill_Unknown";
};

template <>
struct matjson::Serialize<GDDPSkillsetFormat>
{
    static Result<GDDPSkillsetFormat> fromJson(matjson::Value const &value)
    {
        auto f = GDDPSkillsetFormat{
            .displayName = value["display-name"].as<std::string>().unwrapOr("Unknown"),
            .type = value["type"].as<std::string>().unwrapOr("none"),
            .description = value["description"].as<std::string>().unwrapOr("This skill does not have a description."),
            .sprite = value["sprite"].as<std::string>().unwrapOr("DP_Skill_Unknown")};
        return Ok(f);
    }

    static matjson::Value toJson(GDDPSkillsetFormat const &value)
    {
        auto obj = matjson::makeObject({{"display-name", value.displayName},
                                        {"type", value.type},
                                        {"description", value.description},
                                        {"sprite", value.sprite}});
        return obj;
    }
};