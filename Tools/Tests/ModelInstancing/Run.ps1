param()

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
    $root = (Resolve-Path $PSScriptRoot).Path
    while ($root -and !(Test-Path (Join-Path $root "Engine\CommonProps.props"))) {
        $parent = Split-Path $root -Parent
        if ($parent -eq $root) {
            break
        }

        $root = $parent
    }

    if (!$root -or !(Test-Path (Join-Path $root "Engine\CommonProps.props"))) {
        throw "Could not locate Usagi repository root from $PSScriptRoot"
    }

    return $root
}

$RepoRoot = Get-RepoRoot
$BuildRoot = Join-Path $RepoRoot "Tools\test-build\ModelInstancing"
$TempRoot = Join-Path $BuildRoot "TempUsagi"
$OutRoot = Join-Path $BuildRoot "out"

Remove-Item -LiteralPath $BuildRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $TempRoot, $OutRoot | Out-Null
New-Item -ItemType Directory -Path (Join-Path $TempRoot "Data\Entities") | Out-Null

@'
Inherits:
  - PropBase
ModelComponent:
  name: Models/PBRSample/PBRSample
'@ | Set-Content -Encoding ASCII -Path (Join-Path $TempRoot "Data\Entities\PBRSampleProp.yml")

$LevelPath = Join-Path $BuildRoot "model_instances.lvl"
@'
<game xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="gap" name="Game">
  <gameObjectFolder name="GameObjects" visible="true" locked="false">
    <folder name="Instances" visible="true" locked="false">
      <gameObject xsi:type="gameObjectType" name="InstancedModelA" translate="1 2 3" rotate="0 0 0" scale="1 1 1" visible="true">
        <resource xsi:type="resourceReferenceType" uri="Entities/PBRSampleProp.yml" />
      </gameObject>
      <gameObject xsi:type="gameObjectType" name="InstancedModelB" translate="4 5 6" rotate="0 0 0" scale="1 1 1" visible="true">
        <resource xsi:type="resourceReferenceType" uri="Entities/PBRSampleProp.yml" />
      </gameObject>
    </folder>
  </gameObjectFolder>
</game>
'@ | Set-Content -Encoding ASCII -Path $LevelPath

$OriginalUsagiDir = $env:USAGI_DIR
$env:USAGI_DIR = $TempRoot

try {
    $LevelConverter = Join-Path $RepoRoot "Tools\python\lvl2vhir\lvl2vhir.py"
    $HierarchyOut = Join-Path $OutRoot "model_instances.yml"
    $PositionsOut = Join-Path $OutRoot "model_instances_pos.yml"
    $InstancesOut = Join-Path $OutRoot "model_instances_inst.yml"

    & python $LevelConverter --dist 10 $LevelPath $HierarchyOut $PositionsOut $InstancesOut
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}
finally {
    $env:USAGI_DIR = $OriginalUsagiDir
}

foreach ($Path in @($HierarchyOut, $PositionsOut, $InstancesOut)) {
    if (!(Test-Path $Path)) {
        throw "Expected model instancing conversion output was not produced: $Path"
    }
}

$Instances = Get-Content $InstancesOut -Raw

if ($Instances -notmatch "Instance:\s+true") {
    throw "Instance conversion did not mark the exported set as instanced."
}
if ($Instances -notmatch "Format:\s+transform") {
    throw "Instance conversion did not emit transform instance data."
}
if ($Instances -notmatch "Length:\s+2") {
    throw "Instance conversion did not emit the expected instance count."
}
if ($Instances -notmatch "ModelName:\s+PBRSample/PBRSample\.vmdc") {
    throw "Instance conversion did not preserve the current .vmdc model resource name."
}
if ($Instances -notmatch "name:\s+PBRSampleProp\.yml") {
    throw "Instance conversion did not preserve source entity names."
}

$NodeMatches = [regex]::Matches($Instances, "name:\s+PBRSampleProp\.yml")
if ($NodeMatches.Count -ne 2) {
    throw "Instance conversion emitted $($NodeMatches.Count) nodes instead of 2."
}

foreach ($Expected in @(
    "translation:\s*\r?\n\s*-\s+-1\.0\s*\r?\n\s*-\s+2\.0\s*\r?\n\s*-\s+3\.0",
    "translation:\s*\r?\n\s*-\s+-4\.0\s*\r?\n\s*-\s+5\.0\s*\r?\n\s*-\s+6\.0",
    "scale:\s*\r?\n\s*-\s+1\.0\s*\r?\n\s*-\s+1\.0\s*\r?\n\s*-\s+1\.0"
)) {
    if ($Instances -notmatch $Expected) {
        throw "Instance conversion output was missing expected field pattern: $Expected"
    }
}

$ModelResource = Get-Content (Join-Path $RepoRoot "Engine\Resource\ModelResource.cpp") -Raw
if ($ModelResource -notmatch "AddInstanceBufferInfo") {
    throw "Model resources no longer add instanced pipeline input bindings."
}
if ($ModelResource -notmatch "VERTEX_INPUT_RATE_INSTANCE") {
    throw "Model resources no longer declare an instance-rate transform stream."
}

$ModelCpp = Get-Content (Join-Path $RepoRoot "Engine\Scene\Model\Model.cpp") -Raw
if ($ModelCpp -notmatch "pMesh->bCanInstance") {
    throw "Model loading no longer routes instanced meshes through the instancing path."
}
if ($ModelCpp -notmatch "InstanceMesh") {
    throw "Model loading no longer creates InstanceMesh render nodes."
}

$ModelInstanceRenderer = Get-Content (Join-Path $RepoRoot "Engine\Scene\Model\ModelInstanceRenderer.cpp") -Raw
if ($ModelInstanceRenderer -notmatch "m_instanceData\.push_back") {
    throw "Model instance renderer no longer batches instance transforms."
}
if ($ModelInstanceRenderer -notmatch "m_instanceBuffer\.SetContents") {
    throw "Model instance renderer no longer uploads instance transform buffers."
}

$ModelRenderNodes = Get-Content (Join-Path $RepoRoot "Engine\Scene\Model\ModelRenderNodes.cpp") -Raw
if ($ModelRenderNodes -notmatch "DrawIndexedEx\([^;]+uCount") {
    throw "Model instance drawer no longer draws with the batched instance count."
}
if ($ModelRenderNodes -notmatch "CreateInstanceRenderer") {
    throw "Model instance mesh no longer creates a ModelInstanceRenderer."
}

$ModelShader = Get-Content (Join-Path $RepoRoot "Data\GLSL\shaders\includes\model_transform.inc") -Raw
if ($ModelShader -notmatch "INSTANCED") {
    throw "Model shaders no longer compile an instanced transform path."
}

$ModelEffect = Get-Content (Join-Path $RepoRoot "Data\GLSL\effects\Model.yml") -Raw
if ($ModelEffect -notmatch "ao_mModelMat") {
    throw "Model effect declarations no longer expose the instanced model matrix attribute."
}

Write-Host "Model instancing smoke passed: $InstancesOut"
