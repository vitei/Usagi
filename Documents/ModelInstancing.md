# Model Instancing

## Roadmap 08-03 smoke coverage

`Tools/Tests/ModelInstancing/Run.ps1` is a narrow smoke test for the existing model instance conversion path. It creates a temporary Usagi data root with a minimal entity YAML that references `PBRSample/PBRSample.vmdf`, converts a tiny `.lvl` through `Tools/python/lvl2vhir/lvl2vhir.py`, and verifies the generated instance-set YAML contract:

- `Instance: true`
- `Format: transform`
- `Length: 2`
- `ModelName: PBRSample/PBRSample.vmdf`
- one node per source entity placement, including translated and scaled transform data
- no obsolete `.vmdc` model extension
- `Tools/ruby/maya2pb.rb` can convert the generated instance-set YAML to a non-empty binary output

## Runtime slice

`Model::LoadInstanced` now loads the resource through `ResourceMgr::GetModelAsInstance`, uploads a `Matrix4x3` transform stream, binds it at vertex input slot 2, and renders each model mesh with `DrawIndexedEx(..., instanceCount)`.

The shared model shader declarations include `ao_instanceTransform` at attribute location 11. Non-instanced models receive the identity transform through the existing single-attribute fallback. Instance model resources suppress that fallback for `ao_instanceTransform` and instead declare a `VERTEX_INPUT_RATE_INSTANCE` stream, so the shader consumes the uploaded per-instance transforms.

Remaining blocker: no runtime asset loader currently reads `model::InstanceSet` `.pb` files and calls `Model::LoadInstanced`. Until that loader exists, converted instance-set assets are validated by the smoke test but are not automatically submitted to the scene renderer.
