#pragma once
#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class XPPopup : public Popup<> {
protected:
	bool setup() override;
	CCLayer* m_mainLayer;
	virtual ~XPPopup();
public:
	static XPPopup* create();
};

class DemonXPPopup : public Popup<> {
protected:
	bool setup() override;
	CCLayer* m_mainLayer;
	virtual ~DemonXPPopup();
public:
	int m_levelID = 0;
	static DemonXPPopup* create(int);
};

const std::vector<std::string> skillNames = {
	"Chokepoints",
	"Duals",
	"Fast Paced",
	"High CPS",
	"Memory/Learny",
	"Nerve Control",
	"Ship",
	"Timings",
	"Wave"
};

const std::vector<CCPoint> skillPositions = {
	{-105.f, 15.f},
	{-105.f, -15.f},
	{-105.f, -45.f},
	{-105.f, -75.f},
	{105.f, 15.f},
	{105.f, -15.f},
	{105.f, -45.f},
	{105.f, -75.f},
	{0.f, -105.f}
};

const std::vector<ccColor3B> skillColors = {
	{255, 0, 0}, 
	{255, 128, 0}, 
	{255, 255, 0}, 
	{128, 255, 0}, 
	{0, 255, 0}, 
	{0, 255, 128}, 
	{0, 255, 255}, 
	{0, 128, 255}, 
	{0, 0, 255}
};