#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <future>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "Euroscope/EuroScopePlugIn.h"
#include "lohmann/json.hpp"


#include "constants.h"
#include "helpers.h"
#include "slotlist.h"

using json = nlohmann::json;
//using namespace std::chrono_literals;


/* ### Global variables for inter-thread communication ### */

std::mutex mtx;

// Storing data from json-feed for further processing
std::mutex mtx_json;
json json_stor;

// Message queue for debugging
std::mutex mtx_mes;
std::string global_message;
std::string global_error;

// Running Thread and rate of curl-updates
std::condition_variable cv;
std::atomic_int updaterate;
std::atomic_bool thr_run = ATOMIC_VAR_INIT(true);





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
	void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	slot_tag ProcessFlightPlan(const EuroScopePlugIn::CFlightPlan& fp);
	bool autoConnect;

	void LogDebugMessageThread();
	void LogMessageThread();

	void LogMessage(std::string message);
	void LogMessage(std::string message, std::string type);
	void LogDebugMessage(std::string message);
	void LogDebugMessage(std::string message, std::string type);


private:
	bool debug;
	bool updateCheck;
	int min_TSAT;
	int max_TSAT;
	int max_TSAT_Warn;
	int max_CTOT;
	std::string AIRPORT;


	std::future<std::string> latestVersion;

	aircraft_list aclist{};
	slot_list sllist{};

	void ParseJson(json j);

	void CheckForUpdate();
};

CSlotHel* pPlugin;