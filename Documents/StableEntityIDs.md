# Stable Entity IDs

The legacy `Entity` type is a raw `ComponentEntity*`. Existing runtime code
still uses that pointer for compatibility, but deferred work needs a value that
can be stored and validated before it is resolved back to a live entity.

## Representation

`EntityHandle` is the stable value:

- `uIndex`: the entity pool slot index.
- `uGeneration`: the lifetime generation for that slot.

An all-zero handle is invalid. A handle resolves only when the slot is currently
live and its generation still matches.

## Current Integration

- `ComponentEntity::GetStableID()` returns the handle for a live entity.
- `ComponentEntity::GetEntityFromStableID()` resolves a handle to a live
  `Entity` pointer or returns `nullptr` for stale IDs.
- `ComponentEntity::IsStableIDValid()` provides a debug-friendly validity check.
- `Components::EntityID` still carries the legacy raw pointer in `id`, and now
  also stores `stableId` for new code.

## Migration Rule

New deferred jobs, event queues, and worker-retained references should store
`EntityHandle`. They may resolve to `Entity` only on the owning ECS thread
immediately before use.

Targeted event queues, scheduler tasks, worker mutation deferral, and frame
threading are deferred to later PRs so this branch remains a small identity
foundation.
