#include "pch.h"
#include <thread>
#include <future>
#include <string>

#include "CSlotHel.h"

//CSlotHel* pPlugin;

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
	this->RegisterTagItemFunction("Reload Data", TAG_FUNC_SLOT_LOAD);

	this->debug = false;
	this->updateCheck = false;
	this->autoConnect = false;

	this->min_TSAT = -300;
	this->max_TSAT = 300;
	this->max_TSAT_Warn = 600;
	this->updaterate = 30;

	this->AIRPORT = "LOWW";
	this->LISTappendix = ".standard.departure.json";

	this->LoadSettings();

	if (this->updateCheck) {
		this->latestVersion = std::async(FetchLatestVersion);
	}
	if (this->autoConnect) {
		
		//this->LogMessage("AutoConnect enabled, every " + std::to_string(this->updaterate) + "sec.", "Config");
		
		std::string message = "AutoConnect enabled, every " + std::to_string(this->updaterate) + "sec.";
		EuroScopePlugIn::CPlugIn::DisplayUserMessage(PLUGIN_NAME, "Config: ", message.c_str(), true, true, true, false, false);
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
			msg << "Version " << PLUGIN_VERSION << " loaded. Available commands: load, auto, debug, rate xx (number of seconds)";

			this->LogMessage(msg.str());

			return true;
		}

		if (args[1] == "debug") {
			if (this->debug) {
				this->LogMessage("Disabling debug mode", "Config");
			}
			else {
				this->LogMessage("Enabling debug mode", "Config");
			}

			this->debug = !this->debug;

			this->SaveSettings();

			return true;
		}

		else if (args[1] == "load") {
			this->LogMessage("Try to load data from web...", "Info");

			this->ParseJson(ConnectJson());

			return true;
		}
		else if (args[1] == "auto") {
			
			this->autoConnect = !this->autoConnect;
			
			if (this->autoConnect) {
				this->LogMessage("AutoConnect enabled, every " + std::to_string(this->updaterate) + "sec.", "Config");
			}
			else {
				this->LogMessage("Auto Connection off", "Config");
			}

			this->SaveSettings();
			return true;
		}
		else if (args[1] == "rate") {
			try {
				if (std::stoi(args[2]) >= 10) {	// prevent user to set intervall too low!
					this->updaterate = std::stoi(args[2]);
					this->LogMessage("Update Rate set to " + std::to_string(this->updaterate));
				}
				else {
					this->LogMessage("Update Rate too low. Use higher value (above 10) to prevent crashing ES.", "Error");
				}
			}
			catch (std::exception e)
			{
				this->LogMessage("Wrong parameter for RATE setting, use numeric only", "Error");
			}
			this->SaveSettings();
			return true;
		}
		else if (args[1] == "airport") {
			
			this->AIRPORT = args[2];
			this->LogMessage("Active Airport changed to " + this->AIRPORT, "Config");

			this->SaveSettings();
			return true;
		}
		else if (args[1] == "event") {

			this->LISTappendix = args[2];
			this->LogMessage("List-URL temporarily changed to " + SLOT_SYSTEM_PATH + this->AIRPORT + this->LISTappendix, "Config");

			return true;
		}
	}

	return false;
}

