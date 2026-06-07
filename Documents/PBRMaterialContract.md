# PBR Material Contract

This document defines the first physically based material contract for the
model renderer. It is intentionally additive: existing FBXDefault materials keep
their diffuse/specular/specular-power behavior until a material or effect opts
into the PBR path.

## Current Material Model

Model materials are defined through `Data/GLSL/effects/Model.yml`.
The packed material format is generic rather than PBR-specific:
`Engine/Graphics/Materials/Material.proto` stores texture slots, texture
coordinators, render passes, constant indexes, and raw constant data.

Existing texture hints:
- `DiffuseColor`: base surface color in the current legacy path.
- `NormalMap`: tangent-space normal map, enabled by `HAS_BUMP`.
- `EmissiveFactor`: emissive color texture.
- `SpecularColor`: legacy specular color.
- `Reflection`: legacy cubemap reflection source.

Existing pixel constants in `uMaterial`:
- `emission`: additive emissive color.
- `diffuse`: legacy diffuse color multiplier.
- `specular`: legacy specular color multiplier.
- `reflectionfactor`: legacy cubemap contribution.
- `specularpow`: Blinn/Phong gloss exponent.
- `alpha`: material opacity.
- `bDiffuseMap`, `bBumpMap`, `bSpecMap`, `bEmissiveMap`, `bReflectionMap`: legacy feature flags.

`Data/GLSL/shaders/models/fbxdefault.frag` samples these values, writes
legacy diffuse/specular data to the G-buffer through
`Data/GLSL/shaders/includes/deferred.inc`, or shades forward through
`Data/GLSL/shaders/includes/forward_lighting.inc`.

FBX import currently maps FBX properties to shader sampler hints through
`Tools/Source/Ayataka/fbx/FbxLoad.cpp`, and material override YAML can already
replace effects, defines, textures by hint, and constants by set/name.

## Minimum PBR Fields

The first PBR path should support these fields:

- `BaseColor`: sRGB RGBA texture plus `baseColorFactor`.
- `NormalMap`: existing tangent-space normal map. The existing hint can be
  reused.
- `MetallicRoughness` or `ORM`: linear packed texture. Prefer glTF-style
  `ORM` for new content with `r = occlusion`, `g = roughness`, `b = metallic`;
  use `MetallicRoughness` (`g = roughness`, `b = metallic`) if no occlusion is
  present. Add scalar `metallicFactor` and `roughnessFactor`.
- `EmissiveFactor`: sRGB emissive texture plus `emissiveFactor`. The existing
  hint can be reused.
- `Occlusion`: linear ambient-occlusion texture, preferably `r` channel.
- `Alpha`: keep the existing `alpha` constant and blend/test state.

Initial defaults:
- `baseColorFactor = vec4(1, 1, 1, 1)`
- `metallicFactor = 0`
- `roughnessFactor = 0.5`
- `emissiveFactor = vec3(0, 0, 0)`
- `normalScale = 1`
- `occlusionStrength = 1`
- `alphaCutoff = 0.5`

## Texture Color Space

- `BaseColor` and `EmissiveFactor` are source-color textures and should use
  sRGB decode.
- `NormalMap`, `MetallicRoughness`, `ORM`, and `Occlusion` are data textures
  and must stay linear.
- Lighting, BRDF evaluation, bloom, tone/post, and G-buffer values remain in
  linear space. Final display encoding is handled by the display color transform.

## Migration Plan

1. Add a new opt-in model effect or define, for example `PBR_MATERIAL`, rather
   than changing `FBXDefault` behavior in place.
2. Reuse `Material.proto` for the first pass; the existing texture slots and
   raw constant data are enough for the minimum PBR contract.
3. Add PBR texture hints and constants to `Model.yml` while preserving existing
   legacy fields.
4. Teach Ayataka FBX import to map modern/PBR FBX properties when present.
   Fallback conversion can use `baseColor = diffuse`, `metallic = 0`, and a
   roughness approximation derived from shininess/specular power.
5. Use existing material overrides as the manual migration path: old assets can
   opt into the new effect and set `BaseColor`, `ORM`/`MetallicRoughness`, and
   scalar factors without changing binary model structure.
6. Add shared BRDF helpers for GGX distribution, Smith masking, and Schlick
   Fresnel. Use the same helper in forward and deferred paths.
7. Update deferred packing only after the BRDF contract is stable. The current
   G-buffer stores diffuse RGB plus `specularpow / 128` and a separate specular
   RGB target, which is not an ideal roughness/metallic layout.

## Compatibility Rules

- Materials without PBR fields render through the current legacy path.
- If a material opts into PBR but omits a texture, the scalar defaults above are
  used.
- Existing material animation and override names remain valid.
- Legacy `SpecularColor`, `Reflection`, and `specularpow` do not map directly to
  PBR. Importers may approximate roughness from `specularpow`, but authored PBR
  values should take precedence.

## First Implementation Slice

The safest first code change is a dormant PBR effect variant:
- Add PBR constants and texture hints to `Model.yml`.
- Add BRDF helper functions in a new include.
- Add a `PBR_MATERIAL` branch in `fbxdefault.frag` that is not enabled by
  default.
- Keep deferred G-buffer packing unchanged until a focused G-buffer migration
  can be tested.
