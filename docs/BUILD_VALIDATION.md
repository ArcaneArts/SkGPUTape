# Prototype build validation — 2026-09-17

- DLL source commit: `e6b7e3a`.
- [Windows build](https://github.com/ArcaneArts/SkGPUTape/actions/runs/35219136683): Release and Debug builds passed, Windows discovery tests passed, CPack and artifact upload passed.
- Native macOS discovery tests also passed.
- Inspected Release binary: PE32+ x86-64 DLL, exports `SKSEPlugin_Load`, `SKSEPlugin_Query`, `SKSEPlugin_Version`.
- Imports: KERNEL32, ole32, VERSION, USER32, SHELL32; no separately shipped C++ runtime DLL required by the plugin's import table.
- Locally delivered ZIP moves the two documentation files into `SkGPUTape/docs` to match README links. DLL bytes are identical to the CI artifact. CMake installation paths are corrected for subsequent builds.
- ZIP integrity and required paths were checked. Local package SHA-256: `fbed7a20dc460426831f6ecfa4d359776c42f0bc524bcc2f03965d6c4f2ff065`.
- Skyrim execution, actual asset load success, rendering, DXVK pipelines and GPU residency remain **untested**. This binary implements Phase 1–2 only.

## 0.1.1 — 2026-09-17

- Source commit: `bdc24e7`.
- [Windows build](https://github.com/ArcaneArts/SkGPUTape/actions/runs/35221161178): Release and Debug passed.
- Runtime regression passed: synthetic 1.5.97, 1.6.1170, 1.7.99 and 1.7.104 databases; production runtime detection from a 1.7.104 PE version resource; correct ID branch and offset lookup.
- Discovery tests passed on Windows and macOS.
- Release ZIP downloaded directly from CI and integrity/layout checked; x64 DLL and all three SKSE exports verified. Test fixtures are excluded from the ZIP.
- ZIP SHA-256: `0b3493b8a6742ee05a2b76b8391a51e429a884c8dfef14b23f8949c239ffb9da`.
- Actual Skyrim/MO2/CrossOver execution remains to be confirmed. This corrects runtime classification and does not establish full 1.7 engine ABI compatibility.
