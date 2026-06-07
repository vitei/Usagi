# UI Layout Contract

This document defines the target contract for a Usagi-native UI/layout authoring
format. It is intended to replace legacy artist workflows that depended on
NintendoWare tooling, while keeping runtime data aligned with the systems Usagi
already has: protocol-buffer resources, texture/font resources, localized string
tables, and 2D overlay rendering.

The contract is deliberately source-format first. Runtime implementation can be
introduced incrementally, but new authored UI data should not require
NintendoWare files, NintendoWare binaries, or a converter that interprets them as
the canonical source.

## Audit Summary

Current runtime pieces:

- `Engine/Layout` is a text and string-table layer, not a full layout system.
  `StringTable` loads `VPB/<basename>_<region>_<language>.vpb` and maps key
  strings to localized text plus a named text style.
- `Engine/Layout/Fonts` renders UTF-8 text through MSDF/SDF font atlases. The
  runtime `Text` object supports font, text, position, color, gradient colors,
  background color, alignment flags, width limit, line spacing, character
  spacing, scale, tab width, and top-left-origin UV flipping.
- Text rendering has fixed first-pass limits: `MAX_CHARS = 200` and
  `MAX_LINES = 20` in `TextDrawer`. Width wrapping is simple space-based wrapping.
  Inline color and scale tags are parsed; ruby, bold, and shadow tags are present
  but effectively incomplete.
- `FontCreator` already consumes YAML and outputs the runtime pair the engine
  expects: a `.vpb` `FontDefinition` plus a `.ktx` texture atlas. Its source YAML
  fields are `SourceFont`, `FontName`, `LetterList`, `Size`, `Spacing`, `Bold`,
  `Underscore`, and `FixedWidth`, though bold and underscore are read but not
  materially implemented.
- `Engine/GUI` is an ImGui-backed debug/tool UI. It has useful concepts for
  editor controls and preview tooling, but it is immediate-mode and should not
  be treated as the authored game UI runtime. Supported tool widgets include
  windows, menus, tabs, text, buttons, texture buttons/images, sliders, integer
  and float inputs, check boxes, combo boxes, color edit, text input, and delayed
  tooltips.
- Localization is limited to regions `JP`, `US`, `EU` and languages
  `Japanese`, `English`, `French`, `Spanish`, `German`, `Italian`, and `Dutch`.
  Language codes exist as `ja`, `en`, `fr`, `es`, `de`, `it`, and `nl`.
- Existing `Data` contains no complete UI layout examples. It does contain
  texture assets, entity YAML, shader/effect YAML, and `Data/GLSL/effects/Text.yml`.
  The data pipeline already uses YAML plus ERB and `yml2vpb.rb` for protobuf
  resource output.

Implication: the native UI format should compile into normal Usagi resources
rather than being interpreted directly from artist files at runtime. Text should
reuse existing font and localization contracts. Images, panels, hit regions, and
stateful widgets need a small runtime schema because no equivalent authored
layout runtime exists today.

## Runtime Concepts

The minimum runtime layout model should be a retained hierarchy of nodes under a
layout asset. Each node resolves to draw commands, hit regions, or both.

Required concepts:

- `LayoutAsset`: named resource containing canvas metadata, style references,
  node tree, state definitions, and optional animation timelines.
- `Canvas`: logical authoring size, safe-area policy, scaling mode, and origin.
  The initial runtime should use pixel units in a top-left origin because both
  ImGui overlay rendering and common 2D authoring tools work that way.
- `Node`: common base with stable `id`, optional `name`, `type`, visibility,
  enabled state, transform, anchors, pivot, size, margins, z/order, opacity,
  inherited color multiplier, and optional state variants.
- `Group`: transform/container only. It should not imply clipping unless a
  `clip` property is present.
- `Image`: textured quad with resource path, tint, UV rectangle, sampler mode,
  and blend mode.
- `NineSlice`: textured panel with border in source pixels, optional center fill,
  and the same tint/blend properties as `Image`.
- `Text`: localized or literal string bound to a font style. It should map to
  `usg::Text` capabilities: font, char height/scale, char spacing, line spacing,
  width limit, tab width, color, background color, gradient colors, and alignment.
- `HitArea`: rectangular input target with action/event name and input priority.
  Hit areas can be attached to visual nodes or stand alone.
- `Widget`: named composition pattern for common controls such as button,
  toggle, slider, list item, and dialog. Widgets are authored as node trees plus
  states, not as hard-coded ImGui controls.
