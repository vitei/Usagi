# Release Readiness

The current readiness gate is the lightweight smoke harness under
`Tools/Tests`. It is intended to be runnable before the heavier native runtime
and editor PRs are all available in one branch.

Default local gate:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools\Tests\RunAll.ps1
```

This checks the restored harness surface and runs only suites that are expected
to work without proprietary assets, generated native projects, Vulkan SDK setup,
or full editor integration.

Optional expanded gate:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools\Tests\RunAll.ps1 -IncludeOptional
```

Use the optional gate after preparing the local environment for native/project
generation and data conversion prerequisites.

## Current Coverage

- Baseline build preflight and explicit native-build prerequisite reporting.
- Portable behavior-tree conversion fixtures.
- Level conversion fixture coverage.
- Entity hierarchy fixture coverage.
- Resource pak exporter diagnostics.
- Shader package rendering preflight.
- Release readiness aggregation.
- Lightweight performance baseline report generation.

## Deferred Release Gates

Later feature PRs add more focused gates that should become part of a final
release checklist after those branches land:

- Audio build parity and managed audio CLI integration.
- Managed ToolCore entity and particle document tests.
- Preview host startup and asset-load smoke.
- Model instancing validation.
- Resource async lifecycle validation.
- Stable entity handle and later scheduler/threading validation.
- Optional render/editor launch smoke on machines with graphics prerequisites.

The release checklist should track what actually landed. Do not require a
future suite from an unmerged feature branch as a default gate.
