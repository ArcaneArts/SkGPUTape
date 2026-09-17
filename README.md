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

## Prototype 0.1.1

Implemented: Phase 1–2 plugin, successful PostLoadGame handling, manual list normalization/deduplication, timer-paced game-thread `BSModelDB::Demand`, strong model-root retention, cancellation and progress logging. **This version is loading-only: it does not yet render the models or demonstrate GPU warmup.** See [API research](docs/API_RESEARCH.md) for the verified bindings and proposed visible-render path.

### Install and test

Import `SkGPUTape-0.1.1-win64.zip` from the Windows build artifact into your mod manager. Its `SKSE` folder belongs inside Skyrim's `Data` directory. Requires matching SKSE64 and Address Library for your SE/AE executable; VR is excluded. Edit `SKSE/Plugins/SkGPUTapeModels.txt`, launch via SKSE and load an existing save. The ten bundled clutter paths are starter candidates; check the log for availability in your installation. See [testing instructions](docs/TESTING.md) for log location and comparisons.

### Build on Windows

Install Visual Studio 2022 with Desktop development with C++, CMake 3.25+ and Git. Clone/bootstrap vcpkg and set `VCPKG_ROOT` to its directory, then from a developer PowerShell:

```powershell
cmake --preset windows
cmake --build --preset release --parallel 3
cmake --build --preset debug --parallel 3
cd build/windows
cpack -C Release
```

CMake fetches pinned CommonLibSSE-NG; vcpkg provides pinned-baseline spdlog/rapidcsv with static runtime linkage. Release packaging includes DLL, INI, model list and docs. GitHub Actions runs both configurations and uploads the mod ZIP (download and extract the artifact wrapper to find the installable ZIP).

Portable discovery checks, including on macOS:

```sh
cmake -S . -B build/tests -DSKGPUTAPE_CORE_TESTS_ONLY=ON
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

Configuration is re-read after each successful save load. Start with `ModelsPerBatch=1`; `BatchIntervalMs` spaces dispatches but cannot cap the duration of an individual engine call. `MaxModels=0` removes the list cap. `RetainModels=false` provides a loading-only retention comparison. All options in the shipped INI are implemented; automatic discovery and `WarmFrames` are deliberately deferred.

### 0.1.1 runtime-loader correction

A local patch to the pinned CommonLib revision classifies Skyrim 1.7.x as AE,
selecting `versionlib-1-7-104-0.bin`, format 2, and AE relocation IDs. The previous
build incorrectly classified 1.7 as legacy SE. Missing-file errors now include
the requested path. Address Library files must keep their original filenames.

Windows regression tests exercise 1.5.97, 1.6.1170, 1.7.99 and 1.7.104 with
synthetic databases, plus a PE executable carrying a real 1.7.104 version resource.
These test runtime detection and lookup, not Skyrim engine ABI compatibility or
in-game model loading. Actual 1.7.104 execution remains to be validated in-game.
