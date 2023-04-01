#pragma once

#define PLUGIN_NAME    "SlotHel"
#define PLUGIN_VERSION "0.2.1"
#define PLUGIN_AUTHOR  "David Leitl"
#define PLUGIN_LICENSE "(c) 2023, MIT License"
#define PLUGIN_LATEST_VERSION_URL "https://raw.githubusercontent.com/FreshDave29/SlotHel/master/version.txt"
#define PLUGIN_LATEST_DOWNLOAD_URL "https://github.com/FreshDave29/SlotHel/releaseslatest"

const char SETTINGS_DELIMITER = '|';

const int TAG_ITEM_SLOT = 1;

const int TAG_FUNC_SLOT_MENU = 100;
const int TAG_FUNC_SLOT_LOAD = 101;

const std::string SLOT_SYSTEM_PATH = "https://www.vacc-austria.org/data/subsystem/slots/";

const COLORREF TAG_COLOR_NONE = 0;
const COLORREF TAG_COLOR_RED = RGB(200, 0, 0);
const COLORREF TAG_COLOR_ORANGE = RGB(255, 165, 0);
const COLORREF TAG_COLOR_GREEN = RGB(0, 200, 0);
