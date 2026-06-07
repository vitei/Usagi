# Performance Baselines

Use `Tools\Tests\PerformanceBaseline\Run.ps1` before and after optimization work. The runner records command duration, exit code, stdout/stderr log paths, and a JSON summary under the local `tools\test-build\PerformanceBaseline` output directory by default.

The current baseline uses existing deterministic coverage rather than a bespoke game demo:

- `system-scheduler`: exercises heavy ECS scheduling and scheduler statistics.
- `ecs-frame-threading`: exercises entity hierarchy traversal, dirty IO updates, component mutation guards, and frame-threading stress cases.
- `shader-package-rendering`: packages the model, post-process, and particle shader effects to catch shader/material-path regressions.
- `particle-editor-render-smoke`: optional render startup smoke via `-IncludeRenderSmoke`; add `-RequireValidation` to require staged Vulkan validation layers.

Recommended commands:

```powershell
powershell -ExecutionPolicy Bypass -File .\Tools\Tests\PerformanceBaseline\Run.ps1
powershell -ExecutionPolicy Bypass -File .\Tools\Tests\PerformanceBaseline\Run.ps1 -IncludeRenderSmoke -RequireValidation
```

Roadmap scene coverage:

- Many models: represented by shader/material package coverage until a larger 1.0 demo scene exists.
- Many particles: represented by the Particle Editor smoke target and particle shader package coverage.
- Many lights: represented by model/post-process shader coverage; a larger rendering stress scene should add explicit light-count coverage.
- Heavy ECS: represented by `SystemScheduler` and `ECSFrameThreading`.

When adding new optimization work, keep baseline output outside the repository and compare `baseline.json` duration fields plus the command logs. Add a focused test command to this runner when the optimization has a stable standalone harness.
