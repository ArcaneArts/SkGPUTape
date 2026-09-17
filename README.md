# SkGPUTape

Experimental SKSE64 / CommonLibSSE-NG plugin for Skyrim SE/AE, targeting exploration hitching under CrossOver + DXVK on macOS. The intended approach is to load assets through Skyrim, render them in batches, and retain strong references. Reading files into RAM is not the goal.

## Implementation plan

1. **Plugin and manual model loading:** Windows x64 DLL, SKSE logging, successful post-load event, INI configuration, normalized/deduplicated manual NIF list, bounded game-thread loading and retained engine references.
2. **Visible render proof:** verify current CommonLib headers, clone loaded models and expose one static mesh to an actual camera. Keep loading and rendering metrics separate. Attachment alone is not proof of a draw call.
3. **Batch rendering and measurement:** configurable exposure duration, detach temporary objects, retain roots, and compare loading-only against rendering with DXVK pipeline HUD and a repeatable exploration route.
4. **Hidden/offscreen strategy:** only after visible rendering is verified, investigate engine render targets or a covered scene. Off-camera placement is not sufficient.
5. **Discovery:** loose NIF enumeration, then loaded BSA enumeration respecting engine resource resolution. No custom NIF/DDS parser.
6. **Residency and coverage:** A/B retained references, measure memory and hitching, then consider skeletons, terrain, effects, texture-only assets and renderer resource retention.

Each implementation stage will be committed separately. First delivery should include a mod-manager ZIP containing a real Windows DLL, configuration and model list, plus reproducible Windows build automation. Native macOS compilation is not a substitute for a Windows SKSE build.

## First experiment

Start with a small explicit list of static clutter meshes in `Data/SKSE/Plugins/SkGPUTapeModels.txt`. After a successful save load, log discovered, queued, loaded, failed and retained counts. Use `DXVK_HUD=fps,pipelines` and compare a disabled run, loading-only run, and eventual visible-render run. Pipeline count alone does not prove complete asset coverage or persistent GPU residency.

## Constraints and open questions

- Verify engine API signatures against pinned CommonLibSSE-NG source before implementing calls.
- Engine loading and scene mutations stay on the game thread; discovery may be separated later.
- Root reference ownership keeps engine objects alive but does not guarantee driver-level residency.
- Engine failure return codes can be handled; malformed native assets may still crash Skyrim.
- Skinned/animated meshes need additional investigation before broad automatic discovery.
- Save transitions must cancel pending work and detach experimental objects.
- Actual Skyrim/CrossOver rendering and performance validation requires running the game; a successful build is not that validation.

Implementation and exact build/install instructions follow as the prototype is developed.
