#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

//geode namespace
using namespace geode::prelude;

class DPLayer : public CCLayer {
protected:
	virtual bool init(); //add stuff to the layer
	void backButton(CCObject*); //when you press back
	virtual void keyBackClicked(); //when you press escape
	ListView* m_list;
	CCMenu* m_tabs;
	CCMenu* m_backMenu;
	matjson::Value m_data;
	LoadingCircle* m_loadcircle;
	int m_currentTab;
	CCMenu* m_reload;
	CCLabelBMFont* m_databaseVer;
	bool m_finishedLoading = false;
	bool m_error = false;
	CCLabelBMFont* m_errorText;

	//listeners
	EventListener<web::WebTask> m_listListener;
	EventListener<web::WebTask> m_skillListener;
	EventListener<web::WebTask> m_listener; //default

	virtual ~DPLayer();
public:
	static DPLayer* create(); //to create the layer
	void callback(CCObject*); //callback for the button to go to this layer
	void onTab(CCObject*); //tabs switched?
	void openList(CCObject*); //open list with the id tagged on the btn
	void reloadCallback(CCObject*); //when reload is pressed
	void reloadData(bool); //reload all data
	void reloadList(int type);
	void soonCallback(CCObject*); //Coming Soon
	void achievementsCallback(CCObject*);
	void supportCallback(CCObject*);
	void newsCallback(CCObject*);
	void searchCallback(CCObject*);
	void rouletteCallback(CCObject*);
	void recommendedCallback(CCObject*);
	void xpCallback(CCObject*);
};

struct ListSaveFormat {
	int progress;
	bool completed;
	bool hasRank;
};

template<>
struct matjson::Serialize<ListSaveFormat> {
	static Result<ListSaveFormat> fromJson(matjson::Value const& value) {
		auto lsf = ListSaveFormat{
			.progress = value["progress"].as<int>().unwrapOr(0),
			.completed = value["completed"].asBool().unwrapOr(false),
			.hasRank = value["has-rank"].asBool().unwrapOr(false)
		};

		return Ok(lsf);
	}

	static matjson::Value toJson(ListSaveFormat const& value) {
		auto obj = matjson::makeObject({
			{ "progress", value.progress },
			{ "completed", value.completed },
			{ "has-rank", value.hasRank }
		});
		return obj;
	}
};

enum class DPListType {
    Main,
    Legacy,
    Bonus,
	Monthly,
};