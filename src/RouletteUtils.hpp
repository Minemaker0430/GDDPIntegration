#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class RouletteUtils {
public:
	static std::string toFlags(std::vector<bool> settings);
	static std::vector<bool> fromFlags(std::string settings);

    static std::vector<int> setupLevels(std::vector<matjson::Value> packs, std::string settings, int randomSeed = rand());

    static void exportSettings(std::string settings, int seed);
    static std::vector<std::string> importSettings(std::string settings);

    static void createSave(std::string name, std::vector<int> levels, std::string settings, int seed);
    static void exportSave(matjson::Value save, bool toFile = false);
    static void importSave(std::string saveStr, bool fromFile = false);

	static void deleteSaveAtPos(int pos);
	static void copySaveAtPos(int pos);
};

struct RouletteSaveFormat {
	std::string name = "New Roulette"; //Name of the Save File, can be anything
	std::string settings = "001100"; //Settings flags string
    int seed = rand(); //Random Seed. By default a randomized seed will be provided but you can have set seeds as well.
    std::vector<int> levels = {}; //Levels chosen by the initial Seed. Cannot and will not be changed (unless you edit the save data ig)
    int progress = 0; //Progress in %, base value is 0, clamped at 100
    int skips = 0; //Skips used. Skipping increases Progress by 1 but doesn't increase Score unless an option is enabled. 
    int score = 0; //Total Score. Not dependent on Progress but rather levels cleared.
};

template<>
struct matjson::Serialize<RouletteSaveFormat> {
	static Result<RouletteSaveFormat> fromJson(matjson::Value const& value) {
		auto rsf = RouletteSaveFormat{
			.name = value["name"].asString().unwrapOr("New Roulette"),
			.settings = value["settings"].asString().unwrapOr("001100"),
			.seed = value["seed"].as<int>().unwrapOr(rand()),
			.levels = value["levels"].as<std::vector<int>>().unwrapOrDefault(),
			.progress = value["progress"].as<int>().unwrapOr(0),
			.skips = value["skips"].as<int>().unwrapOr(0),
			.score = value["score"].as<int>().unwrapOr(0)
		};

		return Ok(rsf);
	}

	static matjson::Value toJson(RouletteSaveFormat const& value) {
		auto obj = matjson::makeObject({
			{ "name", value.name },
			{ "settings", value.settings },
			{ "seed", value.seed },
			{ "levels", value.levels },
			{ "progress", value.progress },
			{ "skips", value.skips },
			{ "score", value.score }
		});
		return obj;
	}
};