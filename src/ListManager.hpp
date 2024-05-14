/*
    Created by wint0r
*/

#ifndef __LISTMANAGER_HPP_
#define __LISTMANAGER_HPP_

#include <vector>
#include <matjson.hpp>
#include <optional>
#include <string>
#include <map>
#include <Geode/Geode.hpp>

enum class RatingType {
    Classic,
    Platformer,
    Pending
};

struct ListRating {
    RatingType type;
    bool isExtreme;
    std::string tier;
    int id;
    std::string name;
};

const std::map<std::string, std::string> NLW_TO_GDDP = {
    { "Beginner", "Platinum" },
    { "Easy", "Platinum" },
    { "Medium", "Sapphire" },
    { "Hard", "Sapphire" },
    { "Very Hard", "Sapphire" },
    { "Insane", "Jade" },
    { "Extreme", "Emerald" },
    { "Remorseless", "Emerald" },
    { "Relentless", "Ruby" },
    { "Terrifying", "Ruby" },

    // I KNOW THESE ARE NOT NLW TIERS i'm just lazy ok :sob:
    { "Jade", "Jade" },
    { "Ruby", "Ruby" },
    { "Diamond", "Diamond" },
    { "Onyx", "Onyx" },
    { "Amethyst", "Amethyst" },
    { "Azurite", "Azurite" },
    { "Obsidian", "Obsidian" }
};

const std::map<std::string, std::string> IDS_TO_GDDP = {
    { "Beginner", "Gold" },
    { "Easy", "Gold" },
    { "Medium", "Gold" },
    { "Hard", "Amber" },
    { "Very Hard", "Amber" },
    { "Insane", "Amber" },
    { "Extreme", "Amber" },
    { "Remorseless", "Amber" },
    { "Relentless", "Amber" },
    { "Terrifying", "Amber" },
};

struct ListManager {
    static std::vector<ListRating> ratings;
    static bool fetchedNLWRatings, fetchedIDSRatings, fetchedPointercrateRatings;

    static void init();
    static void parseResponse(matjson::Value val, bool isExtreme);
    static void parsePointercrateResponse(matjson::Value val_part1, matjson::Value val_part2);
    //static void throwError(std::string message);
    static std::optional<ListRating> getRating(int levelID);
    static std::string getSpriteName(GJGameLevel* level);
};

#endif