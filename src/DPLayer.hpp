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
};

enum class DPListType {
    Main,
    Legacy,
    Bonus,
	Monthly,
};