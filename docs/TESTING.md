# First in-game experiment

This 0.1.1 build implements Phase 1 and Phase 2 only. It does not attach objects to a scene, issue draws, or guarantee GPU residency. Actual game validation is pending.

1. Install SKSE64 matching your Skyrim executable and the matching SE/AE Address Library for SKSE Plugins. VR is excluded.
2. Import `SkGPUTape-0.1.1-win64.zip` into MO2/Vortex. The archive root is Skyrim's Data directory: `SKSE/Plugins/SkGPUTape.dll`, INI and model list. No ESP is needed. Launch via SKSE.
3. Use a test save in a quiet interior. Load an existing save (new-game automatic loading is not implemented). Close menus to allow batches to run.
4. Look for the start/completion notifications and `Documents/My Games/Skyrim Special Edition/SKSE/SkGPUTape.log` inside the CrossOver bottle's Windows user profile. GOG uses its corresponding game documents folder.
5. Verify the initialized runtime line, successful PostLoadGame, list counts, individual Demand results, completion duration and retained-root count. Included paths are starter candidates, not verified against your installation. If a candidate is missing, replace it with a known installed static NIF; engine resolution handles loose/BSA content.
6. Reload the save during loading; the old pass must stop, roots release, and a fresh pass start. Return to main menu during loading; there must be no further batch demands. An empty/missing list should report the condition without crashing.
7. Compare `Enabled=false`, `RetainModels=false`, and `RetainModels=true` on the same route after fully restarting Skyrim each time. INI is re-read on each successful save load. Record runtime, SKSE version, CrossOver/DXVK versions, list, log, memory and frame-time spikes. Set `DXVK_HUD=fps,pipelines` in the game's launch environment if that DXVK build supports these HUD fields.

Do not interpret a successful Demand as a rendered mesh. Pipeline counts may change from normal gameplay and do not prove a warmup draw. Visible proof and a controlled rendering-only comparison are the next milestone.

Default batch size is 1, interval 100 ms, maximum 50 models. These are pacing parameters, not a bound on the time one engine load can take. Invalid native NIFs can still crash Skyrim; no access-violation swallowing is used. Verbose mode flushes the path before Demand to help locate the last attempted asset. Avoid actors, armor, skeletons and effects in the initial list.

Retention is intentionally process-local and is cleared on pre-load, new game or main-menu opening. The manager lives for the process lifetime to avoid releasing engine references from CRT shutdown after the engine is gone. There are no save-game records and uninstalling the DLL/INI/list removes the plugin.
