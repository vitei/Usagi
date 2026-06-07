# Stable Entity IDs

The legacy `Entity` type is a raw `ComponentEntity*`. Existing runtime code still uses that pointer for compatibility, but threaded systems need a value that can be stored safely across deferred work.

## Representation

`EntityHandle` is the new stable value:

- `uIndex`: the entity pool slot index.
- `uGeneration`: the lifetime generation for that slot.

An all-zero handle is invalid. A handle resolves only when the slot is currently live and its generation still matches.

## Current Integration

- `ComponentEntity::GetStableID()` returns the handle for a live entity.
- `ComponentEntity::GetEntityFromStableID()` resolves a handle to a live `Entity` pointer or returns `nullptr` for stale IDs.
- `ComponentEntity::IsStableIDValid()` provides a debug-friendly validity check.
- `Components::EntityID` still carries the legacy raw pointer in `id`, and now also stores `stableId` for new code.
- `EventManager` accepts `EntityHandle` for targeted events and resolves queued entity events immediately before dispatch.

## Migration Rule

New deferred jobs, event queues, and worker-retained references should store `EntityHandle`. They may resolve to `Entity` only on the owning ECS thread immediately before use.
