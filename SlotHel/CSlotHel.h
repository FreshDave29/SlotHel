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

private:
	bool debug;
	bool updateCheck;
	std::future<std::string> latestVersion;

	void ConnectJson();
	void ParseJson(json j);


	void LogMessage(std::string message);
	void LogMessage(std::string message, std::string handler);
	void LogDebugMessage(std::string message);
	void LogDebugMessage(std::string message, std::string type);

	void CheckForUpdate();
};