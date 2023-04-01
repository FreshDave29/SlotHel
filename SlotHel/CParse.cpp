#include "slotlist.h"
#include "constants.h"
#include "helpers.h"
#include "Euroscope/EuroScopePlugIn.h"
#include "lohmann/json.hpp"

using json = nlohmann::json;

namespace fu {
	class Funct {

	public:
		std::string thread_debug;
		std::string thread_error;

	private:
		aircraft_list aclist{};
		slot_list sllist{};

		void ParseJson(json j);
		slot_tag ProcessFlightPlan(const EuroScopePlugIn::CFlightPlan& fp);

		void LogMessage(std::string, std::string);


	};

}



	void fu::Funct::ParseJson(json j) {

		try {
			this->aclist.entries.clear(); //reset all lists to read new data
			this->sllist.entries.clear(); //reset all lists to read new data


			for (auto& obj : j.items()) {

				// Aircraft List
				if (obj.key() == "aclist") {

					for (auto& [ac, ac2] : obj.value().items()) {
						this->LogMessage("Aircraft read: " + ac, "Debug");

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
						}
						catch (std::exception e)
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

									time_t rawtime_ctot = sl2.value<int>("timestamp_slot", 0);
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

									this->LogMessage("Slot: " + sl, "Debug");
									this->LogMessage("\tCS: " + sl2.value<std::string>("callsign", ""), "Debug");
									this->LogMessage("\tCTOT: " + tempSlot.str_ctot, "Debug");
									this->LogMessage("\tTSAT: " + tempSlot.str_tsat, "Debug");
								}

							}
						}
						else {
							//this->LogDebugMessage("Slot " + sl + " empty!", "Debug");
						}
					}
				}
				else {
					this->LogMessage("List skipped: " + obj.key(), "Debug");
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

	slot_tag fu::Funct::ProcessFlightPlan(const EuroScopePlugIn::CFlightPlan& fp) {

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
						if (strcmp(fp.GetGroundState(), "TAXI") == 0 || strcmp(fp.GetGroundState(), "DEPA") == 0) {
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

	void fu::Funct::LogMessage(std::string, std::string) {
		if (this->debug) {
			EuroScopePlugin DisplayUserMessage(PLUGIN_NAME, type.c_str(), message.c_str(), true, true, true, true, false);
		}
	}

