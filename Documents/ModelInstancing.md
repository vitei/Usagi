# Model Instancing

`Tools/Tests/ModelInstancing/Run.ps1` is a narrow smoke test for the current
model instance conversion and renderer baseline. It creates a temporary Usagi
data root with a minimal entity YAML, converts a tiny `.lvl` through
`Tools/python/lvl2vhir/lvl2vhir.py`, and verifies the generated instance-set
YAML contract:

- `Instance: true`
- `Format: transform`
- two nodes for the two level objects
- the current `.vmdc` model resource name emitted by the level converter
- left-handed converted translations

The test also checks the renderer-side hooks that Alex's branch already has:
instance model resources declare an instance-rate transform stream, model meshes
with instancing support create `InstanceMesh` nodes, `ModelInstanceRenderer`
batches transforms into a vertex buffer, and `InstanceDrawer` submits
`DrawIndexedEx` with the batched instance count.

The broader porting branch explored a separate explicit `Model::LoadInstanced`
entry point and `.vmdf` instance-set naming. Those are intentionally not ported
in this PR because the current branch already has a renderer-level batching
path, and switching level-converter asset naming should be reviewed separately
with resource build coverage.
