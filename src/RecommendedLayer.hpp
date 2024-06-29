#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

//geode namespace
using namespace geode::prelude;

class RecommendedLayer : public CCLayer, LevelManagerDelegate {
protected:
	virtual bool init(); //add stuff to the layer
	void backButton(CCObject*); //when you press back
	virtual void keyBackClicked(); //when you press escape

	EventListener<web::WebTask> m_listener;

	GJListLayer* m_list;
	LoadingCircle* m_loadCircle;

	std::vector<std::string> m_IDs;
	bool m_levelsLoaded = true;

	virtual ~RecommendedLayer();
public:
	static RecommendedLayer* create(); //to create the layer

	void reloadLevels(CCObject*);

	void loadLevels();
	void loadLevelsFinished(CCArray*, const char*) override;
	void loadLevelsFailed(const char*) override;
	void loadLevelsFinished(CCArray* levels, const char* key, int) override {
		loadLevelsFinished(levels, key);
	}
	void loadLevelsFailed(const char* key, int) override {
		loadLevelsFailed(key);
	}
};