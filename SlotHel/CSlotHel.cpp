#include "pch.h"

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

			this->ConnectJson();


			return true;
		}
	}

	return false;
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

void CSlotHel::ConnectJson()
{

	try {
		
		const std::string url = SLOT_SYSTEM_PATH + "LOWW.standard.departure.json";
		
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

			ParseJson(json::parse(*httpData.get()));
		}
		else
		{
			this->LogMessage("TimeOut or No Connection, Code: "+ std::to_string(httpCode) + "-" + url, "Error");
		}
		
	}
	catch (std::exception e)
	{
			this->LogMessage("Failed to read slot data from web system. Error: " + std::string(e.what()), "Error");
			return;
	}
}

void CSlotHel::ParseJson(json j) {

	try{
		aircraft_list aclist{};
		slot_list sllist{};


		for (auto& obj : j.items()) {
			this->LogDebugMessage("Start checking lists", "Debug");
				// Aircraft List
				if (obj.key() == "aclist") {
					//if (obj.value()["aclist"].empty()) {
					//	this->LogDebugMessage("No aircraft on Ground - no Slotlist created", "Debug");
					//}
					//else {

						for (auto& ac : obj.value().items()) {
							this->LogDebugMessage("Aircraft read: " + ac.key(), "Debug");

							aircraft_entry tempAircraft{
								ac.value()["callsign"],
								ac.value()["clearance_state"],
								ac.value()["pushback_state"],
								ac.value()["taxi_state"],
								ac.value()["clearance_time"],
								ac.value()["pushback_time"],
								ac.value()["taxi_time"]
							};
							aclist.entries.push_back(tempAircraft);
							this->LogDebugMessage("Aircraft added to list: " + ac.key(), "Debug");

						}
					//}
				}
				// Slot List
				/*if (obj.key() == "slotlist") {
					for (auto& sl : obj.value().items()) {
						this->LogDebugMessage("Start reading slot data...", "Debug");
						auto aciter = aclist.entries.begin();

						if (sl.value()["callsign"] == aciter->callsign) {

							tm ptm;

							time_t rawtime = sl.value()["timestamp_slot"];

							gmtime_s(&ptm, &rawtime);

							slot_entry tempSlot{
								sl.value()["nr"],
								rawtime,
								ptm.tm_year,
								ptm.tm_mon,
								ptm.tm_mday,
								ptm.tm_hour,
								ptm.tm_min,
								ptm.tm_sec,
								sl.value()["callsign"]
							};

							this->LogDebugMessage("Slot: " + sl.key(), "Debug");
							this->LogDebugMessage("CS: " + sl.value()["callsign"], "Debug");
							this->LogDebugMessage("Time: " + std::to_string(ptm.tm_hour) + ":" + std::to_string(ptm.tm_hour), "Debug");
						}
					}
				}*/
		
		}
	}
	catch (std::exception e)
	{
		this->LogMessage("Failed to parse Json Data, Error: " + std::string(e.what()), "Error");
		return;
	}
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