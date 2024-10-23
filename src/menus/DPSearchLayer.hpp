#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

//geode namespace
using namespace geode::prelude;

class DPSearchLayer : public CCLayer, LevelManagerDelegate {
protected:
	virtual bool init(std::vector<int>); //add stuff to the layer
	void backButton(CCObject*); //when you press back
	virtual void keyBackClicked(); //when you press escape

	matjson::Value m_data;

	EventListener<web::WebTask> m_listener;

	GJListLayer* m_list;
	LoadingCircle* m_loadCircle;

	CCMenu* m_pagesMenu;
	CCMenuItemSpriteExtra* m_left;
	CCMenuItemSpriteExtra* m_right;
	CCMenuItemSpriteExtra* m_first;
	CCMenuItemSpriteExtra* m_last;
	CCMenuItemSpriteExtra* m_select;
	
	CCMenu* m_filterMenu;
	CCMenuItemToggler* m_reverseBtn;
	CCMenuItemToggler* m_packBtn;
	CCMenuItemToggler* m_difficultyBtn;
	CCMenuItemToggler* m_nameBtn;
	CCMenuItemToggler* m_ageBtn;
	
	CCLabelBMFont* m_errorText;
	CCLabelBMFont* m_bottomText;

	std::vector<int> m_searchList;
	std::vector<std::string> m_IDs;

	std::vector<int> m_filterPacks;
	std::vector<int> m_filterDifficulty;
	std::vector<int> m_filterAlphabetic;
	std::vector<int> m_filterAge;

	bool m_levelsLoaded = true;
	bool m_loadingCancelled = false;
	int m_filter = 0;
	bool m_reversed = false;

	virtual ~DPSearchLayer();
public:
	static DPSearchLayer* create(std::vector<int>); //to create the layer

	int m_page = 0;

	std::vector<int> compareDifficulty(matjson::Value, std::vector<int>);
	std::vector<int> compareName(matjson::Value, std::vector<int>);

	void reloadLevels(CCObject*);
	void setFilter(CCObject*);
	void pageRight(CCObject*);
	void pageLeft(CCObject*);
	void pageFirst(CCObject*);
	void pageLast(CCObject*);
	void pageSelect(CCObject*);
	void reverseList(CCObject*);

	void loadLevels(int);
	void loadLevelsAtPage(int);
	void loadLevelsFinished(CCArray*, const char*) override;
	void loadLevelsFailed(const char*) override;
	void loadLevelsFinished(CCArray* levels, const char* key, int) override {
		loadLevelsFinished(levels, key);
	}
	void loadLevelsFailed(const char* key, int) override {
		loadLevelsFailed(key);
	}
};

enum class SearchFilter {
    Pack,
    Difficulty,
    Alphabetic,
	Age
};

struct CompareDifficulty
{
    int key;
    int value;

    CompareDifficulty(int k, const int& s) : key(k), value(s) {}

    bool operator < (const CompareDifficulty& str) const
    {
        return (value < str.value);
    }
};

struct CompareName
{
    int key;
    std::string stringValue;

    CompareName(int k, const std::string& s) : key(k), stringValue(s) {}

    bool operator < (const CompareName& str) const
    {
        return (stringValue < str.stringValue);
    }
};

class PagePopup : public Popup<> {
protected:
	bool setup() override;
	CCLayer* m_mainLayer;
	virtual ~PagePopup();

	CCMenu* m_pageSelectMenu;
	CCMenuItemSpriteExtra* m_left;
	CCMenuItemSpriteExtra* m_right;
	CCMenuItemSpriteExtra* m_reset;
	CCMenuItemSpriteExtra* m_confirm;
	TextInput* m_value;

	int m_page = 0;
public:
	static PagePopup* create(int);

	void confirmPage(CCObject*);
	void pageLeft(CCObject*);
	void pageRight(CCObject*);
	void resetPage(CCObject*);
};