#include "pch.h"
#include <thread>
#include <future>
#include <string>

#include "CSlotHel.h"

CSlotHel* pPlugin;

CSlotHel::CSlotHel() : EuroScopePlugIn::CPlugIn(
	EuroScopePlugIn::COMPATIBILITY_CODE,
	PLUGIN_NAME,
	PLUGIN_VERSION,
	PLUGIN_AUTHOR,
	PLUGIN_LICENSE
)
{
	std::ostringstream msg;
	msg << "Version " << PLUGIN_VERSION << " loaded.";

	this->LogMessage(msg.str());

	this->RegisterTagItemType("Slot", TAG_ITEM_SLOT);
	this->RegisterTagItemFunction("Slot Menu", TAG_FUNC_SLOT_MENU);

	this->debug = true;
	this->updateCheck = false;
	this->autoConnect = false;

	//this->LoadSettings();

	if (this->updateCheck) {
		this->latestVersion = std::async(FetchLatestVersion);
	}
}

CSlotHel::~CSlotHel()
{
}

bool CSlotHel::OnCompileCommand(const char* sCommandLine)
{
	std::vector<std::string> args = split(sCommandLine);

	if (starts_with(args[0], ".slothel")) {
		if (args.size() == 1) {
			std::ostringstream msg;
			msg << "Version " << PLUGIN_VERSION << " loaded. Available commands: load, debug, reset, update";

			this->LogMessage(msg.str());

			return true;
		}

		if (args[1] == "debug") {
			if (this->debug) {
				this->LogMessage("Disabling debug mode", "Debug");
			}
			else {
				this->LogMessage("Enabling debug mode", "Debug");
			}

			this->debug = !this->debug;

			//this->SaveSettings();

			return true;
		}
		else if (args[1] == "update") {
			if (this->updateCheck) {
				this->LogMessage("Disabling update check", "Update");
			}
			else {
				this->LogMessage("Enabling update check", "Update");
			}

			this->updateCheck = !this->updateCheck;

			//this->SaveSettings();

			return true;
		}

		else if (args[1] == "reset") {
			this->LogMessage("Resetting plugin state", "Config");


			return true;
		}

		else if (args[1] == "load") {
			this->LogMessage("Try to load data from web", "Debug");

			this->ParseJson(ConnectJson());

			return true;
		}
		else if (args[1] == "auto") {
			
			this->autoConnect = !this->autoConnect;
			this->LogMessage("Auto Connection toggled: " + std::to_string(this->autoConnect), "Config");
			
			return true;
		}
	}

	return false;
}

void CSlotHel::OnTimer(int Counter)
{
	if (this->autoConnect && Counter % 30 == 0) {
		this->ParseJson(ConnectJson());
	}
}

void CSlotHel::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{

	slot_tag st = this->ProcessFlightPlan(FlightPlan);

	switch (ItemCode) {
	case TAG_ITEM_SLOT:
		if (st.tag.empty()) {
			strcpy_s(sItemString, 16, "No Slot");
		}
		else
		{
			strcpy_s(sItemString, 16, st.tag.c_str());
		}

		*pColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
		*pRGB = st.color;

		break;
	}
}



namespace
{
	std::size_t callback(
		const char* in,
		std::size_t size,
		std::size_t num,
		std::string* out)
	{
		const std::size_t totalBytes(size * num);
		out->append(in, totalBytes);
		return totalBytes;
	}
}


