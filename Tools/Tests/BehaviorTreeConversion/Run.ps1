param(
    [switch]$IncludeOptional
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) 'TestHarness.psm1') -Force
Initialize-TestHarness 'Behavior tree conversion smoke'

$RepoRoot = Get-UsagiRoot
$Workspace = New-TestWorkspace 'BehaviorTreeConversion'
$StubRoot = Join-Path $Workspace 'ruby-stubs'
$InputRoot = Join-Path $Workspace 'input'
$OutRoot = Join-Path $Workspace 'out'
New-Item -ItemType Directory -Force -Path $StubRoot, $InputRoot, $OutRoot | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $Workspace '_build\proto') | Out-Null
Set-Content -Encoding ASCII -Path (Join-Path $Workspace '_build\proto\deps.txt') -Value ''

$env:USAGI_DIR = $RepoRoot

$ruby = Get-Command ruby -ErrorAction SilentlyContinue

Invoke-TestStep 'converter scripts exist' {
    foreach ($path in @('Tools\ruby\yml2vbt.rb', 'Tools\ruby\xml2vbt.rb')) {
        Assert-PathExists (Join-Path $RepoRoot $path)
    }
}

if ($ruby) {
    Invoke-TestStep 'converter scripts pass Ruby syntax checks' {
        foreach ($path in @('Tools\ruby\yml2vbt.rb', 'Tools\ruby\xml2vbt.rb')) {
            $fullPath = Join-Path $RepoRoot $path
            & ruby -c $fullPath | Out-Null
            if ($LASTEXITCODE -ne 0) {
                throw "Ruby syntax check failed: $path"
            }
        }
    }
}
else {
    Add-TestSkip 'Ruby is not on PATH; skipped converter syntax and execution checks.'
}

if ($ruby) {
    Invoke-TestStep 'YAML converter handles a minimal engine-only tree with stub protobuf bindings' {
    $engineStubDir = Join-Path $StubRoot 'Engine\AI\BehaviorTree'
    $tankStubDir = Join-Path $StubRoot 'Tank\AI'
    New-Item -ItemType Directory -Force -Path $engineStubDir, $tankStubDir | Out-Null

@'
module Usg
  module Ai
    class FakeMessage
      def initialize(attrs = {})
        attrs.each { |key, value| instance_variable_set("@#{key}", value) }
      end

      def serialize_to_string
        self.class.name
      end
    end

    module CompositeType
      CompositeType_Sequence = 1
      Decorator = 2
      Behavior = 3
    end

    module BehaviorType
      BehaviorType_Wait = 10
    end

    module DecoratorType
    end

    class CompositeHeader < FakeMessage
      attr_accessor :compositeType
    end

    class SequenceHeader < FakeMessage
      attr_accessor :numChildren
    end

    class BehaviorHeader < FakeMessage
      attr_accessor :behaviorType
    end

    class DecoratorHeader < FakeMessage
      attr_accessor :decoratorHeader
    end
  end
end
'@ | Set-Content -Encoding ASCII -Path (Join-Path $engineStubDir 'BehaviorCommon.pb.rb')

@'
module Usg
  module Ai
    module TankBehaviorType
    end

    module TankDecoratorType
    end
  end
end
'@ | Set-Content -Encoding ASCII -Path (Join-Path $tankStubDir 'TankBehaviorCommon.pb.rb')

    $input = Join-Path $InputRoot 'engine_only.btyml'
@'
- Behavior:
    type: Composite
    name: Sequence
    numChildren: 0
'@ | Set-Content -Encoding ASCII -Path $input

    $output = Join-Path $OutRoot 'engine_only.vbt'
    Push-Location $Workspace
    try {
        & ruby -I $StubRoot (Join-Path $RepoRoot 'Tools\ruby\yml2vbt.rb') -o $output $input
        if ($LASTEXITCODE -ne 0) {
            throw "yml2vbt.rb failed with exit code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }

    Assert-PathExists $output
    if ((Get-Item $output).Length -le 0) {
        throw "Converter output was empty: $output"
    }
    }
}

Add-TestSkip 'XML and project-specific behavior coverage is deferred to the portable behavior-tree conversion PR.'
Complete-TestHarness