- `State`: named visual/input variant such as `default`, `focused`, `pressed`,
  `disabled`, `checked`, `hidden`, or project-specific names.
- `Timeline`: optional keyframed animation of node properties. First pass should
  support opacity, position, scale, rotation, color, UV, and visibility only.
- `Binding`: external value reference for text, visibility, numeric bars, or
  selected state. Runtime binding names must be stable strings hashed by the
  usual Usagi string CRC path.

Initial coordinate contract:

- Units are logical pixels at the declared canvas size.
- Origin is top-left.
- Positive X goes right; positive Y goes down.
- Rotation is degrees clockwise around the node pivot.
- Z/order is resolved by pre-order traversal plus optional integer `order`.
- Anchors are normalized parent-relative values `{min: [x, y], max: [x, y]}`.
- Offset/margin values are pixels relative to resolved anchors.
- Text alignment should expose both line alignment and origin alignment so it can
  map to `TextEnums.h` flags.

Canvas scaling modes:

- `fit`: preserve aspect ratio, letterbox/pillarbox.
- `fill`: preserve aspect ratio, crop overflow.
- `stretch`: scale X/Y independently.
- `fixed`: no automatic scaling.
- `safe_fit`: preserve aspect ratio inside platform safe area.

## Source Format

The canonical source format should be YAML. JSON can be supported as a generated
or interchange format, but YAML fits existing Usagi data conventions and permits
comments, anchors, and ERB in the current pipeline.

Recommended source path:

```text
Data/UI/<layout-name>.ui.yml
```

Recommended schema:

```yaml
schema: usagi.ui.layout.v1
name: hud.main
canvas:
  size: [400, 240]
  origin: top_left
  scale: safe_fit
  safeArea: platform

resources:
  textures:
    hud_atlas: Textures/ui/hud_atlas
  fonts:
    body: Fonts/NotoSansBody
  stringTable: HUD

styles:
  text:
    hud_label:
      font: body
      charHeight: 14
      charSpacing: 0
      lineSpacing: 2
      displayLines: 1
      color: [255, 255, 255, 255]
      tabWidth: 2

nodes:
  - id: root
    type: group
    anchors: { min: [0, 0], max: [1, 1] }
    children:
      - id: health_panel
        type: nine_slice
        texture: hud_atlas
        rect: { pos: [12, 12], size: [132, 32] }
        uv: [0, 0, 64, 32]
        border: [8, 8, 8, 8]

      - id: health_label
        type: text
        text: { key: HUD_HEALTH }
        style: hud_label
        rect: { pos: [22, 19], size: [112, 18] }
        align: { line: left, origin: top_left }

      - id: pause_button
        type: widget.button
        action: pause.open
        rect: { pos: [360, 10], size: [28, 28] }
        states:
          default:
            image: { texture: hud_atlas, uv: [64, 0, 92, 28] }
          pressed:
            image: { texture: hud_atlas, uv: [92, 0, 120, 28] }
```

Field rules:

- `schema` is required and must be versioned.
- `name` is required and should be unique within a project.
- Every node must have a stable `id`; ids are the targets for animation,
  bindings, editor selection, and diff review.
- Resource references are logical Usagi resource names, not platform-specific
  output paths. For example `Textures/ui/hud_atlas`, not `_romfiles/...`.
- `text.key` references a localized key in the selected `stringTable`.
  `text.literal` is allowed only for non-shipping debug labels or generated
  layout previews.
- Colors are byte RGBA in source. Conversion may normalize to float colors in
  protobuf output.
- Texture `uv` values should be source pixels in YAML. Conversion normalizes
  them using the texture metadata so artists do not author fragile 0-1 values.
- Layout files may include project-specific metadata under `editor`, but runtime
  converters must ignore unknown `editor` fields.

Unsupported in v1 source:

- Direct NintendoWare layout imports as canonical source.
- Runtime script snippets embedded in layout files.
- Arbitrary shader/material definitions inside layout nodes.
- Complex text shaping promises beyond the glyphs and UTF-8 path the current
  font runtime can render.

## Conversion Outputs

The source converter should produce deterministic runtime data under the existing
data-build model.

Required outputs:

- `VPB/UI/<layout-name>.vpb`: compiled layout asset containing canvas, resource
  references, nodes, states, hit areas, bindings, and timeline data.
- `VPB/<string-table>_<region>_<language>.vpb`: localized string table output
  using the existing `KeystringTable` message when the layout source owns or
  updates strings.
