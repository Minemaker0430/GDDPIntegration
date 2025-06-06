#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

//geode namespace
using namespace geode::prelude;

class DPListLayer : public CCLayer, LevelManagerDelegate {
protected:
	virtual bool init(const char* type, int id, bool isPractice); //add stuff to the layer
	void backButton(CCObject*); //when you press back
	virtual void keyBackClicked(); //when you press escape

	EventListener<web::WebTask> m_listener;

	GJListLayer* m_list;
	CCSprite* m_progressBar;
	LoadingCircle* m_loadCircle;
	CCMenu* m_pagesMenu;
	CCMenuItemSpriteExtra* m_left;
	CCMenuItemSpriteExtra* m_right;
	CCLabelBMFont* m_errorText;

	std::string m_type = "main";
	int m_id = 0;
	std::vector<std::string> m_IDs;
	bool m_levelsLoaded = true;
	int m_page = 0;
	bool m_loadingCancelled = false;

	virtual ~DPListLayer();
public:
	static DPListLayer* create(const char* type, int id, bool isPractice); //to create the layer

	void reloadLevels(CCObject*);
	void pageRight(CCObject*);
	void pageLeft(CCObject*);

	bool m_isPractice = false;

	void updateProgressBar();
	void updateSave();
	void loadLevels(int);
	void loadLevelsFinished(CCArray*, const char*) override;
	void loadLevelsFailed(const char*) override;
	void loadLevelsFinished(CCArray* levels, const char* key, int) override {
		loadLevelsFinished(levels, key);
	}
	void loadLevelsFailed(const char* key, int) override {
		loadLevelsFailed(key);
	}
};