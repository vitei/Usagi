# Release Readiness

The current 1.0 demo scope is represented by deterministic smoke and stress coverage rather than a separate sample game project. The coverage exercises the systems that a modest Usagi demo should prove:

- ECS hierarchy, parent reads, events, pending deletes, and threaded scheduler behavior.
- Resource pak dependency export, level conversion, model instancing conversion, and shader package generation.
- Rendering startup through Particle Editor, including Vulkan validation when requested.
- Particle, post-process, model, and material shader paths.

Use the aggregate runner before release-style validation:

```powershell
powershell -ExecutionPolicy Bypass -File .\Tools\Tests\ReleaseReadiness\Run.ps1
powershell -ExecutionPolicy Bypass -File .\Tools\Tests\ReleaseReadiness\Run.ps1 -IncludeRenderSmoke -RequireValidation
```

The first command stays headless and is suitable for repeated local checks. The second also launches the Particle Editor render smoke target with staged Vulkan validation layers.

The remaining full-demo gap is content, not engine plumbing: a dedicated 1.0 sample project can layer a visible scene on top of these validated paths when suitable assets are available.
