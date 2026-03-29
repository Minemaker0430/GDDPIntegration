#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class RandomLevelPopup : public Popup, LevelManagerDelegate {
protected:
	bool init() override;
	virtual ~RandomLevelPopup();

    std::vector<int> m_levels;
    matjson::Value m_filter;

    LoadingCircle* m_loadCircle;

    void onToggle(CCObject*);
    void onConfirm(CCObject*);
public:
	static RandomLevelPopup* create(std::vector<int>);

    void loadLevelsFinished(CCArray*, const char*) override;
	void loadLevelsFailed(const char*) override;
	void loadLevelsFinished(CCArray* levels, const char* key, int) override {
		loadLevelsFinished(levels, key);
	}
	void loadLevelsFailed(const char* key, int) override {
		loadLevelsFailed(key);
	}
};
