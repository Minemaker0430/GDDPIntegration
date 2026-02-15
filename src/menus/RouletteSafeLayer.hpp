#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

//geode namespace
using namespace geode::prelude;

class RouletteSafeLayer : public CCLayer, LevelManagerDelegate {
protected:
	virtual bool init(std::vector<int> IDs); //add stuff to the layer
	void backButton(CCObject*); //when you press back
	virtual void keyBackClicked(); //when you press escape

	async::TaskHolder<web::WebResponse> m_listener;

	GJListLayer* m_list;
	LoadingCircle* m_loadCircle;
	CCMenu* m_pagesMenu;
	CCMenuItemSpriteExtra* m_left;
	CCMenuItemSpriteExtra* m_right;
	CCLabelBMFont* m_errorText;

	std::vector<int> m_IDs;
	bool m_levelsLoaded = true;
	int m_page = 0;
	bool m_loadingCancelled = false;

	virtual ~RouletteSafeLayer();
public:
	static RouletteSafeLayer* create(std::vector<int> IDs); //to create the layer

	void reloadLevels(CCObject*);
	void pageRight(CCObject*);
	void pageLeft(CCObject*);

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