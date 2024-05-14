#include "ListManager.hpp"
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

const std::string NLW_API_URL = "https://nlw.oat.zone/list?type=all";
const std::string IDS_API_URL = "https://nlw.oat.zone/ids?type=all";
const std::string POINTERCRATE_API_URL_PART_1 = "https://pointercrate.com/api/v2/demons/listed/?limit=100";
const std::string POINTERCRATE_API_URL_PART_2 = "https://pointercrate.com/api/v2/demons/listed/?limit=50&after=100";
const std::string GD_HISTORY_URL = "https://history.geometrydash.eu/api/v1/search/level/advanced/"
                                   "?limit=1000&filter=cache_demon = true AND cache_demon_type = 6";

bool ListManager::fetchedNLWRatings;
bool ListManager::fetchedIDSRatings;
bool ListManager::fetchedPointercrateRatings;
std::vector<ListRating> ListManager::ratings;

std::string getUserAgent() {
    return fmt::format("{}/{}; GeometryDash/{} (GeodeSDK/{}); {}",
		Mod::get()->getID(),
		Mod::get()->getVersion().toString(true),
		GEODE_STR(GEODE_GD_VERSION),
		Loader::get()->getVersion().toString(true),
		GEODE_PLATFORM_NAME
	);
}

void ListManager::init() {
    if (!ListManager::fetchedNLWRatings) {
        web::AsyncWebRequest()
            .userAgent(getUserAgent())
            .fetch(NLW_API_URL)
            .json()
            .then([] (const matjson::Value &val) {
                ListManager::fetchedNLWRatings = true;
                ListManager::parseResponse(val, true);
            })
            .expect([] (const std::string &error) {
                ListManager::fetchedNLWRatings = true;
            });
    }

    if (!ListManager::fetchedIDSRatings) {
        web::AsyncWebRequest()
            .userAgent(getUserAgent())
            .fetch(IDS_API_URL)
            .json()
            .then([] (const matjson::Value &val) {
                ListManager::fetchedIDSRatings = true;
                ListManager::parseResponse(val, false);
            })
            .expect([] (const std::string &error) {
                // todo: error handling...
                ListManager::fetchedIDSRatings = true;
            });
    }

    if (!ListManager::fetchedPointercrateRatings) {
        web::AsyncWebRequest()
            .userAgent(getUserAgent())
            .fetch(POINTERCRATE_API_URL_PART_1)
            .json()
            .then([] (const matjson::Value &val_part1) {
                web::AsyncWebRequest()
                    .userAgent(getUserAgent())
                    .fetch(POINTERCRATE_API_URL_PART_2)
                    .json()
                    .then([val_part1] (const matjson::Value &val_part2) {
                        ListManager::fetchedPointercrateRatings = true;
                        ListManager::parsePointercrateResponse(val_part1, val_part2);
                    })
                    .expect([] (const std::string &error) {
                        ListManager::fetchedPointercrateRatings = true;
                    });
            })
            .expect([] (const std::string &error) {
                ListManager::fetchedPointercrateRatings = true;
            });
    }
}

