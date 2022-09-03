#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <WinInet.h>

#include "constants.h"
#include "Euroscope/EuroScopePlugIn.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;


std::string FetchLatestVersion();


class slothelexception : public std::exception
{
public:
	explicit slothelexception(std::string& what) : std::exception{ what.c_str() } {}
	virtual inline const long icon() const = 0;
	inline void whatMessageBox()
	{
		MessageBox(NULL, this->what(), PLUGIN_NAME, MB_OK | icon());
	}
};

class error : public slothelexception
{
public:
	explicit error(std::string& what) : slothelexception{ what } {}
	inline const long icon() const
	{
		return MB_ICONERROR;
	}
};

class warning : public slothelexception
{
public:
	explicit warning(std::string& what) : slothelexception{ what } {}
	inline const long icon() const
	{
		return MB_ICONWARNING;
	}
};

class information : public slothelexception
{
public:
	explicit information(std::string& what) : slothelexception{ what } {}
	inline const long icon() const
	{
		return MB_ICONINFORMATION;
	}
};

inline std::string GetPluginDirectory()
{
	char buf[MAX_PATH] = { 0 };
	GetModuleFileName(HINSTANCE(&__ImageBase), buf, MAX_PATH);

	std::string::size_type pos = std::string(buf).find_last_of("\\/");

	return std::string(buf).substr(0, pos);
}

inline std::vector<std::string> split(const std::string& s, char delim = ' ')
{
	std::istringstream ss(s);
	std::string item;
	std::vector<std::string> res;

	while (std::getline(ss, item, delim)) {
		res.push_back(item);
	}

	return res;
}

inline std::string join(const std::vector<std::string>& s, const char delim = ' ')
{
	std::ostringstream ss;
	std::copy(s.begin(), s.end(), std::ostream_iterator<std::string>(ss, &delim));
	return ss.str();
}

inline bool starts_with(const std::string& str, const std::string& pre)
{
	return str.rfind(pre, 0) == 0;
}

inline void to_upper(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) -> unsigned char { return std::toupper(c); });
}

inline int round_to_closest(int num, int closest)
{
	return ((num + closest / 2) / closest) * closest;
}