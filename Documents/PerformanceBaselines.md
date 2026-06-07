# Performance Baselines

`Tools/Tests/PerformanceBaseline/Run.ps1` is the lightweight baseline entry
point. In this stage it records harness metadata and produces `baseline.json`
and `baseline.md`; it does not claim stable timing numbers for native rendering,
ECS threading, or editor launch.

Default command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools\Tests\PerformanceBaseline\Run.ps1
```

The default report is useful as a reproducible inventory of the current smoke
surface. Heavy timing suites should be added only when their underlying feature
branches have landed and the required local environment can be detected
reliably.

## Baseline Categories

- Conversion baselines: behavior tree, level conversion, entity hierarchy, and
  resource pak exporter fixtures.
- Build/preflight baselines: baseline build and shader package rendering
  prerequisites.
- Managed tooling baselines: audio, entity, particle, and preview ToolCore tests
  once those stacked branches are reviewed.
- Runtime baselines: model instancing, resource async, scheduler, ECS threading,
  and render/editor smoke after the corresponding runtime PRs land.

## Output Policy

Baseline output belongs outside committed source files. Keep generated JSON,
markdown, and logs under the test workspace created by the runner, and compare
duration fields only between machines/environments that are intentionally
comparable.

When a future optimization adds a stable standalone harness, add that harness to
the baseline runner with a clear category and prerequisites instead of folding
it into an unrelated smoke test.
