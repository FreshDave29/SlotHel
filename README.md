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
![Load SlotHel](https://imgur.com/a/4EWPANh)
`SlotHel` will also confirm successful initialisation by logging its version to the **Messages** chat:
`[08:34:10] SlotHel: Version 0.0.1 loaded.`
5. Close the plugin dialog and open the departure list columns setup dialog (small **S** at the left side of your departure list)

6. (*Optional*) Add the **SlotHel** column to your departure list by clicking **Add Item** and selecting the `SlotHel / Flightplan Validation` **Tag Item type**. Pick a **Header name** and set a **Width** of 4 or greater. This column will display warnings and the status of each flightplan processed by DelHel, but is not strictly required for the plugin to function
7. Assign the `DelHel / Process FPL` action as the **Left button** or **Right button** action of any of your tag items as desired. Triggering this function processes the selected flightplan using the default settings of `DelHel` (described in more detail in the [Process FPL](#process-fpl) section below)
8. (*Optional*) Assign the `DelHel / Validation menu` action as the **Left button** or **Right button** action of any of your tag items as desired. Triggering this function opens the flightplan validation menu, allowing for more fine-grained processing of the selected flightplan (described in more detail in the [Validation Menu](#validation-menu) section below)
9. Close the departure list settings by clicking **OK**