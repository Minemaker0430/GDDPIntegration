#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/async.hpp>
#include <ctime>

//geode namespace
using namespace geode::prelude;
using namespace geode::utils::web;

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
	int m_currentTab = 0;
	CCMenu* m_reload;
	CCLabelBMFont* m_databaseVer;
	bool m_finishedLoading = false;
	bool m_error = false;
	CCLabelBMFont* m_errorText;
	CCLabelBMFont* m_monthlyTimer;

	std::string m_devSecret;

	int m_currentMonth = 0;
	int m_currentYear = 87;

	//listeners
	async::TaskHolder<WebResponse> m_listListener;
	async::TaskHolder<WebResponse> m_skillListener;
	async::TaskHolder<WebResponse> m_listener; //default

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
	void devCallback(CCObject*);
	
	void updateMonthlyTimer(float dt);
	
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

struct ListParameters : public CCObject {
    std::string m_type;
	int m_index;

    ListParameters(std::string type, int index) : m_type(type), m_index(index) {
        // Always remember to call autorelease on your classes!
        this->autorelease();
    }
};