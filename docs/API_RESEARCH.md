# Engine API research and next render experiment

Inspected source: [CommonLibSSE-NG b93280e832f263dbef44e44cbe2936622a02f91a](https://github.com/CharmedBaryon/CommonLibSSE-NG/tree/b93280e832f263dbef44e44cbe2936622a02f91a). This is the pinned upstream default-branch revision used by the build, not an inferred signature from an old example.

## Implemented loader

`include/RE/B/BSModelDB.h` and `src/RE/B/BSModelDB.cpp` expose:

```cpp
BSResource::ErrorCode Demand(const char*, NiPointer<NiNode>&, const DBTraits::ArgsType&);
```

The wrapper calls relocation IDs 74040 / 75782. `DBTraits::ArgsType` defaults to LODmult=0, texLoadLevel=3, unk8=true, unk9=false, unkA=true, postProcess=true. The prototype preserves these defaults rather than guessing meanings for unknown fields. Models are requested relative to the meshes prefix (also used by [Community Shaders grass mesh loading](https://github.com/community-shaders/skyrim-community-shaders/blob/main/src/Features/GrassOptimizations/GrassMeshLibrary.cpp)); canonical list paths keep `meshes\` for logging. Engine load success and a non-null root are both required. Logical engine lookup handles archive/loose overrides; no filesystem existence gate prevents a BSA resource from loading.

`NiPointer` increments/decrements intrusive references. `NiNode` children retain scene objects; `BSGeometry` exposes properties and rendererData. `BSLightingShaderMaterialBase` has NiPointer fields for diffuse, normal and other texture objects and its texture set. `NiSourceTexture::rendererTexture` and geometry rendererData are raw renderer pointers. Keeping a root alive therefore preserves an engine ownership graph, but neither these headers nor Demand establish eager resource creation, draw submission, complete texture retention, or driver residency. No byte estimate or rendered count is fabricated.

## Phase 3 recommendation (not implemented)

The least invasive candidate is an existing active inventory/menu preview scene, with one cloned static clutter NIF and explicit visible inspection. This is a candidate requiring game validation, not a proven safe arbitrary-model renderer.

Verified bindings in `UI3DSceneManager.h/.cpp`:

- `AttachChild(NiAVObject*)` and `AttachChild(NiAVObject*, INTERFACE_LIGHT_SCHEME)`
- `DetachChild(NiAVObject*)`
- camera position/rotation/FOV setters, camera and scene-node fields
- `NiAVObject::Clone()` and `Update(NiUpdateData&)`

Proposed experiment: while a chosen 3D menu is actually rendering, retain the original loaded root, clone a single static model, place and scale the clone within the observed preview camera frustum using its bounds, update transforms, attach it under the inventory light scheme, visually confirm it, then detach on menu close/cancellation before releasing the clone. Keep original roots for the retention experiment. Do not mutate cached shared roots or move objects out of view. Avoid replacing global camera state; if modification is necessary, capture and restore it on every exit path.

Still unresolved: which menu owns and submits the preview on each runtime; culling/bounds behavior for arbitrary scales; clone/update ordering; safe detachment relative to renderer traversal; how to count actual rendered frames; and model-specific shader initialization. An actual render callback or GPU capture must establish submission. Timer ticks are not rendered frames. A UI preview may exercise different shaders/lighting than world rendering, so it cannot cover all gameplay pipelines.

## Deferred investigations

`BSGraphics::Renderer` exposes a D3D11 device/context and renderer state, and CommonLib includes camera, shader accumulator and render-target types. These are not a ready-made safe arbitrary-NIF offscreen render function. Offscreen rendering needs a complete state/target restoration and engine-pass design before implementation.

Resource headers include `BSResource::Location`, `LocationTraverser`, `GlobalLocations`, `Archive` and `LooseFileLocation`. `Location::DoTraversePrefix(const char*, LocationTraverser&)` and `LocationTraverser::ProcessName(const char*, Location&)` are declared, but the exposed `GlobalLocations` class has no singleton getter. These declarations alone do not establish a safe entry point or complete loaded-BSA enumeration. Investigate traversal and archive name availability before selecting discovery implementation. Manual-list mode deliberately avoids inventing an archive enumerator.

Skinned/controller-heavy assets and full shader-graph retention remain separate experiments. No direct DXVK hook, NIF parser or DDS parser is included.

## Starter list provenance

The ten paths were cross-checked against the model fields in these Skyrim.esm overrides in the Requiem source repository. This establishes concrete paths, not load success or render safety in the user's game. No game assets are distributed.

- [Clutter\Dining Set\BasicTankard01.nif](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/BasicTankard01%20-%200319E3_Skyrim.esm.yaml)
- [Clutter\Basket01.NIF](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/Basket01%20-%20012FE7_Skyrim.esm.yaml)
- [Clutter\Basket02.NIF](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/Basket02%20-%20012FE8_Skyrim.esm.yaml)
- [Clutter\Basket03.NIF](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/Basket03%20-%20012FE9_Skyrim.esm.yaml)
- [Clutter\Basket04.NIF](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/Basket04%20-%20012FEA_Skyrim.esm.yaml)
- [Clutter\Basket05.NIF](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/Basket05%20-%20012FEB_Skyrim.esm.yaml)
- [Clutter\Common\Broom.nif](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/Broom01%20-%2006717F_Skyrim.esm.yaml)
- [Clutter\Bucket01.NIF](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/Bucket01%20-%20012FDF_Skyrim.esm.yaml)
- [Clutter\Kettle01.NIF](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/Kettle01%20-%20012FE6_Skyrim.esm.yaml)
- [Clutter\Kitchen\WoodenLadle01.nif](https://github.com/ProbablyManuel/requiem/blob/main/components/plugins/Requiem.esp/MiscItems/WoodenLadle01%20-%200319E5_Skyrim.esm.yaml)
