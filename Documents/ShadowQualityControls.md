# Shadow Quality Controls

This document records the current shadow quality knobs and the first migration
targets for making them explicit.

## Presets

`LightMgr::QualitySettings` exposes named presets:

- `Lightweight()` keeps directional shadows enabled at `1024 x 1024`, keeps
  local shadow casting disabled by default, and uses the cheapest directional
  filter radius.
- `Default()` returns the legacy-equivalent settings: directional shadows at
  `2048 x 2048`, local shadow maps allocated at `1024 x 1024` when local shadow
  casting is enabled by the caller, and filter quality `1`.
- `ModernHardware()` opts into directional shadows at `4096 x 4096`, enables
  spot and point shadow casting, allocates new local shadow maps at
  `2048 x 2048`, and uses filter quality `2`.

## Directional Shadows

- Resolution is selected through `LightMgr::QualitySettings::uShadowQuality`.
  The current map in `Engine/Graphics/Lights/LightMgr.cpp` is:
  - `0`: 1024
  - `1`: 1536
  - `2`: 2048
  - `3`: 4096
- Each shadowed directional light uses four cascades
  (`ShadowCascade::CASCADE_COUNT`).
- Cascade split distances are currently hard-coded in
  `Engine/Graphics/Shadows/ShadowCascade.cpp` as `30, 160, 400, 1000`, then
  scaled by resolution.
- Directional filter radius is selected through
  `LightMgr::QualitySettings::uShadowFilterQuality`.
  - `0`: low filter radius for cheaper, sharper cascades
  - `1`: legacy-equivalent filter radius and default
  - `2`: wider filter radius
  - `3`: widest filter radius
- Bias values remain hard-coded in `ShadowCascade.cpp`.

## Local Shadows

- Point, spot, and projection lights use
  `LightMgr::QualitySettings::uLocalShadowQuality`.
- The local shadow quality map currently matches the directional resolution map:
  - `0`: 1024
  - `1`: 1536
  - `2`: 2048
  - `3`: 4096
- The default local shadow quality is `0`, preserving the previous `1024 x 1024`
  behavior for newly allocated local shadow maps.
- `SpotLight` and `ProjectionLight` allocate projection shadows at this
  resolution during initialization.
- Projection-shadow bias is hard-coded in
  `Engine/Graphics/Shadows/ProjectionShadow.cpp`.
- Spot-light culling currently uses a conservative sphere based on `m_fFar`.

## Point Shadows

- Point-light cube shadows are allocated at the local shadow quality resolution.
- Deferred point-light shadow sampling uses the same 12-tap Poisson pattern as
  the other shadow paths.

## Shader Sampling

- `Data/GLSL/shaders/includes/shadow/poisson_values.inc` defines 12 Poisson
  offsets.
- Directional, projection, and point shadow readers all use this fixed sample
  pattern.

## Hard Limits

- Forward lighting exposes `MAX_LIGHTS = 8` and `MAX_CASCADE_SETS = 2` in both
  CPU constants and shader constants.
- Additional shadowed directional lights beyond two cascade sets are dropped by
  the lighting constant upload path.
- Directional cascade texture layers can be allocated for more lights than the
  shader constant contract can address.

## Next Implementation Targets

1. Move directional split distances and bias into a small quality settings
   structure.
2. Add resize support for existing pooled local lights when quality changes;
   newly allocated local lights already use the selected local shadow quality.
3. Keep a low-cost profile for lightweight targets and add an explicit high
   profile for modern hardware.
4. Keep shader sample-count changes opt-in; changing the Poisson kernel affects
   every shadow path.
