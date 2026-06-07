# Porting Documentation Inventory

This directory includes planning documents recovered while rebasing useful work
onto `v0.3-Development`.

These files are intentionally documentation-only. They preserve design notes,
test plans, migration risks, and reverse-engineering results so later PRs can
port the relevant work in small slices instead of replaying everything as a
large merge.

## Status Legend

- Current reference: still useful as written for port planning.
- Needs v0.3 review: useful, but must be checked against current
  `v0.3-Development` code before implementation.
- Historical only: preserves context, but should not drive code without a fresh
  design pass.

## Documents

| Document | Status | v0.3-Development note |
| --- | --- | --- |
| `AudioToolReplacement.md` | Needs v0.3 review | Keep the CLI/editor replacement plan, but reconcile it with the current audio runtime and pak format before changing build rules. |
| `AudioToolReverseEngineering.md` | Current reference | Binary compatibility findings are useful for future parity tests. |
| `BehaviorTreeConversion.md` | Current reference | Directly supports the portable behavior-tree conversion PR needed for external project workflows. |
| `EntityHierarchyValidation.md` | Needs v0.3 review | Fixtures and validation strategy remain useful; component protobuf names may have changed. |
| `ExternalProjectWorkflow.md` | Needs v0.3 review | Setup flow is useful context, but tool versions and environment variables should be checked against the current README and templates. |
| `GFXContextThreadOwnership.md` | Needs v0.3 review | Thread ownership concerns remain relevant; Alex's Vulkan/display work is now the baseline. |
| `ModelInstancing.md` | Needs v0.3 review | Treat as a requirements and test source until the current model path is compared. |
| `PBRMaterialContract.md` | Historical only | Likely superseded by current material/HDR work; keep for vocabulary and asset-contract context. |
| `PerformanceBaselines.md` | Needs v0.3 review | Baseline categories are useful, but numbers and runners must be regenerated on the target branch. |
| `PreviewHost.md` | Needs v0.3 review | Preview goals remain valid; resource, scene, and render startup must be adapted to current APIs. |
| `ReleaseReadiness.md` | Needs v0.3 review | Keep the coverage model, but update runners as they are reintroduced. |
| `RenderingColorSpacePolicy.md` | Historical only | Preserve policy context, but do not override the current HDR swap-chain behavior without new tests. |
| `RenderTaskBoundaries.md` | Needs v0.3 review | Useful for render threading discussions; must align with current render task ownership. |
| `ShadowQualityControls.md` | Needs v0.3 review | Keep as feature planning only until current shadow code is inspected. |
| `StableEntityIDs.md` | Needs v0.3 review | Stable identity goals remain relevant, but ECS/framework changes are high-conflict and should be late PRs. |
| `SystemDependencyGraph.md` | Needs v0.3 review | Useful for scheduler planning after current system metadata is audited. |
| `SystemScheduler.md` | Needs v0.3 review | Keep as scheduler design input, not as a direct implementation recipe. |
| `UILayoutContract.md` | Needs v0.3 review | Useful for managed editor UI work; should be applied after tool shell foundations land. |

## Porting Guidance

- Do not treat these documents as proof that the described implementation exists
  on `v0.3-Development`.
- Later PRs should either update the relevant document to match the ported code
  or add a note that the old design was intentionally not carried forward.
- When a document describes runtime behavior, prefer tests from the matching PR
  plan item as the acceptance criteria.
