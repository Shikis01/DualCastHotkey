# DualCastHotkey

**What it does**

DualCastHotkey is a lightweight SKSE plugin that lets the player trigger a "dual-cast" with a single configurable keyboard shortcut. Press and hold the configured key to charge a dual-cast (both hands), then release to fire — replicating the game's normal charge/release behavior.

**Key behaviors**

- The hotkey only triggers when the player has *spells equipped in both hands* (not weapons or shields).
- Pressing the hotkey sends the game the simulated left/right/dual attack inputs to begin charging; releasing the hotkey sends the release events with the measured held duration so charging works as normal.
- The plugin is silent in-game (no console output).
- No perk is required by the plugin to use the hotkey — the plugin simply simulates the dual-cast input.

**Configuration (INI)**

Configuration is done via a simple INI file:

  Data\\SKSE\\Plugins\\DualCastHotkey.ini

Example contents (default):

  ; DualCastHotkey configuration file
  ; Set `Hotkey` to a single character (A-Z, 0-9), a common name (Space, Enter, F1..F12), or a numeric key code.
  ; Examples: Hotkey=H   Hotkey=Space   Hotkey=59
  ; Default: H

  Hotkey=H

- The plugin will create the INI with a header if it is missing.
- Supported formats: single letters (A..Z), numeric digits (0..9), common names ("Space", "Enter"), function keys ("F1".."F12"), or numeric codes.

**Requirements & Compatibility**

- Skyrim Special Edition (SSE) with SKSE64 installed (matching SKSE expectations).
- No SkyUI / MCM is required. The plugin intentionally uses a plain INI for configuration.
- Should be compatible with other mods that do not conflict with input events. Change the key outside the game if you experience conflicts or if it simply doesn't work.

**Files of interest**

- `plugin.cpp` — main plugin logic (input sink, config read/write, INI creation, event simulation). Though not in the files uploaded to nexusmods, I'll link a GitHub repo if requested.
- `Data\\SKSE\\Plugins\\DualCastHotkey.ini` — generated default configuration; edit to change hotkey.

**Development notes**

- The plugin previously included optional Papyrus/MCM support; that was removed in favor of a simple INI-based configuration because it was a bit complicated for such a small feature.
- The code includes helpful comments describing the functions and behavior.

**Limitations & Notes**

- The plugin simulates user events (left/right/dual attack) — behavior depends on game state and other mods.
- It does not check or enforce perk requirements; if you want perk-based gating, that behavior can be re-added. And changed.
---
Developed in January 2026 by Shikis01
Some files were made originally by @mrowrpurr on Github.
---
Thanks to @mrowrpurr on github for her fantastic tutorial on how to start modding with SKSE and C++
