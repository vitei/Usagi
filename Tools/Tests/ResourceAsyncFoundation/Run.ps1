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
$ResourceBase = Get-Content (Join-Path $RepoRoot "Engine\Resource\ResourceBase.h") -Raw
$ResourceData = Get-Content (Join-Path $RepoRoot "Engine\Resource\ResourceData.h") -Raw
$ResourceMgr = Get-Content (Join-Path $RepoRoot "Engine\Resource\ResourceMgr.cpp") -Raw

foreach ($State in @(
    "REQUESTED",
    "CPU_LOADING",
    "WAITING_DEPENDENCIES",
    "CPU_READY",
    "QUEUED_GPU_UPLOAD",
    "GPU_UPLOADING",
    "READY",
    "FAILED",
    "CANCELLED",
    "UNLOADING"
)) {
    if ($ResourceBase -notmatch "\b$State\b") {
        throw "ResourceState is missing $State"
    }
}

if ($ResourceBase -notmatch "bool IsReady\(\) const \{ return m_eState == ResourceState::READY; \}") {
    throw "ResourceBase::IsReady no longer maps to ResourceState::READY."
}
if ($ResourceBase -notmatch "void SetReady\(bool bReady\) \{ m_eState = bReady \? ResourceState::READY : ResourceState::REQUESTED; \}") {
    throw "ResourceBase::SetReady no longer preserves legacy ready/requested behavior."
}
if ($ResourceBase -notmatch "ResourceState\s+m_eState") {
    throw "ResourceBase does not store ResourceState."
}

foreach ($Expected in @(
    "struct ResourceRequestDependency",
    "struct ResourceRequest",
    "QueueRequest",
    "GetNextQueuedRequest",
    "FindRequest",
    "AddRequestDependency",
    "SetRequestState",
    "CompleteRequest",
    "HasQueuedRequests",
    "ClearRequests"
)) {
    if ($ResourceData -notmatch [regex]::Escape($Expected)) {
        throw "ResourceData request foundation is missing: $Expected"
    }
}

if ($ResourceData -notmatch "usg::vector<ResourceRequest>\s+m_requests") {
    throw "ResourceData does not store queued resource requests."
}

if ($ResourceData -notmatch "if \(uPriority > pRequest->uPriority\)") {
    throw "QueueRequest no longer raises duplicate request priority."
}
if ($ResourceData -notmatch "m_requests\[i\]\.eState != ResourceState::REQUESTED") {
    throw "GetNextQueuedRequest no longer filters non-requested states."
}
if ($ResourceData -notmatch "m_requests\[i\]\.uPriority > pNextRequest->uPriority") {
    throw "GetNextQueuedRequest no longer selects the highest priority request."
}
if ($ResourceData -notmatch "request\.dependencies\.push_back") {
    throw "AddRequestDependency no longer records dependencies."
}
if ($ResourceData -notmatch "ResourceState::READY : ResourceState::FAILED") {
    throw "CompleteRequest no longer marks success or failure from the completed handle."
}

foreach ($Deferred in @(
    "ResourceCpuLoadWorker",
    "RequestSkeletalAnimation",
    "ProcessCompletedResourceLoads",
    "MessageQueue<ResourceCpuLoadJob>"
)) {
    if ($ResourceMgr -match $Deferred) {
        throw "Resource async foundation unexpectedly includes worker-loading code: $Deferred"
    }
}

Write-Host "Resource async foundation smoke passed."