json CSlotHel::ConnectJson()
{

	try {

		//const std::string url = SLOT_SYSTEM_PATH + "LOWW.standard.departure.json";
		const std::string url = "http://192.168.0.4/data/LOWW.standard.departure.json"; //debug

		CURL* curl = curl_easy_init();

		// Set remote URL.
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Don't bother trying IPv6, which would increase DNS resolution time.
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

		// Don't wait forever, time out after 5 seconds.
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

		// Follow HTTP redirects if necessary.
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		// Response information.
		long httpCode(0);
		std::unique_ptr<std::string> httpData(new std::string());

		// Hook up data handling function.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

		// Hook up data container (will be passed as the last parameter to the
		// callback handling function).  Can be any pointer type, since it will
		// internally be passed as a void pointer.
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

		// Run our HTTP GET command, capture the HTTP response code, and clean up.
		curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		curl_easy_cleanup(curl);

		if (httpCode == 200)
		{
			this->LogDebugMessage("Got successful response from " + url, "Debug");

			// Response looks good - done using Curl now.  Try to parse the results
			// and print them out.

			return json::parse(*httpData.get());
		}
		else
		{
			this->LogMessage("TimeOut or No Connection, Code: " + std::to_string(httpCode) + "-" + url, "Error");
			return NULL;
		}

	}
	catch (std::exception e)
	{
		this->LogMessage("Failed to read slot data from web system. Error: " + std::string(e.what()), "Error");
		return NULL;
	}
}

void CSlotHel::ParseJson(json j) {

	try {
		this->aclist.entries.clear(); //reset all lists to read new data
		this->sllist.entries.clear(); //reset all lists to read new data


		for (auto& obj : j.items()) {
			this->LogDebugMessage("Start checking lists: " + obj.key(), "Debug");
			// Aircraft List
			if (obj.key() == "aclist") {
				//if (obj.value()["aclist"].empty()) {
				//	this->LogDebugMessage("No aircraft on Ground - no Slotlist created", "Debug");
				//}
				//else {

				for (auto& [ac, ac2] : obj.value().items()) {
					this->LogDebugMessage("Aircraft read: " + ac, "Debug");

					//if (ac2.value<std::string>("aircraft_state", "") == "gate") {
						aircraft_entry tempAircraft{
							ac2.value<std::string>("callsign",""),
							ac2.value<std::string>("clearance_state",""),
							ac2.value<std::string>("pushback_state", ""),
							ac2.value<std::string>("taxi_state", ""),
							ac2.value<std::string>("aircraft_state",""),
							ac2.value<int>("clearance_time",0),
							ac2.value<int>("pushback_time",0),
							ac2.value<int>("taxi_time",0)
						};
						this->aclist.entries.push_back(tempAircraft);
						//this->LogDebugMessage("Aircraft added to list: " + ac.key(), "Debug");
					//}
				}
				//}
			}
			// Slot List
			else if (obj.key() == "slotlist") {
				for (auto& [sl, sl2] : obj.value().items()) { // slotlist items = slotnumbers
					//this->LogDebugMessage("Start reading slot data... " + sl.key(), "Debug");	
					//this->LogDebugMessage("Aircraft in Slot: " + to_string(sl.value()["callsign"]), "Debug");
					if (sl2.value<std::string>("callsign", "") != "") // if slot is not empty
					{
						for (auto aciter = this->aclist.entries.begin(); aciter != this->aclist.entries.end(); aciter++) {
							//this->LogDebugMessage("Checking " + aciter->callsign + " vs " + sl2.value<std::string>("callsign", ""), "Debug");

							if (sl2.value<std::string>("callsign", "") == aciter->callsign) {

								// read slot time (=ctot) and calculate tsat (ctot-taxi-push)
								tm ptm_tsat;
								tm ptm_ctot;

								time_t rawtime_ctot = sl2.value<int>("timestamp_slot",0);
								time_t rawtime_tsat = rawtime_ctot;

								gmtime_s(&ptm_ctot, &rawtime_ctot);
								gmtime_s(&ptm_tsat, &rawtime_tsat);


								ptm_tsat.tm_min -= aciter->t_taxi;
								ptm_tsat.tm_min -= aciter->t_pushback;

								rawtime_tsat = _mkgmtime(&ptm_tsat);
								gmtime_s(&ptm_tsat, &rawtime_tsat);

								// parse times to human readable format
								char c_ctot[12];
								char c_tsat[12];

								strftime(c_ctot, 12, "%R", &ptm_ctot);
								strftime(c_tsat, 12, "%R", &ptm_tsat);

								// create slot entry
								slot_entry tempSlot{
									sl2.value<int>("nr",0),
									rawtime_ctot,
									ptm_ctot.tm_hour,
									ptm_ctot.tm_min,
									rawtime_tsat,
									ptm_tsat.tm_hour,
									ptm_tsat.tm_min,
									sl2.value<std::string>("callsign",""),
									std::string(c_ctot),
									std::string(c_tsat)
								};

								this->sllist.entries.push_back(tempSlot);

								this->LogDebugMessage("Slot: " + sl, "Debug");
								this->LogDebugMessage("\tCS: " + sl2.value<std::string>("callsign",""), "Debug");
								this->LogDebugMessage("\tCTOT: " + tempSlot.str_ctot, "Debug");
								this->LogDebugMessage("\tTSAT: " + tempSlot.str_tsat, "Debug");
							}

						}
					}
					else {
						//this->LogDebugMessage("Slot " + sl + " empty!", "Debug");
					}
				}
			}
			else {
				this->LogDebugMessage("List skipped: " + obj.key(), "Debug");
				continue;
			}

		}
	}
	catch (std::exception e)
	{
		this->LogMessage("Failed to parse Json Data, Error: " + std::string(e.what()), "Error");
		return;
	}
}

