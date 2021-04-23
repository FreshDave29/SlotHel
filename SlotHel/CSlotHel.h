#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <future>
#include <fstream>
#include <filesystem>
#include <algorithm>

#include "Euroscope/EuroScopePlugIn.h"
//#include "semver/semver.hpp"
#include "lohmann/json.hpp"
#include "curl/curl.h"

#include "constants.h"
#include "helpers.h"
#include "slotlist.h"


using json = nlohmann::json;
using namespace std::chrono_literals;

class CSlotHel : public EuroScopePlugIn::CPlugIn 
{
public:
	CSlotHel();
	virtual ~CSlotHel();

	bool OnCompileCommand(const char* sCommandLine);
	void SaveSettings();
	void LoadSettings();
	void OnTimer(int Counter);
	void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	slot_tag ProcessFlightPlan(const EuroScopePlugIn::CFlightPlan& fp);

private:
	bool debug;
	bool updateCheck;
	bool autoConnect;
	int min_TSAT;
	int max_TSAT;
	int max_TSAT_Warn;
	int max_CTOT;
	int updaterate;
	std::future<std::string> latestVersion;

	aircraft_list aclist{};
	slot_list sllist{};

	json ConnectJson();
	void ParseJson(json j);


	void LogMessage(std::string message);
	void LogMessage(std::string message, std::string handler);
	void LogDebugMessage(std::string message);
	void LogDebugMessage(std::string message, std::string type);

	void CheckForUpdate();
};