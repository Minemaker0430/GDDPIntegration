#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class RoulettePopup : public Popup<>, LevelManagerDelegate {
protected:
	bool setup() override;
	void onClose(CCObject*) override;
	virtual ~RoulettePopup();

	CCLayer* m_mainLayer;
	ListView* m_list;
	TextInput* m_value;
	LoadingCircle* m_loadcircle;

	bool m_levelLoaded = false;
	bool m_loadingCancelled = false;

	/*
	0 = Skips Enabled
	1 = Skips Award Points (Cheat)
	2 = Completed Levels
	3 = Uncompleted Levels
	4 = Perfect Run (Challenge)
	5 = The Gauntlet (Challenge)
	6+ = Packs
	*/
	std::vector<bool> m_settings = {false, false, true, true, false, false};
	int m_saveID = 0;
	GJGameLevel* m_currentLevel;

	EventListener<web::WebTask> m_listener;

	void loadRouletteSave(int id);
	void loadWinScreen(int id);

	void saveProgress(bool skip);

	void fetchLevel(int levelID);

	void onPlay(CCObject*);
	void onSkip(CCObject*);
	void onNext(CCObject*);
	void onSafe(CCObject*);

	void onBack(CCObject*);
	void onEnterSave(CCObject*);
	void onNewSave(CCObject*);
	void onDeleteSave(CCObject*);
	void onCopy(CCObject*);
	void onExport(CCObject*);
	void onRename(CCObject*);

	void onSettings(CCObject*);
	void onPackToggle(CCObject*);
	void onPackCheckAll(CCObject*);
	void onPackUncheckAll(CCObject*);
	void onImportSettings(CCObject*);
	void onFinalizeSave(CCObject*);
public:
	static RoulettePopup* create();

	std::vector<bool> m_storedSettings = {false, false, true, true, false, false}; //For use in the settings menu ONLY. Allows the settings to be saved in the Popup for creating the file.
	int m_storedSeed = -1; //For use in the settings menu ONLY. Allows the seed to be saved in the Popup for creating the file.

	void loadSaveMenu();
	void loadSettingsMenu();

	void loadLevelsFinished(CCArray*, const char*) override;
	void loadLevelsFailed(const char*) override;
	void loadLevelsFinished(CCArray* levels, const char* key, int) override {
		loadLevelsFinished(levels, key);
	}
	void loadLevelsFailed(const char* key, int) override {
		loadLevelsFailed(key);
	}
};

class RouletteRenamePopup : public Popup<> {
protected:
	bool setup() override;
	virtual ~RouletteRenamePopup();

	CCLayer* m_mainLayer;
	TextInput* m_value;

	int m_saveID = 0;

	void onExportSettings(CCObject*);
	void onConfirm(CCObject*);
public:
	static RouletteRenamePopup* create(int id);
};

class RouletteNewPopup : public Popup<> {
protected:
	bool setup() override;
	virtual ~RouletteNewPopup();

	CCLayer* m_mainLayer;

	void onCreate(CCObject*);
	void onImport(CCObject*);
public:
	static RouletteNewPopup* create();
};

class RouletteImportPopup : public Popup<> {
protected:
	bool setup() override;
	virtual ~RouletteImportPopup();

	CCLayer* m_mainLayer;
	TextInput* m_value;

	bool m_isSettings = false;

	void onPaste(CCObject*);
	void onConfirm(CCObject*);
public:
	static RouletteImportPopup* create(bool isSettings = false);
};

class RouletteSettingsPopup : public Popup<> {
protected:
	bool setup() override;
	void onClose(CCObject*) override;
	virtual ~RouletteSettingsPopup();

	CCLayer* m_mainLayer;
	TextInput* m_value;

	int m_seed = -1;
	std::vector<bool> m_settings = {false, false, true, true, false, false};

	void onToggle(CCObject*);
public:
	static RouletteSettingsPopup* create(int seed = -1, std::vector<bool> settings = {false, false, true, true, false, false});
};