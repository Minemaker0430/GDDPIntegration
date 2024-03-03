#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class DPLayer : public CCLayer {
protected:
	virtual bool init(); //add stuff to the layer
	void backButton(CCObject*); //when you press back
	virtual void keyBackClicked(); //when you press escape
	ListView* m_list;
	CCMenu* m_tabs;
	matjson::Value m_data;
	LoadingCircle* m_loadcircle;
	int m_currentTab;
	CCMenu* m_reload;
	CCLabelBMFont* m_databaseVer;
	void reloadList(int type);
	virtual ~DPLayer();
public:
	static DPLayer* create(); //to create the layer
	void callback(CCObject*); //callback for the button to go to this layer
	void onTab(CCObject*); //tabs switched?
	void openList(CCObject*); //open list with the id tagged on the btn
	void reloadData(CCObject*); //when reload button is pressed
	void infoCallback(CCObject*);
	void soonCallback(CCObject*); //Coming Soon
	void achievementsCallback(CCObject*);
};

enum class DPListType {
	Main,
	Legacy,
	Bonus,
	Monthly,
};

class DPStatsPopup : public geode::Popup<std::string const&> {
protected:
	bool setup(std::string const& value) override {
		auto winSize = CCDirector::sharedDirector()->getWinSize();

		// convenience function provided by Popup 
		// for adding/setting a title to the popup
		this->setTitle("Stats");

		auto label = CCLabelBMFont::create(value.c_str(), "bigFont.fnt");
		label->setPosition(winSize / 2);
		this->addChild(label);

		return true;
	}

public:
	static DPStatsPopup* create(std::string const& text) {
		auto ret = new DPStatsPopup();
		if (ret && ret->init(240.f, 160.f, text)) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}
};