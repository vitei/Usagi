# Rendering Color Space Policy

This document records the intended color-space rules for the renderer before gamma-correction shader and display changes are made.

## Goals

- Lighting, material evaluation, bloom, tone mapping, and post effects operate in linear color.
- Color textures authored in sRGB are decoded to linear by the texture sampler.
- Data textures remain linear and are never sampled through an sRGB view.
- The final image is encoded exactly once for the display output.

## Current Format Inventory

### Texture Inputs

- `ColorFormat::SRGBA` maps to sRGB formats:
  - Vulkan: `VK_FORMAT_B8G8R8A8_SRGB`
  - OpenGL: `GL_SRGB8_ALPHA8`
- GLI compressed texture loading already preserves sRGB compressed formats:
  - DXT1, DXT3, DXT5, BC7 sRGB map to Vulkan sRGB block formats.
- `Texture::CreateRaw` callers currently pass explicit `ColorFormat` values. UI font textures use `ColorFormat::RGBA_8888` and should remain linear alpha/data unless the caller explicitly opts into sRGB.

### Linear Data Textures

These formats must remain linear:

- Normals: `ColorFormat::NORMAL`
- Depth and shadow data: `DepthFormat::*`, `ColorFormat::SHADOW`, linear depth targets
- Scalar/vector utility targets: `R_8`, `RG_8`, `R_16F`, `RG_16F`, `R_32F`, `R_32`, `RG_32F`
- Material data targets that pack non-color values, such as specular power or masks

### Render Targets

- HDR scene buffers use `ColorFormat::RGB_HDR` and are linear.
- Bloom buffers use `ColorFormat::RGB_HDR` and are linear.
- Deferred buffers are mixed:
  - Diffuse and emissive contain color values but are render targets, so they should store linear values.
  - Normal and specular buffers are data targets and must stay linear/data encoded.
- LDR post buffers currently use `ColorFormat::RGBA_8888`; they should be treated as linear intermediates until the final display encode.

### Display Output

- Vulkan swapchain selection currently accepts the surface-reported format/color space and maps it back through `GFXDevice_ps::GetUSGFormat`.
- `VK_COLORSPACE_SRGB_NONLINEAR_KHR` means the presentation engine expects sRGB/nonlinear display values.
- The final output path must perform exactly one linear-to-sRGB encode before presenting to an sRGB/nonlinear swapchain.

## Policy

1. Source asset textures that represent albedo/base color/UI color should use sRGB formats.
2. Source asset textures that represent normals, roughness, metallic, masks, lookup tables, height, depth, or IDs must use linear formats.
3. All render targets before final presentation are linear unless their name and creation site explicitly say they are an encoded display target.
4. Tone mapping converts HDR linear scene color to LDR linear display-referred color.
5. Gamma/sRGB encoding happens after tone mapping and after post effects that expect linear input.
6. Swapchain images are not treated as general scene render targets. They receive the final encoded image or a direct render pass that is known to write encoded values.

## Implementation Plan

1. Add small helper functions for format intent:
   - `IsSRGBFormat(ColorFormat)`
   - `IsLinearDataFormat(ColorFormat)`
   - `IsHDRFormat(ColorFormat)`
2. Add a display-output policy flag or helper on the Vulkan display path so code can tell whether the swapchain expects sRGB/nonlinear values.
3. Keep HDR, bloom, deferred, and LDR intermediate render targets linear.
4. Add a final output shader path that encodes linear color to sRGB when presenting to an sRGB/nonlinear swapchain.
5. Avoid changing texture import defaults until asset metadata or naming rules can distinguish albedo/base color from packed data.
6. Add a visual test scene with:
   - Linear grayscale ramp
   - sRGB albedo texture
   - Normal/data texture sample
   - HDR bloom/tone-map sample

## First Safe Code Slice

Add format-intent helpers and use them in debug assertions or diagnostics first. This creates a shared vocabulary without changing rendering output.

After that, wire display-output encoding in the post-processing/final blit path and keep direct swapchain rendering opt-in until its shaders are audited.
