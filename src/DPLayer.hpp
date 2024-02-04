#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class DPLayer : public CCLayer {
protected:
	virtual bool init(); //add stuff to the layer
	void backButton(CCObject*); //when you press back
	virtual void keyBackClicked(); //when you press escape
	virtual ~DPLayer();
public:
	static DPLayer* create(); //to create the layer
	void callback(CCObject*); //callback for the button to go to this layer
	void onTab(CCObject*); //tabs switched?
	void openList(CCObject*); //open list with the id tagged on the btn
	void generateMainList(CCArray*);
	void generateLegacyList(CCObject*);
	void generateBonusList(CCObject*);
	void generateMonthlyList(CCObject*);
	void infoCallback(CCObject*);
	void pageLeftCallback(CCObject*);
	void pageRightCallback(CCObject*);
	void soonCallback(CCObject*); //Coming Soon
	void switchtoMainCallback(CCObject*);
	void switchtoBonusCallback(CCObject*);
	void updateCallback(CCObject*);
};

enum class DPListType {
    Main,
    Legacy,
    Bonus,
	Monthly,
};