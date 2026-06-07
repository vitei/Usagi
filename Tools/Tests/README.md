# Tools Test Harness

This directory restores the minimal PR 02 verification surface on top of
`upstream/v0.3-Development`.

Run the default smoke baseline from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File Tools\Tests\RunAll.ps1
```

The default suite is intentionally small. It checks that the current tool
entry points, scripts, and lightweight fixtures are present and runnable
without requiring Visual Studio, proprietary game assets, generated protobuf
Ruby bindings, or a Python 2 runtime.

Some historical tests are preserved as explicit skip or optional
paths because they depend on future PRs in `PR_PLAN.md`:

- full engine/build compilation belongs behind `BaselineBuild -FullBuild`
- full behavior-tree conversion waits for portable converter work
- full Python level conversion waits for Python/tooling modernization
- resource and shader package generation are opt-in native smoke paths
- release and performance baselines aggregate the current lightweight suites
