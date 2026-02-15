#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class DPPackCell : public CCLayer {
protected:
    matjson::Value m_pack; // Pack Data
    std::string m_index; // Should be main/legacy/bonus/monthly
    int m_id;

    virtual bool init();
public:
    static DPPackCell* create(matjson::Value data, std::string index, int id);
    virtual ~DPPackCell();
};