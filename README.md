# SlotHel
`SlotHel` integrates the VACC-Austria Web-based Slotmanagement into EuroScope by reading the provided JSON.

Each Aircraft and Slot are read in, saved and continuously refreshed. The slot data (TSAT & CTOT) for each callsign are provided into a EuroScope DepartureList column.
For better slot handling, the times are marked by colors depending on current UTC time (too early, on time, too late).

## Table of Contents

- [Getting started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
  - [Basics](#basics)
  - [Tag items](#tag-items)
  - [Tag functions](#tag-functions)
  - [Chat commands](#chat-commands)
- [Contributing](#contributing)
  - [Development setup](#development-setup)
- [License](#license)

## Getting started

### Prerequisites

Since `SlotHel` was developed as an EuroScope plugin, it requires a working installation [EuroScope](https://euroscope.hu/). The initial development was started using EuroScope version [`v3.2.1.26`](https://www.euroscope.hu/wp/2021/02/07/v3-2-1-26/), although the plugin should most likely also work fine with previous and later versions. As development continues, compatibility to the latest **beta** versions of EuroScope will be maintained as long as possible and feasible.

### Installation

1. Download the latest release (`SlotHel.zip`) of `SlotHel` from the [**Releases**](https://github.com/FreshDave29/SlotHel/releases/latest) section of this repository
2. Extract `SlotHel.dll` and place int somewhere inside your EuroScope sectorfile/profile setup, where other plugins are already set up
3. Start EuroScope and open the **Plug-ins** dialog in the settings menu (**OTHER SET**)
![Plug-ins dialog](https://i.imgur.com/SrVtRp9.png)
4. **Load** the plugin by selecting the `SlotHel.dll` you extracted and ensure the proper version is displayed

`SlotHel` will also confirm successful initialisation by logging its version to the **Messages** chat:
`[08:34:10] SlotHel: Version 0.0.1 loaded.`
5. Close the plugin dialog and open the departure list columns setup dialog (small **S** at the left side of your departure list)

6. Add the **SlotHel** column to your departure list by clicking **Add Item** and selecting the `SlotHel / Slot` **Tag Item type**. Pick a **Header name** and set a **Width** of 12 or greater. This column will visualize the slot and show corresponding colors for warnings.
7. Assign the `SlotHel / SlotMenu` action as the **Left button** or **Right button** action of any of your tag items as desired. Triggering this function will refresh the slot data from the web.
8. Close the departure list settings by clicking **OK**


## Contributing

If you have a suggestion for the project or encountered an error, please open an [issue](https://github.com/FreshDave29/SlotHel/issues) on GitHub. Please provide a summary of your idea or problem, optionally with some logs or screenshots and ways to replicate for the latter.  
The current development state as well as a rough collection of ideas can also be found in the `SlotHel` [project board](https://github.com/FreshDave29/SlotHel/projects/1) on GitHub.

[Pull requests](https://github.com/FreshDave29/SlotHel/pulls) are highly welcome, feel free to extend the plugin's functionality as you see fit and submit a request to this repository to make your changes available to everyone. 
Please keep in mind this plugin attempts to provide features in a relatively generic way so it can be used by vACCs with different needs - try refraining from "hard-coding" any features that might just apply to a specific airport or vACC.

### Development setup

`DelHel` currently has no external development dependencies aside [Visual Studio](https://visualstudio.microsoft.com/vs/). Initial development started using Visual Studio 2019, although later versions should most likely remain compatible.

To allow for debugging, the project has been configured to launch EuroScope as its debug command. Since your installation path of EuroScope will most likely be different, you **must** set an environment variable `EUROSCOPE_ROOT` to the **directory** EuroScope is installed in (**not** the actual `EuroScope.exe` executable), for instance `E:\EuroScope`.  
Note: triggering a breakpoint seems to cause both EuroScope and Visual Studio to freak out, resulting in high resource usage and slugging mouse movements, thus only being of limited usefulnes. **NEVER** debug your EuroScope plugin using a live connection as halting EuroScope apparently messes with the VATSIM data feed under certain circumstances.

`DelHel` is compiled using Windows SDK Version 10.0 with a platform toolset for Visual Studio 2019 (v142) using the ISO C++17 Standard.

This repository contains all external dependencies used by the project in their respective `include` and `lib` folders:

- `EuroScope`: EuroScope plugin library
- `nlohmann/json`: [JSON for Modern C++](https://github.com/nlohmann/json/) ([v3.9.1](https://github.com/nlohmann/json/releases/tag/v3.9.1), [MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)), used for parsing the airport config JSON
- `libcurl`: [Library for HTTP Requests in C++](https://curl.se/) ([v7.76.1](https://curl.se/download.html)), Open Source, used for requesting the JSON file from Webserver

## License

[MIT License](LICENSE)