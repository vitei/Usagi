param()

$ErrorActionPreference = 'Stop'

$RepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$BuildRoot = Join-Path $RepoRoot 'Tools\test-build\BehaviorTreeConversion'
$StubRoot = Join-Path $BuildRoot 'ruby-stubs'
$InputRoot = Join-Path $BuildRoot 'input'
$OutRoot = Join-Path $BuildRoot 'out'

Remove-Item -LiteralPath $BuildRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $StubRoot, $InputRoot, $OutRoot | Out-Null
New-Item -ItemType Directory -Path (Join-Path $BuildRoot '_build\proto') | Out-Null
Set-Content -Encoding ASCII -Path (Join-Path $BuildRoot '_build\proto\deps.txt') -Value ''

$env:USAGI_DIR = $RepoRoot

$EngineStubDir = Join-Path $StubRoot 'Engine\AI\BehaviorTree'
$ProjectStubDir = Join-Path $StubRoot 'Project\AI'
New-Item -ItemType Directory -Path $EngineStubDir, $ProjectStubDir | Out-Null

@'
module Usg
  module Ai
    class FakeMessage
      def initialize(attrs = {})
        self.attributes = attrs
      end

      def self.fields
        {}
      end

      def fields
        self.class.fields
      end

      def attributes=(attrs)
        attrs.each do |key, value|
          instance_variable_set("@#{key}", value)
        end
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
'@ | Set-Content -Encoding ASCII -Path (Join-Path $EngineStubDir 'BehaviorCommon.pb.rb')

@'
module Usg
  module Ai
    module ProjectBehaviorType
      ProjectBehaviorType_Idle = 100
    end
  end
end
'@ | Set-Content -Encoding ASCII -Path (Join-Path $ProjectStubDir 'ProjectBehaviorCommon.pb.rb')

@'
require 'rexml/document'

