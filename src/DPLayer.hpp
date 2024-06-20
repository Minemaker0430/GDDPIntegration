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
};

struct ListSaveFormat {
	int progress;
	bool completed;
	bool hasRank;
};

template<>
struct matjson::Serialize<ListSaveFormat> {
	static ListSaveFormat from_json(matjson::Value const& value) {
		return ListSaveFormat{
			.progress = value["progress"].as_int(),
			.completed = value["completed"].as_bool(),
			.hasRank = value["has-rank"].as_bool()
		};
	}

	static matjson::Value to_json(ListSaveFormat const& value) {
		auto obj = matjson::Object();
		obj["progress"] = value.progress;
		obj["completed"] = value.completed;
		obj["has-rank"] = value.hasRank;
		return obj;
	}

	static bool is_json(Value const& value) {
		// TODO: this is intentionally lazy..
		return value.is_object();
	}
};

enum class DPListType {
    Main,
    Legacy,
    Bonus,
	Monthly,
};