- `Fonts/<font-name>.vpb` and `Fonts/<font-name>.ktx`: generated by
  `FontCreator` from project font source YAML.
- Texture outputs in the existing texture pipeline, referenced by logical names
  from the layout VPB.
- Dependency files for Ninja so changing a layout, included YAML, texture atlas,
  font definition, or localization file rebuilds the right runtime output.

Recommended new protobuf messages:

- `UILayout`: canvas, resource table, root node index, node array, state table,
  timeline table, binding table.
- `UINode`: id CRC, parent index, type enum, base transform/layout, style refs,
  payload union index, flags.
- `UIImagePayload`: texture resource index, normalized UVs, tint, sampler,
  blend.
- `UINineSlicePayload`: image payload plus normalized border values.
- `UITextPayload`: string key CRC or literal text, style index, align flags,
  width limit.
- `UIHitPayload`: action CRC/string, priority, enabled state mask.
- `UIStateOverride`: node id CRC plus property overrides.
- `UITimeline`: target node id CRC, channel enum, keyframes.

Conversion validation:

- Fail if a node id is duplicated.
- Fail if a resource alias is missing, unused only as a warning.
- Fail if text key lookup is missing for any shipping locale.
- Fail if a text style references a missing font.
- Fail if a texture UV rectangle is outside the source texture.
- Warn if text can exceed current `TextDrawer` limits of 200 bytes/chars or
  20 lines, depending on runtime implementation.
- Warn when a font `LetterList` lacks glyphs used by localized strings.
- Fail if an action/binding name is empty.
- Emit a stable, sorted summary of resources and string keys for code review.

## Preview Requirements

The preview tool should use the same converter and runtime data path as the game
where possible. ImGui is appropriate for preview chrome, inspector panels, and
editing controls, but the preview viewport should render the compiled layout
through the same retained UI renderer intended for runtime.

Required preview capabilities:

- Open a `.ui.yml` source file and display the compiled layout at common target
  canvases such as 400x240, 800x480, 1280x720, and 1920x1080.
- Toggle region/language using the existing locale enum values.
- Toggle scale modes and safe-area insets.
- Display missing resources, missing strings, unsupported glyphs, text overflow,
  and hit-area bounds as overlays.
- Select nodes by clicking in the viewport and show resolved transform, anchors,
  size, order, state, resource, string key, and text bounds.
- Switch widget states and play timelines.
- Hot reload YAML, string tables, fonts, and textures.
- Export a deterministic screenshot for visual regression tests.

Tool UI expectations:

- Use `Engine/GUI`/ImGui for editor windows, tabs, menus, file dialogs, sliders,
  checkboxes, combo boxes, color edit, and texture previews.
- Keep game-layout nodes independent from ImGui widgets so authored UI remains
  portable to runtime builds.
- When previewing text, show both source key and localized output in the
  inspector.

## Runtime Implementation Slices

1. Define `UILayout.proto` and a converter from `.ui.yml` to `.vpb`.
2. Add a retained UI renderer for `group`, `image`, `nine_slice`, and `text`
   nodes using the existing 2D projection and font renderer.
3. Add hit-test traversal and action dispatch.
4. Add state override resolution for buttons/toggles and disabled/hidden states.
5. Add timeline playback.
6. Add editor preview integration and screenshot-based regression checks.

The first slice can ship without animation and widgets if it can render static
images and localized text deterministically.

## Open Questions

- Should UI draw through a new `Engine/UI` module or extend `Engine/Layout`?
  `Engine/Layout` currently means text/font/string table, so a new module may
  make ownership clearer.
- Should layout use a single atlas per layout or arbitrary texture references?
  Arbitrary references are simpler; atlases are better for draw-call reduction.
- Should text source normalize around style names from `KeystringTable` or keep
  layout-local styles that compile into the layout VPB?
- How much complex text support is required for Japanese and European languages:
  line breaking, punctuation rules, fallback fonts, kerning, and shaping?
- Should string tables remain separate VPBs or should layout packages embed
  their required string subset for faster loading?
- What is the input event contract between UI hit areas and gameplay systems:
  ECS events, direct callbacks, or a command queue?
- Are runtime UI animations authored in layout YAML sufficient, or should the
  engine consume a separate animation clip format?
- Which platforms require safe-area presets beyond rectangular insets?
- Do we need author-time import from NintendoWare as a one-time migration aid?
  If yes, the migrated `.ui.yml` must become canonical immediately after import.