module Nokogiri
  VERSION = 'behavior-conversion-test-stub'

  def self.XML(io)
    yield Config.new if block_given?
    Document.new(REXML::Document.new(io.read))
  end

  class Config
    def strict
    end
  end

  class Document
    attr_reader :root

    def initialize(document)
      @document = document
      @root = wrap(document.root)
    end

    def xpath(query)
      elements = []
      @document.elements.each('//*') { |element| elements << element }

      result = case query
      when '//*'
        elements
      when %r{\A//\*\[@start='true'\]\z}
        elements.select { |element| element.attributes['start'] == 'true' }
      when %r{\A//\*\[@name='([^']+)'\]\z}
        name = Regexp.last_match(1)
        elements.select { |element| element.attributes['name'] == name }
      when %r{\A//xmlns:transition\[@toState='([^']+)'\]\z}
        to_state = Regexp.last_match(1)
        elements.select { |element| element.name == 'transition' && element.attributes['toState'] == to_state }
      when %r{\A//xmlns:transition\[@fromState='([^']+)'\]\z}
        from_state = Regexp.last_match(1)
        elements.select { |element| element.name == 'transition' && element.attributes['fromState'] == from_state }
      else
        []
      end

      result.map { |element| wrap(element) }
    end

    private

    def wrap(element)
      element && Node.new(element)
    end
  end

  class Node
    attr_reader :name

    def initialize(element)
      @element = element
      @name = element.name
    end

    def [](key)
      @element.attributes[key]
    end

    def attributes
      Hash[@element.attributes.map { |key, value| [key, value] }]
    end

    def eql?(other)
      other.is_a?(Node) && other.element.equal?(@element)
    end

    def hash
      @element.object_id.hash
    end

    protected

    attr_reader :element
  end
end
'@ | Set-Content -Encoding ASCII -Path (Join-Path $StubRoot 'nokogiri.rb')

$EngineYml = Join-Path $InputRoot 'engine_only.btyml'
@'
- Behavior:
    type: Composite
    name: Sequence
    numChildren: 0
'@ | Set-Content -Encoding ASCII -Path $EngineYml

$ProjectYml = Join-Path $InputRoot 'project_behavior.btyml'
@'
- Behavior:
    type: Action
    name: ProjectBehaviorType_Idle
    hasData: false
'@ | Set-Content -Encoding ASCII -Path $ProjectYml

$EngineXml = Join-Path $InputRoot 'engine_only.btxml'
@'
<stateMachine xmlns="gap">
  <composite_Sequence name="Root" start="true" numChildren="1" PositionX="0" />
  <behavior_Wait name="Wait" PositionX="1" />
  <transition fromState="Root" toState="Wait" />
</stateMachine>
'@ | Set-Content -Encoding ASCII -Path $EngineXml

$ProjectXml = Join-Path $InputRoot 'project_behavior.btxml'
@'
<stateMachine xmlns="gap">
  <composite_Sequence name="Root" start="true" numChildren="1" PositionX="0" />
  <projectbehavior_Idle name="Idle" PositionX="1" />
  <transition fromState="Root" toState="Idle" />
</stateMachine>
'@ | Set-Content -Encoding ASCII -Path $ProjectXml

function Invoke-RubyConverter {
  param(
    [string[]] $Arguments,
    [switch] $ExpectFailure
  )

  Push-Location $BuildRoot
  try {
    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    $output = & ruby @Arguments 2>&1
    $exitCode = $LASTEXITCODE
  } finally {
    $ErrorActionPreference = $previousErrorActionPreference
    Pop-Location
  }

  if ($ExpectFailure) {
    if ($exitCode -eq 0) {
      throw "Expected converter failure, but command succeeded: ruby $($Arguments -join ' ')"
    }
  } elseif ($exitCode -ne 0) {
    $output | Write-Host
    throw "Converter failed with exit code ${exitCode}: ruby $($Arguments -join ' ')"
  }

  return ($output | Out-String)
}

$YmlConverter = Join-Path $RepoRoot 'Tools\ruby\yml2vbt.rb'
$XmlConverter = Join-Path $RepoRoot 'Tools\ruby\xml2vbt.rb'

Invoke-RubyConverter -Arguments @(
  '-I', $StubRoot,
  $YmlConverter,
  '--no-default-project-behavior-proto',
  '-o', (Join-Path $OutRoot 'engine_only.vbt'),
  $EngineYml
) | Out-Null

$missingProjectOutput = Invoke-RubyConverter -ExpectFailure -Arguments @(
  '-I', $StubRoot,
  $YmlConverter,
  '--no-default-project-behavior-proto',
  '-o', (Join-Path $OutRoot 'missing_project.vbt'),
  $ProjectYml
)

if ($missingProjectOutput -notmatch 'Could not resolve behavior tree protobuf type' -or
    $missingProjectOutput -notmatch 'ProjectBehaviorType_Idle') {
  throw "Missing project behavior diagnostic did not name the unresolved protobuf type: $missingProjectOutput"
}
if ($missingProjectOutput -notmatch '--project-behavior-proto') {
  throw "Missing project behavior diagnostic did not mention --project-behavior-proto: $missingProjectOutput"
}

Invoke-RubyConverter -Arguments @(
  '-I', $StubRoot,
  $YmlConverter,
  '--no-default-project-behavior-proto',
  '--project-behavior-proto', 'Project/AI/ProjectBehaviorCommon.pb.rb',
  '-o', (Join-Path $OutRoot 'project_behavior.vbt'),
  $ProjectYml
) | Out-Null

Invoke-RubyConverter -Arguments @(
  '-I', $StubRoot,
  $XmlConverter,
  '--no-default-project-behavior-proto',
  '-o', (Join-Path $OutRoot 'engine_only_xml.vbt'),
  $EngineXml
) | Out-Null

$missingXmlPrefixOutput = Invoke-RubyConverter -ExpectFailure -Arguments @(
  '-I', $StubRoot,
  $XmlConverter,
  '--no-default-project-behavior-proto',
  '-o', (Join-Path $OutRoot 'missing_project_prefix_xml.vbt'),
  $ProjectXml
)

if ($missingXmlPrefixOutput -notmatch '--behavior-prefix') {
  throw "XML project behavior prefix diagnostic did not mention --behavior-prefix: $missingXmlPrefixOutput"
}

$missingXmlProjectOutput = Invoke-RubyConverter -ExpectFailure -Arguments @(
  '-I', $StubRoot,
  $XmlConverter,
  '--no-default-project-behavior-proto',
  '--behavior-prefix', 'projectbehavior_:ProjectBehaviorType_',
  '-o', (Join-Path $OutRoot 'missing_project_xml.vbt'),
  $ProjectXml
)

if ($missingXmlProjectOutput -notmatch 'Could not resolve behavior tree protobuf type' -or
    $missingXmlProjectOutput -notmatch 'ProjectBehaviorType_Idle') {
  throw "XML project behavior diagnostic did not name the unresolved protobuf type: $missingXmlProjectOutput"
}

Invoke-RubyConverter -Arguments @(
  '-I', $StubRoot,
  $XmlConverter,
  '--no-default-project-behavior-proto',
  '--behavior-prefix', 'projectbehavior_:ProjectBehaviorType_',
  '--project-behavior-proto', 'Project/AI/ProjectBehaviorCommon.pb.rb',
  '-o', (Join-Path $OutRoot 'project_behavior_xml.vbt'),
  $ProjectXml
) | Out-Null

foreach ($Path in @(
  'engine_only.vbt',
  'project_behavior.vbt',
  'engine_only_xml.vbt',
  'project_behavior_xml.vbt'
)) {
  $FullPath = Join-Path $OutRoot $Path
  if (-not (Test-Path $FullPath)) {
    throw "Expected converter output was not produced: $FullPath"
  }
}

Write-Host "Behavior tree conversion smoke passed: $OutRoot"
