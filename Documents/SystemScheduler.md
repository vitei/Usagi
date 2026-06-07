# System Scheduler

`SystemScheduler` is the scheduling layer for ECS signal dispatch.

With the default zero-worker configuration, scheduler tasks drain inline and
preserve:

- priority-sorted runner order
- stack-backed `Signal` and event payload lifetimes
- existing entity traversal order inside `Signal`
- existing profiling timer coverage around each runner

`SystemCoordinator::Trigger` and `TriggerFromRoot` are the only call sites routed
through the scheduler initially. That keeps frame signals, queued events, and
physics/raycast callbacks on the same behavior-preserving path.

Root signals are split into branch callbacks before execution reaches
`TaskRunner`. The scheduler still drains each branch immediately in sibling
order, so branch visibility is available without changing traversal behavior.
Scheduler stats track both runner dispatch count and root-branch task count, so
the branch fan-out can be inspected without changing execution policy.

`TaskRunner` is synchronous even when workers are enabled: the caller also helps
execute tasks and does not return until all tasks finish. Scheduler work can opt
into workers without changing the payload lifetime contract.

Use `SystemCoordinator::ConfigureSystemScheduler` to set the worker count and
maximum task count. The default configuration remains zero workers.
`TaskRunner` treats a maximum task count of zero as unlimited; nonzero limits
are asserted at dispatch time. Worker-backed dispatch also asserts that only one
batch is active at a time.

Targeted and root signal dispatch can submit a contiguous compatible-system
batch to `TaskRunner`. With zero workers this drains inline in the original
runner order; with workers configured, systems in the same batch may run
concurrently. Root signal batches are flattened into one task list so system
batching and root-branch batching do not nest `TaskRunner` dispatches.

While a signal is being dispatched, `SystemCoordinator` asserts against entity IO
membership mutation through `UpdateEntityIO` and `RemoveEntityIO`. Entity changes
should be applied in the existing pre-dispatch check phase or deferred until a
later safe phase.

When scheduler workers are configured, signal tasks also mark system execution
active. Low-level component, entity hierarchy, system IO membership, dirty flag,
pending-delete, activation, and deactivation mutation paths assert while that
worker-backed execution is active. This keeps the threaded path from racing ECS
structure changes before those operations have an explicit synchronization or
deferred-queue design. Zero-worker dispatch does not enable this guard, so the
existing serial behavior remains available by default.

Two worker-time requests are deferred rather than asserted:

- `ComponentType::RequestFree()` records the entity stable handle and component
  type ID, then marks the pending delete after dispatch returns.
- `ComponentEntity::SetChanged()` records the entity stable handle, then marks
  the entity dirty after dispatch returns.

Deferred requests are flushed after each signal dispatch and before
`ComponentManager::CheckEntities()`. The flush only marks pending work; actual
component frees and IO membership updates still run in the existing safe entity
check phase. Deferred requests that point at stale entity handles or missing
components are ignored.

## Coverage status

Current verification includes build-level checks: regenerate boilerplate when
templates change, build the Framework target, then sweep dependent engine module
targets.

`Tools/Tests/SystemScheduler/Run.ps1` builds a standalone scheduler harness. It
uses a synthetic system IO hierarchy to verify root-branch fan-out, targeted
system batches, worker-backed batch overlap, execution guard visibility, and
scheduler stats without booting a full project. Full scheduler acceptance still
needs an engine demo or frame-level harness that can compare single-worker and
multi-worker branch results.
