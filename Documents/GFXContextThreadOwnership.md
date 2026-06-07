# GFXContext Thread Ownership

This document defines the ownership rules for future threaded GPU command list
generation. The current renderer records serially, but these rules are the
contract that deferred command contexts must preserve.

## Context Ownership

- A `GFXContext` is single-owner while it is recording.
- The owner thread is established by the code that calls `GFXContext::Begin`.
- Only the owner thread may call draw, transfer, render-target, descriptor, and
  debug-tag methods until `GFXContext::End` completes.
- A context may be reused by another thread only after the previous owner has
  ended recording and the frame submission path no longer references that
  context's command buffer.
- The immediate context remains owned by the main render thread.

## Command Pools

- Vulkan command buffers are allocated from the command pool owned by each
  `GFXContext_ps`.
- Worker contexts must use their own command pools. They must not allocate or
  reset command buffers from the device upload/shared command pool.
- Command-buffer collection must preserve explicit submission order. Parallel
  generation is allowed only for command buffers whose render pass dependencies
  are already ordered by the collector.

## Descriptor Updates

- Descriptor set contents are not worker-mutable during command recording.
- `DescriptorSet::UpdateDescriptors` and other APIs that call
  `vkUpdateDescriptorSets` must run in the main/update phase before worker
  command recording starts.
- Worker contexts may bind descriptor sets that are already finalized for the
  frame.

## Resource Lifetime

- Textures, buffers, render targets, pipeline states, descriptor sets, and
  constant sets referenced by a worker context must remain alive until the
  command buffer has been submitted and the frame fence covering that submit has
  completed.
- Dynamic resources must continue to use the existing frame-age cleanup rules.
  Threaded recording must not shorten the lifetime window.
- Resource upload and layout-transition helpers that use shared device command
  buffers stay on the main render/update path unless they are moved to
  per-context ownership explicitly.

## Render Targets

- A render target may be written by only one recording context at a time.
- Read-after-write and write-after-write dependencies are represented by the
  command-buffer collection order and existing render-pass/layout transitions.
- Shadow, scene, post, and display command buffers must be submitted in the same
  visible order as the current serial renderer until a more explicit render task
  graph exists.

## First Implementation Boundary

The first threaded-command-list implementation should add collection and
submission plumbing without changing renderer behavior:

- The collector starts each frame empty.
- The immediate context records exactly as it does today.
- `GFXDevice_ps::End` submits the collector's ordered command buffers.
- The initial collector contains only the immediate context command buffer.
- Deferred context creation and worker recording come after this submission path
  is covered by smoke testing.
