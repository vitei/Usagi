# Render Task Boundaries

The first command-generation split points should stay coarse and match existing renderer sequencing. Fine-grained node-level command buffers are not useful until descriptor ownership, resource lifetimes, and render-pass transitions are explicitly thread-safe.

## Initial Task Types

- View layer tasks: one task per `RenderLayer` in `ViewContext::DrawScene`.
- Shadow tasks: one task per shadow map or cascade pass once shadow recording is moved out of the immediate context.
- Post-effect tasks: one task per post effect that owns its render target transitions.
- Display transfer tasks: one task per display transfer/present path after render-complete semaphores are explicit.

## Current Implementation Point

`ViewContext::DrawScene` now builds an explicit `RenderLayerTask` for each layer and records it through task helper functions. The helpers still execute immediately on the supplied `GFXContext`, so rendering behavior is unchanged. They define the serial boundary that a later scheduler can redirect to deferred contexts.

## Constraints

- Preserve render-layer order unless a layer explicitly declares independent inputs and outputs.
- Keep descriptor updates and global constant updates on the owner thread.
- Do not split individual `RenderNode::Draw` calls into separate command buffers.
- Do not record post effects in parallel until their source/destination render targets are declared.
