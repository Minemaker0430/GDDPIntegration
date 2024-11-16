/*
    Created by wint0r
    Modified by Me
*/

#include "ListManager.hpp"
#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

const std::string GDDL_API_URL = "https://docs.google.com/spreadsheets/d/1qKlWKpDkOpU1ZF6V6xGfutDY2NvcA8MNPnsv6GBkKPQ/gviz/tq?tqx=out:csv&sheet=GDDL"; //"https://gdladder.com/api/theList";

bool ListManager::fetchedGDDLRatings;
std::vector<ListRating> ListManager::ratings;

std::string getUserAgent() {
    return fmt::format("{}/{}; GeometryDash/{} (GeodeSDK/{}); {}",
        Mod::get()->getID(),
        Mod::get()->getVersion().toVString(true),
        GEODE_STR(GEODE_GD_VERSION),
        Loader::get()->getVersion().toVString(true),
        GEODE_PLATFORM_NAME
    );
}

void ListManager::init() {
    GDDLListener::create();

    return;
}

void ListManager::parseResponse(std::string val) {

    //Thank you hiimjustin :pray:

    // MAKE SURE WE ARE NOT SPLITTING THE COMMAS IN THE QUOTATION MARKS
    auto lines = string::split(val, "\n");
    auto keys = string::split(lines[0].substr(1, lines[0].size() - 2), "\",\"");
    //static auto demons = matjson::Array();
    for (size_t i = 1; i < lines.size(); i++) {
        auto values = string::split(lines[i].substr(1, lines[i].size() - 2), "\",\"");
        ListRating demon;
        for (size_t j = 0; j < keys.size(); j++) {
            auto key = keys[j];
            auto value = values[j];
            if (key == "ID")
            {
                demon.id = std::stoi(value);
            }
            else if (key == "Name") {
                demon.name = value;
            }
            else if (key == "Tier")
            {
                demon.tier = value != "" ? round(std::stod(value) * 100) / 100 : 0;
            }
        }
        ListManager::ratings.push_back(demon);
    }

    return;

    /*auto levels = val.as_array();
    for (auto level : levels) {
        ListRating rating;

        std::string name = level["Name"].is_string() ? level["Name"].as_string() : "?";
    
        rating.id = level["ID"].is_number() ? level["ID"].as_int() : -1;
        rating.name = name;
        rating.tier = level["Rating"].is_number() ? level["Rating"].as_int() : 0;

        ListManager::ratings.push_back(rating);
    }*/
}

int levenshteinDistance(std::string a, std::string b) {
    int aLen = a.length();
    int bLen = b.length();

    if (aLen == 0)
        return bLen;
    else if (bLen == 0)
        return aLen;

    std::vector<std::vector<int>> matrix;
    matrix.resize(aLen + 1);
    for (int i = 0; i <= aLen; i++) {
        matrix[i].resize(bLen + 1);
        matrix[i][0] = i;
    }
    for (int i = 0; i <= bLen; i++) {
        matrix[0][i] = i;
    }

    for (int i = 1; i <= aLen; i++) {
        for (int j = 1; j <= bLen; j++) {
            int cost = a[i - 1] == b[j - 1] ? 0 : 1;
            matrix[i][j] = std::min({
                matrix[i - 1][j] + 1,
                matrix[i][j - 1] + 1,
                matrix[i - 1][j - 1] + cost
                });
        }
    }

    return matrix[aLen][bLen];
}

std::optional<ListRating> ListManager::getRating(int levelID) {
    for (auto rating : ListManager::ratings) {
        if (levelID == rating.id && rating.tier != 0) {
            return rating;
        }
    }

    return std::nullopt;
}

std::string ListManager::getSpriteName(GJGameLevel* level) {
    auto ratingOpt = ListManager::getRating(level->m_levelID.value());

    if (level->isPlatformer()) {
        return "";
    }

    if (ratingOpt.has_value()) {
        auto rating = ratingOpt.value();
        if (level->m_demonDifficulty == 5) { //Insane
            if (rating.tier > 20) {
                return "DP_Amber";
            }
            else if (GDDL_TO_GDDP.contains(round(rating.tier))) {
                return fmt::format("DP_{}", GDDL_TO_GDDP.at(round(rating.tier)));
            }
            else {
                return "";
            }
        }
        else if (level->m_demonDifficulty == 6) { //Extreme
            if (rating.tier < 21) {
                return "DP_Platinum";
            }
            if (GDDL_TO_GDDP.contains(round(rating.tier))) {
                return fmt::format("DP_{}", GDDL_TO_GDDP.at(round(rating.tier)));
            }
            else {
                return "";
            }
        }
        else if (level->m_demonDifficulty == 3) {
            return "DP_Beginner";
        }
        else if (level->m_demonDifficulty == 4) {
            return "DP_Bronze";
        }
        else if (level->m_stars == 10 && level->m_demonDifficulty == 0) {
            return "DP_Silver";
        }
        else {
            return "";
        }
    }
    else {
        return "";
    }

    return "";
}

GDDLListener* GDDLListener::create() {
    auto pRet = new GDDLListener();
    if (pRet && pRet->init()) {
        pRet->enable();
        return pRet;
    }
    CC_SAFE_DELETE(pRet); //don't crash if it fails
    return nullptr;
}

bool GDDLListener::init() {

    if (!ListManager::fetchedGDDLRatings) {
        this->bind([this](web::WebTask::Event* e) {
            if (auto res = e->getValue()) {
                if (res->ok() && res->string().isOk()) {
                    auto response = res->string().unwrap();

                    log::info("Successfully obtained GDDL Data.");

                    ListManager::fetchedGDDLRatings = true;
                    ListManager::parseResponse(response);
                }
                else {
                    ListManager::fetchedGDDLRatings = true;
                    log::info("Something went wrong obtaining the GDDL Data. ({})", res->code());
                }
            }
            else if (e->isCancelled()) {
                log::info("Cancelled GDDL request.");
            }
        });

        auto req = web::WebRequest();
        req.userAgent(getUserAgent());
        this->setFilter(req.get(GDDL_API_URL));
    }

    return true;
}

GDDLListener::~GDDLListener() {
    this->getFilter().cancel();
}