void CSlotHel::OnTimer(int Counter)
{
	if (this->autoConnect && Counter % this->updaterate == 0) {
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

void CSlotHel::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {
	switch (FunctionId) {
	case TAG_FUNC_SLOT_MENU:

		this->OpenPopupList(Area, "Slot Menu", 1);
		this->AddPopupListElement("Reload", NULL, TAG_FUNC_SLOT_LOAD, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, false);
		break;
	case TAG_FUNC_SLOT_LOAD:
		this->LogMessage("Try to load data from web...", "Info");
		this->ParseJson(ConnectJson());
		break;
	}
}

void CSlotHel::LoadSettings()
{
	const char* settings = this->GetDataFromSettings(PLUGIN_NAME);
	if (settings) {
		std::vector<std::string> splitSettings = split(settings, SETTINGS_DELIMITER);

		if (splitSettings.size() < 8) {
			this->LogMessage("Invalid saved settings found, reverting to default.");

			this->SaveSettings();

			return;
		}

		std::istringstream(splitSettings[0]) >> this->debug;
		std::istringstream(splitSettings[1]) >> this->autoConnect;
		std::istringstream(splitSettings[2]) >> this->min_TSAT;
		std::istringstream(splitSettings[3]) >> this->max_TSAT;
		std::istringstream(splitSettings[4]) >> this->max_TSAT_Warn;
		std::istringstream(splitSettings[5]) >> this->max_CTOT;
		std::istringstream(splitSettings[6]) >> this->updaterate;
		std::istringstream(splitSettings[7]) >> this->AIRPORT;

		this->LogDebugMessage("Successfully loaded settings.");
	}
	else {
		this->LogMessage("No saved settings found, using defaults.");
	}
}

void CSlotHel::SaveSettings()
{
	std::ostringstream ss;
	ss << this->debug << SETTINGS_DELIMITER
		<< this->autoConnect << SETTINGS_DELIMITER
		<< this->min_TSAT << SETTINGS_DELIMITER
		<< this->max_TSAT << SETTINGS_DELIMITER
		<< this->max_TSAT_Warn << SETTINGS_DELIMITER
		<< this->max_CTOT << SETTINGS_DELIMITER
		<< this->updaterate << SETTINGS_DELIMITER
		<< this->AIRPORT;

	this->SaveDataToSettings(PLUGIN_NAME, "SlotHel settings", ss.str().c_str());
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

		const std::string url = SLOT_SYSTEM_PATH + AIRPORT + LISTappendix;
		
		//const std::string url = "http://192.168.0.4/data/LOWW.standard.departure.json"; //debug

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

			// Aircraft List
			if (obj.key() == "aclist") {

				for (auto& [ac, ac2] : obj.value().items()) {
					this->LogDebugMessage("Aircraft read: " + ac, "Debug");

					try {
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
					} catch (std::exception e)
					{
						this->LogMessage("Failed to parse Json Data for AC " + ac, "Error");
					}

				}

			}
			// Slot List
			else if (obj.key() == "slotlist") {
				for (auto& [sl, sl2] : obj.value().items()) { // slotlist items = slotnumbers

					if (sl2.value<std::string>("callsign", "") != "") // if slot is not empty
					{
						for (auto aciter = this->aclist.entries.begin(); aciter != this->aclist.entries.end(); aciter++) {

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
					if (strcmp(fp.GetGroundState(), "TAXI") == 0 || strcmp(fp.GetGroundState(), "DEPA") == 0 ){
						st.tag = "-" + sliter->str_ctot + "-";
						st.color = TAG_COLOR_GREEN;
						break;
					}

					//check time for startup & push
					double diff = difftime(time(0), sliter->tsat_raw);
					if (diff >= this->min_TSAT && diff < this->max_TSAT) {	// 5 min before and after TSAT  - OK
						st.color = TAG_COLOR_GREEN;
						break;
					}
					else if (diff >= this->max_TSAT && diff < this->max_TSAT_Warn) { // 5 to 10min after TSAT - Warning
						st.color = TAG_COLOR_ORANGE;
						break;
					}
					else if (diff >= this->max_TSAT_Warn) { // later than 10 min after TSAT - Overdue, new slot needed
						st.tag = "OVERDUE"; // overdue, for sorting
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

/*void CSlotHel::CheckForUpdate()
{
	try
	{
		semver::version latest{ this->latestVersion.get() };
		semver::version current{ PLUGIN_VERSION };

		if (latest > current) {
			std::ostringstream ss;
			ss << "A new version (" << latest << ") of " << PLUGIN_NAME << " is available, download it at " << PLUGIN_LATEST_DOWNLOAD_URL;

			this->LogMessage(ss.str(), "Update");
		}
	}
	catch (std::exception& e)
	{
		MessageBox(NULL, e.what(), PLUGIN_NAME, MB_OK | MB_ICONERROR);
	}

	this->latestVersion = std::future<std::string>();
}*/

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	*ppPlugInInstance = pPlugin = new CSlotHel();
}

void __declspec (dllexport) EuroScopePlugInExit(void)
{
	delete pPlugin;
}