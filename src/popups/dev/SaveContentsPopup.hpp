#pragma once
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/JsonValidation.hpp>

//geode namespace
using namespace geode::prelude;

class SaveContentsPopup : public Popup<> {
    protected:
        bool setup() override;
        void onClose(CCObject*) override;
        virtual ~SaveContentsPopup();
    
        CCLayer* m_mainLayer;
        LoadingCircle* m_loadcircle;
        CCLabelBMFont* m_loadText;
        ListView* m_list;
        
        matjson::Value m_responseValues;

        // constants
        const std::string MAIN_LIST = "https://api.github.com/repos/Minemaker0430/gddp-mod-database/contents/main-list.json";
        const std::string DEV_LIST = "https://api.github.com/repos/Minemaker0430/gddp-mod-dev-data/contents/list.json";
        const std::string SKILLSET_LIST = "https://api.github.com/repos/Minemaker0430/gddp-mod-database/contents/skill-badges.json";
        const std::string DEV_SKILLSET_LIST = "https://api.github.com/repos/Minemaker0430/gddp-mod-dev-data/contents/skillsets.json";

        EventListener<web::WebTask> m_listener;
        EventListener<web::WebTask> m_listener2;
        EventListener<web::WebTask> m_listener3;
        EventListener<web::WebTask> m_listener4;
    
        void parseResponse(std::string res);
        void setContents();
        void compareChanges();

        void onConfirm(CCObject*);
    public:
        matjson::Value m_dataOld;
        matjson::Value m_dataNew;
        std::string m_dataSha;
        matjson::Value m_skillsetsOld;
        matjson::Value m_skillsetsNew;
        std::string m_skillsetsSha;

        std::string m_accessCode;

        int m_uploadType;

        void getContents();

        static SaveContentsPopup* create();
    };

    enum class UploadType {
        Dev,
        Main,
        Revert
    };