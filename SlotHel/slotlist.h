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
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;

	std::string callsign;
};

struct slot_list {
	std::vector<slot_entry> entries;
};