slot_tag CSlotHel::ProcessFlightPlan(const EuroScopePlugIn::CFlightPlan& fp) {

	slot_tag st{
		"",
		TAG_COLOR_NONE
	};

	std::string fp_cs = fp.GetCallsign();
	auto sliter = this->sllist.entries.begin();

	for (auto sliter = this->sllist.entries.begin(); sliter != this->sllist.entries.end(); sliter++) {

		if (sliter->callsign == fp_cs) {
			st.tag = sliter->str_tsat + "/" + sliter->str_ctot;

			
			for (auto aciter = this->aclist.entries.begin(); aciter != this->aclist.entries.end(); aciter++) {
				
				if (sliter->callsign == aciter->callsign) {
					// check if aircraft has left gate
					if (aciter->st_aircraft != "gate") {
						st.tag = "00/" + sliter->str_ctot;
						st.color = TAG_COLOR_GREEN;
						break;
					}

					//check time for startup & push
					double diff = difftime(time(0), sliter->tsat_raw);
					if (diff >= -300 && diff < 300) {	// 5 min before and after TSAT  - OK
						st.color = TAG_COLOR_GREEN;
						break;
					}
					else if (diff >= 300 && diff < 600) { // 5 to 10min after TSAT - Warning
						st.color = TAG_COLOR_ORANGE;
						break;
					}
					else if (diff >= 600) { // later than 10 min after TSAT - Overdue, new slot needed
						st.tag = "-OVERDUE-"; // overdue, for sorting
						st.color = TAG_COLOR_RED;
						break;
					}
					else {
						st.color = TAG_COLOR_NONE;
					}
				}
			}
		}
	}

	return st;

}




void CSlotHel::LogMessage(std::string message)
{
	this->DisplayUserMessage("Message", PLUGIN_NAME, message.c_str(), true, true, true, false, false);
}

void CSlotHel::LogMessage(std::string message, std::string type)
{
	this->DisplayUserMessage(PLUGIN_NAME, type.c_str(), message.c_str(), true, true, true, true, false);
}

void CSlotHel::LogDebugMessage(std::string message)
{
	if (this->debug) {
		this->LogMessage(message);
	}
}

void CSlotHel::LogDebugMessage(std::string message, std::string type)
{
	if (this->debug) {
		this->LogMessage(message, type);
	}
}

void CSlotHel::CheckForUpdate()
{
	try
	{
		/*semver::version latest{ this->latestVersion.get() };
		semver::version current{ PLUGIN_VERSION };

		if (latest > current) {
			std::ostringstream ss;
			ss << "A new version (" << latest << ") of " << PLUGIN_NAME << " is available, download it at " << PLUGIN_LATEST_DOWNLOAD_URL;

			this->LogMessage(ss.str(), "Update");
		}*/
	}
	catch (std::exception& e)
	{
		MessageBox(NULL, e.what(), PLUGIN_NAME, MB_OK | MB_ICONERROR);
	}

	this->latestVersion = std::future<std::string>();
}

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	*ppPlugInInstance = pPlugin = new CSlotHel();
}

void __declspec (dllexport) EuroScopePlugInExit(void)
{
	delete pPlugin;
}