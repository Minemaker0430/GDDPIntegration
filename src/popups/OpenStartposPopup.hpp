#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class OpenStartposPopup : public CCLayer, LevelManagerDelegate {
protected:
	virtual bool init();
	virtual ~OpenStartposPopup();

    int m_levelID = 0;
    LoadingCircle* m_loadCircle;
public:
	static OpenStartposPopup* create(int);

	void loadLevelsFinished(CCArray*, const char*) override;
	void loadLevelsFailed(const char*) override;
	void loadLevelsFinished(CCArray* levels, const char* key, int) override {
		loadLevelsFinished(levels, key);
	}
	void loadLevelsFailed(const char* key, int) override {
		loadLevelsFailed(key);
	}
};