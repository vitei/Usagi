# Behavior Tree Conversion

`Tools/ruby/yml2vbt.rb` and `Tools/ruby/xml2vbt.rb` always require the engine behavior tree protobuf binding:

```text
Engine/AI/BehaviorTree/BehaviorCommon.pb.rb
```

Project-specific behavior bindings are configurable. The converters still try to load the legacy Tank binding when it is present so existing Tank projects keep working, but missing Tank files are no longer fatal for engine-only projects.

## Project Behavior Bindings

Use `--project-behavior-proto` for each project behavior protobuf Ruby binding required by a behavior tree:

```powershell
ruby Tools/ruby/yml2vbt.rb `
  -I _build/ruby `
  --project-behavior-proto Project/AI/ProjectBehaviorCommon.pb.rb `
  -o out/tree.vbt Data/BehaviorTrees/tree.btyml
```

The option is repeatable. Pass `--no-default-project-behavior-proto` when a project should not even attempt the optional legacy Tank require.

If a behavior tree references a type that cannot be resolved, the converters report the missing protobuf type and point at `--project-behavior-proto` instead of failing while requiring `Tank/AI/TankBehaviorCommon.pb.rb`.

## XML Type Prefixes

XML behavior tree node names map from an XML prefix to a protobuf enum prefix. Built-in mappings are:

```text
behavior_      -> BehaviorType_
decorator_     -> DecoratorType_
tankbehavior_  -> TankBehaviorType_
tankdecorator_ -> TankDecoratorType_
```

Add project action prefixes with `--behavior-prefix`:

```powershell
ruby Tools/ruby/xml2vbt.rb `
  -I _build/ruby `
  --project-behavior-proto Project/AI/ProjectBehaviorCommon.pb.rb `
  --behavior-prefix projectbehavior_:ProjectBehaviorType_ `
  -o out/tree.vbt Data/BehaviorTrees/tree.btxml
```

Add project decorator prefixes with `--decorator-prefix`:

```powershell
ruby Tools/ruby/xml2vbt.rb `
  -I _build/ruby `
  --project-behavior-proto Project/AI/ProjectBehaviorCommon.pb.rb `
  --decorator-prefix projectdecorator_:ProjectDecoratorType_ `
  -o out/tree.vbt Data/BehaviorTrees/tree.btxml
```

## Test Runner

Run the focused smoke test with:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Tests/BehaviorTreeConversion/Run.ps1
```

The runner creates temporary protobuf stubs and verifies:

- engine-only YAML conversion does not require Tank behavior bindings
- project YAML behavior types fail with a targeted diagnostic until their project proto is configured
- engine-only XML conversion uses engine behavior prefixes
- custom XML behavior prefixes resolve when paired with a project behavior proto
