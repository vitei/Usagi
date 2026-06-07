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
$Header = Get-Content (Join-Path $RepoRoot "Engine\Framework\ComponentEntity.h") -Raw
$Source = Get-Content (Join-Path $RepoRoot "Engine\Framework\ComponentEntity.cpp") -Raw
$EventManager = Get-Content (Join-Path $RepoRoot "Engine\Framework\EventManager.h") -Raw
$SystemCoordinator = Get-Content (Join-Path $RepoRoot "Engine\Framework\SystemCoordinator.cpp") -Raw

foreach ($Expected in @(
    "struct EntityHandle",
    "uint32 uIndex",
    "uint32 uGeneration",
    "bool IsValid() const",
    "operator==",
    "operator!=",
    "enum StableEntityHandleStatus",
    "STABLE_ENTITY_HANDLE_VALID",
    "STABLE_ENTITY_HANDLE_INVALID",
    "STABLE_ENTITY_HANDLE_OUT_OF_RANGE",
    "STABLE_ENTITY_HANDLE_EMPTY_SLOT",
    "STABLE_ENTITY_HANDLE_INACTIVE",
    "STABLE_ENTITY_HANDLE_STALE_GENERATION",
    "GetStableID",
    "GetEntityFromStableID",
    "IsStableIDValid",
    "GetStableIDStatusName",
    "EntityHandle stableId"
)) {
    if ($Header -notmatch [regex]::Escape($Expected)) {
        throw "Stable entity handle header contract is missing: $Expected"
    }
}

foreach ($Expected in @(
    "s_stableEntityLookup",
    "RegisterStableEntity(this)",
    "UnregisterStableEntity(this)",
    "s_stableEntityLookup.clear()",
    "entity->stableId = e->GetStableID()",
    "entity->m_uGeneration != id.uGeneration",
    "STABLE_ENTITY_HANDLE_STALE_GENERATION",
    "STABLE_ENTITY_HANDLE_EMPTY_SLOT",
    "STABLE_ENTITY_HANDLE_OUT_OF_RANGE",
    "SetStableEntityHandleStatus(pStatus, STABLE_ENTITY_HANDLE_VALID)",
    "GetStableIDStatusName"
)) {
    if ($Source -notmatch [regex]::Escape($Expected)) {
        throw "Stable entity handle source contract is missing: $Expected"
    }
}

if ($Source -notmatch "\+\+m_uGeneration") {
    throw "Entity generations are not incremented on activation."
}
if ($Source -notmatch "if \(m_uGeneration == 0\)") {
    throw "Entity generation wraparound no longer skips invalid zero."
}
if ($Source -notmatch "return nullptr") {
    throw "Stale stable handles no longer resolve to nullptr."
}
if ($Source -notmatch '"stale_generation"' -or $Source -notmatch '"empty_slot"' -or $Source -notmatch '"out_of_range"') {
    throw "Stable handle diagnostics no longer expose failure status names."
}

foreach ($Deferred in @(
    "RegisterEventWithEntity(EntityHandle",
    "EventOnEntityBase::GetEntity",
    "SystemScheduler",
    "TaskRunner"
)) {
    $Pattern = [regex]::Escape($Deferred)
    if ($EventManager -match $Pattern -or $SystemCoordinator -match $Pattern) {
        throw "Stable handle foundation unexpectedly includes deferred scheduler/event work: $Deferred"
    }
}

Write-Host "Stable entity handle smoke passed."
