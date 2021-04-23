#pragma once

#include <string>
#include <vector>
#include <time.h>

class aircraft_entry
{
public:
	std::string callsign;
	std::string st_clearance;	// status clearance
	std::string st_pushback;	// status pushback
	std::string st_taxi;		// status taxi
	std::string st_aircraft;	// status aircraft
	int t_clearance;	// time clearance
	int t_pushback;		// time pushback
	int t_taxi;			// time taxi
};

struct aircraft_list {
	std::vector<aircraft_entry> entries;
};

class slot_entry 
{
public:
	int number;
	time_t timestamp_raw;
	int ctot_hour;
	int ctot_minute;

	time_t tsat_raw;
	int tsat_hour;
	int tsat_minutes;

	std::string callsign;
	std::string str_ctot;
	std::string str_tsat;
};

struct slot_list {
	std::vector<slot_entry> entries;
};

struct slot_tag {
	std::string tag;
	COLORREF color;
};

