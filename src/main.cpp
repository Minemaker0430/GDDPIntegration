//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

#include "Settings.hpp"
#include "ListManager.hpp"

//geode namespace
using namespace geode::prelude;

SettingNode* SectionSettingValue::createNode(float width) {
	return SectionSettingNode::create(this, width);
}

$on_mod(Loaded) {
	Mod::get()->addCustomSetting<SectionSettingValue>("bypass-section", "");
	Mod::get()->addCustomSetting<SectionSettingValue>("menu-section", "");
	Mod::get()->addCustomSetting<SectionSettingValue>("cosmetic-section", "");
	Mod::get()->addCustomSetting<SectionSettingValue>("compatibility-section", "");

	ListManager::init();
}