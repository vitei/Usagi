# Rendering Color Space Policy

This note records the current rendering color-space expectations on the
v0.3-Development HDR path. It is a validation aid, not a request to change
swap-chain behavior.

## Policy

- Lighting, material evaluation, bloom, tone mapping, and post effects operate
  on linear color unless a shader or target explicitly names an encoded output.
- Albedo, UI color, and other authored display-color textures should use sRGB
  texture formats so sampling decodes them to linear.
- Normal, roughness, metallic, mask, lookup, depth, ID, and other data textures
  must stay in linear/data formats.
- HDR scene and bloom buffers remain linear HDR intermediates.
- LDR post-process buffers are treated as linear intermediates until a final
  output conversion shader writes the display-referred representation.
- Alex's HDR output shaders (`LinearToHDR`, `LinearToST2084`, and
  `LinearToExtended`) own final HDR/display conversion. Tests in this PR should
  validate those files and shader packaging without replacing or bypassing the
  HDR swap-chain path.

## Validation

- `Data/GLSL/shaders/includes/colorspace.inc` should continue to provide the
  standard sRGB transfer constants for UI/text and other SDR conversion users.
- `Data/GLSL/effects/PostProcess.yml` should continue to package the HDR output
  effects used by the current renderer.
- Rendering tests should prefer package/build smoke checks and small math
  invariants before touching runtime rendering code.
