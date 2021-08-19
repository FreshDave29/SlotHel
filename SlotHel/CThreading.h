#pragma once

#include <string>
#include <chrono>
#include <thread>

#include "Euroscope/EuroScopePlugIn.h"
#include "lohmann/json.hpp"
#include "curl/curl.h"

using json = nlohmann::json;


class CThreading : public virtual CSlotHel{

public:
	CThreading();
	//~CThreading();

	void thread_run();

private:
	std::string message;
	json ConnectJson();
	json json_tmp;

};

CThreading* thr = new CThreading();
std::thread thread_ingo(&CThreading::thread_run, thr);