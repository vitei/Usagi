# System Dependency Graph

`SystemCoordinator::LockRegistration` records read-only dependency metadata for
registered systems after all generated boilerplate has been registered.

The first graph surface is intentionally observational. It does not reorder
systems or change signal dispatch. It exposes:

- required component masks, matching the existing `GetSystemKey` filters
- read component masks generated from system `Inputs`
- write component masks generated from system `Outputs`
- parent-read masks for `Inputs` that search outside `FromSelf`
- system priority/category values used by signal runner sorting
- collision-listener metadata

`SystemsHaveComponentAccessConflict` reports a conservative conflict when either
system writes a component type the other system reads or writes. Parent-read
masks are kept separately so a later scheduler can avoid running a parent
writer alongside a child system that reads through the hierarchy.

This graph is a scheduling input, not a scheduler. The current runtime still
uses the existing priority-sorted signal runners and serial dispatch path.

After registration is locked, `SystemCoordinator` also builds observational
signal execution batches. Batches are priority-bounded, contiguous in sorted
runner order, and split when generated component access conflicts are found.
Dispatch still uses the serial runner loop; the batch data is exposed so later
scheduler work can consume the same conservative grouping without changing the
zero-worker fallback order.
