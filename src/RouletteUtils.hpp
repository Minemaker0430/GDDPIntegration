#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class RouletteUtils {
public:
	static int toFlags(std::vector<bool>);
	static std::vector<bool> fromFlags(int);

    static std::vector<int> setupLevels(std::vector<matjson::Value> packs, int randomSeed = rand());

    static void exportSettings(std::vector<bool>, int);
    static std::vector<int> importSettings(std::string);

    static void createSave(std::string, std::vector<int>, int, int);
    static void exportSave(matjson::Value save, bool toFile = false);
    static void importSave(std::string saveStr, bool fromFile = false);
};

struct RouletteSaveFormat {
	std::string name = "New Roulette"; //Name of the Save File, can be anything
	int settings = 0; //Settings flags number.
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
			.name = value["name"].asString().unwrap(),
			.settings = value["settings"].as<int>().unwrap(),
			.seed = value["seed"].as<int>().unwrap(),
			.levels = value["levels"].as<std::vector<int>>().unwrap(),
			.progress = value["progress"].as<int>().unwrap(),
			.skips = value["skips"].as<int>().unwrap(),
			.score = value["score"].as<int>().unwrap()
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