void ListManager::parseResponse(matjson::Value val, bool isExtreme) {
    if (!val.is_array()) {
        // todo: show error to user
        return;
    }

    auto levels = val.as_array();
    for (auto level : levels) {
        ListRating rating;

        std::string name = level["name"].is_string() ? level["name"].as_string() : "?";
        std::string type = level["type"].as_string();
        if (type == "platformer") {
            rating.type = RatingType::Platformer;
        } else if (type == "pending") {
            rating.type = RatingType::Pending;
        } else {
            rating.type = RatingType::Classic;
        }
        rating.id = level["id"].is_number() ? level["id"].as_int() : -1;
        rating.name = name;
        rating.tier = level["tier"].as_string();
        rating.isExtreme = isExtreme;

        if (name == "Falcon16"
            || name == "Shock Therapy"
            || name == "Ziroikabi"
            || name == "DMG CTRL"
            || name == "Hyper Paradox"
            || name == "Quantum Processing"
            || name == "Raisins")
            rating.tier = "Jade";

        ListManager::ratings.push_back(rating);
    }
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

void ListManager::parsePointercrateResponse(matjson::Value val_part1, matjson::Value val_part2) {
    if (!val_part1.is_array() || !val_part2.is_array()) {
        // todo: show error to user
        return;
    }

    auto levels = val_part1.as_array();
    auto levels_part2 = val_part2.as_array();
    for (auto &level : levels_part2) {
        levels.push_back(level);
    }

    int rubyEnd = -1;
    int diamondEnd = -1;
    int onyxEnd = -1;
    int amethystEnd = -1;
    for (auto &level : levels) {
        std::string name = level["name"].is_string() ? level["name"].as_string() : "?";
        if (name == "Hardry")
            rubyEnd = level["position"].as_int();
        else if (name == "qoUEO")
            diamondEnd = level["position"].as_int();
        else if (name == "poocubed")
            onyxEnd = level["position"].as_int();
        else if (name == "Acheron")
            amethystEnd = level["position"].as_int();
    }

    auto dataResult = web::fetchJSON(GD_HISTORY_URL);
    bool useGDH = true;
    matjson::Value gdhData;
    if (dataResult.isErr()) {
        log::error("uh. not good");
        useGDH = false;
    } else {
        gdhData = dataResult.ok().value();
    }

    for (auto level : levels) {
        ListRating rating;

        rating.isExtreme = true;
        rating.type = RatingType::Classic;
        rating.name = level["name"].is_string() ? level["name"].as_string() : "?";
        
        rating.id = level["level_id"].is_number() ? level["level_id"].as_int() : -1;
        if (rating.id < 0 && useGDH) {
            auto extremes = gdhData["hits"].as_array();
            matjson::Array hits;
            for (int i = 0; i < extremes.size(); i++) {
                if (extremes[i]["cache_level_name"].as_string() == rating.name) {
                    hits.push_back(extremes[i]);
                }
            }

            if (hits.size() == 1) {
                rating.id = hits[0]["online_id"].as_int();
            } else {
                // fetch the level in case the level is outside the 1000 level cache
                if (hits.size() == 0) {
                    std::string filter = fmt::format("cache_demon = true AND cache_level_name='{}'", rating.name);
                    auto dataResult = web::fetchJSON(
                        fmt::format("https://history.geometrydash.eu/api/v1/search/level/advanced/?filter={}", filter)
                    );
                    if (dataResult.isOk()) {
                        hits = dataResult.ok().value()["hits"].as_array();
                    }
                }

                int i = 0;
                int minDist = INT32_MAX;
                for (int j = 0; j < hits.size(); j++) {
                    auto hit = hits[j];
                    auto creator = hit["cache_username"].as_string();
                    auto matchCreator = level["publisher"]["name"].as_string();
                    int distance = levenshteinDistance(creator, matchCreator);
                    if (distance < minDist) {
                        i = j;
                        minDist = distance;
                    }
                }
                rating.id = hits[i]["online_id"].as_int();
            }
        }

        int position = level["position"].is_number() ? level["position"].as_int() : -1;
        if (position >= rubyEnd || rating.name == "Ouroboros")
            rating.tier = "Ruby";
        else if (position >= diamondEnd || rating.name == "Lotus Flower")
            rating.tier = "Diamond";
        else if (position >= onyxEnd)
            rating.tier = "Onyx";
        else if (position >= amethystEnd)
            rating.tier = "Amethyst";
        else /*if (position >= azuriteEnd)*/
            rating.tier = "Azurite";
        //else
        //    rating = "Obsidian";
        // ^^^ theoretical ^^^
        ListManager::ratings.push_back(rating);
    }
}

std::optional<ListRating> ListManager::getRating(int levelID) {
    for (auto rating : ListManager::ratings) {
		if (levelID == rating.id && rating.tier != "Fuck") {
			return rating;
		}
	}

    return std::nullopt;
}

std::string ListManager::getSpriteName(GJGameLevel *level) {
    auto ratingOpt = ListManager::getRating(level->m_levelID.value());
    if (ratingOpt.has_value()) {
        auto rating = ratingOpt.value();
        if (rating.isExtreme) {
            return fmt::format("DP_{}", NLW_TO_GDDP.at(rating.tier));
        } else {
            return fmt::format("DP_{}", IDS_TO_GDDP.at(rating.tier));
        }
    } else if (level->m_demonDifficulty == 3) {
        return "DP_Beginner";
    } else if (level->m_demonDifficulty == 4) {
        return "DP_Bronze";
    } else if (level->m_stars == 10 && level->m_demonDifficulty == 0) {
        return "DP_Silver";
    } else {
        return "";
    }
}