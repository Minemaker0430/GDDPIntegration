/*
    Created by wint0r
    Modified by Me
*/

#ifndef __LISTMANAGER_HPP_
#define __LISTMANAGER_HPP_

#include <vector>
#include <matjson.hpp>
#include <optional>
#include <string>
#include <map>
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

//geode namespace
using namespace geode::prelude;

struct ListRating {
    int tier = 0;
    int id = -1;
    std::string name = "?";
};

const std::map<int, std::string> GDDL_TO_GDDP = { //These ratings are based on the highest GDDL Tier in each GDDP Tier
    { 14, "Gold" }, //not sure if there's any tier 14 insanes but might as well check anyways
    { 15, "Gold" },
    { 16, "Gold" },
    { 17, "Gold" },
    { 18, "Amber" },
    { 19, "Amber" },
    { 20, "Amber" },
    { 21, "Platinum" },
    { 22, "Platinum" },
    { 23, "Sapphire" },
    { 24, "Sapphire" },
    { 25, "Sapphire" },
    { 26, "Jade" },
    { 27, "Jade" },
    { 28, "Emerald" },
    { 29, "Emerald" },
    { 30, "Ruby" },
    { 31, "Ruby" },
    { 32, "Diamond" },
    { 33, "Diamond" },
    { 34, "Onyx" },
    { 35, "Amethyst" }
};

struct ListManager {
    static std::vector<ListRating> ratings;
    static bool fetchedGDDLRatings;

    static void init();
    static void parseResponse(matjson::Value val);
    //static void throwError(std::string message);
    static std::optional<ListRating> getRating(int levelID);
    static std::string getSpriteName(GJGameLevel* level);
};

class GDDLListener : public EventListener<web::WebTask> {
protected:
    virtual bool init();
    virtual ~GDDLListener();
public:
    static GDDLListener* create();

};

#endif