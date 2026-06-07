# Resource Async Loading

`Tools/Tests/ResourceAsyncFoundation/Run.ps1` validates the first async resource
loading foundation slice. This PR adds state and request metadata only; it does
not start worker threads, move resource CPU loading off-thread, or change any
resource load call sites.

The new `ResourceState` enum gives every `ResourceBase` an explicit lifecycle:
requested, CPU loading, dependency waiting, CPU ready, queued GPU upload, GPU
uploading, ready, failed, cancelled, and unloading. Existing `IsReady()` and
`SetReady()` behavior remains compatible by mapping to `READY` and `REQUESTED`.

`ResourceData` now owns lightweight `ResourceRequest` records. A request tracks
the resource name, type, priority, tag, static/dynamic ownership, state,
dependencies, and completed handle. The queue helpers deduplicate requests,
raise priority when the same resource is queued again, select the highest
priority requested item, and mark completion or failure.

The first follow-up lifecycle slice adds request transition helpers for CPU
load, dependency wait, CPU-ready, GPU-queued, and GPU-upload states. It also
adds cancellation, explicit failure, terminal-state checks, inflight detection,
completed-request cleanup, and terminal-request retry behavior. This keeps the
queue usable by later worker and GPU-upload slices without starting worker
threads in this branch.

The deferred worker-loading work should build on this in later slices:
skeletal-animation CPU preload, particle CPU data split, worker result draining,
shutdown cancellation, and queue ownership. Those changes are intentionally
kept out of this PR because they affect threading and resource cleanup order.
