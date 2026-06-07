# Entity Hierarchy Validation

`Tools/Tests/EntityHierarchy/Run.ps1` is a focused smoke runner for `Tools/ruby/process_hierarchy.rb`.
It validates a self-contained YAML fixture through the real converter entry point while using small Ruby protobuf stubs instead of a generated `_build/ruby` tree.

The fixture covers:

- `Children` recursion and child entity headers.
- `Overrides` applied by `EntityWithID` to a child entity.
- `InitializerEvents` event lookup and serialization.
- `Inherits` include resolution through `-I`.
- `~Merge` sugar translating to `Processor.Merge`.

Run it from the repository root:

```powershell
.\Tools\Tests\EntityHierarchy\Run.ps1
```

The runner writes only disposable output under `Tools/test-build/EntityHierarchy` and creates an empty `_build/proto/deps.txt` when the generated dependency map is absent, because `tracker.rb` expects that file during converter startup.
